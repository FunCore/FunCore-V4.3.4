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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "ulduar.h"

enum FreyaYells
{
    // Freya
    SAY_AGGRO                                    = 0,
    SAY_AGGRO_WITH_ELDER                         = 1,
    SAY_SLAY                                     = 2,
    SAY_DEATH                                    = 3,
    SAY_BERSERK                                  = 4,
    SAY_SUMMON_CONSERVATOR                       = 5,
    SAY_SUMMON_TRIO                              = 6,
    SAY_SUMMON_LASHERS                           = 7,
    SAY_YS_HELP                                  = 8,
    EMOTE_ALLIES                                 = 9,
    EMOTE_GIFT                                   = 10,

    // Elder Brightleaf
    SAY_BRIGHTLEAF_AGGRO                         = 0,
    SAY_BRIGHTLEAF_SLAY                          = 1,
    SAY_BRIGHTLEAF_DEATH                         = 2,

    // Elder Ironbranch
    SAY_IRONBRANCH_AGGRO                         = 3,
    SAY_IRONBRANCH_SLAY                          = 4,
    SAY_IRONBRANCH_DEATH                         = 5,

    // Elder Stonebark
    SAY_STONEBARK_AGGRO                          = 6,
    SAY_STONEBARK_SLAY                           = 7,
    SAY_STONEBARK_DEATH                          = 8,
};

enum Achievements
{
    // Con-speed-atory timed achievement.
    // TODO: Should be started when 1st trash is killed.
    ACHIEV_CON_SPEED_ATORY_START_EVENT           = 21597,
    SPELL_ACHIEVEMENT_CHECK                      = 65074,

    // Lumberjacked timed achievement.
    // TODO: should be started when 1st elder is killed.
    // Spell should be casted when 3rd elder is killed.
    ACHIEV_LUMBERJACKED                          = 21686,
    SPELL_LUMBERJACKED_ACHIEVEMENT_CHECK         = 65296,

    ACHIEVMENT_CON_SPEED_ATORY_10                = 2980,
    ACHIEVMENT_CON_SPEED_ATORY_25                = 2981,

    ACHIEVMENT_LUMBERJACKED_10                   = 2979,
    ACHIEVMENT_LUMBERJACKED_25                   = 3118,
};

enum FreyaSpells
{
    // Freya
    SPELL_BERSERK                                = 47008,
    SPELL_ATTUNED_TO_NATURE                      = 62519,
    SPELL_TOUCH_OF_EONAR_10                      = 62528,
    SPELL_TOUCH_OF_EONAR_25                      = 62892,
    SPELL_SUNBEAM_10                             = 62623,
    SPELL_SUNBEAM_25                             = 62872,
    SPELL_FREYA_GROUND_TREMOR_10                 = 62437,
    SPELL_FREYA_GROUND_TREMOR_25                 = 62859,
    SPELL_ROOTS_FREYA_10                         = 62438,
    SPELL_ROOTS_FREYA_25                         = 62861,
    SPELL_STONEBARK_ESSENCE                      = 62483,
    SPELL_IRONBRANCH_ESSENCE                     = 62484,
    SPELL_BRIGHTLEAF_ESSENCE                     = 62485,
    SPELL_DRAINED_OF_POWER                       = 62467,

    // Stonebark
    SPELL_FISTS_OF_STONE                         = 62344,
    SPELL_GROUND_TREMOR_10                       = 62325,
    SPELL_GROUND_TREMOR_25                       = 62932,
    SPELL_PETRIFIED_BARK_10                      = 62337,
    SPELL_PETRIFIED_BARK_25                      = 62933,
    SPELL_PETRIFIED_BARK_DMG                     = 62379,

    // Ironbranch
    SPELL_IMPALE_10                              = 62310,
    SPELL_IMPALE_25                              = 62928,
    SPELL_IRON_ROOTS_10                          = 62283,
    SPELL_IRON_ROOTS_25                          = 62930,
    SPELL_THORN_SWARM_10                         = 62285,
    SPELL_THORN_SWARM_25                         = 62931,

    // Brightleaf
    SPELL_FLUX_AURA                              = 62239,
    SPELL_FLUX                                   = 62262,
    SPELL_FLUX_PLUS                              = 62251,
    SPELL_FLUX_MINUS                             = 62252,
    SPELL_SOLAR_FLARE_10                         = 62240,
    SPELL_SOLAR_FLARE_25                         = 62920,
    SPELL_UNSTABLE_SUN_BEAM_SUMMON               = 62207, // Trigger 62221

    // Stack Removing of Attuned to Nature
    SPELL_REMOVE_25STACK                         = 62521,
    SPELL_REMOVE_10STACK                         = 62525,
    SPELL_REMOVE_2STACK                          = 62524,

    // Achievement spells
    SPELL_DEFORESTATION_CREDIT                   = 65015,
    SPELL_KNOCK_ON_WOOD_CREDIT                   = 65074,

    // Wave summoning spells
    SPELL_SUMMON_WAVE_10                         = 62688, // Trigger 62687 in Spell Effect - Trigger is used directly here
    SPELL_SUMMON_LASHERS                         = 62687,
    SPELL_SUMMON_TRIO                            = 62686,
    SPELL_SUMMON_ANCIENT_CONSERVATOR             = 62685,

    // Detonating Lasher
    SPELL_DETONATE_10                            = 62598,
    SPELL_DETONATE_25                            = 62937,
    SPELL_FLAME_LASH                             = 62608,

    // Ancient Water Spirit
    SPELL_TIDAL_WAVE_EFFECT                      = 62654,
    SPELL_TIDAL_WAVE_10                          = 62653,
    SPELL_TIDAL_WAVE_25                          = 62935,
    SPELL_WATER_REVIVE_VISUAL                    = 62849,

    // Storm Lasher
    SPELL_LIGHTNING_LASH_10                      = 62648,
    SPELL_LIGHTNING_LASH_25                      = 62939,
    SPELL_STORMBOLT_10                           = 62649,
    SPELL_STORMBOLT_25                           = 62938,
    SPELL_STORM_REVIVE_VISUAL                    = 62851,

    // Snaplasher
    SPELL_HARDENED_BARK_10                       = 62664,
    SPELL_HARDENED_BARK_25                       = 64191,
    SPELL_LASHER_REVIVE_VISUAL                   = 62848,
    SPELL_BARK_AURA                              = 62663,

    // Ancient Conservator
    SPELL_CONSERVATOR_GRIP                       = 62532,
    SPELL_NATURES_FURY_10                        = 62589,
    SPELL_NATURES_FURY_25                        = 63571,
    SPELL_SUMMON_PERIODIC                        = 62566,
    SPELL_SPORE_SUMMON_NW                        = 62582,
    SPELL_SPORE_SUMMON_NE                        = 62591,
    SPELL_SPORE_SUMMON_SE                        = 62592,
    SPELL_SPORE_SUMMON_SW                        = 62593,

    // Healthly Spore
    SPELL_HEALTHY_SPORE_VISUAL                   = 62538,
    SPELL_GROW                                   = 62559,
    SPELL_POTENT_PHEROMONES                      = 62541,

    // Eonar's Gift
    SPELL_LIFEBINDERS_GIFT_10                    = 62584,
    SPELL_LIFEBINDERS_GIFT_25                    = 64185,
    SPELL_PHEROMONES                             = 62619,
    SPELL_EONAR_VISUAL                           = 62579,
    SPELL_SUMMON_EONAR_GIFT                      = 62572,
    SPELL_LIFEBINDERS_GIFT_SUMMON_1              = 62568,
    SPELL_LIFEBINDERS_GIFT_TRIGGER_MISSILE_2     = 62870,
    SPELL_LIFEBINDERS_GIFT_SUMMON_2              = 62869,

    // Nature Bomb
    SPELL_NATURE_BOMB                            = 64587,
    SPELL_NATURE_BOMB_HM                         = 64650,
    SPELL_OBJECT_BOMB                            = 64600,
    SPELL_SUMMON_NATURE_BOMB                     = 64606,
    SPELL_NATURE_BOMB_VISUAL                     = 64648, // Projectile Visual ... Dummy

    // Unstable Sun Beam
    SPELL_UNSTABLE_SUN_BEAM_PERIODIC             = 62211,
    SPELL_UNSTABLE_ENERGY_10                     = 62217, // Remove Unstable Sun Beam Buff
    SPELL_UNSTABLE_ENERGY_25                     = 62922,
    SPELL_PHOTOSYNTHESIS                         = 62209,
    SPELL_UNSTABLE_SUN_BEAM_TRIGGERED            = 62243,
    SPELL_FREYA_UNSTABLE_SUNBEAM                 = 62450, // Or maybe 62866?

    // Sun Beam
    SPELL_FREYA_UNSTABLE_ENERGY_10               = 62451,
    SPELL_FREYA_UNSTABLE_ENERGY_25               = 62865,
    SPELL_FREYA_UNSTABLE_ENERGY_VISUAL           = 62216,

    // Attuned To Nature spells
    SPELL_ATTUNED_TO_NATURE_2_DOSE_REDUCTION     = 62524,
    SPELL_ATTUNED_TO_NATURE_10_DOSE_REDUCTION    = 62525,
    SPELL_ATTUNED_TO_NATURE_25_DOSE_REDUCTION    = 62521,
};

