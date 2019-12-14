/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "GridNotifiers.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "ulduar.h"

// Thorim Spells
enum Spells
{
    SPELL_SHEAT_OF_LIGHTNING    = 62276,
    SPELL_STORMHAMMER           = 62042,
    SPELL_DEAFENING_THUNDER     = 62470,
    SPELL_CHARGE_ORB            = 62016,
    SPELL_SUMMON_LIGHTNING_ORB  = 62391,
    SPELL_TOUCH_OF_DOMINION     = 62565,
    SPELL_CHAIN_LIGHTNING_10    = 62131,
    SPELL_CHAIN_LIGHTNING_25    = 64390,
    SPELL_LIGHTNING_CHARGE      = 62279,
    SPELL_LIGHTNING_DESTRUCTION = 62393,
    SPELL_LIGHTNING_RELEASE     = 62466,
    SPELL_LIGHTNING_PILLAR      = 62976,
    SPELL_UNBALANCING_STRIKE    = 62130,
    SPELL_BERSERK_PHASE_1       = 62560,
    SPELL_BERSERK_PHASE_2       = 26662
};

#define SPELL_CHAIN_LIGHTNING RAID_MODE(SPELL_CHAIN_LIGHTNING_10, SPELL_CHAIN_LIGHTNING_25)

enum Events // Only start > 0 is relevant
{
    EVENT_SAY_AGGRO_2 = 1,
    EVENT_STORMHAMMER,
    EVENT_CHARGE_ORB,
    EVENT_SUMMON_WARBRINGER,
    EVENT_SUMMON_EVOKER,
    EVENT_SUMMON_COMMONER,
    EVENT_BERSERK_PHASE_1,
    EVENT_BERSERK_PHASE_2,
    EVENT_UNBALANCING_STRIKE,
    EVENT_CHAIN_LIGHTNING,
    EVENT_TRANSFER_ENERGY,
    EVENT_RELEASE_LIGHTNING_CHARGE,
};

enum Yells
{
    SAY_AGGRO        = 0,
    SAY_SPECIAL      = 1,
    SAY_JUMPDOWN     = 2,
    SAY_SLAY         = 3,
    SAY_BERSERK      = 4,
    SAY_WIPE         = 5,

    SAY_DEATH        = 0,
    SAY_END_NORMAL   = 1,
    SAY_END_HARD     = 2,
    SAY_YS_HELP      = 3,
};

enum Actions
{
    ACTION_PREPHASE_ADDS_DIED = 1,
    ACTION_DOSCHEDULE_RUNIC_SMASH,
    ACTION_BERSERK,
    MAX_HARD_MODE_TIME = 3*MINUTE*IN_MILLISECONDS
};

// Achievements
#define ACHIEVEMENT_SIFFED                      RAID_MODE(2977, 2978)
#define ACHIEVEMENT_LOSE_ILLUSION               RAID_MODE(3176, 3183)

enum Creatures
{
    NPC_SIF                         = 33196,
    NPC_LIGHTNING_ORB               = 33138,
    NPC_THUNDER_ORB                 = 33378,
    NPC_THORIM_COMBAT_TRIGGER       = 34055,
    NPC_THORIM_GOLEM_RH_BUNNY       = 33140, // TODO: does some stupid things
    NPC_THORIM_GOLEM_LH_BUNNY       = 33141, // TODO: does some stupid things
};

#define SPELL_AURA_OF_CELERITY                  62320
#define SPELL_CHARGE                            32323

enum GameObjects
{
    GO_LEVER = 194264,
     // Should be opened by GO_LEVER
};

// Runic Colossus (Mini Boss) Spells
enum RunicSpells
{
    SPELL_SMASH                                 = 62339,
    SPELL_RUNIC_BARRIER                         = 62338,
    SPELL_RUNIC_CHARGE                          = 62613,
    SPELL_RUNIC_SMASH                           = 62465,
    SPELL_RUNIC_SMASH_LEFT                      = 62057,
    SPELL_RUNIC_SMASH_RIGHT                     = 62058
};

// Ancient Rune Giant (Mini Boss) Spells
enum AncientSpells
{
    SPELL_RUNIC_FORTIFICATION                   = 62942,
    SPELL_RUNE_DETONATION                       = 62526,
    SPELL_STOMP                                 = 62411
};

enum ThorimChests
{
    CACHE_OF_STORMS_10                          = 194312,
    CACHE_OF_STORMS_HARDMODE_10                 = 194313,
    CACHE_OF_STORMS_25                          = 194315,
    CACHE_OF_STORMS_HARDMODE_25                 = 194314
};

const Position Pos[7] =
{
    {2095.53f, -279.48f, 419.84f, 0.504f},
    {2092.93f, -252.96f, 419.84f, 6.024f},
    {2097.86f, -240.97f, 419.84f, 5.643f},
    {2113.14f, -225.94f, 419.84f, 5.259f},
    {2156.87f, -226.12f, 419.84f, 4.202f},
    {2172.42f, -242.70f, 419.84f, 3.583f},
    {2171.92f, -284.59f, 419.84f, 2.691f}
};

const Position PosOrbs[7] =
{
    {2104.99f, -233.484f, 433.576f, 5.49779f},
    {2092.64f, -262.594f, 433.576f, 6.26573f},
    {2104.76f, -292.719f, 433.576f, 0.78539f},
    {2164.97f, -293.375f, 433.576f, 2.35619f},
    {2164.58f, -233.333f, 433.576f, 3.90954f},
    {2145.81f, -222.196f, 433.576f, 4.45059f},
    {2123.91f, -222.443f, 433.576f, 4.97419f}
};

const Position PosCharge[7] =
{
    {2108.95f, -289.241f, 420.149f, 5.49779f},
    {2097.93f, -262.782f, 420.149f, 6.26573f},
    {2108.66f, -237.102f, 420.149f, 0.78539f},
    {2160.56f, -289.292f, 420.149f, 2.35619f},
    {2161.02f, -237.258f, 420.149f, 3.90954f},
    {2143.87f, -227.415f, 420.149f, 4.45059f},
    {2125.84f, -227.439f, 420.149f, 4.97419f}
};

#define POS_X_ARENA  2181.19f
#define POS_Y_ARENA  -299.12f

struct SummonLocation
{
    Position pos;
    uint32 entry;
};

// Forwarding definition, since required by add-location and predicates
enum ArenaAddEntries
{
    NPC_DARK_RUNE_CHAMPION      = 32876,
    NPC_DARK_RUNE_COMMONER      = 32904,
    NPC_DARK_RUNE_EVOKER        = 32878,
    NPC_DARK_RUNE_WARBRINGER    = 32877,
    NPC_IRON_RING_GUARD         = 32874,
    NPC_IRON_HONOR_GUARD        = 32875,
    NPC_DARK_RUNE_ACOLYTE_ARENA = 33110
};

SummonLocation preAddLocations[]=
{
    {{2149.68f, -263.477f, 419.679f, 3.120f}, NPC_JORMUNGAR_BEHEMOTH},
    {{2131.31f, -271.640f, 419.840f, 2.188f}, NPC_MERCENARY_CAPTAIN_A},
    {{2127.24f, -259.182f, 419.974f, 5.917f}, NPC_MERCENARY_CAPTAIN_A},
    {{2123.32f, -254.770f, 419.840f, 6.170f}, NPC_MERCENARY_CAPTAIN_A},
    {{2120.10f, -258.990f, 419.840f, 6.250f}, NPC_MERCENARY_CAPTAIN_A},
    {{2129.09f, -277.142f, 419.756f, 1.222f}, NPC_DARK_RUNE_ACOLYTE}
};


