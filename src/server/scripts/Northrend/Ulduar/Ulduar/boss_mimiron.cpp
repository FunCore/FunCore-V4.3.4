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

#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "ulduar.h"
#include "Vehicle.h"

enum Yells
{
    SAY_AGGRO                                   = 0,
    SAY_HARDMODE_ON                             = 1,
    SAY_MKII_ACTIVATE                           = 2,
    SAY_MKII_SLAY                               = 3,
    SAY_MKII_DEATH                              = 4,
    SAY_VX001_ACTIVATE                          = 5,
    SAY_VX001_SLAY                              = 6,
    SAY_VX001_DEATH                             = 7,

    SAY_AERIAL_ACTIVATE                         = 0,
    SAY_AERIAL_SLAY                             = 1,
    SAY_AERIAL_DEATH                            = 2,
    SAY_V07TRON_ACTIVATE                        = 3,
    SAY_V07TRON_SLAY                            = 4,
    SAY_V07TRON_DEATH                           = 5,
    SAY_BERSERK                                 = 6,
    SAY_YS_HELP                                 = 7,
};

#define SAY_TIME_1                              "Auto-destruction dans 10 minutes !"
#define SAY_TIME_2                              "Auto-destruction dans 9 minutes !"
#define SAY_TIME_3                              "Auto-destruction dans 8 minutes !"
#define SAY_TIME_4                              "Auto-destruction dans 7 minutes !"
#define SAY_TIME_5                              "Auto-destruction dans 6 minutes !"
#define SAY_TIME_6                              "Auto-destruction dans 5 minutes !"
#define SAY_TIME_7                              "Auto-destruction dans 4 minutes !"
#define SAY_TIME_8                              "Auto-destruction dans 3 minutes !"
#define SAY_TIME_9                              "Auto-destruction dans 3 minutes !"
#define SAY_TIME_10                             "Auto-destruction dans 1 minute !"
#define SAY_TIME_UP                             "Fin de la sequence d'auto-destruction. Passez une bonne journee !"
#define SAY_TIME_CANCEL                         "Auto-destruction annulee. Transmition du code A905."
#define SAY_ALARM_HARD_MODE                     "Auto-destruction initialisee."

enum Spells
{
   SPELL_JETPACK                               = 63341, // Unused
   SPELL_EMERGENCY_MODE                        = 64582,
   SPELL_SELF_REPAIR                           = 64383,

   // Leviathan MK II
   SPELL_MINES_SPAWN                           = 65347,
   SPELL_FLAME_SUPPRESSANT_MK                  = 64570,
   SPELL_NAPALM_SHELL_10                       = 63666,
   SPELL_NAPALM_SHELL_25                       = 65026,
   SPELL_PLASMA_BLAST_10                          = 62997,
   SPELL_PLASMA_BLAST_25                          = 64529,
   SPELL_PROXIMITY_MINES                       = 63027,
   SPELL_SHOCK_BLAST                           = 63631,
   SPELL_EXPLOSION                             = 66351,
   // VX 001
   SPELL_FROSTBOMB                             = 64623, // 64627 ?
   SPELL_FROST_BOMB_VISUAL                     = 64624,
   SPELL_FROST_BOMB_EXPLOSION_10               = 64626,
   SPELL_FROST_BOMB_EXPLOSION_25               = 65333,
   SPELL_LASER_BARRAGE                         = 63274,

   SPELL_P3WX2_LASER_BARRAGE                   = 63293, // Casted through SPELL_SPINNING_UP
   SPELL_RAPID_BURST                           = 63382,
   SPELL_RAPID_BURST_LEFT_10                   = 63387,
   SPELL_RAPID_BURST_RIGHT_10                  = 64019,
   SPELL_RAPID_BURST_LEFT_25                   = 64531,
   SPELL_RAPID_BURST_RIGHT_25                  = 64532,
   SPELL_ROCKET_STRIKE                         = 63036,
   SPELL_ROCKET_STRIKE_AURA                    = 64064,
   SPELL_ROCKET_STRIKE_DMG                     = 63041,
   SPELL_SPINNING_UP                           = 63414,
   SPELL_HEAT_WAVE_10                          = 63677,
   SPELL_HEAT_WAVE_25                          = 64533,
   SPELL_HAND_PULSE_10                         = 64348,
   SPELL_HAND_PULSE_25                         = 64537,

   // Aerial Command Unit
   SPELL_PLASMA_BALL                           = 63689,
   SPELL_MAGNETIC_CORE                         = 64436,
   SPELL_MAGNETIC_CORE_VISUAL                  = 64438,
   SPELL_BOOM_BOT                              = 63767,
   SPELL_BOOM_BOT_PERIODIC                     = 63801,
   SPELL_MAGNETIC_FIELD                        = 64668,
   SPELL_HOVER                                 = 57764, // Set Hover position
   SPELL_BERSERK                               = 47008,

   // Additonal spells
   SPELL_DEAFENING_SIREN                       = 64616,

   SPELL_SELF_DESTRUCTION                      = 64610,
   SPELL_SELF_DESTRUCTION_VISUAL               = 64613,
   SPELL_FLAME_SUPPRESSANT_VX001               = 65192,
   SPELL_SUMMON_FLAMES_INITIAL                 = 64563,
   SPELL_FLAME                                 = 64561,
   SPELL_NOT_SO_FRIENDLY_FIRE                  = 65040,

   SPELL_WATER_SPRAY                           = 64619,
   SPELL_RIDE_VEHICLE                          = 46598,
};

#define SPELL_NAPALM_SHELL RAID_MODE(SPELL_NAPALM_SHELL_10, SPELL_NAPALM_SHELL_25)
#define SPELL_HEAT_WAVE RAID_MODE(SPELL_HEAT_WAVE_10, SPELL_HEAT_WAVE_25)
#define SPELL_HAND_PULSE RAID_MODE(SPELL_HAND_PULSE_10, SPELL_HAND_PULSE_25)
#define SPELL_PLASMA_BLAST RAID_MODE(SPELL_PLASMA_BLAST_10, SPELL_PLASMA_BLAST_25)

enum Actions
{
    DO_START_ENCOUNTER = 1,
    DO_ACTIVATE_VX001,
    DO_START_VX001,
    DO_ACTIVATE_AERIAL,
    DO_START_AERIAL,
    DO_DISABLE_AERIAL,
    DO_ACTIVATE_V0L7R0N,
    DO_ACTIVATE_DEATH_TIMER,
    DO_LEVIATHAN_ASSEMBLED,
    DO_LEVIATHAN_SELF_REPAIR_START,
    DO_LEVIATHAN_SELF_REPAIR_END,
    DO_VX001_ASSEMBLED,
    DO_VX001_SELF_REPAIR_START,
    DO_VX001_SELF_REPAIR_END,
    DO_AERIAL_ASSEMBLED,
    DO_AERIAL_SELF_REPAIR_START,
    DO_AERIAL_SELF_REPAIR_END,
    DO_ENTER_ENRAGE,
    DO_ACTIVATE_HARD_MODE,
    DO_INCREASE_FLAME_COUNT,
    DO_DECREASE_FLAME_COUNT,
    DATA_GET_HARD_MODE,
    DATA_FLAME_COUNT
};

enum Npcs
{
    NPC_ROCKET                                  = 34050,
    NPC_BURST_TARGET                            = 34211,
    NPC_JUNK_BOT                                = 33855,
    NPC_ASSAULT_BOT                             = 34057,
    NPC_BOOM_BOT                                = 33836,
    NPC_EMERGENCY_BOT                           = 34147,
    NPC_FLAMES_INITIAL                          = 34363,
    NPC_FLAME_SPREAD                            = 34121,
    NPC_FROST_BOMB                              = 34149,
    NPC_MKII_TURRET                             = 34071,
    NPC_PROXIMITY_MINE                          = 34362
};

// Achievements
#define ACHIEVEMENT_SET_UP_US_THE_BOMB          RAID_MODE(2989, 3237)
// TODO:
// Achiev 2989: Criterias 10543, 10544, 10545
// Achiev 3237: Criterias 10546, 10547, 10548
// Which criteria is reponsible for which step ?
// 26491 - 38080 - 45033 <- what do these numbers mean ?

enum BombIndices // Scripts are present, but criteria-id <-> script is missing
{
    DATA_AVOIDED_PROXIMITY_MINES     = 30,
    DATA_AVOIDED_ROCKET_STRIKES,
    DATA_AVOIDED_BOOM_BOT_EXPLOSION
};

enum MimironChests
{
    CACHE_OF_INNOVATION_10                      = 194789,
    CACHE_OF_INNOVATION_HARDMODE_10             = 194957,
    CACHE_OF_INNOVATION_25                      = 194956,
    CACHE_OF_INNOVATION_HARDMODE_25             = 194958
};

Position const SummonPos[9] =
{
    {2703.93f, 2569.32f, 364.397f, 0},
    {2715.33f, 2569.23f, 364.397f, 0},
    {2726.85f, 2569.28f, 364.397f, 0},
    {2765.24f, 2534.38f, 364.397f, 0},
    {2759.54f, 2544.30f, 364.397f, 0},
    {2753.82f, 2554.22f, 364.397f, 0},
    {2764.95f, 2604.11f, 364.397f, 0},
    {2759.19f, 2594.26f, 364.397f, 0},
    {2753.56f, 2584.30f, 364.397f, 0}
};

#define FLAME_CAP 200 // find a good (blizzlike!) value

/************************************************************************/
/*                              Mimiron                                 */
/************************************************************************/

class boss_mimiron : public CreatureScript
{
    private:
        enum MyEvents
        {
            // General events
            EVENT_CHECK_TARGET = 1,
            EVENT_ENRAGE,
            EVENT_FLAME,

            // Event - stepping for phases
            EVENT_STEP_1,
            EVENT_STEP_2,
            EVENT_STEP_3,
            EVENT_STEP_4,
            EVENT_STEP_5,
            EVENT_STEP_6,
            EVENT_STEP_7,
            EVENT_STEP_8,
            EVENT_STEP_9,

            // Events for bot-alive-checks
            EVENT_CHECK_BOTALIVE,

            // HardMode
            EVENT_9MINS,
            EVENT_8MINS,
            EVENT_7MINS,
            EVENT_6MINS,
            EVENT_5MINS,
            EVENT_4MINS,
            EVENT_3MINS,
            EVENT_2MINS,
            EVENT_1MINS,
            EVENT_TIMEUP,
            EVENT_CANCEL,
        };
        enum
        {
            TIMER_BOT_ALIVE_CHECK = 250,
        };
        enum MyPhase
        {
            PHASE_IDLE = 0,
            PHASE_INTRO,
            PHASE_COMBAT,
            PHASE_VX001_ACTIVATION,
            PHASE_AERIAL_ACTIVATION,
            PHASE_V0L7R0N_ACTIVATION,
        };
    public:
        boss_mimiron() : CreatureScript("boss_mimiron") {}

        struct boss_mimironAI : public BossAI
        {
            boss_mimironAI(Creature* creature) : BossAI(creature, BOSS_MIMIRON)
            {
                me->ApplySpellImmune(0, IMMUNITY_ID, SPELL_ROCKET_STRIKE_DMG, true);
                gotEncounterFinished = false;
            }

            void DespawnCreatures(uint32 entry, float distance)
            {
                std::list<Creature*> creatures;
                GetCreatureListWithEntryInGrid(creatures, me, entry, distance);

                if (creatures.empty())
                    return;

                for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                    (*iter)->DespawnOrUnsummon();
            }

            void Reset()
            {
                me->SetReactState(REACT_PASSIVE);
                gotEncounterFinished = gotEncounterFinished || (instance->GetBossState(BOSS_MIMIRON) == DONE);
                if (gotEncounterFinished)
                    return;

                _Reset();
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1);
                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_USE_STANDING);
                me->SetVisible(true);
                me->ExitVehicle();
                //                me->GetMotionMaster()->MoveTargetedHome();

                instance->SetData(DATA_MIMIRON_ELEVATOR, GO_STATE_ACTIVE);

