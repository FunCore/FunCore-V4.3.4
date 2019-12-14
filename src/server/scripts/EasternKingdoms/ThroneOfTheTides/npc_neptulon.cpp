
#include "GridNotifiers.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "throne_of_the_tides.h"

#define GOSSIP_READY "We are ready!"
#define SAY_1 "As I purify these waters, the servants of filth will surely be stirred. Beware!"
#define SAY_2 "Patience guests. The waters are nearly cleansed."
#define SAY_3 "The beast has returned! It must not pollute my waters!"
#define SAY_DEATH "Your kind... cannot be... trusted..."
#define SAY_CLEANSED "My waters are cleansed! Drink in their power!"

enum Phases
{
    PHASE_NULL                    = 0, // Used to avoid wrong behaviour
    PHASE_FILTHY_INVADERS_1       = 1, // Starting phase
    PHASE_FILTHY_INVADERS_2       = 2, // Murlocs
    PHASE_FILTHY_INVADERS_3       = 3, // Mindlashers
    PHASE_FILTHY_INVADERS_4       = 4, // Behemont
    PHASE_BEAST_RETURN            = 5, // Real phase 2
    PHASE_TIDAL_SURGE             = 6, // Real phase 3
};

const Position spawns[] =
{
    {-122.412041f, 947.492188f, 231.579025f, 2.279974f},
    {-118.400780f, 1014.799866f, 230.195724f, 4.366778f},
};

enum Spells
{
    SPELL_AURA_PURIFIED                     = 76952,
    SPELL_SUMMON_BLIGHT_OF_OZUMAT_LOCAL     = 83524,

    SPELL_MAREE_POWER                       = 76133,

    SPELL_BEHEMOT_BLIGHT_SPRAY              = 83985,

    SPELL_BLIGHT_OF_OZUMAT_LOCAL            = 83525,

    SPELL_ENTANGLING_GRASP                  = 83463,
    SPELL_SUMMON_BLIGHT_OF_OZUMAT_ROOM      = 83606,
    SPELL_BLIGHT_OF_OZUMAT_ROOM_TRIGGER     = 83607,
    SPELL_SUMMON_KITE_ADD_EFFECT            = 83647,
    SPELL_JUMP_TO_GROUND                    = 83652,
    SPELL_CHARGE_TO_WINDOW                  = 83653,
    SPELL_KTE_ADD_VISUAL                    = 83665,
    SPELL_BLIGHT_OF_OZUMAT_ROOM             = 83672,
    SPELL_AURA_OF_DEAD                      = 83970,
    SPELL_ACHIEVEMENT_CREDIT                = 95673,
};

enum OzumatPhase
{
    PHASE_ONE,
    PHASE_TWO,
};

enum actionOzumat
{
    ACTION_CHANGE_PHASE,
};