#define SPELL_TOUCH_OF_EONAR RAID_MODE(SPELL_TOUCH_OF_EONAR_10, SPELL_TOUCH_OF_EONAR_25)
#define SPELL_SUNBEAM RAID_MODE(SPELL_SUNBEAM_10, SPELL_SUNBEAM_25)
#define SPELL_FREYA_UNSTABLE_ENERGY RAID_MODE(SPELL_FREYA_UNSTABLE_ENERGY_10, SPELL_FREYA_UNSTABLE_ENERGY_25)
#define SPELL_IRON_ROOTS RAID_MODE(SPELL_IRON_ROOTS_10, SPELL_IRON_ROOTS_25)
#define SPELL_FREYA_GROUND_TREMOR RAID_MODE(SPELL_FREYA_GROUND_TREMOR_10, SPELL_FREYA_GROUND_TREMOR_25)
#define SPELL_LIFEBINDERS_GIFT RAID_MODE(SPELL_LIFEBINDERS_GIFT_10, SPELL_LIFEBINDERS_GIFT_25)
#define SPELL_DETONATE RAID_MODE(SPELL_DETONATE_10, SPELL_DETONATE_25)
#define SPELL_TIDAL_WAVE RAID_MODE(SPELL_TIDAL_WAVE_10, SPELL_TIDAL_WAVE_25)
#define SPELL_LIGHTNING_LASH RAID_MODE(SPELL_LIGHTNING_LASH_10, SPELL_LIGHTNING_LASH_25)
#define SPELL_STORMBOLT RAID_MODE(SPELL_STORMBOLT_10, SPELL_STORMBOLT_25)
#define SPELL_HARDENED_BARK RAID_MODE(SPELL_HARDENED_BARK_10, SPELL_HARDENED_BARK_25)
#define SPELL_NATURES_FURY RAID_MODE(SPELL_NATURES_FURY_10, SPELL_NATURES_FURY_25)
#define SPELL_SOLAR_FLARE RAID_MODE(SPELL_SOLAR_FLARE_10, SPELL_SOLAR_FLARE_25)
#define SPELL_UNSTABLE_ENERGY RAID_MODE(SPELL_UNSTABLE_ENERGY_10, SPELL_UNSTABLE_ENERGY_25)
#define SPELL_IMPALE RAID_MODE(SPELL_IMPALE_10, SPELL_IMPALE_25)
#define SPELL_THORN_SWARM RAID_MODE(SPELL_THORN_SWARM_10, SPELL_THORN_SWARM_25)
#define SPELL_ROOTS_FREYA RAID_MODE(SPELL_ROOTS_FREYA_10, SPELL_ROOTS_FREYA_25)
#define SPELL_GROUND_TREMOR RAID_MODE(SPELL_GROUND_TREMOR_10, SPELL_GROUND_TREMOR_25)
#define SPELL_PETRIFIED_BARK RAID_MODE(SPELL_PETRIFIED_BARK_10, SPELL_PETRIFIED_BARK_25)

enum FreyaNpcs
{
    NPC_SUN_BEAM                                 = 33170,
    NPC_DETONATING_LASHER                        = 32918,
    NPC_ANCIENT_CONSERVATOR                      = 33203,
    NPC_ANCIENT_WATER_SPIRIT                     = 33202,
    NPC_STORM_LASHER                             = 32919,
    NPC_SNAPLASHER                               = 32916,
    NPC_NATURE_BOMB                              = 34129,
    NPC_EONARS_GIFT                              = 33228,
    NPC_HEALTHY_SPORE                            = 33215,
    NPC_UNSTABLE_SUN_BEAM                        = 33050,
    NPC_IRON_ROOTS                               = 33088,
    NPC_STRENGTHENED_IRON_ROOTS                  = 33168,

    GAMEOBJECT_NATURE_BOMB                       = 194902,
};

enum FreyaActions
{
    ACTION_ELDER_DEATH         = 1,
    ACTION_ELDER_FREYA_KILLED,
    ACTION_ELEMENTAL_DEAD,
};

enum Models
{
    MODEL_INVISIBLE                             = 11686,
};

const uint32 WaveSpells[3] =
{
    SPELL_SUMMON_WAVE_10,
    SPELL_SUMMON_TRIO,
    SPELL_SUMMON_ANCIENT_CONSERVATOR
};

enum FreyaEvents
{
    // Freya
    EVENT_WAVE                                   = 1,
    EVENT_EONAR_GIFT                             = 2,
    EVENT_NATURE_BOMB                            = 3,
    EVENT_UNSTABLE_ENERGY                        = 4,
    EVENT_STRENGTHENED_IRON_ROOTS                = 5,
    EVENT_GROUND_TREMOR                          = 6,
    EVENT_SUNBEAM                                = 7,
    EVENT_ENRAGE                                 = 8,
};

enum Data
{
    DATA_GETTING_BACK_TO_NATURE = 1,
    DATA_KNOCK_ON_WOOD,
    DATA_ELEMENTAL_DIED,
    DATA_LASHER_DIED,
    DATA_TRIO_WAVE_COUNT,
};

#define WAVE_TIME                                60000 // Normal wave is one minute
#define TIME_DIFFERENCE                          10000 // If difference between waveTime and WAVE_TIME is bigger then TIME_DIFFERENCE, schedule EVENT_WAVE in 10 seconds

class npc_iron_roots : public CreatureScript
{
    private:
        enum { SPELL_DEATH_GRIP = 49560, FACTION_HOSTILE = 14 };
    public:
        npc_iron_roots() : CreatureScript("npc_iron_roots") {}

        struct npc_iron_rootsAI : public Scripted_NoMovementAI
        {
            npc_iron_rootsAI(Creature* creature) : Scripted_NoMovementAI(creature) {}

            void InitializeAI()
            {
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_ID, SPELL_DEATH_GRIP, true);
                me->setFaction(FACTION_HOSTILE);
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset()
            {
                summonerGUID = 0;
            }

            void IsSummonedBy(Unit* summoner)
            {
                if (summoner->GetTypeId() != TYPEID_PLAYER)
                    return;
                // Summoner is a player, who should have root aura on self
                summonerGUID = summoner->GetGUID();
                me->SetFacingToObject(summoner);
                me->SetInCombatWith(summoner);
            }

            void JustDied(Unit* /*who*/)
            {
                if (Player* target = ObjectAccessor::GetPlayer(*me, summonerGUID))
                {
                    target->RemoveAurasDueToSpell(SPELL_IRON_ROOTS);
                    target->RemoveAurasDueToSpell(SPELL_ROOTS_FREYA);
                }
                me->RemoveCorpse(false);
            }

            void UpdateAI(uint32 const diff)
            {
                if (checkTimer <= diff)
                {
                    if (Player* victim = ObjectAccessor::GetPlayer(*me, summonerGUID))
                        if (!victim->HasAura(SPELL_IRON_ROOTS) && !victim->HasAura(SPELL_ROOTS_FREYA))
                            me->DespawnOrUnsummon(2*IN_MILLISECONDS);
                    checkTimer = 3*IN_MILLISECONDS;
                }
                else
                    checkTimer -= diff;
            }

            private:
                uint32 checkTimer;
                uint64 summonerGUID;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<npc_iron_rootsAI>(creature);
        }
};

class boss_freya : public CreatureScript
{
    public:
        boss_freya() : CreatureScript("boss_freya") {}

        struct boss_freyaAI : public BossAI
        {
            boss_freyaAI(Creature* creature) : BossAI(creature, BOSS_FREYA)
            {
                pInstance = creature->GetInstanceScript();
                if (pInstance)
                    EncounterFinished = (pInstance->GetBossState(BOSS_FREYA) == DONE);
            }