                phase = PHASE_IDLE;
                events.SetPhase(phase);
                flameCount = 0;
                _botTimer = 0;
                gotHardMode = false;
                enraged = false;
                _checkBotAlive = true;


                DespawnCreatures(NPC_FLAMES_INITIAL, 100.0f);
                DespawnCreatures(NPC_PROXIMITY_MINE, 100.0f);
                DespawnCreatures(NPC_ROCKET, 100.0f);
                DespawnCreatures(NPC_JUNK_BOT, 100.0f);
                DespawnCreatures(NPC_ASSAULT_BOT, 100.0f);
                DespawnCreatures(NPC_BOOM_BOT, 100.0f);
                DespawnCreatures(NPC_EMERGENCY_BOT, 100.0f);

                for (uint8 data = DATA_LEVIATHAN_MK_II; data <= DATA_AERIAL_UNIT; ++data)
                    if (Creature* creature = ObjectAccessor::GetCreature(*me, instance->GetData64(data)))
                        if (creature->isAlive())
                        {
                            creature->ExitVehicle();
                            creature->AI()->EnterEvadeMode();
                        }

                if (GameObject* go = me->FindNearestGameObject(GO_BIG_RED_BUTTON, 200))
                {
                    go->SetGoState(GO_STATE_READY);
                    go->SetLootState(GO_JUST_DEACTIVATED);
                    go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                }

                isSelfRepairing[DATA_LEVIATHAN_MK_II] = false;
                isSelfRepairing[DATA_VX_001] = false;
                isSelfRepairing[DATA_AERIAL_UNIT] = false;