class npc_neptulon : public CreatureScript
{
public:
    npc_neptulon() : CreatureScript("npc_neptulon") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_neptulonAI (creature);
    }

    struct npc_neptulonAI : public ScriptedAI
    {
        npc_neptulonAI(Creature* creature) : ScriptedAI(creature), Summons(me)
        {
            instance = creature->GetInstanceScript();
            done = false;
        }

        void DespawnOzumatSummons(Creature* summon);

        void Reset()
        {
            if (instance)
            {
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MAREE_POWER);
                if (!done)
                    instance->SetData(DATA_OZUMAT, NOT_STARTED);
            }
            me->SetReactState(REACT_PASSIVE);
            me->AttackStop();
            InProgress = false;
            sumSapper.clear();
            sumMindlasher.clear();
            sumMurlocs.clear();
            phase = PHASE_FILTHY_INVADERS_1;
            MindlasherTimer     = 20000;
            sumonslasher        = false;
            behemontTimer       = 30000;
            summonbehemont      = false;
            summonMurlocTimer   = 8000;
            phase2_timer        = 60000;
            ozumatGUID          = 0;
            ozumatSummoned      = false;
            flagged             = false;
            deadSappers         = 0;
            slasherphased       = false;
            mui_event = 8000;
            event = 0;
            _ph = 0;
            mui_beast = 20000;
            me->RemoveAura(SPELL_AURA_PURIFIED);
            speakIntro = true;
            _wipeCheckTimer = 5000;
            Summons.DespawnAll();
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (me->IsWithinDistInMap(who, 15) && !speakIntro)
                Talk(0);
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            switch(summon->GetEntry())
            {
                case NPC_SAPPER:
                    deadSappers += 1;
                    if (deadSappers > 2)
                    {
                        mui_beast = 5000;
                        _ph = 2;
                        Talk(6);
                    }
                    break;
               case NPC_MINDLASHER:
                   if (event == 7)
                       event++;
                   break;
               case BOSS_OZUMAT:
                   Summons.DespawnAll();
                   DespawnOzumatSummons(summon);
            }
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage)
        {
            damage = 0;
        }

        void JustSummoned(Creature* summon)
        {
            switch (summon->GetEntry())
            {
                case NPC_DEEP_MURLOC:
                    sumMurlocs.push_back(summon->GetGUID());
                    summon->AI()->AttackStart(me);
                    break;
                case NPC_MINDLASHER:
                    sumMindlasher.push_back(summon->GetGUID());
                    summon->AI()->AttackStart(me);
                    summon->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    break;
                case NPC_BEHEMOTH:
                    summon->AI()->AttackStart(me);
                    break;
                case NPC_SAPPER:
                    sumSapper.push_back(summon->GetGUID());
                    summon->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                    summon->CastSpell(me, SPELL_ENTANGLING_GRASP, true);
                    break;
                case BOSS_OZUMAT:
                   ozumatGUID = summon->GetGUID();
                   summon->setFaction(14);
                   summon->SetInCombatWithZone();
                   break;
                case NPC_BEAST:
                    if (IsHeroic())
                        summon->CastSpell(summon, SPELL_AURA_OF_DEAD, true);
                    summon->SetInCombatWithZone();
                    break;
                default:
                    summon->SetInCombatWithZone();
                    break;
            }
            Summons.Summon(summon);
        }

        void JustDied(Unit* killer)
        {
            if (killer)
                me->MonsterSay(SAY_DEATH, LANG_UNIVERSAL, killer->GetGUID());
            //            Talk(8);
            Summons.DespawnAll();
        }

        void SummonAdd(uint32 entry, uint32 count)
        {
            if (entry == BOSS_OZUMAT)
            {
                me->SummonCreature(entry, -138.47f, 937.29f, 238.17f, 2.241692f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
                return;
            }

            if (entry == NPC_SAPPER)
            {
                me->SummonCreature(entry, -142.599869f, 1001.389221f, 230.390076f, 4.424302f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                me->SummonCreature(entry, -128.356040f, 980.962253f, 230.372058f, 3.170475f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                me->SummonCreature(entry, -154.590064f, 963.942871f, 229.926407f, 1.250507f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                return;
            }

            for (uint32 x = 0; x < count; ++x)
               me->SummonCreature(entry, RAND(spawns[0], spawns[1]), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!InProgress)
                return;

            if (_wipeCheckTimer <= diff)
            {
                _wipeCheckTimer = 5000;
                Player* player = NULL;
                Trinity::AnyPlayerInObjectRangeCheck check(me, 60.0f);
                Trinity::PlayerSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, player, check);
                me->VisitNearbyWorldObject(60.0f, searcher);
                if (!player)
                    Reset();
            }
            else
                _wipeCheckTimer -= diff;

            if (phase == PHASE_FILTHY_INVADERS_1)
            {
                me->SetFacingTo(0.166467f);
                phase = PHASE_FILTHY_INVADERS_2;
                Talk(2);
                me->AddAura(SPELL_AURA_PURIFIED, me);
            }

            if (_ph == 0)
            {
                if (mui_event <= diff)
                {
                    switch (event)
                    {
                        case 0:
                           SummonAdd(NPC_DEEP_MURLOC, 6);
                           mui_event = 6000;
                           break;
                        case 1:
                           SummonAdd(NPC_MINDLASHER, 1);
                           mui_event = 15000;
                           break;
                        case 2:
                            SummonAdd(NPC_BEHEMOTH, 1);
                            mui_event = 10000;
                            break;
                        case 3:
                            SummonAdd(NPC_DEEP_MURLOC, 6);
                            mui_event = 9000;
                            break;
                        case 4:
                            SummonAdd(NPC_MINDLASHER, 1);
                            mui_event = 20000;
                            break;
                        case 5:
                            SummonAdd(NPC_DEEP_MURLOC, 6);
                            mui_event = 15000;
                            break;
                        case 6:
                            Talk(3);
                            SummonAdd(NPC_MINDLASHER, 1);
                            mui_event = 1000;
                            break;
                        case 7:
                            mui_event = 1000;
                            break;
                        case 8:
                            mui_event = 5000;
                            break;
                        case 9:
                            SummonAdd(NPC_SAPPER, 3);
                            me->RemoveAura(SPELL_AURA_PURIFIED);
                            mui_event = 9000;
                            Talk(4);
                            break;
                        case 10:
                            SummonAdd(NPC_BEAST, 1);
                            mui_event = 1000;
                            _ph = 1;
                            SummonAdd(BOSS_OZUMAT, 1);
                            Talk(5);
                            break;
                        default :
                            mui_event = 1000;
                            break;
                    }
                    if (event <= 11 && event != 7)
                        event++;
                }
                else
                    mui_event -= diff;
            }
            else if (_ph == 1)
            {
                if (mui_beast <= diff)
                {
                    SummonAdd(NPC_BEAST, 1);
                    mui_beast = 20000;
                }
                else mui_beast -= diff;
            }
            else if (_ph == 2)
            {
                if (mui_beast <= diff)
                {
                    if (Creature *Ozumat = Unit::GetCreature(*me, instance->GetData64(DATA_OZUMAT)))
                        Ozumat->AI()->DoAction(ACTION_CHANGE_PHASE);
                    _ph = 3;
                    mui_beast = 6000;
                }
                else mui_beast -= diff;
            }
            else if (_ph == 3)
            {
                if (mui_beast <= diff)
                {
                    Talk(7);
                    if (instance)
                        instance->DoCastSpellOnPlayers(SPELL_MAREE_POWER);
                    _ph = 4;
                    mui_beast = 6000;
                }
                else mui_beast -= diff;
            }
        }


        bool InProgress;
        bool done;
        InstanceScript* instance;
        SummonList Summons;
    private :
        bool sumonslasher, summonbehemont, ozumatSummoned, flagged, slasherphased;
        uint32 phase;
        uint32 summonMurlocTimer;
        uint32 MindlasherTimer;
        std::list<uint64 > sumMurlocs;
        std::list<uint64 > sumMindlasher;
        std::list<uint64 > sumSapper;
        uint32 behemontTimer;
        uint32 phase2_timer;
        uint64 ozumatGUID;
        uint32 deadSappers;
        uint32 event;
        uint32 mui_event;
        uint32 _ph;
        uint32 mui_beast;
        uint32 _wipeCheckTimer;
        bool speakIntro;
    };

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == 1000 && creature->AI())
        {
            if (npc_neptulon::npc_neptulonAI* ai = CAST_AI(npc_neptulon::npc_neptulonAI, creature->AI()))
            {
                ai->InProgress = true;
                ai->DoZoneInCombat();
                ai->Talk(1);

                if (ai->instance)
                    ai->instance->SetData(DATA_OZUMAT, IN_PROGRESS);
            }
            creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }
        player->PlayerTalkClass->SendCloseGossip();
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        InstanceScript* instance = creature->GetInstanceScript();
        if (instance)
        {
            if (instance->GetData(DATA_LADY_NAZJAR_EVENT) == DONE && instance->GetData(DATA_COMMANDER_ULTHOK_EVENT) == DONE && instance->GetData(DATA_ERUNAK_STONESPEAKER_EVENT) == DONE)
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_READY, GOSSIP_SENDER_MAIN, 1000);
                player->PlayerTalkClass->SendGossipMenu(player->GetGossipTextId(creature), creature->GetGUID());
            }
        }
        return true;
    }
};