/************************************************************************/
/*                        Predicates                                    */
/************************************************************************/

class HealerCheck
{
    public:
        HealerCheck(bool shouldBe): __shouldBe(shouldBe) {}
        bool operator() (const Unit* unit)
        {
            return __shouldBe ? __IsHealer(unit) : !__IsHealer(unit);
        }
    private:
        bool __shouldBe;
        bool __IsHealer(const Unit* who)
        {
            return (who->GetEntry() == NPC_DARK_RUNE_ACOLYTE || who->GetEntry() == NPC_DARK_RUNE_EVOKER || who->GetEntry() == NPC_DARK_RUNE_ACOLYTE_ARENA);
        }
};

class ArenaAreaCheck
{
    public:
        ArenaAreaCheck(bool shouldBeIn): __shouldBeIn(shouldBeIn) {}
        bool operator() (const WorldObject* unit)
        {
            return __shouldBeIn ? __IsInArena(unit) : !__IsInArena(unit);
        }
    private:
        bool __shouldBeIn;
        bool __IsInArena(const WorldObject* who)
        {
            return (who->GetPositionX() < POS_X_ARENA && who->GetPositionY() > POS_Y_ARENA);    // TODO: Check if this is ok, end positions ?
        }
};

/************************************************************************/
/*                         Thorim                                       */
/************************************************************************/

class npc_thorim_controller : public CreatureScript
{
    private:
        enum MyEvents
        {
            EVENT_CHECK_PLAYER_IN_RANGE = 1,
        };
    public:
        npc_thorim_controller() : CreatureScript("npc_thorim_controller") {}

        struct npc_thorim_controllerAI : public ScriptedAI
        {
            npc_thorim_controllerAI(Creature* creature) : ScriptedAI(creature), summons(me)
            {
                me->SetCanFly(true);
                me->SetVisible(false);
                instance = creature->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                gotActivated = false;
                spawned = false;
            }

            void Reset()
            {
                if (!gotActivated)
                {
                    instance->HandleGameObject(instance->GetData64(GO_THORIM_LIGHTNING_FIELD), true); // Open the entrance door.
                    instance->HandleGameObject(instance->GetData64(GO_THORIM_DARK_IRON_PROTCULLIS), false); // Close the up-way door.
                    //            instance->OpenDoor(instance->GetData64(GO_THORIM_LIGHTNING_FIELD));
                    //  instance->CloseDoor(instance->GetData64(GO_THORIM_DARK_IRON_PROTCULLIS));
                    events.ScheduleEvent(EVENT_CHECK_PLAYER_IN_RANGE, 1000);
                    if (!spawned)
                    {
                        for (uint8 i = 0; i < 6; i++)   // Spawn Pre-Phase Adds
                            me->SummonCreature(preAddLocations[i].entry, preAddLocations[i].pos, TEMPSUMMON_CORPSE_DESPAWN);
                        spawned = true;
                    }
                }
            }

            void DoAction(int32 const action)
            {
                if (action != 42)
                    return;
                gotActivated = false;
                spawned = false;
                summons.DespawnAll();
            }


            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
                summon->AI()->AttackStart(SelectTarget(SELECT_TARGET_RANDOM));
            }

            void SummonedCreatureDies(Creature* summon, Unit* killer)
            {
                summons.Despawn(summon);
                if (summons.empty())
                {
                    uint64 attackTarget = 0;
                    if (killer != 0)
                        if (Player* player = killer->ToPlayer())
                            attackTarget = player->GetGUID();

                    if (attackTarget == 0)
                        if (Player* target = me->SelectNearestPlayer(30.0f))
                            attackTarget = target->GetGUID();

                    if (Creature* thorim = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_THORIM)))
                        thorim->AI()->SetGUID(attackTarget, ACTION_PREPHASE_ADDS_DIED);
                    instance->HandleGameObject(instance->GetData64(GO_THORIM_LIGHTNING_FIELD), false); // Close the entrance door.
                    instance->HandleGameObject(instance->GetData64(GO_THORIM_DARK_IRON_PROTCULLIS), true); // Open the up-way door.
                    //            instance->CloseDoor(instance->GetData64(GO_THORIM_LIGHTNING_FIELD));
                    //  instance->OpenDoor(instance->GetData64(GO_THORIM_DARK_IRON_PROTCULLIS));
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (instance->GetBossState(BOSS_THORIM) == DONE)
                    return;
                events.Update(diff);
                // MoveInLineOfSight does not seem to work here, so...
                while (uint32 event = events.ExecuteEvent())
                {
                    switch (event)
                    {
                        case EVENT_CHECK_PLAYER_IN_RANGE:
                            if (!gotActivated)
                            {
                                Player* player = 0;
                                Trinity::AnyPlayerInObjectRangeCheck u_check(me, 30.0f, true);
                                Trinity::PlayerSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, player, u_check);
                                me->VisitNearbyObject(30.0f, searcher);
                                if (player)
                                    if (!player->isGameMaster())
                                    {
                                        summons.DoZoneInCombat();
                                        gotActivated = true;
                                    }
                                if (!gotActivated)
                                    events.ScheduleEvent(EVENT_CHECK_PLAYER_IN_RANGE, 1000);
                            }
                            break;
                    }
                }
            }

        private:
            bool gotActivated, spawned;
            EventMap events;
            InstanceScript* instance;
            SummonList summons;
        };

        CreatureAI* GetAI(Creature* c) const
        {
            return new npc_thorim_controllerAI(c);
        }
};

const uint32 ArenaAddEntries[]                  = { NPC_DARK_RUNE_CHAMPION, NPC_DARK_RUNE_COMMONER, NPC_DARK_RUNE_EVOKER, NPC_DARK_RUNE_WARBRINGER,
                                                    NPC_IRON_RING_GUARD, NPC_IRON_HONOR_GUARD, NPC_DARK_RUNE_ACOLYTE_ARENA };

class boss_thorim : public CreatureScript
{
    private:
        enum Phases
        {
            PHASE_NULL = 0,
            PHASE_1,
            PHASE_2
        };
    public:
        boss_thorim() : CreatureScript("boss_thorim") {}

        struct boss_thorimAI : public BossAI
        {
            boss_thorimAI(Creature* creature) : BossAI(creature, BOSS_THORIM)
            {
                gotAddsWiped = false;
                gotEncounterFinished = false;
                homePosition = creature->GetHomePosition();
            }

            void Reset()
            {
                _Reset();
                //                gotEncounterFinished = gotEncounterFinished || (instance->GetBossState(BOSS_THORIM) == DONE);
                if (gotEncounterFinished) // May be called during fight if Thorim gets outfight... hm, should _not_ happen regularly
                {
                    //                    me->setFaction(35);
                    //                    return;
                }
                sif = NULL;
                isEnterCombat = false;
                if (gotAddsWiped)
                    Talk(SAY_WIPE);
                orbList.clear();
                //                _Reset();

                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NON_ATTACKABLE);