                setUpUsTheBomb[DATA_AVOIDED_PROXIMITY_MINES] = true;
                setUpUsTheBomb[DATA_AVOIDED_ROCKET_STRIKES] = true;
                setUpUsTheBomb[DATA_AVOIDED_BOOM_BOT_EXPLOSION] = true;
            }

            void EncounterPostProgress()
            {
                //if (gotEncounterFinished)
                //    return;

                Talk(SAY_V07TRON_DEATH);

                me->SetReactState(REACT_PASSIVE);
                me->RemoveAllAuras();
                me->AttackStop();
                me->setFaction(35);
                me->DeleteThreatList();
                me->CombatStop(true);
                if (instance)
                {
                    instance->SetBossState(BOSS_MIMIRON, DONE);
                     if (gotHardMode)
                        me->SummonGameObject(RAID_MODE(CACHE_OF_INNOVATION_HARDMODE_10, CACHE_OF_INNOVATION_HARDMODE_25), 2744.65f, 2569.46f,
                        364.314f, 3.14159f, 0, 0, 0.7f, 0.7f, 604800);
                    else
                        me->SummonGameObject(RAID_MODE(CACHE_OF_INNOVATION_10, CACHE_OF_INNOVATION_25), 2744.65f, 2569.46f, 364.314f, 3.14159f,
                        0, 0, 0.7f, 0.7f, 604800);

                    instance->DoUpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, NPC_LEVIATHAN_MKII, 1);
                }
                _JustDied();
                me->DespawnOrUnsummon(5000);
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1);
                phase = PHASE_INTRO;
                events.SetPhase(phase);
                if (gotHardMode)
                    events.ScheduleEvent(EVENT_FLAME, 5000);
                events.ScheduleEvent(EVENT_CHECK_TARGET, 7000);
                events.ScheduleEvent(EVENT_ENRAGE, gotHardMode ? 10*60*1000 : 15*60*1000); // Enrage in 10 (hard mode) or 15 min
                events.ScheduleEvent(EVENT_STEP_1, 100, 0, phase);

                if (GameObject* go = me->FindNearestGameObject(GO_BIG_RED_BUTTON, 200))
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                while (uint32 event = events.ExecuteEvent())
                {
                    if(phase == PHASE_AERIAL_ACTIVATION)
                        if (Creature* VX_001 = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_VX_001)))
                            VX_001->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_USE_STANDING);

                    switch (event)
                    {
                        case EVENT_CHECK_TARGET:
                            // prevent mimiron staying infight with leviathan introduced in rev #b40bf69
                            // TODO: find out why this happens
                            if (!SelectTarget(SELECT_TARGET_RANDOM, 0, 200.0f, true))
                            {
                                me->ExitVehicle();
                                EnterEvadeMode();
                                return;
                            }
                            events.ScheduleEvent(EVENT_CHECK_TARGET, 7000);
                            break;
                        case EVENT_CHECK_BOTALIVE:  // Check if all are dead
                            if (isSelfRepairing[DATA_LEVIATHAN_MK_II] && isSelfRepairing[DATA_AERIAL_UNIT] && isSelfRepairing[DATA_VX_001])
                            {
                                // We're down, baby.
                                Creature* Leviathan = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_LEVIATHAN_MK_II));
                                Creature* VX_001 = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_VX_001));
                                Creature* AerialUnit = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_AERIAL_UNIT));
                                if (Leviathan && VX_001 && AerialUnit)
                                {
                                    Leviathan->DisappearAndDie();
                                    VX_001->DisappearAndDie();
                                    AerialUnit->DisappearAndDie();
                                    DespawnCreatures(NPC_FLAMES_INITIAL, 100.0f);
                                    DespawnCreatures(NPC_PROXIMITY_MINE, 100.0f);
                                    DespawnCreatures(NPC_ROCKET, 100);
                                    me->ExitVehicle();
                                    EncounterPostProgress();
                                }
                            }
                            else
                                events.ScheduleEvent(EVENT_CHECK_BOTALIVE, 500);
                            break;
                        case EVENT_ENRAGE:
                            Talk(SAY_BERSERK);
                            for (uint8 data = DATA_LEVIATHAN_MK_II; data <= DATA_AERIAL_UNIT; ++data)
                                if (Creature* creature = ObjectAccessor::GetCreature(*me, instance->GetData64(data)))
                                    creature->AI()->DoAction(DO_ENTER_ENRAGE);
                            enraged = true;
                            if (gotHardMode)
                            {
                                DoCast(me, SPELL_SELF_DESTRUCTION, true);
                                DoCast(me, SPELL_SELF_DESTRUCTION_VISUAL, true);
                            }
                            break;
                        case EVENT_FLAME:
                            for (uint8 i = 0; i < 3; ++i)
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                                    DoCast(target, SPELL_SUMMON_FLAMES_INITIAL, true);
                            events.ScheduleEvent(EVENT_FLAME, 30000);
                            break;
                        case EVENT_STEP_1:
                        {
                            switch (phase)
                            {
                                case PHASE_INTRO:
                                    if(gotHardMode)
                                    {
                                        me->SetName("Computer");
                                        me->MonsterYell(SAY_ALARM_HARD_MODE, LANG_UNIVERSAL, 0);
                                        me->PlayDirectSound(15413);
                                    }
                                    events.ScheduleEvent(EVENT_STEP_2, 10*IN_MILLISECONDS, 0, PHASE_INTRO);
                                    break;
                                case PHASE_VX001_ACTIVATION:
                                    Talk(SAY_MKII_DEATH);
                                    events.ScheduleEvent(EVENT_STEP_2, 10*IN_MILLISECONDS, 0, PHASE_VX001_ACTIVATION);
                                    break;
                                case PHASE_AERIAL_ACTIVATION:
                                    me->ChangeSeat(4);
                                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STAND);
                                    events.ScheduleEvent(EVENT_STEP_2, 2500, 0, PHASE_AERIAL_ACTIVATION);
                                    break;
                                case PHASE_V0L7R0N_ACTIVATION:
                                    if (instance)
                                    {
                                        //me->SetVisible(true);
                                        if (Creature* Leviathan = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_LEVIATHAN_MK_II)))
                                            Leviathan->GetMotionMaster()->MovePoint(0, 2744.65f, 2569.46f, 364.397f);
                                        if (Creature* VX_001 = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_VX_001)))
                                        {
                                            me->EnterVehicle(VX_001, 1);
                                            me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STAND);
                                            Talk(SAY_AERIAL_DEATH);
                                        }
                                    }
                                    events.ScheduleEvent(EVENT_STEP_2, 5*IN_MILLISECONDS, 0, PHASE_V0L7R0N_ACTIVATION);
                                    break;
                                default:
                                    break;
                            }
                            break;
                        }
                        case EVENT_STEP_2:
                        {
                            switch (phase)
                            {
                                case PHASE_INTRO:
                                    if (instance)
                                    {
                                        me->SetName("Mimiron");
                                        Talk(gotHardMode ? SAY_HARDMODE_ON : SAY_AGGRO);
                                    }
                                    events.ScheduleEvent(EVENT_STEP_3, 2*IN_MILLISECONDS, 0, PHASE_INTRO);
                                    break;
                                case PHASE_VX001_ACTIVATION:
                                    me->ChangeSeat(1);
                                    events.ScheduleEvent(EVENT_STEP_3, 2*IN_MILLISECONDS, 0, PHASE_VX001_ACTIVATION);
                                    break;
                                case PHASE_AERIAL_ACTIVATION:
                                    Talk(SAY_VX001_DEATH);
                                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_TALK);
                                    events.ScheduleEvent(EVENT_STEP_3, 5*IN_MILLISECONDS, 0, PHASE_AERIAL_ACTIVATION);
                                    break;
                                case PHASE_V0L7R0N_ACTIVATION:
                                    if (instance)
                                        if (Creature* VX_001 = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_VX_001)))
                                            if (Creature* Leviathan = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_LEVIATHAN_MK_II)))
                                            {
                                                VX_001->SetStandState(UNIT_STAND_STATE_STAND);
                                                VX_001->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_CUSTOM_SPELL_01);
                                                VX_001->EnterVehicle(Leviathan, 7);
                                            }
                                    events.ScheduleEvent(EVENT_STEP_3, 2*IN_MILLISECONDS, 0, PHASE_V0L7R0N_ACTIVATION);
                                    break;
                                default:
                                    break;
                            }
                            break;
                        }
                        case EVENT_STEP_3:
                        {
                            switch (phase)
                            {
                                case PHASE_INTRO:
                                    if(instance)
                                    {
                                        if (Creature* Leviathan = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_LEVIATHAN_MK_II)))
                                            me->EnterVehicle(Leviathan, 4);
                                    }
                                    me->SetName("Computer");
                                    events.ScheduleEvent(EVENT_STEP_4, 2*IN_MILLISECONDS, 0, PHASE_INTRO);
                                    break;
                                case PHASE_VX001_ACTIVATION:
                                    if (instance)
                                        instance->SetData(DATA_MIMIRON_ELEVATOR, GO_STATE_READY);
                                    events.ScheduleEvent(EVENT_STEP_4, 5*IN_MILLISECONDS, 0, PHASE_VX001_ACTIVATION);
                                    break;
                                case PHASE_AERIAL_ACTIVATION:
                                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STAND);
                                    if (instance)
                                    {
                                        //                                    if (Creature* AerialUnit = me->SummonCreature(NPC_AERIAL_COMMAND_UNIT, 2744.65f, 2569.46f, 380, 3.14159f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000))
                                        if (Creature* AerialUnit =  ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_AERIAL_UNIT)))
                                        {
                                            AerialUnit->SetVisible(true);
                                            AerialUnit->SetReactState(REACT_AGGRESSIVE);
                                        }
                                    }
                                    events.ScheduleEvent(EVENT_STEP_4, 5*IN_MILLISECONDS, 0, PHASE_AERIAL_ACTIVATION);
                                    break;
                                case PHASE_V0L7R0N_ACTIVATION:
                                    if (instance)
                                        if (Creature* VX_001 = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_VX_001)))
                                            if (Creature* AerialUnit = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_AERIAL_UNIT)))
                                            {
                                                AerialUnit->SetCanFly(false);
                                                AerialUnit->EnterVehicle(VX_001, 3);
                                                Talk(SAY_V07TRON_ACTIVATE);
                                            }
                                    events.ScheduleEvent(EVENT_STEP_4, 10*IN_MILLISECONDS, 0, PHASE_V0L7R0N_ACTIVATION);
                                    break;
                                default:
                                    break;
                            }
                            break;
                        }
                        case EVENT_STEP_4:
                        {
                            switch (phase)
                            {
                                case PHASE_INTRO:
                                    me->ChangeSeat(2);
                                    if(gotHardMode)
                                    {
                                        me->PlayDirectSound(15415);
                                        me->MonsterYell(SAY_TIME_1, LANG_UNIVERSAL, 0);
                                    }
                                    events.ScheduleEvent(EVENT_STEP_5, 2900, 0, PHASE_INTRO);
                                    break;
                                case PHASE_VX001_ACTIVATION:
                                    if (instance)
                                    {
                                        //                                    if (Creature* VX_001 = me->SummonCreature(NPC_VX_001, 2744.65f, 2569.46f, 364.397f, 3.14159f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000))
                                        if (Creature* VX_001 =  ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_VX_001)))
                                        {
                                            instance->SetData(DATA_MIMIRON_ELEVATOR, GO_STATE_ACTIVE_ALTERNATIVE);
                                            VX_001->SetVisible(true);
                                            VX_001->SetReactState(REACT_AGGRESSIVE);
                                            for (uint8 n = 5; n < 7; ++n)
                                            {
                                                if (Creature* Rocket = me->SummonCreature(NPC_ROCKET, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_MANUAL_DESPAWN))
                                                {
                                                    Rocket->setFaction(14);
                                                    Rocket->SetReactState(REACT_PASSIVE);
                                                    Rocket->EnterVehicle(VX_001, n);
                                                }
                                            }
                                        }
                                    }
                                    events.ScheduleEvent(EVENT_STEP_5, 8*IN_MILLISECONDS, 0, PHASE_VX001_ACTIVATION);
                                    break;
                                case PHASE_AERIAL_ACTIVATION:
                                    me->ExitVehicle();
                                    //me->GetMotionMaster()->MoveJump(2745.06f, 2569.36f, 379.90f, 10, 15);
                                    if (Creature* AerialUnit = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_AERIAL_UNIT)))
                                        me->EnterVehicle(AerialUnit, 0);
                                    events.ScheduleEvent(EVENT_STEP_5, 2*IN_MILLISECONDS, 0, PHASE_AERIAL_ACTIVATION);
                                    break;
                                case PHASE_V0L7R0N_ACTIVATION:
                                    if (instance)
                                    {
                                        if (Creature* Leviathan = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_LEVIATHAN_MK_II)))
                                            Leviathan->AI()->DoAction(DO_LEVIATHAN_ASSEMBLED);
                                        if (Creature* VX_001 = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_VX_001)))
                                            VX_001->AI()->DoAction(DO_VX001_ASSEMBLED);
                                        if (Creature* AerialUnit = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_AERIAL_UNIT)))
                                            AerialUnit->AI()->DoAction(DO_AERIAL_ASSEMBLED);
                                        phase = PHASE_COMBAT;
                                        events.SetPhase(phase);
                                    }
                                    break;
                                default:
                                    break;
                            }
                            break;
                        }
                        case EVENT_STEP_5:
                        {
                            switch (phase) // TODO: Add other phases if required
                            {
                                case PHASE_INTRO:
                                    me->SetName("Mimiron");
                                    me->ChangeSeat(5);
                                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STAND);
                                    events.ScheduleEvent(EVENT_STEP_6, 3*IN_MILLISECONDS, 0, PHASE_INTRO);
                                    break;
                                case PHASE_VX001_ACTIVATION:
                                    if (instance)
                                    {
                                        if (Creature* VX_001 = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_VX_001)))
                                            me->EnterVehicle(VX_001, 0);
                                    }
                                    events.ScheduleEvent(EVENT_STEP_6, 3500, 0, PHASE_VX001_ACTIVATION);
                                    break;
                                case PHASE_AERIAL_ACTIVATION:
                                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_TALK);
                                    Talk(SAY_AERIAL_ACTIVATE);
                                    events.ScheduleEvent(EVENT_STEP_6, 8*IN_MILLISECONDS, 0, PHASE_AERIAL_ACTIVATION);
                                    break;
                                default:
                                    break;
                            }
                            break;
                        }
                        case EVENT_STEP_6:
                        {
                            switch (phase) // TODO: Add other phases if required
                            {
                                case PHASE_INTRO:
                                    Talk(SAY_MKII_ACTIVATE);
                                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_TALK);
                                    events.ScheduleEvent(EVENT_STEP_7, 6*IN_MILLISECONDS, 0, PHASE_INTRO);
                                    break;
                                case PHASE_VX001_ACTIVATION:
                                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_TALK);
                                    Talk(SAY_VX001_ACTIVATE);
                                    events.ScheduleEvent(EVENT_STEP_7, 10*IN_MILLISECONDS, 0, PHASE_VX001_ACTIVATION);
                                    break;
                                case PHASE_AERIAL_ACTIVATION:
                                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STAND);
                                    //me->SetVisible(false);
                                    if (instance)
                                    {
                                        if (Creature* AerialUnit = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_AERIAL_UNIT)))
                                        {
                                            AerialUnit->AI()->DoAction(DO_START_AERIAL);
                                            phase = PHASE_COMBAT;
                                            events.SetPhase(phase);
                                        }
                                    }
                                    break;
                                default:
                                    break;
                            }
                            break;
                        }
                        case EVENT_STEP_7:
                        {
                            switch (phase) // TODO: Add other phases if required
                            {
                                case PHASE_INTRO:
                                    me->ChangeSeat(6);
                                    events.ScheduleEvent(EVENT_STEP_8, 2*IN_MILLISECONDS, 0, PHASE_INTRO);
                                    break;
                                case PHASE_VX001_ACTIVATION:
                                    me->ChangeSeat(1);
                                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_SIT);
                                    events.ScheduleEvent(EVENT_STEP_8, 2*IN_MILLISECONDS, 0, PHASE_VX001_ACTIVATION);
                                    break;
                                default:
                                    break;
                            }
                            break;
                        }
                        case EVENT_STEP_8:
                        {
                            switch (phase) // TODO: Add other phases if required
                            {
                                case PHASE_INTRO:
                                    if (instance)
                                    {
                                        if (Creature* Leviathan = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_LEVIATHAN_MK_II)))
                                        {
                                            me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STAND);
                                            Leviathan->AI()->DoAction(DO_START_ENCOUNTER);
                                            phase = PHASE_COMBAT;
                                            me->SetName("Computer");
                                            events.ScheduleEvent(EVENT_9MINS, 60000);
                                            events.ScheduleEvent(EVENT_8MINS, 120000);
                                            events.ScheduleEvent(EVENT_7MINS, 180000);
                                            events.ScheduleEvent(EVENT_6MINS, 240000);
                                            events.ScheduleEvent(EVENT_5MINS, 300000);
                                            events.ScheduleEvent(EVENT_4MINS, 360000);
                                            events.ScheduleEvent(EVENT_3MINS, 420000);
                                            events.ScheduleEvent(EVENT_2MINS, 480000);
                                            events.ScheduleEvent(EVENT_1MINS, 540000);
                                            events.ScheduleEvent(EVENT_TIMEUP, 600000);
                                            events.SetPhase(phase);
                                        }
                                    }
                                    break;
                                case PHASE_VX001_ACTIVATION:
                                    if (instance)
                                    {
                                        if (Creature* VX_001 = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_VX_001)))
                                            VX_001->HandleEmoteCommand(EMOTE_ONESHOT_EMERGE);
                                    }
                                    events.ScheduleEvent(EVENT_STEP_9, 3500, 0, PHASE_VX001_ACTIVATION);
                                    break;
                                default:
                                    break;
                            }
                            break;
                        }
                        case EVENT_STEP_9:
                        {
                            switch (phase) // TODO: Add other phases if required
                            {
                                case PHASE_VX001_ACTIVATION:
                                    if (instance)
                                        if (Creature* VX_001 = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_VX_001)))
                                        {
                                            VX_001->AddAura(SPELL_HOVER, VX_001); // Hover
                                            VX_001->AI()->DoAction(DO_START_VX001);
                                            phase = PHASE_COMBAT;
                                            events.SetPhase(phase);
                                        }
                                    break;
                                default:
                                    break;
                            }
                            break;
                        }
                    }
                }

                if (gotHardMode)
                {
                    while (uint32 eventId = events.ExecuteEvent())
                    {
                        switch (eventId)
                        {
                            case EVENT_9MINS:
                                me->SetName("Computer");
                                me->PlayDirectSound(15416);
                                me->MonsterYell(SAY_TIME_2, LANG_UNIVERSAL, 0);
    //                            events.ScheduleEvent(EVENT_8MINS, 60000);
                                break;
                            case EVENT_8MINS:
                                me->SetName("Computer");
                                me->PlayDirectSound(15417);
                                me->MonsterYell(SAY_TIME_3, LANG_UNIVERSAL, 0);
    //                            events.ScheduleEvent(EVENT_7MINS, 60000);
                                break;
                            case EVENT_7MINS:
                                me->SetName("Computer");
                                me->PlayDirectSound(15418);
                                me->MonsterYell(SAY_TIME_4, LANG_UNIVERSAL, 0);
    //                            events.ScheduleEvent(EVENT_6MINS, 60000);
                                break;
                            case EVENT_6MINS:
                                me->SetName("Computer");
                                me->PlayDirectSound(15419);
                                me->MonsterYell(SAY_TIME_5, LANG_UNIVERSAL, 0);
    //                            events.ScheduleEvent(EVENT_5MINS, 60000);
                                break;
                            case EVENT_5MINS:
                                me->SetName("Computer");
                                me->PlayDirectSound(15420);
                                me->MonsterYell(SAY_TIME_6, LANG_UNIVERSAL, 0);
    //                            events.ScheduleEvent(EVENT_4MINS, 60000);
                                break;
                            case EVENT_4MINS:
                                me->SetName("Computer");
                                me->PlayDirectSound(15421);
                                me->MonsterYell(SAY_TIME_7, LANG_UNIVERSAL, 0);
    //                            events.ScheduleEvent(EVENT_3MINS, 60000);
                                break;
                            case EVENT_3MINS:
                                me->SetName("Computer");
                                me->PlayDirectSound(15422);
                                me->MonsterYell(SAY_TIME_8, LANG_UNIVERSAL, 0);
    //                            events.ScheduleEvent(EVENT_2MINS, 60000);
                                break;
                            case EVENT_2MINS:
                                me->SetName("Computer");
                                me->PlayDirectSound(15423);
                                me->MonsterYell(SAY_TIME_9, LANG_UNIVERSAL, 0);
    //                            events.ScheduleEvent(EVENT_1MINS, 60000);
                                break;
                            case EVENT_1MINS:
                                me->SetName("Computer");
                                me->PlayDirectSound(15424);
                                me->MonsterYell(SAY_TIME_10, LANG_UNIVERSAL, 0);
    //                            events.ScheduleEvent(EVENT_TIMEUP, 60000);
                                break;
                            case EVENT_TIMEUP:
                                me->SetName("Computer");
                                me->PlayDirectSound(15425);
                                me->MonsterYell(SAY_TIME_UP, LANG_UNIVERSAL, 0);
                                break;
                        }
                    }
                }

                // All sections need to die within 15 seconds, else they respawn
                if (_checkBotAlive)
                    _botTimer = 0;
                else
                {
                    _botTimer += diff;
                    if (_botTimer > 15000) // spell 64383
                    {
                        if (Creature* Leviathan = me->GetCreature(*me, instance->GetData64(DATA_LEVIATHAN_MK_II)))
                            Leviathan->AI()->DoAction(DO_LEVIATHAN_ASSEMBLED);
                        if (Creature* VX_001 = me->GetCreature(*me, instance->GetData64(DATA_VX_001)))
                            VX_001->AI()->DoAction(DO_VX001_ASSEMBLED);
                        if (Creature* AerialUnit = me->GetCreature(*me, instance->GetData64(DATA_AERIAL_UNIT)))
                            AerialUnit->AI()->DoAction(DO_AERIAL_ASSEMBLED);

                        _checkBotAlive = true;
                    }
                    else
                    {
                        Creature* Leviathan = me->GetCreature(*me, instance->GetData64(DATA_LEVIATHAN_MK_II));
                        Creature* VX_001 = me->GetCreature(*me, instance->GetData64(DATA_VX_001));
                        Creature* AerialUnit = me->GetCreature(*me, instance->GetData64(DATA_AERIAL_UNIT));
                        if (Leviathan && VX_001 && AerialUnit)
                        {
                            if (Leviathan->getStandState() == UNIT_STAND_STATE_DEAD &&
                                VX_001->getStandState() == UNIT_STAND_STATE_DEAD &&
                                AerialUnit->getStandState() == UNIT_STAND_STATE_DEAD)
                            {
                                Leviathan->DespawnOrUnsummon(5*MINUTE*IN_MILLISECONDS);
                                VX_001->DisappearAndDie();
                                AerialUnit->DisappearAndDie();
                                me->Kill(Leviathan);
                                DespawnCreatures(NPC_FLAMES_INITIAL, 100.0f);
                                DespawnCreatures(NPC_PROXIMITY_MINE, 100.0f);
                                DespawnCreatures(NPC_ROCKET, 100);
                                me->ExitVehicle();
                                EncounterPostProgress();
                                _checkBotAlive = true;
                            }
                        }
                    }
                }

            }

            uint32 GetData(uint32 type) const
            {
                switch (type)
                {
                    case DATA_GET_HARD_MODE:
                        return gotHardMode ? 1 : 0;
                    case DATA_FLAME_COUNT:
                        return flameCount;
                        /*  case DATA_AVOIDED_ROCKET_STRIKES:
                        return setUpUsTheBomb[DATA_AVOIDED_ROCKET_STRIKES];
                    case DATA_AVOIDED_PROXIMITY_MINES:
                        return setUpUsTheBomb[DATA_AVOIDED_PROXIMITY_MINES];
                    case DATA_AVOIDED_BOOM_BOT_EXPLOSION:
                    return setUpUsTheBomb[DATA_AVOIDED_BOOM_BOT_EXPLOSION];*/
                    default:
                        return 0;
                }
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case DO_ACTIVATE_VX001:
                        phase = PHASE_VX001_ACTIVATION;
                        events.SetPhase(phase);
                        events.ScheduleEvent(EVENT_STEP_1, 100, 0, phase);
                        break;
                    case DO_ACTIVATE_AERIAL:
                        phase = PHASE_AERIAL_ACTIVATION;
                        events.SetPhase(phase);
                        events.ScheduleEvent(EVENT_STEP_1, 5000, 0, phase);
                        break;
                    case DO_ACTIVATE_V0L7R0N:
                        me->SetVisible(true);
                        phase = PHASE_V0L7R0N_ACTIVATION;
                        events.SetPhase(phase);
                        events.ScheduleEvent(EVENT_STEP_1, 1000, 0, phase);
                        break;
                    case DO_ACTIVATE_DEATH_TIMER:
                        _checkBotAlive = false;
                        break;
                    case DO_ACTIVATE_HARD_MODE:     // Cannot be done infight, since the button gets locked on EnterCombat() with Mimiron.
                        gotHardMode = true;
                        DoZoneInCombat();
                        break;
                    case DO_INCREASE_FLAME_COUNT:
                        ++flameCount;
                        break;
                    case DO_DECREASE_FLAME_COUNT:
                        if (flameCount)
                            --flameCount;
                        break;
                    // Repair stuff
                    case DO_LEVIATHAN_SELF_REPAIR_START:
                        isSelfRepairing[DATA_LEVIATHAN_MK_II] = true;
                        events.RescheduleEvent(EVENT_CHECK_BOTALIVE, TIMER_BOT_ALIVE_CHECK);
                    case DO_LEVIATHAN_SELF_REPAIR_END:
                        isSelfRepairing[DATA_LEVIATHAN_MK_II] = false;
                        events.CancelEvent(EVENT_CHECK_BOTALIVE);   // Not needed if at least one is repaired
                    case DO_VX001_SELF_REPAIR_START:
                        isSelfRepairing[DATA_VX_001] = true;
                        events.RescheduleEvent(EVENT_CHECK_BOTALIVE, TIMER_BOT_ALIVE_CHECK);
                    case DO_VX001_SELF_REPAIR_END:
                        isSelfRepairing[DATA_VX_001] = false;
                        events.CancelEvent(EVENT_CHECK_BOTALIVE);   // Not needed if at least one is repaired
                    case DO_AERIAL_SELF_REPAIR_START:
                        isSelfRepairing[DATA_AERIAL_UNIT] = true;
                        events.RescheduleEvent(EVENT_CHECK_BOTALIVE, TIMER_BOT_ALIVE_CHECK);
                    case DO_AERIAL_SELF_REPAIR_END:
                        isSelfRepairing[DATA_AERIAL_UNIT] = false;
                        events.CancelEvent(EVENT_CHECK_BOTALIVE);   // Not needed if at least one is repaired
                        break;
                    // Achiev
                    case DATA_AVOIDED_ROCKET_STRIKES:
                        setUpUsTheBomb[DATA_AVOIDED_ROCKET_STRIKES] = false;
                        break;
                    case DATA_AVOIDED_PROXIMITY_MINES:
                        setUpUsTheBomb[DATA_AVOIDED_PROXIMITY_MINES] = false;
                        break;
                    case DATA_AVOIDED_BOOM_BOT_EXPLOSION:
                        setUpUsTheBomb[DATA_AVOIDED_BOOM_BOT_EXPLOSION] = false;
                        break;
                }
            }

            private:
                std::map< uint32, bool > isSelfRepairing;
                std::map< BombIndices, bool > setUpUsTheBomb;
                MyPhase phase;
                uint32 flameCount;
                uint32 _botTimer;
                bool gotHardMode;
                bool enraged;
                bool gotEncounterFinished;
                bool _checkBotAlive;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<boss_mimironAI>(creature);
        }
};