class boss_ozumat : public CreatureScript
{
public:
    boss_ozumat() : CreatureScript("boss_ozumat") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_ozumatAI (creature);
    }

    struct boss_ozumatAI : public ScriptedAI
    {
        boss_ozumatAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            if (instance)
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MAREE_POWER);
            Finished = false;
            _phase = PHASE_ONE;
            mui_timer_little_chancre = 10000;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            summons.DespawnAll();
        }

        void DoAction(const int32 action)
        {
            if (action == ACTION_CHANGE_PHASE)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                _phase = PHASE_TWO;
                DoCast(SPELL_BLIGHT_OF_OZUMAT_ROOM);
                DoCast(SPELL_SUMMON_BLIGHT_OF_OZUMAT_ROOM);
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            if (instance)
            {
                if (IsHeroic())
                {
                    instance->FinishLfgDungeon(me);
                    instance->CompleteGuildCriteriaForGuildGroup(ACHIEVEMENT_THRONE_OF_THE_TIDES_GUILD_GROUP_CRITERIA);
                }

                if (Creature *Neptulon = Unit::GetCreature(*me, instance->GetData64(DATA_NEPTULON)))
                {
                    Neptulon->DespawnOrUnsummon(35000);
                    if (npc_neptulon::npc_neptulonAI* neptulonAi = CAST_AI(npc_neptulon::npc_neptulonAI, Neptulon->AI()))
                    {
                        neptulonAi->done = true;
                        neptulonAi->Summons.DespawnAll();
                    }
                }
                me->CombatStop();
                instance->DoUpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_ACHIEVEMENT_CREDIT);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MAREE_POWER);
                instance->SetData(DATA_OZUMAT, DONE);
            }
            summons.DespawnAll();
            me->DespawnOrUnsummon();
        }

        void UpdateAI(const uint32 diff)
        {
            switch (_phase)
            {
                case PHASE_ONE:
                    if (mui_timer_little_chancre <= diff)
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            me->CastSpell(target, SPELL_SUMMON_BLIGHT_OF_OZUMAT_LOCAL, true);
                        mui_timer_little_chancre = 10000;
                    }
                    else
                        mui_timer_little_chancre -= diff;
                    break;
                default :
                    break;
            }
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            switch (summon->GetEntry())
            {
                case NPC_BLIGHT_OF_OZUMAT_ROOM:
                    summon->CastSpell(summon, SPELL_BLIGHT_OF_OZUMAT_ROOM_TRIGGER);
                    break;
                default:
                    break;
            }
        }

        SummonList summons;
    private:
        InstanceScript* instance;
        bool Finished;
        uint8 _phase;
        uint32 mui_timer_little_chancre;
    };
};