                phase = PHASE_NULL;
                gotAddsWiped = false;
                HardMode = false;
                gotBerserkedAndOrbSummoned = false;
                summonChampion = false;
                checkTargetTimer = 7000;
                if (Creature* ctrl = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_THORIM_CTRL)))
                {
                    ctrl->Respawn();
                    ctrl->AI()->DoAction(42);
                    ctrl->AI()->Reset();
                }
                // Respawn Mini Bosses
                for (uint8 i = DATA_RUNIC_COLOSSUS; i <= DATA_RUNE_GIANT; i++)  // TODO: Check if we can move this, it's a little bit crazy.
                    if (Creature* MiniBoss = ObjectAccessor::GetCreature(*me, instance->GetData64(i)))
                        MiniBoss->Respawn(true);

                if (GameObject* go = me->FindNearestGameObject(GO_LEVER, 500.0f))
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);

                //                me->GetMotionMaster()->Clear();
                //   me->GetMotionMaster()->MoveRandom(5.0f);

                instance->HandleGameObject(instance->GetData64(GO_THORIM_LIGHTNING_FIELD), true); // Open the entrance door if the raid got past the first adds, since in this case, it will not be performed by the controller bunny.
            }

            void KilledUnit(Unit* /*victim*/)
            {
                Talk(SAY_SLAY);
            }

            void EncounterPostProgress()
            {
                if (gotEncounterFinished)  // lock, function should not be called twice
                    return;

                gotEncounterFinished = true;
                Talk(SAY_DEATH);
                if (sif)
                    sif->DespawnOrUnsummon();
                bool hf4 = false;
                if (Creature* ctrl = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_THORIM_CTRL)))
                {
                    if (ctrl->HasAura(62320))
                        hf4 = true;
                    ctrl->DespawnOrUnsummon();
                }
                if (hf4)
                    instance->DoCompleteAchievement(RAID_MODE(2975, 2976));
                // Kill credit
                instance->DoUpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, 64985);
                // Lose Your Illusion
                if (HardMode)
                {
                    instance->DoCompleteAchievement(ACHIEVEMENT_LOSE_ILLUSION);
                    me->SummonGameObject(RAID_MODE(CACHE_OF_STORMS_HARDMODE_10, CACHE_OF_STORMS_HARDMODE_25), 2134.58f, -286.908f, 419.495f, 1.55988f, 0.0f, 0.0f, 1.0f, 1.0f, 604800);
                }
                else
                    me->SummonGameObject(RAID_MODE(CACHE_OF_STORMS_10, CACHE_OF_STORMS_25), 2134.58f, -286.908f, 419.495f, 1.55988f, 0, 0, 1, 1, 604800);
                me->DespawnOrUnsummon();
                instance->HandleGameObject(instance->GetData64(GO_THORIM_LIGHTNING_FIELD), true); // Open the entrance door.
                instance->HandleGameObject(instance->GetData64(GO_THORIM_DARK_IRON_PROTCULLIS), false); // Close the up-way door.
                me->setFaction(35);
                me->DespawnOrUnsummon(7000);
                _JustDied();
            }

            void EnterCombat(Unit* who)
            {
                if (isEnterCombat)
                    return;
                isEnterCombat = true;
                Talk(SAY_AGGRO);
                _EnterCombat();

                // Spawn Thunder Orbs
                orbList.clear();
                for (uint8 n = 0; n < 7; n++)
                {
                    if (Creature *orb = me->SummonCreature(NPC_THUNDER_ORB, PosOrbs[n], TEMPSUMMON_CORPSE_DESPAWN))
                        orbList.push_back(orb);
                }
                EncounterTime = 0;
                phase = PHASE_1;
                events.SetPhase(phase);
                DoCast(me, SPELL_SHEAT_OF_LIGHTNING);
                events.ScheduleEvent(EVENT_STORMHAMMER, 40000, 0, phase);
                events.ScheduleEvent(EVENT_CHARGE_ORB, 30000, 0, phase);
                events.ScheduleEvent(EVENT_SUMMON_WARBRINGER, 25000, 0, phase);
                events.ScheduleEvent(EVENT_SUMMON_EVOKER, 30000, 0, phase);
                events.ScheduleEvent(EVENT_SUMMON_COMMONER, 35000, 0, phase);
                events.ScheduleEvent(EVENT_BERSERK_PHASE_1, 360000, 0, phase);
                events.ScheduleEvent(EVENT_SAY_AGGRO_2, 10000, 0, phase);

                if (Creature* runic = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_RUNIC_COLOSSUS)))
                {
                    runic->setActive(true);
                    runic->AI()->DoAction(ACTION_DOSCHEDULE_RUNIC_SMASH);  // Signals runic smash rotation
                }

                if (GameObject* go = me->FindNearestGameObject(GO_LEVER, 500.0f))
                    go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);

                me->SetFacingToObject(who);
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveIdle();
            }


            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (phase == PHASE_2 && me->getVictim() && ArenaAreaCheck(false)(me->getVictim()))
                {
                    me->getVictim()->getHostileRefManager().deleteReference(me);
                    return;
                }

                if (checkTargetTimer < diff)
                {
                    if (!SelectTarget(SELECT_TARGET_RANDOM, 0, 200.0f, true))
                    {
                        EnterEvadeMode();
                        return;
                    }
                    checkTargetTimer = 7000;
                }
                else
                    checkTargetTimer -= diff;

                // Thorim should be inside the arena during phase 2
                if (phase == PHASE_2 && ArenaAreaCheck(false)(me))
                {
                    EnterEvadeMode();
                    return;
                }

                EncounterTime += diff;

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SAY_AGGRO_2:
                            Talk(SAY_AGGRO);
                            break;
                        case EVENT_STORMHAMMER:
                            DoCast(SPELL_STORMHAMMER);
                            events.ScheduleEvent(EVENT_STORMHAMMER, urand(15, 20) *IN_MILLISECONDS, 0, PHASE_1);
                            break;
                        case EVENT_CHARGE_ORB:
                            if (Creature *orb = Trinity::Containers::SelectRandomContainerElement(orbList))
                            {
                                //                  orb->CastSpell(orb, SPELL_CHARGE_ORB, true);
                                orb->AddAura(SPELL_CHARGE_ORB, orb);
                                orb->AddAura(SPELL_CHARGE_ORB, orb);
                            }
                            //              DoCastAOE(SPELL_CHARGE_ORB);
                            events.ScheduleEvent(EVENT_CHARGE_ORB, urand(15, 20) *IN_MILLISECONDS, 0, PHASE_1);
                            break;
                        case EVENT_SUMMON_WARBRINGER:
                            me->SummonCreature(ArenaAddEntries[3], Pos[rand()%7], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 120000);
                            if (summonChampion)
                            {
                                me->SummonCreature(ArenaAddEntries[0], Pos[rand()%7], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 120000);
                                summonChampion = false;
                            }
                            else
                                summonChampion = true;
                            events.ScheduleEvent(EVENT_SUMMON_WARBRINGER, 20000, 0, PHASE_1);
                            break;
                        case EVENT_SUMMON_EVOKER:
                            me->SummonCreature(ArenaAddEntries[2], Pos[rand()%7], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 120000);
                            events.ScheduleEvent(EVENT_SUMMON_EVOKER, urand(23, 27) *IN_MILLISECONDS, 0, PHASE_1);
                            break;
                        case EVENT_SUMMON_COMMONER:
                            for (uint8 n = 0; n < urand(5, 7); ++n)
                                me->SummonCreature(ArenaAddEntries[1], Pos[rand()%7], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 120000);
                            events.ScheduleEvent(EVENT_SUMMON_COMMONER, 30000, 0, PHASE_1);
                            break;
                        case EVENT_BERSERK_PHASE_1:
                            DoCast(me, SPELL_BERSERK_PHASE_1);
                            DoCast(me, SPELL_SUMMON_LIGHTNING_ORB, true);
                            Talk(SAY_BERSERK);
                            break;
                        // Phase 2 stuff
                        case EVENT_UNBALANCING_STRIKE:
                            DoCastVictim(SPELL_UNBALANCING_STRIKE);
                            events.ScheduleEvent(EVENT_UNBALANCING_STRIKE, 26000, 0, PHASE_2);
                            break;
                        case EVENT_CHAIN_LIGHTNING:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                                DoCast(target, SPELL_CHAIN_LIGHTNING);
                            events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, urand(7, 15) *IN_MILLISECONDS, 0, PHASE_2);
                            break;
                        case EVENT_TRANSFER_ENERGY:
                            if (Creature* source = me->SummonCreature(NPC_THORIM_COMBAT_TRIGGER, PosCharge[urand(0, 6)], TEMPSUMMON_TIMED_DESPAWN, 10000))
                                source->CastSpell(source, SPELL_LIGHTNING_PILLAR, true);
                            events.ScheduleEvent(EVENT_RELEASE_LIGHTNING_CHARGE, 8000, 0, PHASE_2);
                            break;
                        case EVENT_RELEASE_LIGHTNING_CHARGE:
                            if (Creature* source = me->FindNearestCreature(NPC_THORIM_COMBAT_TRIGGER, 100.0f))
                                DoCast(source, SPELL_LIGHTNING_RELEASE);
                            DoCast(me, SPELL_LIGHTNING_CHARGE, true);
                            events.ScheduleEvent(EVENT_TRANSFER_ENERGY, 8000, 0, PHASE_2);
                            break;
                        case EVENT_BERSERK_PHASE_2:
                            DoCast(me, SPELL_BERSERK_PHASE_2);
                            Talk(SAY_BERSERK);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
                // EnterEvadeIfOutOfCombatArea(diff);
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case ACTION_BERSERK:
                        if (phase != PHASE_1)
                            return;
                        if (!gotBerserkedAndOrbSummoned)
                        {
                            events.RescheduleEvent(EVENT_BERSERK_PHASE_1, 1000);
                            gotBerserkedAndOrbSummoned = true;
                        }
                        break;
                }
            }

            void SetGUID(uint64 guid, int32 data)
            {
                switch (data)
                {
                    case ACTION_PREPHASE_ADDS_DIED:
                        if (!gotAddsWiped)
                        {
                            gotAddsWiped = true;
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            events.Reset();
                            EnterCombat(ObjectAccessor::GetUnit(*me, guid));
                            DoZoneInCombat();
                        }
                        break;
                }
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
                if (me->isInCombat())
                    DoZoneInCombat(summon);

                if (summon->GetEntry() == NPC_LIGHTNING_ORB)
                    summon->CastSpell(summon, SPELL_LIGHTNING_DESTRUCTION, true);
            }

            void SummonedCreatureDies(Creature* summon, Unit* /*killer*/)
            {
                summons.Despawn(summon);
                summon->RemoveCorpse(false);
            }

            void DamageTaken(Unit* attacker, uint32 &damage)
            {
                if (damage >= me->GetHealth())
                {
          //                    damage = 0;
                    EncounterPostProgress();
                    return;
                }

                if (phase == PHASE_1 && attacker && instance)
                {
                    Creature* colossus = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_RUNIC_COLOSSUS));
                    Creature* giant = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_RUNE_GIANT));
                    if (colossus && colossus->isDead() && giant && giant->isDead() && me->IsWithinDistInMap(attacker, 50.0f) && attacker->ToPlayer())
                    {
                        Talk(SAY_JUMPDOWN);
                        phase = PHASE_2;
                        events.SetPhase(PHASE_2);
                        me->RemoveAurasDueToSpell(SPELL_SHEAT_OF_LIGHTNING);
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                        //                        me->GetMotionMaster()->MoveJump(2134.79f, -263.03f, 419.84f, 10.0f, 20.0f);
                        me->NearTeleportTo(2134.79f, -263.03f, 419.84f, me->GetOrientation());
                        summons.DespawnEntry(NPC_THUNDER_ORB); // despawn charged orbs
                        events.ScheduleEvent(EVENT_UNBALANCING_STRIKE, 15000, 0, PHASE_2);
                        events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, 20000, 0, PHASE_2);
                        events.ScheduleEvent(EVENT_TRANSFER_ENERGY, 20000, 0, PHASE_2);
                        events.ScheduleEvent(EVENT_BERSERK_PHASE_2, 300000, 0, PHASE_2);
                        // Check for Hard Mode
                        if (EncounterTime <= MAX_HARD_MODE_TIME)
                        {
                            HardMode = true;
                            // Summon Sif
                            sif = me->SummonCreature(NPC_SIF, 2149.27f, -260.55f, 419.69f, 2.527f, TEMPSUMMON_CORPSE_DESPAWN);
                            // Achievement Siffed
                            if (instance)
                                instance->DoCompleteAchievement(ACHIEVEMENT_SIFFED);
                        }
                        else
                            me->AddAura(SPELL_TOUCH_OF_DOMINION, me);
                    }
                    else damage = 0;
                }
            }

            private:
                Phases phase;
                uint8 PreAddsCount;
                uint32 EncounterTime;
                uint32 checkTargetTimer;
                bool gotAddsWiped;
                bool HardMode;
                bool gotBerserkedAndOrbSummoned;
                bool gotEncounterFinished;
                bool summonChampion;
                Position homePosition;
                Creature *sif;
                std::list<Creature* > orbList;
                bool isEnterCombat;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<boss_thorimAI>(creature);
        }
};