/************************************************************************/
/*                          Leviathan MK II                             */
/************************************************************************/

#define EMOTE_LEVIATHAN                         "Leviathan MK II begins to cast Plasma Blast!"

class boss_leviathan_mk : public CreatureScript
{
    private:
        enum MyPhase
        {
            PHASE_IDLE                              = 0,
            PHASE_LEVIATHAN_SOLO__GLOBAL_1,
            PHASE_LEVIATHAN_ASSEMBLED__GLOBAL_4,
        };
        enum Events
        {
            EVENT_PROXIMITY_MINE                    = 1,
            EVENT_NAPALM_SHELL,
            EVENT_PLASMA_BLAST,
            EVENT_SHOCK_BLAST,
            EVENT_FLAME_SUPPRESSANT,
        };
    public:
        boss_leviathan_mk() : CreatureScript("boss_leviathan_mk") {}

        struct boss_leviathan_mkAI : public BossAI
        {
            boss_leviathan_mkAI(Creature* creature) : BossAI(creature, BOSS_MIMIRON) {}

            void InitializeAI()
            {
                me->ApplySpellImmune(0, IMMUNITY_ID, SPELL_ROCKET_STRIKE_DMG, true);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetReactState(REACT_PASSIVE);
                if (Creature* turret = me->FindNearestCreature(34071, 100))
                {
                    turret->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1);
                    turret->SetReactState(REACT_PASSIVE);
                }
                Reset();
            }

            void Reset()
            {
                events.Reset();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1);
                me->SetStandState(UNIT_STAND_STATE_STAND);
                me->SetReactState(REACT_PASSIVE);
                me->RemoveAllAurasExceptType(SPELL_AURA_CONTROL_VEHICLE);
                phase = PHASE_IDLE;
                events.SetPhase(phase);
                gotMimironHardMode = false;
                me->GetVehicleKit()->RemoveAllPassengers();
                if (Creature* turret = me->FindNearestCreature(34071, 100))
                {
                    turret->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1);
                    turret->SetReactState(REACT_PASSIVE);
                }
            }

            void JustReachedHome()
            {
                me->SetReactState(REACT_PASSIVE);
                if (Creature* turret = me->FindNearestCreature(34071, 100))
                {
                    turret->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1);
                    turret->SetReactState(REACT_PASSIVE);
                }
            }

            void KilledUnit(Unit* /*who*/)
            {
                if (!(rand()%5))
                    if (Creature* Mimiron = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MIMIRON)))
                    {
                        if (phase == PHASE_LEVIATHAN_SOLO__GLOBAL_1)
                            Mimiron->AI()->Talk(SAY_MKII_SLAY);
                        else
                            Mimiron->AI()->Talk(SAY_V07TRON_SLAY);
                    }
            }

            void SpellHit(Unit* caster, SpellInfo const* spell)
            {
                if (!caster || !spell)
                    return;
                if (spell->Id == SPELL_SELF_REPAIR)
                    DoAction(DO_LEVIATHAN_SELF_REPAIR_END);
                if (Creature* Mimiron = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MIMIRON)))
                    Mimiron->AI()->DoAction(DO_LEVIATHAN_SELF_REPAIR_END);
            }

            void DamageTaken(Unit* /*who*/, uint32 &damage)
            {
                if (damage >= me->GetHealth())
                {
                    // Common stuff
                    damage = 0;
                    me->InterruptNonMeleeSpells(true);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1);
                    me->AttackStop();
                    me->SetReactState(REACT_PASSIVE);
                    me->RemoveAllAurasExceptType(SPELL_AURA_CONTROL_VEHICLE);
                    //me->SetHealth(me->GetMaxHealth());
                    switch (phase)
                    {
                        case PHASE_LEVIATHAN_SOLO__GLOBAL_1:
                            if (Creature* Mimiron = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MIMIRON)))
                                Mimiron->AI()->DoAction(DO_ACTIVATE_VX001);
                            if (Creature* turret = CAST_CRE(me->GetVehicleKit()->GetPassenger(3)))
                                turret->Kill(turret, false);
                            me->SetSpeed(MOVE_RUN, 1.5f, true);
                            me->GetMotionMaster()->MovePoint(0, 2790.11f, 2595.83f, 364.32f);
                            break;
                        case PHASE_LEVIATHAN_ASSEMBLED__GLOBAL_4:
                            me->SetStandState(UNIT_STAND_STATE_DEAD);
                            if (Creature* Mimiron = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MIMIRON)))
                                Mimiron->AI()->DoAction(DO_LEVIATHAN_SELF_REPAIR_START);
                            DoCast(me, SPELL_SELF_REPAIR);
                            if (Creature* Mimiron = me->GetCreature(*me, instance->GetData64(BOSS_MIMIRON)))
                                Mimiron->AI()->DoAction(DO_ACTIVATE_DEATH_TIMER);
                            break;
                        default:
                            break;
                    }
                    events.Reset(); // Wipe events, just for the case
                    phase = PHASE_IDLE;
                    events.SetPhase(phase);
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                if (Creature* Mimiron = ObjectAccessor::GetCreature(*me, instance ? instance->GetData64(BOSS_MIMIRON) : 0))
                    gotMimironHardMode = Mimiron->AI()->GetData(DATA_GET_HARD_MODE);

                if (Creature* turret = CAST_CRE(me->GetVehicleKit()->GetPassenger(3)))
                {
                    turret->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1);
                    turret->SetReactState(REACT_AGGRESSIVE);
                    turret->AI()->DoZoneInCombat();
                }
                events.ScheduleEvent(EVENT_SHOCK_BLAST, 30000, 0, PHASE_LEVIATHAN_SOLO__GLOBAL_1);
                events.ScheduleEvent(EVENT_PROXIMITY_MINE, 1000, 0, PHASE_LEVIATHAN_SOLO__GLOBAL_1);
                events.ScheduleEvent(EVENT_PLASMA_BLAST, 10000, 0, PHASE_LEVIATHAN_SOLO__GLOBAL_1);
                if (gotMimironHardMode)
                {
                    DoCast(me, SPELL_EMERGENCY_MODE, true);
                    events.ScheduleEvent(EVENT_FLAME_SUPPRESSANT, 60000, 0, PHASE_LEVIATHAN_SOLO__GLOBAL_1);
                }
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case DO_START_ENCOUNTER:
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_IMMUNE_TO_PC);
                        me->SetReactState(REACT_AGGRESSIVE);
                        phase = PHASE_LEVIATHAN_SOLO__GLOBAL_1;
                        events.SetPhase(phase);
                        DoZoneInCombat();
                        break;
                    case DO_LEVIATHAN_ASSEMBLED:                            // Assemble and self-repair share some stuff, so the fallthrough is intended!
                        if (gotMimironHardMode)
                            if (!me->HasAura(SPELL_EMERGENCY_MODE))
                                DoCast(me, SPELL_EMERGENCY_MODE, true);
                        me->SetHealth( (me->GetMaxHealth() >> 1) );
                        phase = PHASE_LEVIATHAN_ASSEMBLED__GLOBAL_4;
                        events.SetPhase(phase);
                    case DO_LEVIATHAN_SELF_REPAIR_END:
                        me->InterruptNonMeleeSpells(false);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1);
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->SetSpeed(MOVE_RUN, 1.0f, true);
                        me->SetStandState(UNIT_STAND_STATE_STAND);
                        events.RescheduleEvent(EVENT_PROXIMITY_MINE, 1000, 0, PHASE_LEVIATHAN_ASSEMBLED__GLOBAL_4);
                        events.RescheduleEvent(EVENT_SHOCK_BLAST, 30000, 0, PHASE_LEVIATHAN_ASSEMBLED__GLOBAL_4);
                        break;
                    case DO_ENTER_ENRAGE:
                        DoCast(me, SPELL_BERSERK, true);
                        break;
                    default:
                        break;
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || phase == PHASE_IDLE)
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING) || me->HasUnitState(UNIT_STATE_STUNNED))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_PROXIMITY_MINE:
                            for (uint8 i = 0; i < 10; i++)
                                DoCast(SPELL_MINES_SPAWN);
                            DoCast(SPELL_PROXIMITY_MINES);
                            events.RescheduleEvent(EVENT_PROXIMITY_MINE, 35000, phase);
                            break;
                        case EVENT_PLASMA_BLAST:
                            me->MonsterTextEmote(EMOTE_LEVIATHAN, 0, true);
                            DoCast(SPELL_PLASMA_BLAST);
                            events.RescheduleEvent(EVENT_PLASMA_BLAST, urand(30000, 35000), 0, phase);
                            events.RescheduleEvent(EVENT_SHOCK_BLAST, urand(6000, 10000), 0, phase);
                            return;
                        case EVENT_SHOCK_BLAST:
                            DoCastAOE(SPELL_SHOCK_BLAST);
                            events.RescheduleEvent(EVENT_SHOCK_BLAST, 35000, 0, phase);
                            return;
                        case EVENT_FLAME_SUPPRESSANT:
                            DoCastAOE(SPELL_FLAME_SUPPRESSANT_MK);
                            events.RescheduleEvent(EVENT_FLAME_SUPPRESSANT, 60000, 0, PHASE_LEVIATHAN_SOLO__GLOBAL_1);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

            private:
                MyPhase phase;
                bool gotMimironHardMode;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<boss_leviathan_mkAI>(creature);
        }
};