            void Reset()
            {
                if (EncounterFinished) // May be called during fight if Freya gets outfight... hm, should _not_ happen regularly // Resolu , ne sera en fonction 35 QUE si freya est d�j� down, en gros, si elle est mort et qu'un mj la respawn.
                {
                    me->setFaction(35);
                }
                else
                {
                    if (pInstance)
                        pInstance->SetBossState(BOSS_FREYA, NOT_STARTED);
                    _Reset();
                    sp = 0;
                    trioWaveCount = 0;
                    trioWaveController = 0;
                    elderCount = 0;

                    for (uint8 i = 0; i < 3; ++i)
                        for (uint8 n = 0; n < 2; ++n)
                            ElementalGUID[i][n] = 0;
                    for (uint8 i = 0; i < 6; ++i)
                        for (uint8 n = 0; n < 2; ++n)
                            deforestation[i][n] = 0;
                    for (uint8 n = 0; n < 2; ++n)
                    {
                        checkElementalAlive[n] = true;
                        trioDefeated[n] = false;
                    }
                    for (uint8 n = 0; n < 3; ++n)
                        random[n] = false;

                    for (uint8 n = 0; n < 3; ++n)
                    {
                        if (Creature* elder = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_BRIGHTLEAF + n)))
                            if (elder->isAlive())
                            {
                                elder->ResetLootMode();
                                elder->AI()->EnterEvadeMode();
                            }
                    }
                }
                while (GameObject* go = me->FindNearestGameObject(GAMEOBJECT_NATURE_BOMB, 1000))
                    me->RemoveGameObject(go, true);
            }

            void KilledUnit(Unit* /*who*/)
            {
                Talk(SAY_SLAY);
            }

            void DamageTaken(Unit* /*attacker*/, uint32 &amount)
            {
                if (amount >= me->GetHealth())
                {
          //                    amount = 0;
                    EncounterPostProgress();
                }
            }

            void EncounterPostProgress()
            {
                if (EncounterFinished)
                    return;

                EncounterFinished = true;

                //! Freya's chest is dynamically spawned on death by different spells.
                const uint32 summonSpell[2][4] =
                {
                    /*          0Elder, 1Elder, 2Elder, 3Elder */
                    /* 10N */   {62950,  62953,  62952,  62954 },
                    /* 25N */   {62955,  62956,  62957,  62958}
                };
                me->CastSpell((Unit*)NULL, summonSpell[me->GetMap()->GetDifficulty()][elderCount], true);   // GetDifficulty should return 0 or 1 (numeric)

                Talk(SAY_DEATH);
                me->SetReactState(REACT_PASSIVE);
                me->RemoveAllAuras();
                me->AttackStop();
                me->setFaction(35);
                me->DeleteThreatList();
                me->CombatStop(true);
                me->CastSpell(me, SPELL_KNOCK_ON_WOOD_CREDIT, true);

                // getting back to nature achievement
                attunedToNature = me->GetAuraCount(SPELL_ATTUNED_TO_NATURE);

                // achievements credit
                DoCast(me, SPELL_ACHIEVEMENT_CHECK, true);

                Creature* Elder[3];
                for (uint8 n = 0; n < 3; ++n)
                {
                    if ((Elder[n] = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_BRIGHTLEAF + n))) != NULL)
                        if (Elder[n]->isAlive())
                        {
                            Elder[n]->RemoveAllAuras();
                            Elder[n]->AttackStop();
                            Elder[n]->CombatStop(true);
                            Elder[n]->DeleteThreatList();
                            Elder[n]->GetAI()->DoAction(ACTION_ELDER_FREYA_KILLED);
                        }
                }
                me->DespawnOrUnsummon(7500);
                _JustDied();
            }

            void EnterCombat(Unit* who)
            {
                _EnterCombat();
                DoZoneInCombat();
                Creature* Elder[3];
                for (uint8 n = 0; n < 3; ++n)
                {
                    if ((Elder[n] = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_BRIGHTLEAF + n))) != NULL)
                        if (Elder[n]->isAlive())
                        {
                            me->AddAura(SPELL_DRAINED_OF_POWER, Elder[n]);
                            Elder[n]->CastSpell(me, SPELL_IRONBRANCH_ESSENCE, true);
                            Elder[n]->RemoveLootMode(LOOT_MODE_DEFAULT); //! Why?
                            Elder[n]->AI()->AttackStart(who);
                            Elder[n]->AddThreat(who, 250.0f);
                            Elder[n]->SetInCombatWith(who);
                            ++elderCount;
                        }
                }

                // Note: The events below get scheduled until they are cancelled, i.e. by a dying elder (@see SetGUID)
                if (Elder[0] && Elder[0]->isAlive())
                {
                    Elder[0]->CastSpell(me, SPELL_BRIGHTLEAF_ESSENCE, true);
                    events.ScheduleEvent(EVENT_UNSTABLE_ENERGY, urand(10000, 20000));
                }

                if (Elder[1] && Elder[1]->isAlive())
                {
                    Elder[1]->CastSpell(me, SPELL_STONEBARK_ESSENCE, true);
                    events.ScheduleEvent(EVENT_GROUND_TREMOR, urand(10000, 20000));
                }

                if (Elder[2] && Elder[2]->isAlive())
                {
                    Elder[2]->CastSpell(me, SPELL_IRONBRANCH_ESSENCE, true);
                    events.ScheduleEvent(EVENT_STRENGTHENED_IRON_ROOTS, urand(10000, 20000));
                }

                if (elderCount == 0)
                    Talk(SAY_AGGRO);
                else
                    Talk(SAY_AGGRO_WITH_ELDER);

                me->CastCustomSpell(SPELL_ATTUNED_TO_NATURE, SPELLVALUE_AURA_STACK, 150, me, true);

                events.ScheduleEvent(EVENT_WAVE, 10000);
                events.ScheduleEvent(EVENT_NATURE_BOMB, 10000 + urand(10000, 20000)); // Event should at first occur 10-20 seconds after the first wave got spawned, and go on with the same period.
                events.ScheduleEvent(EVENT_EONAR_GIFT, 25000);
                events.ScheduleEvent(EVENT_ENRAGE, 600000);
                events.ScheduleEvent(EVENT_SUNBEAM, urand(5000, 15000));
            }

            uint32 GetData(uint32 type) const
            {
                switch (type)
                {
                    case DATA_GETTING_BACK_TO_NATURE:
                        return attunedToNature;
                    case DATA_KNOCK_ON_WOOD:
                        return elderCount;
                    case DATA_TRIO_WAVE_COUNT:
                        return trioWaveCount;
                }
                return 0;
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ENRAGE:
                            Talk(SAY_BERSERK);
                            DoCast(me, SPELL_BERSERK);
                            break;
                        case EVENT_SUNBEAM:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target, SPELL_SUNBEAM);
                            events.ScheduleEvent(EVENT_SUNBEAM, urand(10000, 15000));
                            break;
                        case EVENT_NATURE_BOMB:
                        {
                            //              if (sp >= 6)
                            if (Aura * aura = me->GetAura(SPELL_ATTUNED_TO_NATURE)) // This is change to phase 2: All stacks are down! On first visit, this prevents the event from being performed.
                                if (aura->GetStackAmount() > 0)
                                {
                                    events.ScheduleEvent(EVENT_NATURE_BOMB, urand(1000, 2000));
                                    return ;
                                }
                            // On every player
                            std::list<Player*> PlayerList;
                            Trinity::AnyPlayerInObjectRangeCheck checker(me, 50.0f);
                            Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, PlayerList, checker);
                            me->VisitNearbyWorldObject(50.0f, searcher);
                            for (std::list<Player*>::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                                (*itr)->CastSpell(*itr, SPELL_SUMMON_NATURE_BOMB, true);
                            events.ScheduleEvent(EVENT_NATURE_BOMB, urand(10000, 12000));
                            break;
                        }
                    case EVENT_UNSTABLE_ENERGY:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            DoCast(target, SPELL_FREYA_UNSTABLE_SUNBEAM, true);
                        events.ScheduleEvent(EVENT_UNSTABLE_ENERGY, urand(15000, 20000));
                        break;
                    case EVENT_WAVE:
                        if (Aura * aura = me->GetAura(SPELL_ATTUNED_TO_NATURE)) // This is change to phase 2: All stacks are down! On first visit, this prevents the event from being performed.
                            if (aura->GetStackAmount() > 0)
                            {
                                if (sp < 6)
                                    SpawnWave();
                                sp++;
                                events.ScheduleEvent(EVENT_WAVE, WAVE_TIME);
                            }
                        break;
                    case EVENT_EONAR_GIFT:
                        DoCast(me, SPELL_SUMMON_EONAR_GIFT);
                        events.ScheduleEvent(EVENT_EONAR_GIFT, urand(40000, 50000));
                        break;
                    case EVENT_STRENGTHENED_IRON_ROOTS:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true, -SPELL_ROOTS_FREYA))
                            target->CastSpell(target, SPELL_ROOTS_FREYA, true); // This must be casted by Target self
                        events.ScheduleEvent(EVENT_STRENGTHENED_IRON_ROOTS, urand(12000, 20000));
                        break;
                    case EVENT_GROUND_TREMOR:
                        DoCastAOE(SPELL_FREYA_GROUND_TREMOR);
                        events.ScheduleEvent(EVENT_GROUND_TREMOR, urand(25000, 28000));
                        break;
                    }
                }

                if (!me->HasAura(SPELL_TOUCH_OF_EONAR))
                    me->CastSpell(me, SPELL_TOUCH_OF_EONAR, true);

                diffTimer += diff;                                               // For getting time difference for Deforestation achievement

                // Elementals must be killed within 12 seconds of each other, or they will all revive and heal
                Creature* Elemental[3][2];
                for (uint8 i = 0; i < 2; ++i)
                {
                    if (checkElementalAlive[i])
                        elementalTimer[i] = 0;
                    else
                    {
                        elementalTimer[i] += diff;
                        if (elementalTimer[i] > 12000)
                        {
                            for (uint8 k = 0; k < 3; ++k)
                                Elemental[k][i] = ObjectAccessor::GetCreature(*me, ElementalGUID[k][i]);
                            if (!trioDefeated[i]) // Do *NOT* merge this bool with bool few lines below!
                            {
                                if (Elemental[0][i] && Elemental[1][i] && Elemental[2][i])
                                {
                                    for (uint8 n = 0; n < 3; ++n)
                                    {
                                        if (Elemental[n][i]->isAlive())
                                            Elemental[n][i]->SetHealth(Elemental[n][i]->GetMaxHealth());
                                        else
                                            Elemental[n][i]->Respawn();
                                    }
                                }
                            }
                            checkElementalAlive[i] = true;
                        }
                        else
                        {
                            if (!trioDefeated[i])
                            {
                                for (uint8 k = 0; k < 3; ++k)
                                    Elemental[k][i] = ObjectAccessor::GetCreature(*me, ElementalGUID[k][i]);
                                if (Elemental[0][i] && Elemental[1][i] && Elemental[2][i])
                                {
                                    if (Elemental[0][i]->isDead() && Elemental[1][i]->isDead() && Elemental[2][i]->isDead())
                                    {
                                        for (uint8 n = 0; n < 3; ++n)
                                        {
                                            summons.remove(Elemental[n][i]->GetGUID());
                                            Elemental[n][i]->DespawnOrUnsummon(5000);
                                            trioDefeated[i] = true; // Stack-decrease spell is automatically performed through SummonedCreatureDespawn
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                DoMeleeAttackIfReady();
            }

            // Check if all Trio NPCs are dead - achievement check
            void LasherDead(uint32 type)                                         // Type must be in format of a binary mask
            {
                uint8 n = 0;

                // Handling received data
                for (uint8 i = 0; i < 5; ++i)                                    // We have created "instances" for keeping informations about last 6 death lashers - needed because of respawning
                {
                    deforestation[i][0] = deforestation[(i + 1)][0];             // Time
                    deforestation[i][1] = deforestation[(i + 1)][1];             // Type
                }
                deforestation[5][0] = diffTimer;
                deforestation[5][1] = type;

                // Check for achievement completion
                if (deforestation[0][1])                                         // Check for proper functionality of binary masks (overflow would not be problem)
                {
                    for (uint8 i = 0; i < 6; ++i)                                // Count binary mask
                    {
                        n += deforestation[i][1];
                    }
                    if ((deforestation[5][0] - deforestation[0][0]) < 10000)     // Time check
                    {
                        if (n == 14 && instance)                                 // Binary mask check - verification of lasher types
                        {
                            instance->DoCastSpellOnPlayers(SPELL_DEFORESTATION_CREDIT);
                        }
                    }
                }
            }

            // Random order of spawning waves
            int GetWaveId()
            {
                if (random[0] && random[1] && random[2])
                    for (uint8 n = 0; n < 3; ++n)
                        random[n] = false;

                uint8 randomId = urand(0, 2);

                while (random[randomId])
                    randomId = urand(0, 2);

                random[randomId] = true;
                return randomId;
            }

            void SpawnWave()
            {
                switch (GetWaveId())
                {
                    case 0:
                        Talk(SAY_SUMMON_LASHERS);
                        for (uint8 n = 0; n < 10; ++n)
                            DoCast(SPELL_SUMMON_LASHERS);
                        break;
                    case 1:
                        Talk(SAY_SUMMON_TRIO);
                        DoCast(SPELL_SUMMON_TRIO);
                        trioWaveCount++;
                        break;
                    case 2:
                        Talk(SAY_SUMMON_CONSERVATOR);
                        DoCast(SPELL_SUMMON_ANCIENT_CONSERVATOR);
                        break;
                }
            }

            void JustSummoned(Creature* summoned)
            {
                switch (summoned->GetEntry())
                {
                    case NPC_SNAPLASHER:
                    case NPC_ANCIENT_WATER_SPIRIT:
                    case NPC_STORM_LASHER:
                        ElementalGUID[trioWaveController][trioWaveCount] = summoned->GetGUID();
                        summons.Summon(summoned);
                        ++trioWaveController;
                        if (trioWaveController > 2)
                            trioWaveController = 0;
                        break;
                    case NPC_DETONATING_LASHER:
                    case NPC_ANCIENT_CONSERVATOR:
                    default:
                        summons.Summon(summoned);
                        break;
                }

                // Need to have it there, or summoned units would do nothing until attacked
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 250.0f, true))
                {
                    summoned->AI()->AttackStart(target);
                    summoned->AddThreat(target, 250.0f);
                    DoZoneInCombat(summoned);
                }
            }

            void SummonedCreatureDespawn(Creature* summoned)
            {
                switch (summoned->GetEntry())
                {
                    case NPC_SNAPLASHER:
                    case NPC_ANCIENT_WATER_SPIRIT:
                    case NPC_STORM_LASHER:
                        summoned->CastSpell(me, SPELL_REMOVE_10STACK, true);
                        break;
                    case NPC_ANCIENT_CONSERVATOR:
                        summoned->CastSpell(me, SPELL_REMOVE_25STACK, true);
                        summoned->DespawnOrUnsummon(5000);
                        summons.Despawn(summoned);
                        break;
                }
            }

            void SummonedCreatureDies(Creature* summoned, Unit* who)
            {
                switch (summoned->GetEntry())
                {
                    case NPC_DETONATING_LASHER:
                        summoned->CastSpell(me, SPELL_REMOVE_2STACK, true);
                        summoned->CastSpell(who, SPELL_DETONATE, true);
                        summoned->DespawnOrUnsummon();
                        summons.Despawn(summoned);
                        break;
                    case NPC_SNAPLASHER:
                    case NPC_ANCIENT_WATER_SPIRIT:
                    case NPC_STORM_LASHER:
                    case NPC_ANCIENT_CONSERVATOR:
                        SummonedCreatureDespawn(summoned);  // To avoid duplicated code
                        break;
                }
            }

            void SetData(uint32 type, uint32 value)
            {
                switch (type)
                {
                    case DATA_ELEMENTAL_DIED:
                        checkElementalAlive[value] = false;
                        break;
                    case DATA_LASHER_DIED:
                        LasherDead(value);
                        break;
                }
            }

            void SetGUID(uint64 guid, int32 id)
            {
                switch (id)
                {
                    case ACTION_ELEMENTAL_DEAD:
                        if (Creature* c = ObjectAccessor::GetCreature(*me, guid))
                        {
                            switch (c->GetEntry())
                            {
                                case NPC_DETONATING_LASHER:
                                    DoCast(me, SPELL_ATTUNED_TO_NATURE_2_DOSE_REDUCTION, true);
                                    break;
                                case NPC_ANCIENT_WATER_SPIRIT:
                                case NPC_STORM_LASHER:
                                case NPC_SNAPLASHER:
                                    DoCast(me, SPELL_ATTUNED_TO_NATURE_10_DOSE_REDUCTION, true);
                                    break;
                                case NPC_ANCIENT_CONSERVATOR:
                                    DoCast(me, SPELL_ATTUNED_TO_NATURE_25_DOSE_REDUCTION, true);
                                    break;
                            }
                        }
                        break;
                    case ACTION_ELDER_DEATH:    // For each dying elder, Freya looses one ability.
                        --elderCount;
                        if (Creature* c = ObjectAccessor::GetCreature(*me, guid))
                        {
                            switch (c->GetEntry())
                            {
                                case NPC_ELDER_BRIGHTLEAF:
                                    events.CancelEvent(EVENT_UNSTABLE_ENERGY);
                                    break;
                                case NPC_ELDER_IRONBRANCH:
                                    events.CancelEvent(EVENT_STRENGTHENED_IRON_ROOTS);
                                    break;
                                case NPC_ELDER_STONEBARK:
                                    events.CancelEvent(EVENT_GROUND_TREMOR);
                                    break;
                            }
                        }
                        break;
                }
            }

            private:
                uint64 ElementalGUID[3][2];

                uint32 deforestation[6][2];
                uint32 elementalTimer[2];
                uint32 diffTimer;
                uint8 trioWaveCount;
                uint8 trioWaveController;
                uint8 elderCount;
                uint8 attunedToNature;

                bool checkElementalAlive[2];
                bool trioDefeated[2];
                bool random[3];
                bool EncounterFinished;
                InstanceScript* pInstance;

      uint32 sp;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<boss_freyaAI>(creature);
        }
};

class boss_elder_brightleaf : public CreatureScript
{
    private:
        enum
        {
            EVENT_SOLAR_FLARE                            = 15,
            EVENT_UNSTABLE_SUN_BEAM                      = 16,
            EVENT_FLUX                                   = 17,
        };
    public:
        boss_elder_brightleaf() : CreatureScript("boss_elder_brightleaf") {}

        struct boss_elder_brightleafAI : public BossAI
        {
            boss_elder_brightleafAI(Creature* creature) : BossAI(creature, BOSS_BRIGHTLEAF) {}

            void Reset()
            {
                _Reset();
                if (me->HasAura(SPELL_DRAINED_OF_POWER))
                    me->RemoveAurasDueToSpell(SPELL_DRAINED_OF_POWER);
                events.ScheduleEvent(EVENT_SOLAR_FLARE, urand(5000, 7000));
                events.ScheduleEvent(EVENT_UNSTABLE_SUN_BEAM, urand(7000, 12000));
                events.ScheduleEvent(EVENT_FLUX, 5000);
                elderCount = 0;
                lumberjack = false;
            }

            void KilledUnit(Unit* /*who*/)
            {
                Talk(SAY_BRIGHTLEAF_SLAY);
            }

            void JustDied(Unit* who)
            {
                _JustDied();
                Talk(SAY_BRIGHTLEAF_DEATH);

                if (who && who->GetTypeId() == TYPEID_PLAYER)
                {
                    // Inform the others that I died
                    if (Creature* Ironbranch = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_IRONBRANCH)))
                        Ironbranch->AI()->DoAction(ACTION_ELDER_DEATH);

                    if (Creature* Stonebark = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_STONEBARK)))
                        Stonebark->AI()->DoAction(ACTION_ELDER_DEATH);

                    if (Creature* freya = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_FREYA)))
                        freya->AI()->SetGUID(me->GetGUID(), ACTION_ELDER_DEATH);
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                if (!me->HasAura(SPELL_DRAINED_OF_POWER))
                    Talk(SAY_BRIGHTLEAF_AGGRO);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || me->HasAura(SPELL_DRAINED_OF_POWER))
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_UNSTABLE_SUN_BEAM:
                            me->CastSpell(me, SPELL_UNSTABLE_SUN_BEAM_SUMMON, true);
                            events.ScheduleEvent(EVENT_UNSTABLE_SUN_BEAM, urand(10000, 15000));
                            break;
                        case EVENT_SOLAR_FLARE:
                        {
                            uint32 stackAmount = 0;
                            if (me->GetAura(SPELL_FLUX_AURA))
                                stackAmount = me->GetAura(SPELL_FLUX_AURA)->GetStackAmount();
                            me->CastCustomSpell(SPELL_SOLAR_FLARE, SPELLVALUE_MAX_TARGETS, stackAmount, me, false);
                            events.ScheduleEvent(EVENT_SOLAR_FLARE, urand(5000, 10000));
                            break;
                        }
                        case EVENT_FLUX:
                            me->RemoveAurasDueToSpell(SPELL_FLUX_AURA);

                            me->RemoveAurasDueToSpell(SPELL_FLUX_PLUS);
                            me->RemoveAurasDueToSpell(SPELL_FLUX_MINUS);

                            me->AddAura(SPELL_FLUX_AURA, me);
                            if (Aura* Flux = me->GetAura(SPELL_FLUX_AURA))
                                Flux->SetStackAmount(urand(1, 8));
                            me->AddAura(RAND(SPELL_FLUX_PLUS, SPELL_FLUX_MINUS), me);
                            events.ScheduleEvent(EVENT_FLUX, 7500);
                            break;
                    }
                }

                if (lumberjack)
                    lumberjackTimer += diff;

                DoMeleeAttackIfReady();
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case ACTION_ELDER_DEATH:
                        ++elderCount;
                        lumberjack = true;
                        break;
                    case ACTION_ELDER_FREYA_KILLED:
                        me->DespawnOrUnsummon(10000);
                        _JustDied();
                        break;
                }
            }

            private:
                uint32 lumberjackTimer;
                uint8 elderCount;
                bool lumberjack;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<boss_elder_brightleafAI>(creature);
        }
};