/************************************************************************/
/* Pre-Phase adds                                                       */
/* Note: The behavior script below will be registered for all pre-phase */
/* indices, so we need some helpers for managing their spells and       */
/* entries.                                                             */
/************************************************************************/
enum PreAddSpells
{
    SPELL_ACID_BREATH_10            = 62315,
    SPELL_ACID_BREATH_25            = 62415,
    SPELL_SWEEP_10                  = 62316,
    SPELL_SWEEP_25                  = 62417,

    SPELL_DEVASTATE                 = 62317,
    SPELL_HEROIC_SWIPE              = 62444,

    SPELL_BARBED_SHOT               = 62318,
    SPELL_SHOOT                     = 16496,

    SPELL_RENEW_10                  = 62333,
    SPELL_RENEW_25                  = 62441,
    SPELL_GREATER_HEAL_10           = 62334,
    SPELL_GREATER_HEAL_25           = 62442
};

#define SPELL_HOLY_SMITE    RAID_MODE(62335, 62443)

enum PrePhaseAddIndex
{
    INDEX_BEHEMOTH                = 0,
    INDEX_MERCENARY_CAPTAIN_A,
    INDEX_MERCENARY_SOLDIER_A,
    INDEX_DARK_RUNE_ACOLYTE,
    INDEX_MERCENARY_CAPTAIN_H,
    INDEX_MERCENARY_SOLDIER_H,
    INDEX_PRE_ADD_NONE
};
const uint32 PrePhaseAddList[] = {NPC_JORMUNGAR_BEHEMOTH, NPC_MERCENARY_CAPTAIN_A, NPC_MERCENARY_SOLDIER_A, NPC_DARK_RUNE_ACOLYTE, NPC_MERCENARY_CAPTAIN_H, NPC_MERCENARY_SOLDIER_H};
const uint32 PrePhaseAddSpells_Primary[2][6] =
{
    {SPELL_ACID_BREATH_10, SPELL_DEVASTATE, SPELL_BARBED_SHOT, SPELL_RENEW_10, SPELL_DEVASTATE, SPELL_BARBED_SHOT},
    {SPELL_ACID_BREATH_25, SPELL_DEVASTATE, SPELL_BARBED_SHOT, SPELL_RENEW_25, SPELL_DEVASTATE, SPELL_BARBED_SHOT}
};
const uint32 PrePhaseAddSpells_Secondary[2][6] =
{
    {SPELL_SWEEP_10, SPELL_HEROIC_SWIPE, SPELL_SHOOT, SPELL_GREATER_HEAL_10, SPELL_HEROIC_SWIPE, SPELL_SHOOT},
    {SPELL_SWEEP_25, SPELL_HEROIC_SWIPE, SPELL_SHOOT, SPELL_GREATER_HEAL_25, SPELL_HEROIC_SWIPE, SPELL_SHOOT}
};