// Levi's turret
class boss_leviathan_mk_turret : public CreatureScript
{
    private:
        enum { SPELL_DEATH_GRIP = 49560 };
    public:
        boss_leviathan_mk_turret() : CreatureScript("boss_leviathan_mk_turret") {}

        struct boss_leviathan_mk_turretAI : public ScriptedAI
        {
            boss_leviathan_mk_turretAI(Creature* c) : ScriptedAI(c) {}

            void InitializeAI()
            {
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_ID, SPELL_DEATH_GRIP, true);
                instance = me->GetInstanceScript();
                Reset();
            }

            void Reset()
            {
                me->SetReactState(REACT_PASSIVE);
                napalmShellTimer = urand(8000, 12000);
            }

            // try to prefer ranged targets
            Unit* GetNapalmShellTarget()
            {
                if (Map* map = me->GetMap())
                {
                    std::list<Player*> playerList;
                    Map::PlayerList const& Players = map->GetPlayers();
                    for (Map::PlayerList::const_iterator itr = Players.begin(); itr != Players.end(); ++itr)
                    {
                        if (Player* player = itr->getSource())
                        {
                            if (player->isDead() || player->isGameMaster())
                                continue;

                            float Distance = player->GetDistance(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                            if (Distance < 15.0f || Distance > 100.0f)
                                continue;

                            playerList.push_back(player);
                        }
                    }
                    if (playerList.empty())
                        return SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true);
                    else
                        return Trinity::Containers::SelectRandomContainerElement(playerList);
                }
                else
                    return 0;
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;
                if (instance && instance->GetBossState(BOSS_MIMIRON) != IN_PROGRESS)
                    return;

                if (napalmShellTimer <= diff)
                {
                    if (Unit* shellTarget = GetNapalmShellTarget())
                        DoCast(shellTarget, SPELL_NAPALM_SHELL);
                    napalmShellTimer = urand(8000, 12000);
                }
                else
                    napalmShellTimer -= diff;
            }

        private:
            uint32 napalmShellTimer;
            InstanceScript* instance;

        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<boss_leviathan_mk_turretAI>(creature);
        }
};

// Npc that triggers the mine explosion
// Bomb will explode after 35 seconds or if a potential target is within half a meter
class npc_proximity_mine : public CreatureScript
{
    public:
        npc_proximity_mine() : CreatureScript("npc_proximity_mine") {}

        struct npc_proximity_mineAI : public Scripted_NoMovementAI
        {
            npc_proximity_mineAI(Creature* creature) : Scripted_NoMovementAI(creature) {}

            void InitializeAI()
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
                Reset();
            }

            void Reset()
            {
                uiBoomTimer = 35000;
                boomLocked = false;
                st = false;
                uist = 3000;
            }

            void SpellHitTarget(Unit* target, SpellInfo const* spell)
            {
                if (!target || !spell)
                    return;

                if (target->GetTypeId() == TYPEID_PLAYER)
                    if (spell->Id == SPELL_EXPLOSION)
                        if (InstanceScript* instance = me->GetInstanceScript())
                            if (Creature* mimiron = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MIMIRON)))
                                mimiron->AI()->DoAction(DATA_AVOIDED_PROXIMITY_MINES);
            }

            void MoveInLineOfSight(Unit* who)
            {
                if (!st)
                    return;
                if (!boomLocked)
                    if (who->ToPlayer())
                        if (me->IsWithinDistInMap(who, 0.5f) && !who->ToPlayer()->isGameMaster())
                        {
                            DoCastAOE(SPELL_EXPLOSION);
                            boomLocked = true;
                            me->DespawnOrUnsummon(200);
                        }
            }

            void DamageTaken(Unit* /*who*/, uint32 &damage)
            {
                damage = 0;
            }

            void UpdateAI(const uint32 diff)
            {
                if (uist <= diff)
                {
                    st = true;
                    uist = 35000;
                }
                else
                    uist -= diff;

                if (uiBoomTimer <= diff)
                {
                    if (!boomLocked)
                    {
                        DoCastAOE(SPELL_EXPLOSION);
                        me->DespawnOrUnsummon(200);
                        boomLocked = true;
                    }
                }
                else uiBoomTimer -= diff;
            }

            private:
            uint32 uiBoomTimer, uist;
            bool boomLocked, st;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_proximity_mineAI(creature);
        }
};

/************************************************************************/
/*                               VX-001                                 */
/************************************************************************/

class boss_vx_001 : public CreatureScript
{
    private:
        enum { SPELL_SELF_STUN = 14821 };
        enum MyPhase
        {
            PHASE_IDLE                          = 0,
            PHASE_VX001_SOLO__GLOBAL_2,
            PHASE_VX001_ASSEMBLED__GLOBAL_4,
        };
        enum Events
        {
            EVENT_RAPID_BURST                   = 1,
            EVENT_PRE_LASER_BARRAGE,
            EVENT_LASER_BARRAGE,
            EVENT_LASER_BARRAGE_END,
            EVENT_ROCKET_STRIKE,
            EVENT_HEAT_WAVE,
            EVENT_HAND_PULSE,
            EVENT_FROST_BOMB,
            EVENT_FLAME_SUPPRESSANT_VX001,
        };
    public:
        boss_vx_001() : CreatureScript("boss_vx_001") {}

        struct boss_vx_001AI : public BossAI
        {
            boss_vx_001AI(Creature* creature) : BossAI(creature, BOSS_MIMIRON) {}

            void InitializeAI()
            {
                me->ApplySpellImmune(0, IMMUNITY_ID, SPELL_ROCKET_STRIKE_DMG, true);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetReactState(REACT_PASSIVE);
                Reset();
            }

            void Reset()
            {
                events.Reset();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_DISABLE_MOVE);
                me->SetStandState(UNIT_STAND_STATE_STAND);
                me->SetVisible(false);
                me->RemoveAllAurasExceptType(SPELL_AURA_CONTROL_VEHICLE);
                phase = PHASE_IDLE;
                events.SetPhase(phase);
                MimironHardMode = false;
            }