class boss_elder_stonebark : public CreatureScript
{
    private:
        enum
        {
            EVENT_GROUND_TREMOR                          = 9,
            EVENT_PETRIFIED_BARK                         = 10,
            EVENT_FISTS_OF_STONE                         = 11,
        };
    public:
        boss_elder_stonebark() : CreatureScript("boss_elder_stonebark") {}

        struct boss_elder_stonebarkAI : public BossAI
        {
            boss_elder_stonebarkAI(Creature* creature) : BossAI(creature, BOSS_STONEBARK) {}

            void Reset()
            {
                _Reset();
                if (me->HasAura(SPELL_DRAINED_OF_POWER))
                    me->RemoveAurasDueToSpell(SPELL_DRAINED_OF_POWER);
                events.ScheduleEvent(EVENT_GROUND_TREMOR, urand(10000, 12000));
                events.ScheduleEvent(EVENT_FISTS_OF_STONE, urand(25000, 35000));
                events.ScheduleEvent(EVENT_PETRIFIED_BARK, urand(37500, 40000));
                elderCount = 0;
                lumberjack = false;
            }

            void KilledUnit(Unit* /*who*/)
            {
                Talk(SAY_STONEBARK_SLAY);
            }

            void JustDied(Unit* who)
            {
                _JustDied();
                Talk(SAY_STONEBARK_DEATH);

                if (who && who->GetTypeId() == TYPEID_PLAYER)
                {
                    // Inform the others that I died
                    if (Creature* Ironbranch = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_IRONBRANCH)))
                        Ironbranch->AI()->DoAction(ACTION_ELDER_DEATH);