class npc_behemot : public CreatureScript
{
public:
    npc_behemot() : CreatureScript("npc_behemot") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_behemotAI (creature);
    }

    struct npc_behemotAI : public ScriptedAI
    {
        npc_behemotAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            mui_blight_spray_timer = 3000;
        }

        void JustDied(Unit* killer)
        {
            if (killer->HasAura(HEROIC_DEFEAT_BEHEMOT_WITH_SURGE_EFFECT) && IsHeroic())
                instance->DoCompleteAchievement(SPELL_MAREE_POWER);
        }

        void UpdateAI(const uint32 diff)
        {
            if (mui_blight_spray_timer <= diff)
            {
                DoCastAOE(SPELL_BEHEMOT_BLIGHT_SPRAY);
                mui_blight_spray_timer = urand(5000, 7000);
            }
            else mui_blight_spray_timer -= diff;
        }

    private :
        InstanceScript* instance;
        uint32 mui_blight_spray_timer;
    };
};

void npc_neptulon::npc_neptulonAI::DespawnOzumatSummons(Creature* summon)
{
    if (summon->AI())
        if (boss_ozumat::boss_ozumatAI* ozumatAi = CAST_AI(boss_ozumat::boss_ozumatAI, summon->AI()))
            ozumatAi->summons.DespawnAll();
}
void AddSC_neptulon()
{
    new npc_neptulon();
    new boss_ozumat();
    new npc_behemot();
}