            void KilledUnit(Unit* /*who*/)
            {
                if (!(rand()%5))
                    if (instance)
                        if (Creature* Mimiron = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MIMIRON)))
                        {
                            if (phase == PHASE_VX001_SOLO__GLOBAL_2)
                                Mimiron->AI()->Talk(SAY_VX001_SLAY);
                            else
                                Mimiron->AI()->Talk(SAY_V07TRON_SLAY);
                        }
            }

            void EnterCombat(Unit* /*who*/)
            {
                if (Creature* Mimiron = ObjectAccessor::GetCreature(*me, instance ? instance->GetData64(BOSS_MIMIRON) : 0))
                    MimironHardMode = Mimiron->AI()->GetData(DATA_GET_HARD_MODE);

                if (MimironHardMode)
                {
                    DoCast(me, SPELL_EMERGENCY_MODE, true);
                    events.ScheduleEvent(EVENT_FROST_BOMB, 15000, 0, PHASE_VX001_SOLO__GLOBAL_2);
                    events.ScheduleEvent(EVENT_FLAME_SUPPRESSANT_VX001, 1000, 0, PHASE_VX001_SOLO__GLOBAL_2);
                }

                events.ScheduleEvent(EVENT_RAPID_BURST, 2500, 0, PHASE_VX001_SOLO__GLOBAL_2);
                events.ScheduleEvent(EVENT_LASER_BARRAGE, urand(35000, 40000), 0, PHASE_VX001_SOLO__GLOBAL_2);
                events.ScheduleEvent(EVENT_ROCKET_STRIKE, 20000, 0, PHASE_VX001_SOLO__GLOBAL_2);
                events.ScheduleEvent(EVENT_HEAT_WAVE, urand(8000, 10000), 0, PHASE_VX001_SOLO__GLOBAL_2);
                me->SetReactState(REACT_PASSIVE);
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case DO_START_VX001:
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_IMMUNE_TO_PC);
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->SetVisible(true);
                        phase = PHASE_VX001_SOLO__GLOBAL_2;
                        events.SetPhase(phase);
                        DoZoneInCombat();
                        break;
                    case DO_VX001_ASSEMBLED:                                // Reassemble and heal share some stuff, fallthrough is intended
                        if (MimironHardMode)
                            DoCast(me, SPELL_EMERGENCY_MODE, true);
                        me->SetHealth( (me->GetMaxHealth() >> 1) );
                        phase = PHASE_VX001_ASSEMBLED__GLOBAL_4;
                        if (Creature* Mimiron = me->GetCreature(*me, instance->GetData64(BOSS_MIMIRON)))
                            Mimiron->AI()->DoAction(DO_ACTIVATE_DEATH_TIMER);
                        events.SetPhase(phase);
                    case DO_VX001_SELF_REPAIR_END:
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1);
                        me->SetStandState(UNIT_STAND_STATE_STAND);
                        events.RescheduleEvent(EVENT_LASER_BARRAGE, urand(35000, 40000), 0, PHASE_VX001_ASSEMBLED__GLOBAL_4);
                        events.RescheduleEvent(EVENT_ROCKET_STRIKE, 20000, 0, PHASE_VX001_ASSEMBLED__GLOBAL_4);
                        events.RescheduleEvent(EVENT_HAND_PULSE, 5000, 0, PHASE_VX001_ASSEMBLED__GLOBAL_4);
                        if (MimironHardMode)
                            events.RescheduleEvent(EVENT_FROST_BOMB, 15000, 0, PHASE_VX001_ASSEMBLED__GLOBAL_4);
                        break;
                    case DO_ENTER_ENRAGE:
                        DoCast(me, SPELL_BERSERK, true);
                        break;
                    default:
                        break;
                }
            }

            void DamageTaken(Unit* /*who*/, uint32 &damage)
            {
                if (damage >= me->GetHealth())
                {
                    damage = 0;
                    me->InterruptNonMeleeSpells(true);
                    me->GetMotionMaster()->Clear(true);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1);
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    me->RemoveAllAurasExceptType(SPELL_AURA_CONTROL_VEHICLE);
                    me->SetStandState(UNIT_STAND_STATE_DEAD);

                    switch (phase)
                    {
                        case PHASE_VX001_SOLO__GLOBAL_2:
                            if (Creature* Mimiron = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MIMIRON)))
                                Mimiron->AI()->DoAction(DO_ACTIVATE_AERIAL);
                            break;
                        case PHASE_VX001_ASSEMBLED__GLOBAL_4:
                            if (Creature* Mimiron = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MIMIRON)))
                                Mimiron->AI()->DoAction(DO_VX001_SELF_REPAIR_START);
                            DoCast(me, SPELL_SELF_REPAIR);
                            break;
                        default:
                            break;
                    }
                    events.Reset();
                    phase = PHASE_IDLE;
                    events.SetPhase(phase);
                }
            }

            void SpellHit(Unit* caster, SpellInfo const* spell)
            {
                if (!caster || !spell)
                    return;
                if (spell->Id == SPELL_SELF_REPAIR)
                    DoAction(DO_VX001_SELF_REPAIR_END);
                if (Creature* Mimiron = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MIMIRON)))
                    Mimiron->AI()->DoAction(DO_VX001_SELF_REPAIR_END);
            }

            void SpellHitTarget(Unit* target, SpellInfo const* spell)
            {
                switch (spell->Id)
                {
                    case SPELL_FROSTBOMB:
                        if (Creature *targets = me->FindNearestCreature(NPC_FLAME_SPREAD, 100.0f))
                            me->SummonCreature(NPC_FROST_BOMB, *targets, TEMPSUMMON_TIMED_DESPAWN, 11000);
                        break;
                    case SPELL_ROCKET_STRIKE:
                        if (!target || !spell)
                            return;
                        if (target->GetTypeId() == TYPEID_PLAYER)
                            if (spell->Id == SPELL_ROCKET_STRIKE)
                                if (InstanceScript* instance = me->GetInstanceScript())
                                    if (Creature* mimiron = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MIMIRON)))
                                        mimiron->AI()->DoAction(DATA_AVOIDED_ROCKET_STRIKES);
                        break;
                    default:
                        break;
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || phase == PHASE_IDLE)
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_RAPID_BURST:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                                if (Creature* BurstTarget = me->SummonCreature(NPC_BURST_TARGET, *target, TEMPSUMMON_TIMED_DESPAWN, 3100))
                                    DoCast(BurstTarget, SPELL_RAPID_BURST);
                            events.RescheduleEvent(EVENT_RAPID_BURST, 5000, 0, PHASE_VX001_SOLO__GLOBAL_2);
                            break;
                        case EVENT_LASER_BARRAGE:
                        {
                            float x, y, z;
                            z = me->GetPositionZ();
                            me->GetNearPoint2D(x, y, 10.0f, me->GetOrientation());
                            if (Creature *c = me->SummonCreature(NPC_BURST_TARGET, x, y, z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 15000))
                            {
                                Position pos;
                                me->GetPosition(&pos);
                                c->SetSpeed(MOVE_WALK, 0.5f);
                                c->SetSpeed(MOVE_RUN, 0.5f);
                                me->DisableSpline();
                                me->CastSpell(c, SPELL_SPINNING_UP, true);
                                c->GetMotionMaster()->MoveAroundPoint(pos, 10.0f, 0, 254);
                            }
                            events.DelayEvents(14500);
                            events.ScheduleEvent(EVENT_LASER_BARRAGE, 60000, 0, phase);
                            break;
                        }
                        case EVENT_ROCKET_STRIKE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                            {
                                Position destination;
                                target->GetPosition(&destination);
                                if (Creature *c = me->SummonCreature(38569, destination, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000))
                                {
                                    if (me->GetVehicleKit()->GetPassenger(5))
                                        me->CastSpell(target, SPELL_ROCKET_STRIKE, true);
                                    c->CastSpell(c, SPELL_ROCKET_STRIKE_AURA, true);
                                }
                            }

                            if (phase == PHASE_VX001_ASSEMBLED__GLOBAL_4)
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                                {
                                    Position destination;
                                    target->GetPosition(&destination);
                                    if (Creature *c = me->SummonCreature(38569, destination, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000))
                                    {
                                        if (me->GetVehicleKit()->GetPassenger(6))
                                            me->CastSpell(target, SPELL_ROCKET_STRIKE, true);
                                        c->CastSpell(c, SPELL_ROCKET_STRIKE_AURA, true);
                                    }
                                }
                            events.RescheduleEvent(EVENT_ROCKET_STRIKE, urand(20000, 25000), 0, phase);
                            break;
                        case EVENT_HEAT_WAVE:
                            DoCastAOE(SPELL_HEAT_WAVE);
                            events.RescheduleEvent(EVENT_HEAT_WAVE, 10000, 0, PHASE_VX001_SOLO__GLOBAL_2);
                            break;
                        case EVENT_HAND_PULSE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                            {
                                DoCast(target, SPELL_HAND_PULSE);
                            }
                            events.RescheduleEvent(EVENT_HAND_PULSE, urand(3000, 4000), 0 , PHASE_VX001_ASSEMBLED__GLOBAL_4);
                            break;
                        case EVENT_FROST_BOMB:
                            if (me->FindNearestCreature(NPC_FLAME_SPREAD, 100.0f))
                            {
                                DoCast(SPELL_FROSTBOMB);
                                events.RescheduleEvent(EVENT_FROST_BOMB, 45000, 0, phase);
                            }
                            else
                                events.RescheduleEvent(EVENT_FROST_BOMB, 5000, 0, phase);
                            break;
                        case EVENT_FLAME_SUPPRESSANT_VX001:
                            DoCastAOE(SPELL_FLAME_SUPPRESSANT_VX001);
                            events.RescheduleEvent(EVENT_FLAME_SUPPRESSANT_VX001, 10000, 0, PHASE_VX001_SOLO__GLOBAL_2);
                            break;
                        default:
                            break;
                    }
                }
            }

            private:
                MyPhase phase;
                bool MimironHardMode;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<boss_vx_001AI>(creature);
        }
};

class npc_rocket_strike_target : public CreatureScript
{
    public:
        npc_rocket_strike_target() : CreatureScript("npc_rocket_strike_target") {}

        struct npc_rocket_strike_targetAI : public Scripted_NoMovementAI
        {
            npc_rocket_strike_targetAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
                casted = false;
            }

            void InitializeAI()
            {

            }

            void Reset()
            {
                casted = false;
            }

            void SpellHit(Unit* /*caster*/, SpellInfo const* spell)
            {
                if (!spell)
                    return;
                if (spell->Id == 64064)
                {
                    uiBoom = 5000;
                    casted = true;
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!casted)
                    return ;
                if (uiBoom <= diff)
                {
                    /*                    if (Player * player = me->SelectNearestPlayer(3))
                    {
                        //              player->CastSpell(player, SPELL_ROCKET_STRIKE_DMG, true);
                        }*/
                    uiBoom = 30000;
                    me->DespawnOrUnsummon();
                }
                else
                    uiBoom -= diff;
            }

            private:
                bool casted;
                uint32 uiBoom;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_rocket_strike_targetAI(creature);
        }
};

class npc_rocket_strike : public CreatureScript
{
    public:
        npc_rocket_strike() : CreatureScript("npc_rocket_strike") {}

        struct npc_rocket_strikeAI : public Scripted_NoMovementAI
        {
            npc_rocket_strikeAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
                casted = false;
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
            }

            void InitializeAI()
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
            }

            void Reset()
            {
                casted = false;
            }

            void UpdateAI(uint32 const /*diff*/)
            {
                if (!casted)
                {
                    casted = true;
                    DoCast(me, SPELL_ROCKET_STRIKE_AURA);
                    me->DespawnOrUnsummon(10000);
                }
            }

        private:
            bool casted;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_rocket_strikeAI(creature);
        }
};

class spell_rapid_burst : public SpellScriptLoader
{
    public:
        spell_rapid_burst() : SpellScriptLoader("spell_rapid_burst") {}

        class spell_rapid_burst_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rapid_burst_AuraScript);

            void HandleDummyTick(AuraEffect const* aurEff)
            {
                if (!GetTarget())
                    return;

                if (Unit* caster = GetCaster())
                {
                    switch (caster->GetMap()->GetDifficulty())
                    {
                        case RAID_DIFFICULTY_10MAN_NORMAL:
                            caster->CastSpell(GetTarget(), RAND(SPELL_RAPID_BURST_LEFT_10, SPELL_RAPID_BURST_RIGHT_10), true, NULL, aurEff);
                            break;
                        case RAID_DIFFICULTY_25MAN_NORMAL:
                            caster->CastSpell(GetTarget(), RAND(SPELL_RAPID_BURST_LEFT_25, SPELL_RAPID_BURST_RIGHT_25), true, NULL, aurEff);
                            break;
                        default:
                            break;
                    }
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_rapid_burst_AuraScript::HandleDummyTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_rapid_burst_AuraScript();
        }
};


/************************************************************************/
/*                      Aerial Command Unit                             */
/************************************************************************/
class boss_aerial_unit : public CreatureScript
{
    private:
        enum MyPhase
        {
            PHASE_IDLE = 0,
            PHASE_AERIAL_SOLO__GLOBAL_3,
            PHASE_AERIAL_ASSEMBLED__GLOBAL_4,
        };
        enum Events
        {
            EVENT_PLASMA_BALL = 1,
            EVENT_REACTIVATE_AERIAL,
            EVENT_SUMMON_JUNK_BOT,
            EVENT_SUMMON_ASSAULT_BOT,
            EVENT_SUMMON_BOOM_BOT
        };
    public:
        boss_aerial_unit() : CreatureScript("boss_aerial_unit") {}

        struct boss_aerial_unitAI : public BossAI
        {
            boss_aerial_unitAI(Creature* creature) : BossAI(creature, BOSS_MIMIRON), phase(PHASE_IDLE) {}

            void InitializeAI()
            {
                me->ApplySpellImmune(0, IMMUNITY_ID, SPELL_ROCKET_STRIKE_DMG, true);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetUnitMovementFlags(MOVEMENTFLAG_FLYING);
                me->SetReactState(REACT_PASSIVE);
                Reset();
            }

            void Reset()
            {
                events.Reset();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_DISABLE_MOVE);
                me->SetReactState(REACT_PASSIVE);
                me->SetStandState(UNIT_STAND_STATE_STAND);
                me->SetVisible(false);
                me->RemoveAllAurasExceptType(SPELL_AURA_CONTROL_VEHICLE);
                me->SetCanFly(true);
                phase = PHASE_IDLE;
                events.SetPhase(PHASE_IDLE);
                summons.DespawnAll();
                gotMimironHardMode = false;
                ui_checkpos = 1000;
            }