                    if (Creature* Brightleaf = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_BRIGHTLEAF)))
                        Brightleaf->AI()->DoAction(ACTION_ELDER_DEATH);

                    if (Creature* freya = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_FREYA)))
                        freya->AI()->SetGUID(me->GetGUID(), ACTION_ELDER_DEATH);
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                if (!me->HasAura(SPELL_DRAINED_OF_POWER))
                    Talk(SAY_STONEBARK_AGGRO);
            }

            void DamageTaken(Unit* who, uint32& damage)
            {
                if (who == me)
                    return;

                if (me->HasAura(SPELL_PETRIFIED_BARK))
                {
                    int32 reflect = damage;
                    who->CastCustomSpell(who, SPELL_PETRIFIED_BARK_DMG, &reflect, NULL, NULL, true);
                    damage = 0;
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || me->HasAura(SPELL_DRAINED_OF_POWER))
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_PETRIFIED_BARK:
                            DoCast(me, SPELL_PETRIFIED_BARK);
                            events.ScheduleEvent(EVENT_PETRIFIED_BARK, urand(30000, 50000));
                            break;
                        case EVENT_FISTS_OF_STONE:
                            DoCastVictim(SPELL_FISTS_OF_STONE);
                            events.ScheduleEvent(EVENT_FISTS_OF_STONE, urand(20000, 30000));
                            break;
                        case EVENT_GROUND_TREMOR:
                            if (!me->HasAura(SPELL_FISTS_OF_STONE))
                                DoCastVictim(SPELL_GROUND_TREMOR);
                            events.ScheduleEvent(EVENT_GROUND_TREMOR, urand(10000, 20000));
                            break;
                    }
                }

                if (lumberjack)
                    lumberjackTimer += diff;

                DoMeleeAttackIfReady();
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case ACTION_ELDER_DEATH:
                        ++elderCount;
                        lumberjack = true;
                        break;
                    case ACTION_ELDER_FREYA_KILLED:
                        me->DespawnOrUnsummon(10000);
                        _JustDied();
                        break;
                }
            }

            private:
                uint32 lumberjackTimer;
                uint8 elderCount;
                bool lumberjack;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<boss_elder_stonebarkAI>(creature);
        }
};

class boss_elder_ironbranch : public CreatureScript
{
    private:
        enum
        {
            EVENT_IMPALE                                 = 12,
            EVENT_IRON_ROOTS                             = 13,
            EVENT_THORN_SWARM                            = 14,
        };
    public:
        boss_elder_ironbranch() : CreatureScript("boss_elder_ironbranch") {}

        struct boss_elder_ironbranchAI : public BossAI
        {
            boss_elder_ironbranchAI(Creature* creature) : BossAI(creature, BOSS_IRONBRANCH) {}

            void Reset()
            {
                _Reset();
                if (me->HasAura(SPELL_DRAINED_OF_POWER))
                    me->RemoveAurasDueToSpell(SPELL_DRAINED_OF_POWER);
                events.ScheduleEvent(EVENT_IMPALE, urand(18000, 22000));
                events.ScheduleEvent(EVENT_IRON_ROOTS, urand(12000, 17000));
                events.ScheduleEvent(EVENT_THORN_SWARM, urand(7500, 12500));
                elderCount = 0;
                lumberjack = false;
            }

            void KilledUnit(Unit* /*who*/)
            {
                Talk(SAY_IRONBRANCH_SLAY);
            }

            void JustDied(Unit* who)
            {
                _JustDied();
                Talk(SAY_IRONBRANCH_DEATH);

                if (who && who->GetTypeId() == TYPEID_PLAYER)
                {
                    if (Creature* Brightleaf = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_BRIGHTLEAF)))
                        Brightleaf->AI()->DoAction(ACTION_ELDER_DEATH);

                    if (Creature* Stonebark = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_STONEBARK)))
                        Stonebark->AI()->DoAction(ACTION_ELDER_DEATH);

                    if (Creature* freya = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_FREYA)))
                        freya->AI()->SetGUID(me->GetGUID(), ACTION_ELDER_DEATH);
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                if (!me->HasAura(SPELL_DRAINED_OF_POWER))
                    Talk(SAY_IRONBRANCH_AGGRO);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || me->HasAura(SPELL_DRAINED_OF_POWER))
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_IMPALE:
                            DoCastVictim(SPELL_IMPALE);
                            events.ScheduleEvent(EVENT_IMPALE, urand(15000, 25000));
                            break;
                        case EVENT_IRON_ROOTS:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true, -SPELL_IRON_ROOTS))
                                target->CastSpell(target, SPELL_IRON_ROOTS, true);
                            events.ScheduleEvent(EVENT_IRON_ROOTS, urand(10000, 20000));
                            break;
                        case EVENT_THORN_SWARM:
                            DoCastVictim(SPELL_THORN_SWARM);
                            events.ScheduleEvent(EVENT_THORN_SWARM, urand(8000, 13000));
                            break;
                    }
                }

                if (lumberjack)
                    lumberjackTimer += diff;

                DoMeleeAttackIfReady();
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case ACTION_ELDER_DEATH:
                        ++elderCount;
                        lumberjack = true;
                        break;
                    case ACTION_ELDER_FREYA_KILLED:
                        me->DespawnOrUnsummon(10000);
                        _JustDied();
                        break;
                }
            }

            private:
                uint32 lumberjackTimer;
                uint8 elderCount;
                bool lumberjack;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<boss_elder_ironbranchAI>(creature);
        }
};

class npc_detonating_lasher : public CreatureScript
{
    private:
        enum { EVENT_LASH = 1, EVENT_CHANGE_TARGET };
    public:
        npc_detonating_lasher() : CreatureScript("npc_detonating_lasher") {}

        struct npc_detonating_lasherAI : public ScriptedAI
        {
            npc_detonating_lasherAI(Creature* creature) : ScriptedAI(creature) {}