class PrePhaseAddHelper
{
    private:
        enum ManCnt { In10Man = 0, In25Man };
    public:
        enum Index
        {
            INDEX_PRIMARY,
            INDEX_SECONDARY
        };

        PrePhaseAddHelper(Difficulty raidDifficulty)
        {
            if (raidDifficulty == RAID_DIFFICULTY_25MAN_NORMAL || raidDifficulty == RAID_DIFFICULTY_25MAN_HEROIC) // should not be heroic, just for the case
                diffi = In10Man;
            else
                diffi = In25Man;
        }

        PrePhaseAddIndex operator[](uint32 creatureEntry)
        {
            for (uint8 i = 0; i < 6; i++)
                if (PrePhaseAddList[i] == creatureEntry)
                    return PrePhaseAddIndex(i);
            return INDEX_PRE_ADD_NONE;
        }

        uint32 operator()(PrePhaseAddIndex myId, Index idx)
        {
            if (myId < INDEX_PRE_ADD_NONE)
            {
                if (idx == INDEX_PRIMARY)
                    return PrePhaseAddSpells_Primary[diffi][myId];
                else
                    return PrePhaseAddSpells_Secondary[diffi][myId];
            }
            return 0;
        }

    private:
        ManCnt diffi;
};

class npc_thorim_pre_phase_add : public CreatureScript
{
    private:
        enum { EVENT_PRIMARY_SKILL = 1, EVENT_SECONDARY_SKILL, EVENT_CHECK_PLAYER_IN_RANGE };
    public:
        npc_thorim_pre_phase_add() : CreatureScript("npc_thorim_pre_phase_add") {}

        struct npc_thorim_pre_phaseAI : public ScriptedAI
        {
            npc_thorim_pre_phaseAI(Creature *pCreature) : ScriptedAI(pCreature), myHelper(GetDifficulty())
            {
                pInstance = pCreature->GetInstanceScript();
                me->setFaction(14);
                me->SetReactState(REACT_AGGRESSIVE);
                myIndex = myHelper[me->GetEntry()];
                amIHealer = HealerCheck(true)(me);
            }

            void Reset()
            {
                events.Reset();
                events.ScheduleEvent(EVENT_CHECK_PLAYER_IN_RANGE, 1*IN_MILLISECONDS);
            }

            void IsSummonedBy(Unit* /*summoner*/)
            {
                Reset();
            }

            void EnterCombat(Unit* /*target*/)
            {
                events.ScheduleEvent(EVENT_PRIMARY_SKILL, urand(3000, 6000));
                events.ScheduleEvent(EVENT_SECONDARY_SKILL, urand (12000, 15000));
            }

            void JustDied(Unit* /*victim*/)
            {
                if (Creature* pThorim = ObjectAccessor::GetCreature(*me, pInstance->GetData64(BOSS_THORIM)))
                    pThorim->AI()->DoAction(ACTION_PREPHASE_ADDS_DIED);
            }

            void AttackStart(Unit* target)
            {
                if (myIndex == INDEX_DARK_RUNE_ACOLYTE)
                    AttackStartCaster(target, 30.0f);
                else
                    ScriptedAI::AttackStart(target);
            }

            void UpdateAI(uint32 const diff)
            {
                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 event = events.ExecuteEvent())
                {
                    switch (event)
                    {
                        case EVENT_CHECK_PLAYER_IN_RANGE:
                            if (!me->isInCombat())
                            {
                                Player* player = 0;
                                Trinity::AnyPlayerInObjectRangeCheck u_check(me, 30.0f, true);
                                Trinity::PlayerSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, player, u_check);
                                me->VisitNearbyObject(30.0f, searcher);
                                if (player)
                                    if (!player->isGameMaster())
                                        AttackStart(player);
                                events.ScheduleEvent(EVENT_CHECK_PLAYER_IN_RANGE, 1000);
                            }
                            break;
                        case EVENT_PRIMARY_SKILL:
                            if (Unit* target = amIHealer ? (me->GetHealthPct() > 40 ? DoSelectLowestHpFriendly(40) : me) : me->getVictim())
                            {
                                DoCast(target, myHelper(myIndex, PrePhaseAddHelper::INDEX_PRIMARY));
                                events.ScheduleEvent(EVENT_PRIMARY_SKILL, urand(10000, 15000));
                            }
                            else
                                events.ScheduleEvent(EVENT_PRIMARY_SKILL, urand(2000, 3000));
                            break;
                        case EVENT_SECONDARY_SKILL:
                            if ((amIHealer ? (me->GetHealthPct() > 40 ? DoSelectLowestHpFriendly(40) : me) : me->getVictim()) != NULL)
                            {
                                DoCast(myHelper(myIndex, PrePhaseAddHelper::INDEX_SECONDARY));
                                events.ScheduleEvent(EVENT_PRIMARY_SKILL, urand(4000, 8000));
                            }
                            else
                                events.ScheduleEvent(EVENT_PRIMARY_SKILL, urand(1000, 2000));
                            break;
                    }
                }

                if (myIndex == INDEX_DARK_RUNE_ACOLYTE)
                    DoSpellAttackIfReady(SPELL_HOLY_SMITE);
                else
                    DoMeleeAttackIfReady();
            }

            private:
                InstanceScript* pInstance;
                PrePhaseAddHelper myHelper;
                PrePhaseAddIndex myIndex;
                EventMap events;
                bool amIHealer;
        };

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetUlduarAI<npc_thorim_pre_phaseAI>(pCreature);
        }
};