            void KilledUnit(Unit* /*who*/)
            {
                if (!(rand()%5))
                    if (instance)
                        if (Creature* Mimiron = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MIMIRON)))
                        {
                            if (phase == PHASE_AERIAL_SOLO__GLOBAL_3)
                                Mimiron->AI()->Talk(SAY_V07TRON_SLAY);
                            else
                                Mimiron->AI()->Talk(SAY_V07TRON_SLAY);
                        }
            }

            void SpellHit(Unit* caster, SpellInfo const* spell)
            {
                if (!caster || !spell)
                    return;
                if (spell->Id == SPELL_SELF_REPAIR)
                    DoAction(DO_AERIAL_SELF_REPAIR_END);
                if (Creature* Mimiron = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MIMIRON)))
                    Mimiron->AI()->DoAction(DO_AERIAL_SELF_REPAIR_END);
            }

            void EnterCombat(Unit* /*who*/)
            {
                if (Creature* Mimiron = ObjectAccessor::GetCreature(*me, instance ? instance->GetData64(BOSS_MIMIRON) : 0))
                    gotMimironHardMode = Mimiron->AI()->GetData(DATA_GET_HARD_MODE);

                if (gotMimironHardMode)
                    DoCast(me, SPELL_EMERGENCY_MODE, true);

                events.ScheduleEvent(EVENT_PLASMA_BALL, 1000);
                events.ScheduleEvent(EVENT_SUMMON_JUNK_BOT, 10000, 0, PHASE_AERIAL_SOLO__GLOBAL_3);
                me->GetMotionMaster()->MovePoint(0, me->GetPositionX(), me->GetPositionY(), 380.04f);
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case DO_START_AERIAL:
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_IMMUNE_TO_PC);
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->SetVisible(true);
                        phase = PHASE_AERIAL_SOLO__GLOBAL_3;
                        events.SetPhase(PHASE_AERIAL_SOLO__GLOBAL_3);
                        DoZoneInCombat();
                        break;
                    case DO_DISABLE_AERIAL:
                        if (phase == PHASE_AERIAL_SOLO__GLOBAL_3)
                        {
                            me->CastStop();
                            me->SetReactState(REACT_PASSIVE);
                            me->GetMotionMaster()->Clear();
                            // check if the aura works correctly (+50% damage from all sources)
                            DoCast(me, SPELL_MAGNETIC_CORE);
                            DoCast(me, SPELL_MAGNETIC_CORE_VISUAL);
                            // Move to floor.
                            Position destination;
                            me->GetPosition(&destination);
                            destination.m_positionZ = 368.965f;
                            me->GetMotionMaster()->MoveLand(1, destination);  // Check if MoveLand is ok here, a flying unit should have a landing animation, but... just 4 the case
                            events.DelayEvents(20000);
                            events.ScheduleEvent(EVENT_REACTIVATE_AERIAL, 20000, 0, PHASE_AERIAL_SOLO__GLOBAL_3);
                        }
                        break;
                    case DO_AERIAL_ASSEMBLED:
                        me->SetHealth( (me->GetMaxHealth() >> 1) );               // Once again, re-assemble and repairing share some stuff, so the fallthrough is intended!
                        phase = PHASE_AERIAL_ASSEMBLED__GLOBAL_4;
                        if (Creature* Mimiron = me->GetCreature(*me, instance->GetData64(BOSS_MIMIRON)))
                            Mimiron->AI()->DoAction(DO_ACTIVATE_DEATH_TIMER);
                        events.SetPhase(phase);
                    case DO_AERIAL_SELF_REPAIR_END:
                        if (gotMimironHardMode)
                            DoCast(me, SPELL_EMERGENCY_MODE, true);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1);
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->SetStandState(UNIT_STAND_STATE_STAND);
                        events.RescheduleEvent(EVENT_PLASMA_BALL, 2000);
                        break;
                    case DO_ENTER_ENRAGE:
                        DoCast(me, SPELL_BERSERK, true);
                        break;
                    default:
                        break;
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || phase == PHASE_IDLE)
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (ui_checkpos <= diff)
                {
                    if (me->GetPositionZ() < 367.0f)
                    {
                        Position destination;
                        me->GetPosition(&destination);
                        destination.m_positionZ = 380.04f;
                        me->GetMotionMaster()->MoveTakeoff(1, destination); // Check if MoveTakeoff is ok here, a flying unit should have a landing animation, but... just 4 the case
                    }
                    ui_checkpos = 1000;
                }
                else
                    ui_checkpos -= diff;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_PLASMA_BALL:
                            if (phase == PHASE_AERIAL_SOLO__GLOBAL_3 && me->getVictim())
                            {
                                float x = me->getVictim()->GetPositionX();
                                float y = me->getVictim()->GetPositionY();
                                float z = me->getVictim()->GetPositionZ();
                                if (me->IsWithinDist3d(x, y, z, 30))
                                {
                                    me->GetMotionMaster()->Initialize();
                                    DoCastVictim(SPELL_PLASMA_BALL);
                                }
                                else me->GetMotionMaster()->MovePoint(0, x, y, 380.04f);
                            }
                            else if (phase == PHASE_AERIAL_ASSEMBLED__GLOBAL_4 && me->getVictim())
                            {
                                if (me->getVictim()->IsWithinDist3d(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 30))
                                    DoCastVictim(SPELL_PLASMA_BALL);
                                else if (Unit* target = SelectTarget(SELECT_TARGET_NEAREST, 0))
                                    DoCast(target, SPELL_PLASMA_BALL);
                            }
                            events.RescheduleEvent(EVENT_PLASMA_BALL, 2000);
                            break;
                        case EVENT_REACTIVATE_AERIAL:
                        {
                            me->RemoveAurasDueToSpell(SPELL_MAGNETIC_CORE_VISUAL);
                            Position destination;
                            me->GetPosition(&destination);
                            destination.m_positionZ = 380.04f;
                            me->GetMotionMaster()->MoveTakeoff(1, destination); // Check if MoveTakeoff is ok here, a flying unit should have a landing animation, but... just 4 the case
                            me->SetReactState(REACT_AGGRESSIVE);
                            break;
                        }
                        case EVENT_SUMMON_JUNK_BOT:
                            for (uint8 n = 0; n < 2; n++)
                                me->SummonCreature(NPC_JUNK_BOT, SummonPos[rand()%9], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                            events.RescheduleEvent(EVENT_SUMMON_ASSAULT_BOT, 10000, 0, PHASE_AERIAL_SOLO__GLOBAL_3);
                            break;
                        case EVENT_SUMMON_ASSAULT_BOT:
                            me->SummonCreature(NPC_ASSAULT_BOT, SummonPos[rand()%9], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
                            if (gotMimironHardMode)
                                for (uint8 i = 0; i < 2; i++)
                                    me->SummonCreature(NPC_EMERGENCY_BOT, SummonPos[rand()%9], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                            events.RescheduleEvent(EVENT_SUMMON_BOOM_BOT, 10000, 0, PHASE_AERIAL_SOLO__GLOBAL_3);
                            break;
                        case EVENT_SUMMON_BOOM_BOT:
                            me->SummonCreature(NPC_BOOM_BOT, 2744.65f, 2569.46f, 364.397f, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                            events.RescheduleEvent(EVENT_SUMMON_JUNK_BOT, 10000, 0, PHASE_AERIAL_SOLO__GLOBAL_3);
                            break;
                        default:
                            break;
                    }
                }
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);

                if (summon->GetEntry() == NPC_EMERGENCY_BOT)
                    return;

                summon->AI()->DoZoneInCombat();
                if (gotMimironHardMode)
                    summon->CastSpell(summon, SPELL_EMERGENCY_MODE, true);
            }

            void DamageTaken(Unit* /*who*/, uint32 &damage)
            {
                if (damage >= me->GetHealth())
                {
                    damage = 0;
                    me->InterruptNonMeleeSpells(true);
                    me->GetMotionMaster()->Clear(true);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1);
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    me->RemoveAllAurasExceptType(SPELL_AURA_CONTROL_VEHICLE);
                    //me->SetHealth(me->GetMaxHealth());
                    me->SetStandState(UNIT_STAND_STATE_DEAD);

                    switch (phase)
                    {
                        case PHASE_AERIAL_SOLO__GLOBAL_3:
                            events.CancelEvent(EVENT_SUMMON_JUNK_BOT);
                            if (Creature* Mimiron = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MIMIRON)))
                                Mimiron->AI()->DoAction(DO_ACTIVATE_V0L7R0N);
                            break;
                        case PHASE_AERIAL_ASSEMBLED__GLOBAL_4:
                            if (Creature* Mimiron = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MIMIRON)))
                                Mimiron->AI()->DoAction(DO_AERIAL_SELF_REPAIR_START);
                            DoCast(me, SPELL_SELF_REPAIR);
                            break;
                        default:
                            break;
                    }
                    events.Reset();
                    phase = PHASE_IDLE;
                    events.SetPhase(phase);
                }
            }

            private:
                MyPhase phase;
                bool gotMimironHardMode;
                uint32 ui_checkpos;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetUlduarAI<boss_aerial_unitAI>(creature);
        }
};

/* TODO:
- check if spawn mechanism works correctly
*/
class npc_magnetic_core : public CreatureScript
{
    public:
        npc_magnetic_core() : CreatureScript("npc_magnetic_core") {}

        struct npc_magnetic_coreAI : public Scripted_NoMovementAI
        {
            npc_magnetic_coreAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
                me->DespawnOrUnsummon(21000);
                if (Creature* AerialUnit = me->FindNearestCreature(NPC_AERIAL_COMMAND_UNIT, 100.0f, true))
                {
                    AerialUnit->AI()->DoAction(DO_DISABLE_AERIAL); // Causes the NPC to land
                    // me->GetMotionMaster()->MoveFall();
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_magnetic_coreAI(creature);
        }
};

class npc_assault_bot : public CreatureScript
{
    public:
        npc_assault_bot() : CreatureScript("npc_assault_bot") {}

        struct npc_assault_botAI : public ScriptedAI
        {
            npc_assault_botAI(Creature* creature) : ScriptedAI(creature) {}

            void Reset()
            {
                _fieldTimer = urand(4000, 6000);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                if (_fieldTimer <= diff)
                {
                    DoCastVictim(SPELL_MAGNETIC_FIELD);
                    _fieldTimer = urand(15000, 20000);
                }
                else
                    _fieldTimer -= diff;

                DoMeleeAttackIfReady();
            }

            void SpellHit(Unit* /*caster*/, SpellInfo const* spell)
            {
                // Achievement Not-So-Friendly Fire
                if (spell->Id == SPELL_ROCKET_STRIKE_DMG)
                    if (Player* player = me->SelectNearestPlayer(100.0f))
                        player->CastSpell(player, SPELL_NOT_SO_FRIENDLY_FIRE, true);
            }

            private:
                uint32 _fieldTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_assault_botAI(creature);
        }
};

class npc_emergency_bot : public CreatureScript
{
    private:
        enum { SPELL_DEATH_GRIP = 49560 };
    public:
        npc_emergency_bot() : CreatureScript("npc_emergency_bot") {}

        struct npc_emergency_botAI : public ScriptedAI
        {
            npc_emergency_botAI(Creature* creature) : ScriptedAI(creature) {}

            void InitializeAI()
            {
                me->setFaction(14);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_ID, SPELL_DEATH_GRIP, true);
                me->SetReactState(REACT_PASSIVE);
                Reset();
            }

            void Reset()
            {
                me->GetMotionMaster()->MoveRandom(15);
                _sprayTimer = 5000;

                if (Is25ManRaid())
                    DoCast(me, SPELL_DEAFENING_SIREN, true);
            }

            void UpdateAI(uint32 const diff)
            {
                if (_sprayTimer <= diff)
                {
                    DoCast(SPELL_WATER_SPRAY);
                    _sprayTimer = 10000;
                }
                else _sprayTimer -= diff;
            }

            private:
                uint32 _sprayTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_emergency_botAI(creature);
        }
};

class npc_mimiron_bomb_bot : public CreatureScript
{
    public:
        npc_mimiron_bomb_bot() : CreatureScript("npc_mimiron_bomb_bot") {}

        struct mob_mimiron_bomb_botAI : public ScriptedAI
        {
            mob_mimiron_bomb_botAI(Creature* creature) : ScriptedAI(creature) {}