            void InitializeAI()
            {
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
            }

            void Reset()
            {
                if (Unit* target = me->SelectNearbyTarget(0, 100.0f))   // Try to find another target
                    AttackStart(target);

                events.ScheduleEvent(EVENT_LASH, 5000);
                events.ScheduleEvent(EVENT_CHANGE_TARGET, 7500);
            }

            void DamageTaken(Unit* /*attacker*/, uint32 &damage)    // TODO: Check possible double-call
            {
                if (damage >= me->GetHealth())
                {
                    if (InstanceScript* instance = me->GetInstanceScript())
                        if (Creature* freya = me->GetCreature(*me, instance->GetData64(BOSS_FREYA)))
                            freya->AI()->SetGUID(me->GetGUID(), ACTION_ELEMENTAL_DEAD);

                    me->DespawnOrUnsummon(1);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                while (uint32 event = events.ExecuteEvent())
                {
                    switch (event)
                    {
                        case EVENT_LASH:
                            DoCast(SPELL_FLAME_LASH);
                            events.ScheduleEvent(EVENT_LASH, urand(5000, 10000));
                            break;
                        case EVENT_CHANGE_TARGET:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            {
                                // Switching to other target - modify aggro of new target by 20% from current target's aggro
                                me->AddThreat(target, me->getThreatManager().getThreat(me->getVictim(), false) * 1.2f);
                                me->AI()->AttackStart(target);
                            }
                            events.ScheduleEvent(EVENT_CHANGE_TARGET, urand(5000, 10000));
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }

            private:
                EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<npc_detonating_lasherAI>(creature);
        }
};

class npc_ancient_water_spirit : public CreatureScript
{
    public:
        npc_ancient_water_spirit() : CreatureScript("npc_ancient_water_spirit") {}

        struct npc_ancient_water_spiritAI : public ScriptedAI
        {
            npc_ancient_water_spiritAI(Creature* creature) : ScriptedAI(creature) {}

            void InitializeAI()
            {
                if (InstanceScript* instance = me->GetInstanceScript())
                    if (Creature* Freya = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_FREYA)))
                        waveCount = Freya->AI()->GetData(DATA_TRIO_WAVE_COUNT);
            }

            void Reset()
            {
                if (Unit* target = me->SelectNearbyTarget(NULL,100.0f))
                    AttackStart(target);
                tidalWaveTimer = 10000;
            }

            void DamageTaken(Unit* /*attacker*/, uint32 &damage)
            {
                if (damage >= me->GetHealth())
                {
                    if (InstanceScript* instance = me->GetInstanceScript())
                        if (Creature* freya = me->GetCreature(*me, instance->GetData64(BOSS_FREYA)))
                        {
                            freya->AI()->SetGUID(me->GetGUID(), ACTION_ELEMENTAL_DEAD);
                            freya->AI()->SetData(DATA_ELEMENTAL_DIED, waveCount);
                            freya->AI()->SetData(DATA_LASHER_DIED, 1);
                        }
                    //                    me->DespawnOrUnsummon(1);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                if (tidalWaveTimer <= diff)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    {
                        DoCast(target, SPELL_TIDAL_WAVE);
                        DoCast(target, SPELL_TIDAL_WAVE_EFFECT, true);
                    }
                    tidalWaveTimer = urand(12000, 25000);
                }
                else
                    tidalWaveTimer -= diff;

                DoMeleeAttackIfReady();
            }

            private:
                uint32 tidalWaveTimer;
                uint8 waveCount;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<npc_ancient_water_spiritAI>(creature);
        }
};

class npc_storm_lasher : public CreatureScript
{
    private:
        enum { EVENT_LIGHTNING_LASH = 1, EVENT_STORMBOLT };
    public:
        npc_storm_lasher() : CreatureScript("npc_storm_lasher") {}

        struct npc_storm_lasherAI : public ScriptedAI
        {
            npc_storm_lasherAI(Creature* creature) : ScriptedAI(creature) {}

            void InitializeAI()
            {
                if (InstanceScript* instance = me->GetInstanceScript())
                    if (Creature* Freya = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_FREYA)))
                        waveCount = Freya->AI()->GetData(DATA_TRIO_WAVE_COUNT);
            }

            void Reset()
            {
                if (Unit* target = me->SelectNearbyTarget(NULL,100.0f))
                    AttackStart(target);
                events.ScheduleEvent(EVENT_LIGHTNING_LASH, 10000);
                events.ScheduleEvent(EVENT_STORMBOLT, 5000);
            }

            void DamageTaken(Unit* /*attacker*/, uint32 &damage)
            {
                if (damage >= me->GetHealth())
                {
                    if (InstanceScript* instance = me->GetInstanceScript())
                        if (Creature* freya = me->GetCreature(*me, instance->GetData64(BOSS_FREYA)))
                        {
                            freya->AI()->SetGUID(me->GetGUID(), ACTION_ELEMENTAL_DEAD);
                            freya->AI()->SetData(DATA_ELEMENTAL_DIED, waveCount);
                            freya->AI()->SetData(DATA_LASHER_DIED, 2);
                        }
                    //                    me->DespawnOrUnsummon(1);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                while (uint32 event = events.ExecuteEvent())
                {
                    switch (event)
                    {
                        case EVENT_LIGHTNING_LASH:
                            DoCast(SPELL_LIGHTNING_LASH);
                            events.ScheduleEvent(EVENT_LIGHTNING_LASH, urand(7000, 14000));
                            break;
                        case EVENT_STORMBOLT:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target, SPELL_STORMBOLT);
                            events.ScheduleEvent(EVENT_STORMBOLT, urand(8000, 12000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

            private:
                EventMap events;
                uint8 waveCount;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<npc_storm_lasherAI>(creature);
        }
};

class npc_snaplasher : public CreatureScript
{
    public:
        npc_snaplasher() : CreatureScript("npc_snaplasher") {}

        struct npc_snaplasherAI : public ScriptedAI
        {
            npc_snaplasherAI(Creature* creature) : ScriptedAI(creature) {}

            void InitializeAI()
            {
                if (InstanceScript* instance = me->GetInstanceScript())
                    if (Creature* Freya = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_FREYA)))
                        waveCount = Freya->AI()->GetData(DATA_TRIO_WAVE_COUNT);
            }

            void Reset()
            {
                if (Unit* target = me->SelectNearbyTarget(NULL,100.0f))
                    AttackStart(target);
            }

            void DamageTaken(Unit* /*attacker*/, uint32 &damage)
            {
                if (damage >= me->GetHealth())
                {
                    if (InstanceScript* instance = me->GetInstanceScript())
                        if (Creature* freya = me->GetCreature(*me, instance->GetData64(BOSS_FREYA)))
                        {
                            freya->AI()->SetGUID(me->GetGUID(), ACTION_ELEMENTAL_DEAD);
                            freya->AI()->SetData(DATA_ELEMENTAL_DIED, waveCount);
                            freya->AI()->SetData(DATA_LASHER_DIED, 4);
                        }
                    //                    me->DespawnOrUnsummon(1);
                }
            }

            void UpdateAI(uint32 const /*diff*/)
            {
                if (!UpdateVictim())
                    return;

                if (!me->HasAura(SPELL_BARK_AURA))
                    DoCast(SPELL_HARDENED_BARK);

                DoMeleeAttackIfReady();
            }

            private:
                uint8 waveCount;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<npc_snaplasherAI>(creature);
        }
};

class npc_ancient_conservator : public CreatureScript
{
    private:
        enum { EVENT_NATURES_FURY = 1, EVENT_HEALTHY_SPORE };
    public:
        npc_ancient_conservator() : CreatureScript("npc_ancient_conservator") {}

        struct npc_ancient_conservatorAI : public ScriptedAI
        {
            npc_ancient_conservatorAI(Creature* creature) : ScriptedAI(creature) {}

            void Reset()
            {
                events.ScheduleEvent(EVENT_NATURES_FURY, 7500);
                events.ScheduleEvent(EVENT_HEALTHY_SPORE, 3500);
                SummonHealthySpores(2);
            }

            void SummonHealthySpores(uint8 sporesCount)
            {
                // TODO: Check if this cast works, otherwise, summon them manually.
                for (uint8 n = 0; n < sporesCount; ++n)
                {
                    DoCast(SPELL_SUMMON_PERIODIC);
                    DoCast(SPELL_SPORE_SUMMON_NE);
                    DoCast(SPELL_SPORE_SUMMON_SE);
                    DoCast(SPELL_SPORE_SUMMON_SW);
                }
            }

            void DamageTaken(Unit* /*attacker*/, uint32 &damage)
            {
                if (damage >= me->GetHealth())
                {
                    if (InstanceScript* instance = me->GetInstanceScript())
                        if (Creature* freya = me->GetCreature(*me, instance->GetData64(BOSS_FREYA)))
                            freya->AI()->SetGUID(me->GetGUID(), ACTION_ELEMENTAL_DEAD);
                        me->DespawnOrUnsummon(1);
                }
            }

            void EnterCombat(Unit* who)
            {
                // TODO: fix this one, might somehow cause crit error ?
                DoCast(who, SPELL_CONSERVATOR_GRIP, true);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                while (uint32 event = events.ExecuteEvent())
                {
                    switch (event)
                    {
                        case EVENT_HEALTHY_SPORE:
                            SummonHealthySpores(1);
                            events.ScheduleEvent(EVENT_HEALTHY_SPORE, urand(15000, 17500));
                            break;
                        case EVENT_NATURES_FURY:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true, -SPELL_NATURES_FURY))
                                DoCast(target, SPELL_NATURES_FURY);
                            me->AddAura(SPELL_CONSERVATOR_GRIP, me);
                            events.ScheduleEvent(EVENT_NATURES_FURY, 5000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }

            private:
                EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<npc_ancient_conservatorAI>(creature);
        }
};

class npc_sun_beam : public CreatureScript
{
    public:
        npc_sun_beam() : CreatureScript("npc_sun_beam") {}

        struct npc_sun_beamAI : public Scripted_NoMovementAI
        {
            npc_sun_beamAI(Creature* creature) : Scripted_NoMovementAI(creature) {}

            void InitializeAI()
            {
                me->SetReactState(REACT_PASSIVE);
        //                DoCast(SPELL_FREYA_UNSTABLE_ENERGY);
            }

            void Reset()
            {
                _unstableEnergyTimer = 1000;
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->SetDisplayId(MODEL_INVISIBLE);
          me->DespawnOrUnsummon();
        //                me->DespawnOrUnsummon(12000);
        //      DoCast(me, SPELL_FREYA_UNSTABLE_ENERGY_VISUAL, true); // visual
            }

            void UpdateAI(uint32 const diff)    // hm... behavior correct ?
            {
                if (_unstableEnergyTimer <= diff)
                {
          //                    DoCast(SPELL_FREYA_UNSTABLE_ENERGY);
                }
                else
                    _unstableEnergyTimer -= diff;
            }

            private:
                uint32 _unstableEnergyTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<npc_sun_beamAI>(creature);
        }
};

class npc_healthy_spore : public CreatureScript
{
    public:
        npc_healthy_spore() : CreatureScript("npc_healthy_spore") {}

        struct npc_healthy_sporeAI : public Scripted_NoMovementAI
        {
      npc_healthy_sporeAI(Creature* creature) : Scripted_NoMovementAI(creature), _instance(creature->GetInstanceScript())
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC);
            }

            void Reset()
            {
                me->SetReactState(REACT_PASSIVE);
                DoCast(me, SPELL_HEALTHY_SPORE_VISUAL);
                DoCast(me, SPELL_POTENT_PHEROMONES);
                DoCast(me, SPELL_GROW);
                lifeTimer = urand(22000, 30000);
            }

            void UpdateAI(uint32 const diff)
            {
                if (_instance)
          {
                    if (_instance->GetBossState(BOSS_FREYA) != IN_PROGRESS)
                        me->DisappearAndDie();
          }
                else
                    me->DisappearAndDie();

                if (lifeTimer <= diff)
                {
                    me->RemoveAurasDueToSpell(SPELL_GROW);
                    me->DespawnOrUnsummon(2200);
                    lifeTimer = urand(22000, 30000);
                }
                else
                    lifeTimer -= diff;
            }

            private:
                uint32 lifeTimer;
      InstanceScript* _instance;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<npc_healthy_sporeAI>(creature);
        }
};