/************************************************************************/
/* Adds in arena-phase                                                  */
/* Note: The behavior script below will be registered for all           */
/* arena-phase indices, so we need some helpers for managing their      */
/* spells and entries.                                                  */
/************************************************************************/
enum ArenaAddsSpells
{
    // Primary spells
    SPELL_MORTAL_STRIKE         = 35054,
    SPELL_LOW_BLOW              = 62326,
    SPELL_RUNIC_LIGHTNING_10    = 62327,
    SPELL_RUNIC_LIGHTNING_25    = 62445,
    SPELL_RUNIC_STRIKE          = 62322,
    SPELL_WHIRLING_TRIP         = 64151,
    SPELL_CLEAVE                = 42724,
    //SPELL_RENEW_10              = 62333,  // Used from previous definition
    //SPELL_RENEW_25              = 62441,
    // Secondary spells
    SPELL_WHIRLWIND             = 15578,
    SPELL_PUMMEL                = 38313,
    SPELL_RUNIC_SHIELD_10       = 62321,
    SPELL_RUNIC_SHIELD_25       = 62529,
    SPELL_IMPALE_10             = 62331,
    SPELL_IMPALE_25             = 62418,
    SPELL_SHIELD_SMASH_10       = 62332,
    SPELL_SHIELD_SMASH_25       = 62420,
    //SPELL_GREATER_HEAL_10       = 62334,  // Used from previous definition
    //SPELL_GREATER_HEAL_25       = 62442,
    // Some tertiary skills
    SPELL_RUNIC_MENDING_10      = 62328,
    SPELL_RUNIC_MENDING_25      = 62446,
};
#define SPELL_RUNIC_MENDING     RAID_MODE(SPELL_RUNIC_MENDING_10, SPELL_RUNIC_MENDING_25)
enum ArenaAddIndex
{
    INDEX_DARK_RUNE_CHAMPION = 0,
    INDEX_DARK_RUNE_COMMONER,
    INDEX_DARK_RUNE_EVOKER,
    INDEX_DARK_RUNE_WARBRINGER,
    INDEX_IRON_RING_GUARD,
    INDEX_IRON_HONOR_GUARD,
    INDEX_ARENA_DARK_RUNE_ACOLYTE,
    INDEX_ARENA_ADD_NONE,
};
const uint32 ArenaAddSpells_Primary[2][7] =
{
    {SPELL_MORTAL_STRIKE, SPELL_LOW_BLOW, SPELL_RUNIC_LIGHTNING_10, SPELL_RUNIC_STRIKE, SPELL_WHIRLING_TRIP, SPELL_CLEAVE, SPELL_RENEW_10},
    {SPELL_MORTAL_STRIKE, SPELL_LOW_BLOW, SPELL_RUNIC_LIGHTNING_25, SPELL_RUNIC_STRIKE, SPELL_WHIRLING_TRIP, SPELL_CLEAVE, SPELL_RENEW_25}
};
const uint32 ArenaAddSpells_Secondary[2][7] =
{
    {SPELL_WHIRLWIND, SPELL_PUMMEL, SPELL_RUNIC_SHIELD_10, 0, SPELL_IMPALE_10, SPELL_SHIELD_SMASH_10, SPELL_GREATER_HEAL_10},
    {SPELL_WHIRLWIND, SPELL_PUMMEL, SPELL_RUNIC_SHIELD_25, 0, SPELL_IMPALE_25, SPELL_SHIELD_SMASH_25, SPELL_GREATER_HEAL_25}
};

class ArenaPhaseAddHelper
{
    private:
        enum ManCnt { In10Man = 0, In25Man };
    public:
        enum Index
        {
            INDEX_PRIMARY,
            INDEX_SECONDARY
        };

        ArenaPhaseAddHelper(Difficulty raidDifficulty)
        {
            if (raidDifficulty == RAID_DIFFICULTY_25MAN_NORMAL || raidDifficulty == RAID_DIFFICULTY_25MAN_HEROIC) // should not be heroic, just for the case
                diffi = In10Man;
            else
                diffi = In25Man;
        }

        ArenaAddIndex operator[](uint32 creatureEntry)
        {
            for (uint8 i = 0; i < 7; i++)
                if (ArenaAddEntries[i] == creatureEntry)
                    return ArenaAddIndex(i);
            return INDEX_ARENA_ADD_NONE;
        }

        uint32 operator()(ArenaAddIndex myId, Index idx)
        {
            if (myId < INDEX_ARENA_ADD_NONE)
            {
                if (idx == INDEX_PRIMARY)
                    return ArenaAddSpells_Primary[diffi][myId];
                else
                    return ArenaAddSpells_Secondary[diffi][myId];
            }
            return 0;
        }

    private:
        ManCnt diffi;
};

class npc_thorim_arena_phase_add : public CreatureScript
{
    private:
        enum { EVENT_PRIMARY_SKILL = 1, EVENT_SECONDARY_SKILL, EVENT_CHARGE };
    public:
        npc_thorim_arena_phase_add() : CreatureScript("npc_thorim_arena_phase_add") {}

        struct npc_thorim_arena_phaseAI : public ScriptedAI
        {
            npc_thorim_arena_phaseAI(Creature* creature) : ScriptedAI(creature), myHelper(GetDifficulty())
            {
                _instance = creature->GetInstanceScript();
                me->setFaction(14);
                myIndex = myHelper[me->GetEntry()];
                IsInArena = ArenaAreaCheck(false)(me);
                amIhealer = HealerCheck(true)(me);
            }

            bool isOnSameSide(const Unit* who)
            {
                return (IsInArena == ArenaAreaCheck(false)(who));
            }

            void DamageTaken(Unit* attacker, uint32 &damage)
            {
                if (!isOnSameSide(attacker))
                    damage = 0;
            }

            void Reset()
            {
                events.ScheduleEvent(EVENT_PRIMARY_SKILL, urand(3000, 6000));
                events.ScheduleEvent(EVENT_SECONDARY_SKILL, urand (7000, 9000));
                if (myIndex == INDEX_DARK_RUNE_CHAMPION)
                    events.ScheduleEvent(EVENT_CHARGE, 8000);
                hf4 = false;
            }

            void EnterCombat(Unit* /*who*/)
            {
                if (myIndex == INDEX_DARK_RUNE_WARBRINGER)
                {
                    hf4 = true;
                    DoCast(me, SPELL_AURA_OF_CELERITY);
                }
            }

            // this should only happen if theres no alive player in the arena -> summon orb
            // might be called by mind control release or controllers death
            void EnterEvadeMode()
            {
                if (Creature* thorim = me->GetCreature(*me, _instance ? _instance->GetData64(BOSS_THORIM) : 0))
                    thorim->AI()->DoAction(ACTION_BERSERK);
                _EnterEvadeMode();
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (me->getVictim() && !isOnSameSide(me->getVictim()))
                {
                    me->getVictim()->getHostileRefManager().deleteReference(me);
                    return;
                }

                events.Update(diff);

                while (uint32 event = events.ExecuteEvent())
                {
                    switch (event)
                    {
                        case EVENT_PRIMARY_SKILL:
                            if (Unit* target = amIhealer ? (me->GetHealthPct() > 40 ? DoSelectLowestHpFriendly(40) : me) : me->getVictim())
                            {
                                if (myIndex != INDEX_DARK_RUNE_EVOKER)  // Specialize
                                    DoCast(target, SPELL_RUNIC_MENDING);
                                else
                                    DoCast(target, myHelper(myIndex, ArenaPhaseAddHelper::INDEX_PRIMARY));

                                events.ScheduleEvent(EVENT_PRIMARY_SKILL, urand(3000, 6000));
                            }
                            else
                                events.ScheduleEvent(EVENT_PRIMARY_SKILL, urand(1000, 2000));
                            break;
                        case EVENT_SECONDARY_SKILL:
                            if (Unit* target = amIhealer ? (me->GetHealthPct() > 40 ? DoSelectLowestHpFriendly(40) : me) : me->getVictim())
                            {
                                DoCast(target, myHelper(myIndex, ArenaPhaseAddHelper::INDEX_SECONDARY));
                                events.ScheduleEvent(EVENT_SECONDARY_SKILL, urand(12000, 16000));
                            }
                            else
                                events.ScheduleEvent(EVENT_SECONDARY_SKILL, urand(2000, 4000));
                            break;
                        case EVENT_CHARGE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40, true))
                                DoCast(target, SPELL_CHARGE);
                            events.ScheduleEvent(EVENT_CHARGE, 12000);
                            break;
                    }
                }

                if (myIndex == INDEX_ARENA_DARK_RUNE_ACOLYTE)
                    DoSpellAttackIfReady(SPELL_HOLY_SMITE);
                else
                    DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* _instance;
            ArenaAddIndex myIndex;
            EventMap events;
            ArenaPhaseAddHelper myHelper;
            bool IsInArena;
            bool amIhealer;
            bool hf4;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<npc_thorim_arena_phaseAI>(creature);
        }
};