            void InitializeAI()
            {
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
            }

            Unit* SelectPlayerTargetInRange(float range)
            {
                Player* target = 0;
                Trinity::AnyPlayerInObjectRangeCheck u_check(me, range, true);
                Trinity::PlayerSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, target, u_check);
                me->VisitNearbyObject(range, searcher);
                return target;
            }

            void Reset()
            {
                _despawn = false;
                if (Unit* target = SelectPlayerTargetInRange(500.0f))
                {
                    me->AddThreat(target, std::numeric_limits<float>::max());
                    me->GetMotionMaster()->MoveFollow(target, 100.0f, 0.0f);
                }
            }

            void SpellHitTarget(Unit* target, SpellInfo const* spell)
            {
                if (!target || !spell)
                    return;

                if (target->GetTypeId() == TYPEID_PLAYER)
                    if (spell->Id == SPELL_BOOM_BOT || spell->Id == SPELL_BOOM_BOT_PERIODIC)
                        if (InstanceScript* instance = me->GetInstanceScript())
                            if (Creature* mimiron = ObjectAccessor::GetCreature(*me, instance->GetData64(BOSS_MIMIRON)))
                                mimiron->AI()->DoAction(DATA_AVOIDED_BOOM_BOT_EXPLOSION);
            }

            void JustDied(Unit* /*killer*/)
            {
                DoCast(me, SPELL_BOOM_BOT, true);
            }

            void UpdateAI(uint32 const /*diff*/)
            {
                if (!UpdateVictim())
                    return;

                if (!_despawn && me->IsWithinMeleeRange(me->getVictim()))
                {
                    _despawn = true;
                    DoCast(me, SPELL_BOOM_BOT, true);
                    me->DespawnOrUnsummon(1500);
                }
            }

            private:
                bool _despawn;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_mimiron_bomb_botAI(creature);
        }
};

/************************************************************************/
/*                    Evil button (HM activator)                        */
/************************************************************************/

class go_not_push_button : public GameObjectScript
{
    public:
        go_not_push_button() : GameObjectScript("go_mimiron_hard_mode_activator") {}

        bool OnGossipHello(Player* player, GameObject* go)
        {
            if (InstanceScript* instance = go->GetInstanceScript())
                if ((instance->GetBossState(BOSS_MIMIRON) != IN_PROGRESS && instance->GetBossState(BOSS_MIMIRON) != DONE) && player)
                    if (Creature* mimiron = ObjectAccessor::GetCreature((*player), instance->GetData64(BOSS_MIMIRON)))
                    {
                        mimiron->AI()->DoAction(DO_ACTIVATE_HARD_MODE);
                        go->UseDoorOrButton();
                        return true;
                    }
            return false;
        }
};

/************************************************************************/
/*                            Helper NPCs                               */
/************************************************************************/

class npc_mimiron_flame_trigger : public CreatureScript
{
    public:
        npc_mimiron_flame_trigger() : CreatureScript("npc_mimiron_flame_trigger") {}

        struct npc_mimiron_flame_triggerAI : public ScriptedAI
        {
            npc_mimiron_flame_triggerAI(Creature* creature) : ScriptedAI(creature) {}

            void InitializeAI()
            {
                instance = me->GetInstanceScript();
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
            }

            void Reset()
            {
                flameTimer = 2000;
                me->SetInCombatWithZone();
            }

            void SpellHit(Unit* /*caster*/, SpellInfo const* spell)
            {
                switch (spell->Id)
                {
                    case SPELL_FLAME_SUPPRESSANT_MK:
                    case SPELL_FLAME_SUPPRESSANT_VX001:
                    case SPELL_FROST_BOMB_EXPLOSION_10:
                    case SPELL_FROST_BOMB_EXPLOSION_25:
                    case SPELL_WATER_SPRAY:
                        flameTimer = 1000;
                        me->DespawnOrUnsummon(500);
                        break;
                    default:
                        break;
                }
            }

            void JustSummoned(Creature* /*summon*/)
            {
                if (Creature* mimiron = ObjectAccessor::GetCreature(*me, instance ? instance->GetData64(BOSS_MIMIRON) : 0))
                    mimiron->AI()->DoAction(DO_INCREASE_FLAME_COUNT);
            }

            void UpdateAI(uint32 const diff)
            {
                if (flameTimer <= diff)
                {
                    // check if flame cap is reached
                    if (Creature* mimiron = ObjectAccessor::GetCreature(*me, instance ? instance->GetData64(BOSS_MIMIRON) : 0))
                        if (mimiron->AI()->GetData(DATA_FLAME_COUNT) >= FLAME_CAP)
                        {
                            me->DespawnOrUnsummon();
                            return;
                        }

                        DoZoneInCombat(me, 100.0f);

                        if (Player* nearest = me->SelectNearestPlayer(100.0f))
                        {
                            me->GetMotionMaster()->Clear();
                            me->GetMotionMaster()->MoveFollow(nearest, 0.0f, 0.0f);
                        }

                        me->SummonCreature(NPC_FLAME_SPREAD, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                        flameTimer = 4000;
                }
                else
                    flameTimer -= diff;
            }

            private:
                InstanceScript* instance;
                uint32 flameTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_mimiron_flame_triggerAI(creature);
        }
};

class npc_mimiron_flame_spread : public CreatureScript
{
    public:
        npc_mimiron_flame_spread() : CreatureScript("npc_mimiron_flame_spread") {}

        struct npc_mimiron_flame_spreadAI : public ScriptedAI
        {
            npc_mimiron_flame_spreadAI(Creature* creature) : ScriptedAI(creature) {}

            void InitializeAI()
            {
                instance = me->GetInstanceScript();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED | UNIT_FLAG_DISABLE_MOVE);
                me->SetReactState(REACT_PASSIVE);
                Reset();
            }

            void Reset()
            {
                DoCast(me, SPELL_FLAME, true);
            }

            void SpellHit(Unit* /*caster*/, SpellInfo const* spell)
            {
                switch (spell->Id)
                {
                    case SPELL_FLAME_SUPPRESSANT_MK:
                    case SPELL_FLAME_SUPPRESSANT_VX001:
                    case SPELL_FROST_BOMB_EXPLOSION_10:
                    case SPELL_FROST_BOMB_EXPLOSION_25:
                    case SPELL_WATER_SPRAY:
                        if (Creature* mimiron = ObjectAccessor::GetCreature(*me, instance ? instance->GetData64(BOSS_MIMIRON) : 0))
                            mimiron->AI()->DoAction(DO_DECREASE_FLAME_COUNT);
                        me->DespawnOrUnsummon(500);
                        break;
                    default:
                        break;
                }
            }

            void UpdateAI(uint32 const /*diff*/)
            {
                if (instance && instance->GetBossState(BOSS_MIMIRON) != IN_PROGRESS)
                    me->DespawnOrUnsummon();
            }

            private:
                InstanceScript* instance;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_mimiron_flame_spreadAI(creature);
        }
};

class npc_frost_bomb : public CreatureScript
{
    public:
        npc_frost_bomb() : CreatureScript("npc_frost_bomb") {}

        struct npc_frost_bombAI : public Scripted_NoMovementAI
        {
            npc_frost_bombAI(Creature* creature) : Scripted_NoMovementAI(creature) {}

            void InitializeAI()
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
                Reset();
            }

            void Reset()
            {
                DoCast(me, SPELL_FROST_BOMB_VISUAL, true);
                frostTimer = 10000;
            }

            void UpdateAI(uint32 const diff)
            {
                if (frostTimer <= diff)
                {
                    DoCast(me, RAID_MODE(SPELL_FROST_BOMB_EXPLOSION_10, SPELL_FROST_BOMB_EXPLOSION_25), true);
                    me->DespawnOrUnsummon();
                    frostTimer = 10000;
                }
                else frostTimer -= diff;
            }

            private:
                uint32 frostTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_frost_bombAI(creature);
        }
};

/************************************************************************/
/*                          Achievements                                */
/************************************************************************/

class achievement_firefighter : public AchievementCriteriaScript
{
public:
    achievement_firefighter(const char* name) : AchievementCriteriaScript(name) {}

    bool OnCheck(Player* player, Unit* /*target*/)
    {
        if (player)
            if (InstanceScript* instance = player->GetInstanceScript())
                if (Creature* mimiron = ObjectAccessor::GetCreature(*player, instance->GetData64(BOSS_MIMIRON)))
                    return (mimiron->AI()->GetData(DATA_GET_HARD_MODE));
        return false;
    }
};

class achievement_set_up_us_the_bomb__proximity_mines : public AchievementCriteriaScript
{
public:
    achievement_set_up_us_the_bomb__proximity_mines(const char* name) : AchievementCriteriaScript(name) {}

    bool OnCheck(Player* player, Unit* /*target*/)
    {
        if (player)
            if (InstanceScript* instance = player->GetInstanceScript())
                if (Creature* mimiron = ObjectAccessor::GetCreature(*player, instance->GetData64(BOSS_MIMIRON)))
                    return (mimiron->AI()->GetData(DATA_AVOIDED_PROXIMITY_MINES));
        return false;
    }
};

class achievement_set_up_us_the_bomb__rocket_strikes : public AchievementCriteriaScript
{
public:
    achievement_set_up_us_the_bomb__rocket_strikes(const char* name) : AchievementCriteriaScript(name) {}

    bool OnCheck(Player* player, Unit* /*target*/)
    {
        if (player)
            if (InstanceScript* instance = player->GetInstanceScript())
                if (Creature* mimiron = ObjectAccessor::GetCreature(*player, instance->GetData64(BOSS_MIMIRON)))
                    return (mimiron->AI()->GetData(DATA_AVOIDED_ROCKET_STRIKES));
        return false;
    }
};

class achievement_set_up_us_the_bomb__boombot_explosion : public AchievementCriteriaScript
{
public:
    achievement_set_up_us_the_bomb__boombot_explosion(const char* name) : AchievementCriteriaScript(name) {}

    bool OnCheck(Player* player, Unit* /*target*/)
    {
        if (player)
            if (InstanceScript* instance = player->GetInstanceScript())
                if (Creature* mimiron = ObjectAccessor::GetCreature(*player, instance->GetData64(BOSS_MIMIRON)))
                    return (mimiron->AI()->GetData(DATA_AVOIDED_BOOM_BOT_EXPLOSION));
        return false;
    }
};

void AddSC_boss_mimiron()
{
    new boss_mimiron();
    new boss_leviathan_mk();
    new boss_leviathan_mk_turret();
    new boss_vx_001();
    new boss_aerial_unit();
    new npc_proximity_mine();
    new npc_rocket_strike();
    new npc_magnetic_core();
    new npc_assault_bot();
    new npc_emergency_bot();
    new npc_mimiron_bomb_bot();
    new npc_mimiron_flame_trigger();
    new npc_mimiron_flame_spread();
    new npc_frost_bomb();
    new npc_rocket_strike_target();
    new spell_rapid_burst();
    new go_not_push_button();

    new achievement_firefighter("achievement_firefighter");     // Achiev 3180 / Criteria 10450
    new achievement_firefighter("achievement_firefighter_25");  // Achiev 3189 / Criteria 10463

    // TODO: Find correct criterias for the following achievements:
    //    new achievement_set_up_us_the_bomb__boombot_explosion("achievement_set_up_us_the_bomb__boombot");
    //  new achievement_set_up_us_the_bomb__boombot_explosion("achievement_set_up_us_the_bomb__boombot_25");
    //  new achievement_set_up_us_the_bomb__proximity_mines("achievement_set_up_us_the_bomb__proximity");
    //  new achievement_set_up_us_the_bomb__proximity_mines("achievement_set_up_us_the_bomb__proximity_25");
    //  new achievement_set_up_us_the_bomb__rocket_strikes("achievement_set_up_us_the_bomb__rockets");
    // new achievement_set_up_us_the_bomb__rocket_strikes("achievement_set_up_us_the_bomb__rockets_25");
}

#undef SPELL_NAPALM_SHELL
#undef SPELL_HEAT_WAVE
#undef SPELL_HAND_PULSE