class npc_eonars_gift : public CreatureScript
{
public:
  npc_eonars_gift() : CreatureScript("npc_eonars_gift") {}

  struct npc_eonars_giftAI : public Scripted_NoMovementAI
  {
    npc_eonars_giftAI(Creature* creature) : Scripted_NoMovementAI(creature), _instance(creature->GetInstanceScript()) {}

    void InitializeAI()
    {
      DoCast(me, SPELL_PHEROMONES, true);
      DoCast(me, SPELL_EONAR_VISUAL, true);
      me->ClearUnitState(UNIT_STATE_CASTING);
      DoCast(me, SPELL_GROW);
      Reset();
    }

    void Reset()
    {
      lifeBindersGiftTimer = 12000;
    }


    void UpdateAI(uint32 const diff)
    {
      /*          if (_instance)
              {
              if (_instance->GetBossState(BOSS_FREYA) != IN_PROGRESS)
              me->DisappearAndDie();
              }
              else
              me->DisappearAndDie();*/

      if (lifeBindersGiftTimer <= diff)
      {
          me->RemoveAurasDueToSpell(SPELL_GROW);
          DoCast(SPELL_LIFEBINDERS_GIFT);
          me->DespawnOrUnsummon(2500);
          lifeBindersGiftTimer = 12000;
      }
      else
          lifeBindersGiftTimer -= diff;
    }

  private:
    uint32 lifeBindersGiftTimer;
    InstanceScript* _instance;
  };

  CreatureAI* GetAI(Creature* creature) const
  {
    return GetUlduarAI<npc_eonars_giftAI>(creature);
  }
};

class npc_nature_bomb : public CreatureScript
{
public:
  npc_nature_bomb() : CreatureScript("npc_nature_bomb") {}

  struct npc_nature_bombAI : public Scripted_NoMovementAI
  {
    npc_nature_bombAI(Creature* creature) : Scripted_NoMovementAI(creature) {}

    void Reset()
    {
      bombTimer = urand(8000, 10000);
      DoCast(SPELL_OBJECT_BOMB);
      me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    }

    void UpdateAI(uint32 const diff)
    {
        if (bombTimer <= diff)
        {
            if (GameObject* go = me->FindNearestGameObject(GAMEOBJECT_NATURE_BOMB, 1.0f))
            {
                go->SetGoState(GO_STATE_ACTIVE);
                DoCast(me, SPELL_NATURE_BOMB);
                me->RemoveGameObject(go, true);
                me->RemoveFromWorld();
            }
        }
        else
            bombTimer -= diff;
    }

  private:
    uint32 bombTimer;
  };

  CreatureAI* GetAI(Creature* creature) const
  {
    return GetUlduarAI<npc_nature_bombAI>(creature);
  }
};

class npc_unstable_sun_beam : public CreatureScript
{
    public:
        npc_unstable_sun_beam() : CreatureScript("npc_unstable_sun_beam") {}

        struct npc_unstable_sun_beamAI : public Scripted_NoMovementAI
        {
            npc_unstable_sun_beamAI(Creature* creature) : Scripted_NoMovementAI(creature) {}

            void Reset()
            {
                me->SetDisplayId(MODEL_INVISIBLE);
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                DoCast(me, SPELL_PHOTOSYNTHESIS);
                DoCast(me, SPELL_UNSTABLE_SUN_BEAM_PERIODIC);
                despawnTimer = urand(7000, 12000);
            }

            void MoveInLineOfSight(Unit* target)
            {
                if (target)
                    if (Creature* c = target->ToCreature())
                        if (c->GetEntry() == NPC_ELDER_BRIGHTLEAF)
                            if (me->IsWithinDist2d(c, 4))
                  {
                                if (!c->HasAura(SPELL_PHOTOSYNTHESIS))                  // Simulates something like "move aura photosynthesis from elder to me"
                  me->AddAura(SPELL_PHOTOSYNTHESIS, c);
                else
                  if (c->HasAura(SPELL_PHOTOSYNTHESIS))
                                    c->RemoveAurasDueToSpell(SPELL_PHOTOSYNTHESIS);
                  }
            }

            void UpdateAI(uint32 const diff)
            {
                if (despawnTimer <= diff)
                {
                    DoCastAOE(SPELL_UNSTABLE_ENERGY, true);
                    me->DisappearAndDie();
                }
                else
                    despawnTimer -= diff;
            }

            void SpellHitTarget(Unit* target, SpellInfo const* spell)
            {
                if (target && spell->Id == SPELL_UNSTABLE_ENERGY)
                {
                    target->RemoveAurasDueToSpell(SPELL_UNSTABLE_SUN_BEAM_PERIODIC);
                    target->RemoveAurasDueToSpell(SPELL_UNSTABLE_SUN_BEAM_TRIGGERED);
                }
            }

            private:
                uint32 despawnTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<npc_unstable_sun_beamAI>(creature);
        }
};

class IsNoAllyOfNature
{
    public:
        bool operator() (WorldObject* unit)
        {
            if (unit->ToCreature())
            {
                switch (unit->ToCreature()->GetEntry())
                {
                    case NPC_DETONATING_LASHER:
                    case NPC_ANCIENT_WATER_SPIRIT:
                    case NPC_STORM_LASHER:
                    case NPC_SNAPLASHER:
                    case NPC_ANCIENT_CONSERVATOR:
                        return false;
                    default:
                        break;
                }
            }
            return true;
        }
};

class spell_essence_targeting_ironbranch : public SpellScriptLoader
{
    public:
        spell_essence_targeting_ironbranch() : SpellScriptLoader("spell_elder_ironbranch_essence_targeting") {}

        class spell_essence_targeting_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_essence_targeting_SpellScript);

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                unitList.remove_if(IsNoAllyOfNature());
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_essence_targeting_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_essence_targeting_SpellScript();
        }
};

class spell_essence_targeting_brightleaf : public SpellScriptLoader
{
public:
    spell_essence_targeting_brightleaf() : SpellScriptLoader("spell_elder_brightleaf_essence_targeting") {}

    class spell_essence_targeting_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_essence_targeting_SpellScript);

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            unitList.remove_if(IsNoAllyOfNature());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_essence_targeting_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_essence_targeting_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ALLY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_essence_targeting_SpellScript();
    }
};


class spell_aggregation_pheromones_targeting : public SpellScriptLoader
{
    public:
        spell_aggregation_pheromones_targeting() : SpellScriptLoader("spell_aggregation_pheromones_targeting") {}