/************************************************************************/
/* Runic Colossus                                                       */
/************************************************************************/
SummonLocation colossusAddLocations[]=
{
  {{2218.38f, -297.50f, 412.18f, 1.030f}, 32874},
  {{2235.07f, -297.98f, 412.18f, 1.613f}, 32874},
  {{2235.26f, -338.34f, 412.18f, 1.589f}, 32874},
  {{2217.69f, -337.39f, 412.18f, 1.241f}, 32874},
  {{2227.58f, -308.30f, 412.18f, 1.591f}, 33110},
  {{2227.47f, -345.37f, 412.18f, 1.566f}, 33110}
};
#define EMOTE_BARRIER                           "Runic Colossus surrounds itself with a crackling Runic Barrier!"

class npc_runic_colossus : public CreatureScript
{
    private:
        enum { EVENT_BARRIER = 1, EVENT_SMASH, EVENT_CHARGE, EVENT_RUNIC_SMASH };
    public:
        npc_runic_colossus() : CreatureScript("npc_runic_colossus") {}

        struct npc_runic_colossusAI : public ScriptedAI
        {
            npc_runic_colossusAI(Creature* creature) : ScriptedAI(creature), summons(me)
            {
                instance = creature->GetInstanceScript();
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                mui_smash = 1000;
                RunicSmashPhase = 1;
                Side = 0;
            }

            void Reset()
            {
                isMelleeRange = false;

                me->setActive(false);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

                // Runed Door closed
                if (instance)
                    instance->SetData(DATA_RUNIC_DOOR, GO_STATE_READY);

                // Spawn trashes
                summons.DespawnAll();
                for (uint8 i = 0; i < 6; i++)
                    me->SummonCreature(colossusAddLocations[i].entry, colossusAddLocations[i].pos.GetPositionX(), colossusAddLocations[i].pos.GetPositionY(), colossusAddLocations[i].pos.GetPositionZ(),
                                       colossusAddLocations[i].pos.GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
            }

            void JustDied(Unit* /*victim*/)
            {
                // Runed Door opened
                if (instance)
                    instance->SetData(DATA_RUNIC_DOOR, GO_STATE_ACTIVE);
            }

            void DoAction(const int32 action)
            {
                switch (action)
                {
                    case ACTION_DOSCHEDULE_RUNIC_SMASH:
                        mui_smash = 1000;
                        //                        events.ScheduleEvent(EVENT_RUNIC_SMASH, 1000);
                        RunicSmashPhase = 1;
                        break;
                }
            }

            void DoRunicSmash()
            {
                for (uint8 i = 0; i < 9; i++)
                    if (Creature* bunny = me->SummonCreature(NPC_THORIM_GOLEM_RH_BUNNY, Side ? 2236.0f : 2219.0f, i * 10 - 380.0f, 412.2f, 0, TEMPSUMMON_TIMED_DESPAWN, 5000))
                        bunny->AI()->SetData(1, (i + 1)* 200);

                for (uint8 i = 0; i < 9; i++)
                    if (Creature* bunny = me->SummonCreature(NPC_THORIM_GOLEM_LH_BUNNY, Side ? 2246.0f : 2209.0f, i * 10 - 380.0f, 412.2f, 0, TEMPSUMMON_TIMED_DESPAWN, 5000))
                        bunny->AI()->SetData(1, (i + 1)* 200);
            }

            void EnterCombat(Unit* who)
            {
                if (who->GetTypeId() != TYPEID_PLAYER)
                    return ;
                RunicSmashPhase = 0;
        //                me->InterruptNonMeleeSpells(true);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                isMelleeRange = true;
                events.ScheduleEvent(EVENT_BARRIER, urand(3000, 10000));
                events.ScheduleEvent(EVENT_SMASH, urand (15000, 16000));
                events.ScheduleEvent(EVENT_CHARGE, urand (12000, 14000));
            }

            void UpdateAI(uint32 const diff)
            {
                if (isMelleeRange == false)
                {
                    if (mui_smash <= diff)
                    {
                        if (RunicSmashPhase == 1)
                        {
                            Side = urand(0, 1);
                            if (Side == 1)
                                DoCast(me, SPELL_RUNIC_SMASH_LEFT);
                            else
                                DoCast(me, SPELL_RUNIC_SMASH_RIGHT);
                            RunicSmashPhase = 2;
                            mui_smash = 5200;
                        }
                        else if (RunicSmashPhase == 2)
                        {
                            RunicSmashPhase = 1;
                            DoRunicSmash();
                            mui_smash = 500;
                        }
                    }
                    else
                        mui_smash -= diff;
                }

                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 event = events.ExecuteEvent())
                {
                    switch (event)
                    {
                        case EVENT_BARRIER:
                            me->MonsterTextEmote(EMOTE_BARRIER, 0, true);
                            DoCast(me, SPELL_RUNIC_BARRIER);
                            events.ScheduleEvent(EVENT_BARRIER, urand(35000, 45000));
                            break;
                        case EVENT_SMASH:
                            DoCast(me, SPELL_SMASH);
                            events.ScheduleEvent(EVENT_SMASH, urand(15000, 18000));
                            break;
                        case EVENT_CHARGE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, -8.0f, true))
                            {
                                DoCast(target, SPELL_RUNIC_CHARGE);
                                events.ScheduleEvent(EVENT_CHARGE, 20000);
                            }
                            else
                                events.ScheduleEvent(EVENT_CHARGE, 2000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

            private:
                InstanceScript* instance;
                SummonList summons;
                EventMap events;

                uint8 Side;
                uint8 RunicSmashPhase;
                uint32 BarrierTimer;
                uint32 SmashTimer;
                uint32 ChargeTimer;
                uint32 RunicSmashTimer;

                bool isMelleeRange;
                uint32 mui_smash;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<npc_runic_colossusAI>(creature);
        }
};

class npc_runic_smash : public CreatureScript
{
    public:
        npc_runic_smash() : CreatureScript("npc_runic_smash") {}

        struct npc_runic_smashAI : public Scripted_NoMovementAI
        {
            npc_runic_smashAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
                //                me->SetReactState(REACT_PASSIVE);
                me->SetDisplayId(16925);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }

            void Reset()
            {
                ExplodeTimer = 1000;
            }

            void SetData(uint32 /*type*/, uint32 data)
            {
                ExplodeTimer = data;
            }

            void UpdateAI(uint32 const diff)
            {
                if (ExplodeTimer <= diff)
                {
                    DoCastAOE(SPELL_RUNIC_SMASH, true);
                    ExplodeTimer = 10000;
                }
                else ExplodeTimer -= diff;
            }

            private:
                uint32 ExplodeTimer;
        };


        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<npc_runic_smashAI>(creature);
        }
};

/************************************************************************/
/* Rune Giant                                                           */
/************************************************************************/
SummonLocation giantAddLocations[]=
{
    {{2198.05f, -428.77f, 419.95f, 6.056f}, 32875},
    {{2220.31f, -436.22f, 412.26f, 1.064f}, 32875},
    {{2158.88f, -441.73f, 438.25f, 0.127f}, 32875},
    {{2198.29f, -436.92f, 419.95f, 0.261f}, 33110},
    {{2230.93f, -434.27f, 412.26f, 1.931f}, 33110}
};

#define EMOTE_MIGHT                             "Ancient Rune Giant fortifies nearby allies with runic might!"