        class spell_aggregation_pheromones_targeting_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_aggregation_pheromones_targeting_SpellScript);

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                // remove caster if this is the only target
                if (unitList.size() < 2)
                    unitList.clear();
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_aggregation_pheromones_targeting_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_aggregation_pheromones_targeting_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ALLY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_aggregation_pheromones_targeting_SpellScript::FilterTargets, EFFECT_2, TARGET_UNIT_SRC_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_aggregation_pheromones_targeting_SpellScript();
        }
};

// temporary to trigger spell on proper target
class spell_elder_brightleaf_unstable_sun_beam : public SpellScriptLoader
{
    public:
        spell_elder_brightleaf_unstable_sun_beam() : SpellScriptLoader("spell_elder_brightleaf_unstable_sun_beam") {}

        class spell_elder_brightleaf_unstable_sun_beam_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_elder_brightleaf_unstable_sun_beam_SpellScript);

            void HandleForceCast(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);

                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();

                uint32 triggered_spell_id = GetSpellInfo()->Effects[effIndex].TriggerSpell;

                if (caster && target && triggered_spell_id)
                    target->CastSpell(target, triggered_spell_id, true, 0, 0, caster->GetGUID());
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_elder_brightleaf_unstable_sun_beam_SpellScript::HandleForceCast, EFFECT_1, SPELL_EFFECT_FORCE_CAST);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_elder_brightleaf_unstable_sun_beam_SpellScript();
        }
};

class spell_freya_attuned_to_nature_dose_reduction : public SpellScriptLoader
{
    public:
        spell_freya_attuned_to_nature_dose_reduction() : SpellScriptLoader("spell_freya_attuned_to_nature_dose_reduction") {}

        class spell_freya_attuned_to_nature_dose_reduction_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_freya_attuned_to_nature_dose_reduction_SpellScript)

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                Unit* target = GetHitUnit();
                SpellInfo const* spellInfo = GetSpellInfo();
                switch (spellInfo->Id)
                {
                    case SPELL_ATTUNED_TO_NATURE_2_DOSE_REDUCTION:
                        if (target->HasAura(GetEffectValue()))
                            for (uint8 n = 0; n < 2; ++n)
                                target->RemoveAuraFromStack(GetEffectValue(), 0, AURA_REMOVE_BY_DEFAULT);
                        break;
                    case SPELL_ATTUNED_TO_NATURE_10_DOSE_REDUCTION:
                        if (target->HasAura(GetEffectValue()))
                            for (uint8 n = 0; n < 10; ++n)
                                target->RemoveAuraFromStack(GetEffectValue(), 0, AURA_REMOVE_BY_DEFAULT);
                        break;
                    case SPELL_ATTUNED_TO_NATURE_25_DOSE_REDUCTION:
                        if (target->HasAura(GetEffectValue()))
                            for (uint8 n = 0; n < 25; ++n)
                                target->RemoveAuraFromStack(GetEffectValue(), 0, AURA_REMOVE_BY_DEFAULT);
                        break;
                    default:
                        break;
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_freya_attuned_to_nature_dose_reduction_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_freya_attuned_to_nature_dose_reduction_SpellScript();
        }
};

class spell_freya_iron_roots : public SpellScriptLoader
{
    public:
        spell_freya_iron_roots() : SpellScriptLoader("spell_freya_iron_roots") {}

        class spell_freya_iron_roots_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_freya_iron_roots_SpellScript);

            void HandleSummon(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                uint32 entry = uint32(GetSpellInfo()->Effects[effIndex].MiscValue);

                Position pos;
                GetCaster()->GetPosition(&pos);
                // Not good at all, but this prevents having roots in a different position then player
                if (Creature* Roots = GetCaster()->SummonCreature(entry, pos))
                    GetCaster()->NearTeleportTo(Roots->GetPositionX(), Roots->GetPositionY(), Roots->GetPositionZ(), GetCaster()->GetOrientation());
            }

            void Register()
            {
                OnEffectHit += SpellEffectFn(spell_freya_iron_roots_SpellScript::HandleSummon, EFFECT_0, SPELL_EFFECT_SUMMON);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_freya_iron_roots_SpellScript();
        }
};

class spell_freya_natural_bomb_spell : public SpellScriptLoader
{
    public:
        spell_freya_natural_bomb_spell() : SpellScriptLoader("spell_freya_natural_bomb_spell") {}

        class spell_freya_natural_bomb_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_freya_natural_bomb_SpellScript);

            void OnHitEffect()
            {
                if (Unit* target = GetHitUnit())
                    target->CastSpell(target, SPELL_SUMMON_NATURE_BOMB, true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_freya_natural_bomb_SpellScript::OnHitEffect);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_freya_natural_bomb_SpellScript();
        }
};

class achievement_getting_back_to_nature : public AchievementCriteriaScript
{
    public:
        achievement_getting_back_to_nature(const char* name) : AchievementCriteriaScript(name) {}

        bool OnCheck(Player* /*player*/, Unit* target)
        {
            if (!target)
                return false;

            if (Creature* Freya = target->ToCreature())
                if (Freya->AI()->GetData(DATA_GETTING_BACK_TO_NATURE) >= 25)
                    return true;

            return false;
        }
};

class achievement_knock_on_wood : public AchievementCriteriaScript
{
   public:
       achievement_knock_on_wood(const char* name) : AchievementCriteriaScript(name) {}

       bool OnCheck(Player* /*player*/, Unit* target)
       {
           if (!target)
               return false;

           if (Creature* Freya = target->ToCreature())
               if (Freya->AI()->GetData(DATA_KNOCK_ON_WOOD) >= 1)
                   return true;

           return false;
       }
};

class achievement_knock_knock_on_wood : public AchievementCriteriaScript
{
   public:
       achievement_knock_knock_on_wood(const char* name) : AchievementCriteriaScript(name) {}

       bool OnCheck(Player* /*player*/, Unit* target)
       {
           if (!target)
               return false;

           if (Creature* Freya = target->ToCreature())
               if (Freya->AI()->GetData(DATA_KNOCK_ON_WOOD) >= 2)
                   return true;

           return false;
       }
};

class achievement_knock_knock_knock_on_wood : public AchievementCriteriaScript
{
   public:
       achievement_knock_knock_knock_on_wood(const char* name) : AchievementCriteriaScript(name) {}

       bool OnCheck(Player* /*player*/, Unit* target)
       {
           if (!target)
               return false;

           if (Creature* Freya = target->ToCreature())
               if (Freya->AI()->GetData(DATA_KNOCK_ON_WOOD) == 3)
                   return true;

           return false;
       }
};

void AddSC_boss_freya()
{
    new boss_freya();
    new boss_elder_brightleaf();
    new boss_elder_ironbranch();
    new boss_elder_stonebark();
    new npc_ancient_conservator();
    new npc_snaplasher();
    new npc_storm_lasher();
    new npc_ancient_water_spirit();
    new npc_detonating_lasher();
    new npc_sun_beam();
    new npc_nature_bomb();
    new npc_eonars_gift();
    new npc_healthy_spore();
    new npc_unstable_sun_beam();
    new npc_iron_roots();
    // new spell_essence_targeting("spell_elder_stonebark_essence_targeting");
    new spell_essence_targeting_ironbranch();
    new spell_essence_targeting_brightleaf();
    new spell_aggregation_pheromones_targeting();       // more or less for NPCs that are always in this area
    new spell_freya_attuned_to_nature_dose_reduction();
    new spell_freya_iron_roots();
    new spell_elder_brightleaf_unstable_sun_beam();
    new achievement_getting_back_to_nature("achievement_getting_back_to_nature");           // 10m 2982 [10445]
    new achievement_getting_back_to_nature("achievement_getting_back_to_nature_25");        // 25m 2983 [10758]
    new achievement_knock_on_wood("achievement_knock_on_wood");                             // 10m 3177 [10447]
    new achievement_knock_on_wood("achievement_knock_on_wood_25");                          // 25m 3185 [10459]
    new achievement_knock_knock_on_wood("achievement_knock_knock_on_wood");                 // 10m 3178 [10448]
    new achievement_knock_knock_on_wood("achievement_knock_knock_on_wood_25");              // 25m 3186 [10460]
    new achievement_knock_knock_knock_on_wood("achievement_knock_knock_knock_on_wood");     // 10m 3179 [10449]
    new achievement_knock_knock_knock_on_wood("achievement_knock_knock_knock_on_wood_25");  // 25m 3187 [10461]
}

#undef SPELL_TOUCH_OF_EONAR
#undef SPELL_SUNBEAM
#undef SPELL_FREYA_UNSTABLE_ENERGY
#undef SPELL_IRON_ROOTS
#undef SPELL_FREYA_GROUND_TREMOR
#undef SPELL_LIFEBINDERS_GIFT
#undef SPELL_DETONATE
#undef SPELL_TIDAL_WAVE
#undef SPELL_LIGHTNING_LASH
#undef SPELL_STORMBOLT
#undef SPELL_HARDENED_BARK
#undef SPELL_NATURES_FURY
#undef SPELL_SOLAR_FLARE
#undef SPELL_UNSTABLE_ENERGY
#undef SPELL_IMPALE
#undef SPELL_THORN_SWARM
#undef SPELL_ROOTS_FREYA
#undef SPELL_GROUND_TREMOR
#undef SPELL_PETRIFIED_BARK