class npc_ancient_rune_giant : public CreatureScript
{
    private:
        enum { EVENT_STOMP = 1, EVENT_DETONATION };
    public:
        npc_ancient_rune_giant() : CreatureScript("npc_ancient_rune_giant") {}

        struct npc_ancient_rune_giantAI : public ScriptedAI
        {
            npc_ancient_rune_giantAI(Creature* creature) : ScriptedAI(creature), summons(me)
            {
                instance = creature->GetInstanceScript();
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            }

            void Reset()
            {
                events.ScheduleEvent(EVENT_STOMP, urand(10000, 12000));
                events.ScheduleEvent(EVENT_DETONATION, 25000);

                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

                // Stone Door closed
                if (instance)
                    instance->SetData(DATA_STONE_DOOR, GO_STATE_READY);

                // Spawn trashes
                summons.DespawnAll();
                for (uint8 i = 0; i < 5; i++)
                    me->SummonCreature(giantAddLocations[i].entry, giantAddLocations[i].pos.GetPositionX(), giantAddLocations[i].pos.GetPositionY(), giantAddLocations[i].pos.GetPositionZ(), giantAddLocations[i].pos.GetOrientation(),TEMPSUMMON_CORPSE_TIMED_DESPAWN,3000);
            }

            void JustSummoned(Creature *summon)
            {
                summons.Summon(summon);
            }

            void EnterCombat(Unit* /*who*/)
            {
                me->MonsterTextEmote(EMOTE_MIGHT, 0, true);
                DoCast(me, SPELL_RUNIC_FORTIFICATION, true);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }

            void JustDied(Unit* /*victim*/)
            {
                // Stone Door opened
                if (instance)
                    instance->SetData(DATA_STONE_DOOR, GO_STATE_ACTIVE);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 event = events.ExecuteEvent())
                {
                    switch (event)
                    {
                        case EVENT_STOMP:
                            DoCast(me, SPELL_STOMP);
                            events.ScheduleEvent(EVENT_STOMP, urand(10000, 12000));
                            break;
                        case EVENT_DETONATION:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40, true))
                            {
                                DoCast(target, SPELL_RUNE_DETONATION);
                                events.ScheduleEvent(EVENT_DETONATION, urand(10000, 12000));
                            }
                            else
                                events.ScheduleEvent(EVENT_DETONATION, urand(2000, 3000));
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }

            private:
                InstanceScript* instance;
                SummonList summons;
                EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<npc_ancient_rune_giantAI>(creature);
        }
};

class npc_sif : public CreatureScript
{
    private:
        enum { EVENT_FROSTBOLT = 1, EVENT_FROSTBOLT_VOLLEY, EVENT_BLIZZARD, EVENT_FROSTNOVA };
        enum SifSpells
        {
            SPELL_FROSTBOLT_VOLLEY = 62580,
            SPELL_FROSTNOVA        = 62597,
            SPELL_BLIZZARD         = 62603,
            SPELL_FROSTBOLT        = 69274
        };
    public:
        npc_sif() : CreatureScript("npc_sif") {}

        struct npc_sifAI : public ScriptedAI
        {
            npc_sifAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }

            void Reset()
            {
                events.ScheduleEvent(EVENT_FROSTBOLT, 2000);
                events.ScheduleEvent(EVENT_FROSTBOLT_VOLLEY, 15000);
                events.ScheduleEvent(EVENT_BLIZZARD, 30000);
                events.ScheduleEvent(EVENT_FROSTNOVA, urand(20000, 25000));
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                while (uint32 event = events.ExecuteEvent())
                {
                    switch (event)
                    {
                        case EVENT_FROSTBOLT:
                            if (instance->GetBossState(BOSS_THORIM) == DONE)
                            {
                                me->DespawnOrUnsummon();
                                return;
                            }
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60, true))
                                DoCast(target, SPELL_FROSTBOLT);
                            events.ScheduleEvent(EVENT_FROSTBOLT, 4000);
                            break;
                        case EVENT_FROSTBOLT_VOLLEY:
                            if (instance->GetBossState(BOSS_THORIM) == DONE)
                            {
                                me->DespawnOrUnsummon();
                                return;
                            }
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40, true))
                            {
                                DoResetThreat();
                                me->AddThreat(target, std::numeric_limits<float>::max());
                                DoCast(target, SPELL_FROSTBOLT_VOLLEY, true);
                                events.ScheduleEvent(EVENT_FROSTBOLT_VOLLEY, urand(15000, 20000));
                            }
                            else
                                events.ScheduleEvent(EVENT_FROSTBOLT_VOLLEY, urand(1500, 2000));
                            break;
                        case EVENT_BLIZZARD:
                            if (instance->GetBossState(BOSS_THORIM) == DONE)
                            {
                                me->DespawnOrUnsummon();
                                return;
                            }
                            //                            DoCast(me, SPELL_BLIZZARD, true);
                            //DoCast(me, 62577, true);
                            events.ScheduleEvent(EVENT_BLIZZARD, 45000);
                            break;
                        case EVENT_FROSTNOVA:
                            if (instance->GetBossState(BOSS_THORIM) == DONE)
                            {
                                me->DespawnOrUnsummon();
                                return;
                            }
                            DoCastAOE(SPELL_FROSTNOVA, true);
                            events.ScheduleEvent(EVENT_FROSTNOVA, urand(20000, 25000));
                            break;
                    }
                }
            }

        private:
            EventMap events;
            InstanceScript* instance;

        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<npc_sifAI>(creature);
        }
};

class spell_stormhammer_targeting : public SpellScriptLoader
{
    public:
        spell_stormhammer_targeting() : SpellScriptLoader("spell_stormhammer_targeting") {}

        class spell_stormhammer_targeting_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_stormhammer_targeting_SpellScript);

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                _target = NULL;
                unitList.remove_if(ArenaAreaCheck(false));

                if (unitList.empty())
                    return;

                _target = Trinity::Containers::SelectRandomContainerElement(unitList)->ToUnit();
                SetTarget(unitList);
            }

            void SetTarget(std::list<WorldObject*>& unitList)
            {
                unitList.clear();

                if (_target)
                    unitList.push_back(_target);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_stormhammer_targeting_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_stormhammer_targeting_SpellScript::SetTarget, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_stormhammer_targeting_SpellScript::SetTarget, EFFECT_2, TARGET_UNIT_SRC_AREA_ENEMY);
            }

            private:
                Unit* _target;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_stormhammer_targeting_SpellScript();
        }
};


class npc_orb_thorim : public CreatureScript
{
    public:
        npc_orb_thorim() : CreatureScript("npc_orb_thorim") {}

        struct npc_orb_thorimAI : public Scripted_NoMovementAI
        {
            npc_orb_thorimAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset()
            {
                me->SetCanFly(true);
            }

            void DamageTaken(Unit* , uint32 &damage)
            {
                damage = 0;
            }

            void UpdateAI(uint32 const )
            {
            }

        };


        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<npc_orb_thorimAI>(creature);
        }
};

void AddSC_boss_thorim()
{
    new boss_thorim();
    new npc_thorim_controller();
    new npc_thorim_pre_phase_add();
    new npc_thorim_arena_phase_add();
    new npc_runic_colossus();
    new npc_runic_smash();
    new npc_ancient_rune_giant();
    new npc_sif();
    new spell_stormhammer_targeting();
    new npc_orb_thorim();
}

#undef SPELL_CHAIN_LIGHTNING
#undef SPELL_RUNIC_MENDING
