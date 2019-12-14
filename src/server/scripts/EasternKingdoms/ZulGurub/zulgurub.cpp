#include "ScriptMgr.h"
#include "Vehicle.h"
#include "MoveSplineInit.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "zulgurub.h"

enum
{
    EVENT_YOGA_FLAME        = 1,
};

class npc_tiki_torch : public CreatureScript
{
    public:
        npc_tiki_torch() : CreatureScript("npc_tiki_torch") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return GetZulGurubAI<npc_tiki_torchAI>(creature);
        }

        struct npc_tiki_torchAI : public ScriptedAI
        {
            npc_tiki_torchAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void SetData(uint32 type, uint32 data)
            {
                if (type == DATA_POSITION_ID)
                    events.ScheduleEvent(EVENT_YOGA_FLAME, 1000 * data);
            }

            void UpdateAI(uint32 const diff)
            {
                events.Update(diff);

                if (events.ExecuteEvent() == EVENT_YOGA_FLAME)
                {
                    me->CastSpell(me, 97000, false);
                    events.ScheduleEvent(EVENT_YOGA_FLAME, 6000);
                }
            }
        };
};

enum ZulgrubEnum
{
    EVENT_ROLLING_BOULDERS_LEFT             = 1,
    EVENT_ROLLING_BOULDERS_CENTER           = 2,
    EVENT_ROLLING_BOULDERS_RIGHT            = 3,

    EVENT_KNOCK_AWAY                        = 4,
    EVENT_THUNDERCLAP                       = 5,

    EVENT_BOULDER_BOOM                      = 1,

    NPC_BOULDER_LEFT                        = 52351,
    NPC_BOULDER_CENTER                      = 52354,
    NPC_BOULDER_RIGHT                       = 52353,

    POINT_BOULDER                           = 1,
    POINT_BACK                              = 2,
    ACTION_START_MOVING                     = 1,
};

class npc_berserking_boulder_roller : public CreatureScript
{
    public:
        npc_berserking_boulder_roller() : CreatureScript("npc_berserking_boulder_roller") { }

    private:

        struct npc_berserking_boulder_rollerAI : public ScriptedAI
        {
            npc_berserking_boulder_rollerAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void InitializeAI()
            {
                JustReachedHome();
            }

            void JustReachedHome()
            {
                events.Reset();
                events.ScheduleEvent(EVENT_ROLLING_BOULDERS_CENTER, 1000);
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.Reset();
                events.ScheduleEvent(EVENT_KNOCK_AWAY, urand(7000, 14000));
                events.ScheduleEvent(EVENT_THUNDERCLAP, urand(3000, 7000));
            }

            void JustDied(Unit* /*killer*/)
            {
                events.Reset();
            }

            void JustSummoned(Creature* summoned)
            {
                Position pos;
                summoned->GetPosition(&pos);
                if (Creature* boulder = summoned->SummonCreature(52350, pos, TEMPSUMMON_TIMED_DESPAWN, 9500))
                {
                    boulder->EnterVehicle(summoned);
                    boulder->SetReactState(REACT_PASSIVE);
                    boulder->CastSpell(boulder, 96833, true);
                }

                summoned->CastSpell(summoned, 96833, true);
                summoned->SetReactState(REACT_PASSIVE);
                summoned->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, false);
                summoned->AI()->DoAction(ACTION_START_MOVING);
            }

            void UpdateAI(uint32 const diff)
            {
                events.Update(diff);

                if (!UpdateVictim())
                {
                    if (uint32 eventId = events.ExecuteEvent())
                    {
                        switch (eventId)
                        {
                            case EVENT_ROLLING_BOULDERS_LEFT:
                                me->CastSpell(me, 96827, false);
                                events.ScheduleEvent(RAND(EVENT_ROLLING_BOULDERS_CENTER, EVENT_ROLLING_BOULDERS_RIGHT), urand(2000, 3000));
                                break;
                            case EVENT_ROLLING_BOULDERS_CENTER:
                                me->CastSpell(me, 96828, false);
                                events.ScheduleEvent(RAND(EVENT_ROLLING_BOULDERS_LEFT, EVENT_ROLLING_BOULDERS_RIGHT), urand(2000, 3000));
                                break;
                            case EVENT_ROLLING_BOULDERS_RIGHT:
                                me->CastSpell(me, 96829, false);
                                events.ScheduleEvent(RAND(EVENT_ROLLING_BOULDERS_LEFT, EVENT_ROLLING_BOULDERS_CENTER), urand(2000, 3000));
                                break;
                        }
                    }

                    return;
                }

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_KNOCK_AWAY:
                            DoCastVictim(97616);
                            events.ScheduleEvent(EVENT_KNOCK_AWAY, urand(7000, 14000));
                            break;
                        case EVENT_THUNDERCLAP:
                            DoCast(15588);
                            events.ScheduleEvent(EVENT_THUNDERCLAP, urand(3000, 7000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_berserking_boulder_rollerAI(creature);
    }
};

class npc_boulder : public CreatureScript
{
    public:
        npc_boulder() : CreatureScript("npc_boulder") { }

    private:
        struct npc_boulderAI : public ScriptedAI
        {
            npc_boulderAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset()
            {
                canleap = false;
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_START_MOVING)
                {
                    canleap = true;
                    me->GetNearPoint2D(x, y, 120.0f, me->GetOrientation());
                    me->GetMotionMaster()->MovePoint(POINT_BACK, x, y, 80.5f);
                }
            }

            void SpellHitTarget(Unit* target, SpellInfo const* spell)
            {
                if (canleap)
                    if (spell->Id == 96834)
                        target->GetMotionMaster()->MoveJumpTo(x, y, 81.0f);
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type == POINT_MOTION_TYPE && id == POINT_BACK)
                    events.ScheduleEvent(EVENT_BOULDER_BOOM, 0);
            }

            void UpdateAI(uint32 const diff)
            {
                events.Update(diff);

                if (events.ExecuteEvent() == EVENT_BOULDER_BOOM)
                {
                    me->CastSpell(me, 96836, false);
                    me->DespawnOrUnsummon(1000);
                }
            }

        private:
            float x, y;
            bool canleap;
            EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_boulderAI(creature);
        }
};

class spell_exploding_boulder : public SpellScriptLoader
{
    public:
        spell_exploding_boulder() : SpellScriptLoader("spell_exploding_boulder") { }

    private:
        class spell_exploding_boulder_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_exploding_boulder_SpellScript)

            void CastSpell(SpellEffIndex effIndex)
            {
                GetCaster()->RemoveAura(GetSpellInfo()->Effects[effIndex].BasePoints);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_exploding_boulder_SpellScript::CastSpell, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_exploding_boulder_SpellScript();
        }
};

class BoulderSmashCheck : public std::unary_function<Unit*, bool>
{
    public:
        explicit BoulderSmashCheck(Unit* _caster) : caster(_caster) { }

        bool operator()(WorldObject* object)
        {
            if (!caster)
                return true;

            return !(caster->isInFront(object, 2.5f) && object->GetTypeId() == TYPEID_PLAYER);
        }

    private:
        Unit* caster;
};

class spell_boulder_smash : public SpellScriptLoader
{
    public:
        spell_boulder_smash() : SpellScriptLoader("spell_boulder_smash") { }

    private:
        class spell_boulder_smash_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_boulder_smash_SpellScript);

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                if (!GetCaster()->GetVehicleBase())
                    return;

                unitList.remove_if(BoulderSmashCheck(GetCaster()->GetVehicleBase()));
            }

            void LeapBack(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                if (!GetCaster()->GetVehicleBase() || !GetHitUnit()->ToPlayer())
                    return;

                float speedXY = float(GetSpellInfo()->Effects[effIndex].MiscValue) / 10;
                float speedZ = float(GetSpellInfo()->Effects[effIndex].BasePoints / 10);
                float vcos = std::cos(GetCaster()->GetVehicleBase()->GetOrientation());
                float vsin = std::sin(GetCaster()->GetVehicleBase()->GetOrientation());
                GetHitUnit()->SendMoveKnockBack(GetHitUnit()->ToPlayer(), speedXY, -speedZ, vcos, vsin);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_boulder_smash_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_boulder_smash_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_boulder_smash_SpellScript::FilterTargets, EFFECT_2, TARGET_UNIT_DEST_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_boulder_smash_SpellScript::LeapBack, EFFECT_0, SPELL_EFFECT_LEAP_BACK);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_boulder_smash_SpellScript();
        }
};

enum
{
    EVENT_BONE_VOLLEY           = 1,
};

class npc_tiki_lord_zimwae : public CreatureScript
{
    class ZimwaeEvent : public BasicEvent
    {
        public:
            ZimwaeEvent(Creature* creature) : me(creature) {}

        private:
            bool Execute(uint64 /*time*/, uint32 /*diff*/)
            {
                me->CastSpell(me, 97237, false);
                return true;
            }

        private:
            Creature* me;
    };

    public:
        npc_tiki_lord_zimwae() : CreatureScript("npc_tiki_lord_zimwae") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_tiki_lord_zimwaeAI(creature);
        }

        struct npc_tiki_lord_zimwaeAI : public ScriptedAI
        {
            npc_tiki_lord_zimwaeAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_BONE_VOLLEY, urand(1000, 3000));
            }

            void JustDied(Unit* /*killer*/)
            {
                events.Reset();
                me->RemoveAllAuras();
                me->CastSpell(me, 97236, false);
                me->m_Events.AddEvent(new ZimwaeEvent(me), me->m_Events.CalculateTime(1500));
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (events.ExecuteEvent() == EVENT_BONE_VOLLEY)
                {
                    me->CastSpell((Unit*)NULL, 97339, false);
                    events.ScheduleEvent(EVENT_BONE_VOLLEY, 1000);
                }
            }
        };
};

enum
{
    EVENT_TIKI_TORCH           = 1,
};

class npc_tiki_lord_muloa : public CreatureScript
{
    class MuloaEvent : public BasicEvent
    {
        public:
            MuloaEvent(Creature* creature) : me(creature) {}

        private:
            bool Execute(uint64 /*time*/, uint32 /*diff*/)
            {
                me->CastSpell(me, 97147, false);
                return true;
            }

        private:
            Creature* me;
    };

    public:
        npc_tiki_lord_muloa() : CreatureScript("npc_tiki_lord_muloa") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_tiki_lord_muloaAI(creature);
        }

        struct npc_tiki_lord_muloaAI : public ScriptedAI
        {
            npc_tiki_lord_muloaAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_TIKI_TORCH, 1000);
            }

            void JustDied(Unit* /*killer*/)
            {
                events.Reset();
                me->RemoveAllAuras();
                me->CastSpell(me, 97148, false);
                me->m_Events.AddEvent(new MuloaEvent(me), me->m_Events.CalculateTime(1500));
                std::list<Creature*> stalkers;
                me->GetCreatureListWithEntryInGrid(stalkers, 45979, 15.0f);

                if (stalkers.empty())
                    return;

                for (std::list<Creature*>::const_iterator itr = stalkers.begin(); itr != stalkers.end(); ++itr)
                    (*itr)->RemoveAura(96885);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (events.ExecuteEvent() == EVENT_BONE_VOLLEY)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        me->CastSpell(target, 96822, false);

                    events.ScheduleEvent(EVENT_TIKI_TORCH, 8000);
                }
            }
        };
};

const Position GubFishSP[3]=
{
    {-11869.5f, -1504.78f, 8.93495f, 0.0f},
    {-11858.2f, -1510.84f, 8.10302f, 0.0f},
    {-11852.2f, -1499.59f, 8.80203f, 0.0f},
};

class npc_gub : public CreatureScript
{
    public:
        npc_gub() : CreatureScript("npc_gub") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_gubAI (creature);
        }

        struct npc_gubAI : public ScriptedAI
        {
            npc_gubAI(Creature* creature) : ScriptedAI(creature) { }

            uint64 FishTargetGUID[3];
            bool Frenzy;

            void InitializeAI()
            {
                me->CastSpell(me, 97014, false);

                for (int i = 0; i < 3; ++i)
                    if (Creature* fish_target = me->SummonCreature(45979, GubFishSP[i]))
                    {
                        FishTargetGUID[i] = fish_target->GetGUID();
                        fish_target->CastSpell(me, 97015, false);
                    }

                Reset();
            }

            void Reset()
            {
                Frenzy = false;
            }

            void JustReachedHome()
            {
                me->CastSpell(me, 97014, false);

                for (int i = 0; i < 3; ++i)
                    if (Creature* fish_target = ObjectAccessor::GetCreature(*me, FishTargetGUID[i]))
                        fish_target->CastSpell(me, 97015, false);
            }

            void DamageTaken(Unit* /*attacker*/, uint32 &/*damage*/)
            {
                if (!Frenzy && me->GetHealthPct() <= 35.0f)
                {
                    Frenzy = true;
                    me->CastSpell(me, 81173, false);
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                me->RemoveAllAuras();
            }

            void UpdateAI(uint32 const /*diff*/)
            {
                if (!UpdateVictim())
                    return;

                DoMeleeAttackIfReady();
            }
        };
};

enum
{
    EVENT_POISON_BOLT_VOLLEY        = 1,
};

class npc_chosen_of_hethiss : public CreatureScript
{
    public:
        npc_chosen_of_hethiss() : CreatureScript("npc_chosen_of_hethiss") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_chosen_of_hethissAI (creature);
        }

        struct npc_chosen_of_hethissAI : public ScriptedAI
        {
            npc_chosen_of_hethissAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void InitializeAI()
            {
                JustReachedHome();
                Reset();
            }

            void Reset()
            {
                events.Reset();
            }

            void JustReachedHome()
            {
                me->SetStandState(UNIT_STAND_STATE_KNEEL);
            }

            void EnterCombat(Unit* /*who*/)
            {
                if (!me->HasAura(97019))
                    me->CastSpell(me, 97019, false);

                me->SetStandState(UNIT_STAND_STATE_STAND);
                events.ScheduleEvent(EVENT_POISON_BOLT_VOLLEY, urand(1000, 3000));
            }

            void JustDied(Unit* /*killer*/)
            {
                events.Reset();
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (events.ExecuteEvent() == EVENT_BONE_VOLLEY)
                {
                    if (me->HasAura(97019))
                        me->CastSpell((Unit*)NULL, 97018, false);

                    events.ScheduleEvent(EVENT_POISON_BOLT_VOLLEY, urand(1000, 3000));
                }

                DoMeleeAttackIfReady();
            }
        };
};

class npc_witch_doctor_quin : public CreatureScript
{
    public:
        npc_witch_doctor_quin() : CreatureScript("npc_witch_doctor_quin") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_witch_doctor_quinAI (creature);
        }

        struct npc_witch_doctor_quinAI : public ScriptedAI
        {
            npc_witch_doctor_quinAI(Creature* creature) : ScriptedAI(creature) { }

            void EnterCombat(Unit* /*who*/)
            {
                Talk(0);
                me->CastSpell((Unit*)NULL, 97016, false);
                me->SetTarget(0);
            }
        };
};

enum
{
    EVENT_SHADOW_BOLT_VOLLEY        = 1,
    EVENT_PSYCHIC_SCREAM            = 2,
};

class npc_mortaxx : public CreatureScript
{
    public:
        npc_mortaxx() : CreatureScript("npc_mortaxx") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_mortaxxAI (creature);
        }

        struct npc_mortaxxAI : public ScriptedAI
        {
            npc_mortaxxAI(Creature* creature) : ScriptedAI(creature){}

            EventMap events;

            void Reset()
            {
                me->CastSpell(me, 75965, false);
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                me->RemoveAura(75965);
                events.ScheduleEvent(EVENT_SHADOW_BOLT_VOLLEY, urand(3000, 7000));
                events.ScheduleEvent(EVENT_PSYCHIC_SCREAM, urand(7000, 21000));
            }

            void JustDied(Unit* /*killer*/)
            {
                events.Reset();
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SHADOW_BOLT_VOLLEY:
                            me->CastSpell((Unit*)NULL, 17228, false);
                            events.ScheduleEvent(EVENT_SHADOW_BOLT_VOLLEY, urand(3000, 7000));
                            break;
                        case EVENT_PSYCHIC_SCREAM:
                            me->CastSpell((Unit*)NULL, 34322, false);
                            events.ScheduleEvent(EVENT_PSYCHIC_SCREAM, urand(7000, 21000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

const Position MauriTotemSP[2]=
{
    {-11885.7f, -1319.80f, 78.6447f, 0.523599f},
    {-11886.0f, -1319.92f, 78.6269f, 5.550150f},
};

const Position MauriCauldronSP = {-11885.9f, -1319.81f, 77.9252f, 0.0f};

const uint32 TotemSpell[2]=
{
    96936,
    96937,
};

enum
{
    EVENT_WHISPERS_OF_HETHISS       = 1, // Script for spell implemented in Venoxis script.
    EVENT_POISON_BOLT               = 2,
};

class npc_venomancer_mauri : public CreatureScript
{
    public:
        npc_venomancer_mauri() : CreatureScript("npc_venomancer_mauri") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_venomancer_mauriAI (creature);
        }

        struct npc_venomancer_mauriAI : public ScriptedAI
        {
            npc_venomancer_mauriAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;
            uint64 TotemGUID[2];
            uint64 CauldronGUID;

            void InitializeAI()
            {
                for (int i = 0; i < 2; ++i)
                    if (Creature* totem = me->SummonCreature(45979, MauriTotemSP[i]))
                    {
                        TotemGUID[i] = totem->GetGUID();
                        totem->CastSpell(me, TotemSpell[i], false);
                    }

                if (Creature* cauldron = me->SummonCreature(52529, MauriCauldronSP))
                {
                    CauldronGUID = cauldron->GetGUID();

                    if (me->isAlive())
                    {
                        cauldron->CastSpell(cauldron, 97122, false);
                        cauldron->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    }
                }

                Reset();
            }

            void Reset()
            {
                events.Reset();
            }

            void JustReachedHome()
            {
                for (int i = 0; i < 2; ++i)
                    if (Creature* totem = ObjectAccessor::GetCreature(*me, TotemGUID[i]))
                        totem->CastSpell(me, TotemSpell[i], false);
            }

            void EnterCombat(Unit* /*who*/)
            {
                for (int i = 0; i < 2; ++i)
                    if (Creature* totem = ObjectAccessor::GetCreature(*me, TotemGUID[i]))
                        totem->CastStop();

                Talk(0);
                events.ScheduleEvent(EVENT_WHISPERS_OF_HETHISS, 8000);
                events.ScheduleEvent(EVENT_POISON_BOLT, 5000);
            }

            void JustDied(Unit* /*killer*/)
            {
                Talk(1);
                events.Reset();

                if (Creature* cauldron = ObjectAccessor::GetCreature(*me, CauldronGUID))
                {
                    cauldron->RemoveAura(97122);
                    cauldron->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_WHISPERS_OF_HETHISS:
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                                me->CastSpell(target, 96466, false);

                            events.ScheduleEvent(EVENT_WHISPERS_OF_HETHISS, 15000);
                            break;
                        }
                        case EVENT_POISON_BOLT:
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                                me->CastSpell(target, 96918, false);

                            events.ScheduleEvent(EVENT_POISON_BOLT, 5000);
                            break;
                        }
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

const Position TkuluTotemSP[2]=
{
    {-11962.6f, -1643.23f, 37.5636f, 3.54302f},
    {-11962.6f, -1643.24f, 37.5545f, 3.52556f},
};

const Position TkuluCauldronSP = {-11963.8f, -1643.33f, 36.7988f, 0.0f};

enum
{
    EVENT_TOXIC_LINK                  = 1, // Script for spell implemented in Venoxis script.
};

class npc_venomancer_tkulu : public CreatureScript
{
    public:
        npc_venomancer_tkulu() : CreatureScript("npc_venomancer_tkulu") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_venomancer_tkuluAI (creature);
        }

        struct npc_venomancer_tkuluAI : public ScriptedAI
        {
            npc_venomancer_tkuluAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;
            uint64 TotemGUID[2];
            uint64 CauldronGUID;

            void InitializeAI()
            {
                for (int i = 0; i < 2; ++i)
                    if (Creature* totem = me->SummonCreature(45979, TkuluTotemSP[i]))
                    {
                        TotemGUID[i] = totem->GetGUID();
                        totem->CastSpell(me, TotemSpell[i], false);
                    }

                if (Creature* cauldron = me->SummonCreature(52529, TkuluCauldronSP))
                {
                    CauldronGUID = cauldron->GetGUID();

                    if (me->isAlive())
                    {
                        cauldron->CastSpell(cauldron, 97122, false);
                        cauldron->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    }
                }

                Reset();
            }

            void Reset()
            {
                events.Reset();
            }

            void JustReachedHome()
            {
                for (int i = 0; i < 2; ++i)
                    if (Creature* totem = ObjectAccessor::GetCreature(*me, TotemGUID[i]))
                        totem->CastSpell(me, TotemSpell[i], false);
            }

            void EnterCombat(Unit* /*who*/)
            {
                for (int i = 0; i < 2; ++i)
                    if (Creature* totem = ObjectAccessor::GetCreature(*me, TotemGUID[i]))
                        totem->CastStop();

                Talk(0);
                events.ScheduleEvent(EVENT_TOXIC_LINK, urand(5000, 10000));
                events.ScheduleEvent(EVENT_POISON_BOLT, urand(5000, 10000));
            }

            void JustDied(Unit* /*killer*/)
            {
                Talk(1);
                events.Reset();

                if (Creature* cauldron = ObjectAccessor::GetCreature(*me, CauldronGUID))
                {
                    cauldron->RemoveAura(97122);
                    cauldron->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_TOXIC_LINK:
                            me->CastSpell((Unit*)NULL, 96475, false);
                            events.ScheduleEvent(EVENT_TOXIC_LINK, 10000);
                            break;
                        case EVENT_POISON_BOLT:
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                                    me->CastSpell(target, 96918, false);

                                events.ScheduleEvent(EVENT_POISON_BOLT, urand(5000, 10000));
                            }
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

enum
{
    EVENT_BUBBLING_MIXTURE      = 1,
    EVENT_CRUSH_HERBS           = 2,
    EVENT_GOUT_OF_FLAME         = 3,

    EVENT_DRAIN_BLUE_CAULDRON   = 4,
    EVENT_FROSTBURN_FORMULA     = 5,

    EVENT_DRAIN_RED_CAULDRON    = 6,
    EVENT_DRAGONS_BREATH        = 7,

    EVENT_DRAIN_GREEN_CAULDRON  = 8,
};

struct npc_cauldron_mixerAI : public ScriptedAI
{
    npc_cauldron_mixerAI(Creature* creature) : ScriptedAI(creature) { }

    EventMap events;

    void Reset()
    {
        events.Reset();
    }

    void EnterCombat(Unit* /*who*/)
    {
        events.ScheduleEvent(EVENT_BUBBLING_MIXTURE, urand(7000, 21000));
        events.ScheduleEvent(EVENT_CRUSH_HERBS, urand(7000, 21000));
        events.ScheduleEvent(EVENT_GOUT_OF_FLAME, urand(7000, 21000));
    }

    void JustDied(Unit* /*killer*/)
    {
        events.Reset();
    }

    virtual void DoExtraEvent(uint32 event_Id) = 0;

    void UpdateAI(uint32 const diff)
    {
        events.Update(diff);

        if (!UpdateVictim())
        {
            if (uint32 eventId = events.ExecuteEvent())
                DoExtraEvent(eventId);

            return;
        }

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_BUBBLING_MIXTURE:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                            me->CastSpell(target, 96804, false);

                        events.ScheduleEvent(EVENT_BUBBLING_MIXTURE, urand(7000, 21000));
                    }
                    break;
                case EVENT_CRUSH_HERBS:
                    me->CastSpell(me, 97386, false);
                    events.ScheduleEvent(EVENT_CRUSH_HERBS, urand(7000, 21000));
                    break;
                case EVENT_GOUT_OF_FLAME:
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                            me->CastSpell(target, 96413, false);

                        events.ScheduleEvent(EVENT_GOUT_OF_FLAME, urand(7000, 21000));
                    }
                    break;
                default:
                    DoExtraEvent(eventId);
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }
};

class npc_gurubashi_cauldron_mixer_frost : public CreatureScript
{
    public:
        npc_gurubashi_cauldron_mixer_frost() : CreatureScript("npc_gurubashi_cauldron_mixer_frost") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_gurubashi_cauldron_mixer_frostAI (creature);
        }

        struct npc_gurubashi_cauldron_mixer_frostAI : public npc_cauldron_mixerAI
        {
            npc_gurubashi_cauldron_mixer_frostAI(Creature* creature) : npc_cauldron_mixerAI(creature) { }

            void Reset()
            {
                npc_cauldron_mixerAI::Reset();
                events.ScheduleEvent(EVENT_DRAIN_BLUE_CAULDRON, urand(3000, 7000));
            }

            void EnterCombat(Unit* who)
            {
                events.Reset();
                events.ScheduleEvent(EVENT_FROSTBURN_FORMULA, urand(3000, 7000));
                npc_cauldron_mixerAI::EnterCombat(who);
            }

            void DoExtraEvent(uint32 event_Id)
            {
                switch (event_Id)
                {
                    case EVENT_DRAIN_BLUE_CAULDRON:
                        me->CastSpell((Unit*)NULL, 96488, false);
                        events.ScheduleEvent(EVENT_DRAIN_BLUE_CAULDRON, urand(7000, 14000));
                        break;
                    case EVENT_FROSTBURN_FORMULA:
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_NEAREST, 0, 50.0f, true))
                                me->CastSpell(target, 96449, false);

                            events.ScheduleEvent(EVENT_FROSTBURN_FORMULA, urand(7000, 14000));
                        }
                        break;
                }
            }
        };
};

class npc_gurubashi_cauldron_mixer_fire : public CreatureScript
{
    public:
        npc_gurubashi_cauldron_mixer_fire() : CreatureScript("npc_gurubashi_cauldron_mixer_fire") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_gurubashi_cauldron_mixer_fireAI (creature);
        }

        struct npc_gurubashi_cauldron_mixer_fireAI : public npc_cauldron_mixerAI
        {
            npc_gurubashi_cauldron_mixer_fireAI(Creature* creature) : npc_cauldron_mixerAI(creature) { }

            void Reset()
            {
                npc_cauldron_mixerAI::Reset();
                events.ScheduleEvent(EVENT_DRAIN_RED_CAULDRON, urand(3000, 7000));
            }

            void EnterCombat(Unit* who)
            {
                events.Reset();
                events.ScheduleEvent(EVENT_DRAGONS_BREATH, urand(3000, 7000));
                npc_cauldron_mixerAI::EnterCombat(who);
            }

            void DoExtraEvent(uint32 event_Id)
            {
                switch (event_Id)
                {
                    case EVENT_DRAIN_RED_CAULDRON:
                        me->CastSpell((Unit*)NULL, 96486, false);
                        events.ScheduleEvent(EVENT_DRAIN_RED_CAULDRON, urand(7000, 14000));
                        break;
                    case EVENT_DRAGONS_BREATH:
                        me->CastSpell((Unit*)NULL, 96447, false);
                        events.ScheduleEvent(EVENT_DRAGONS_BREATH, urand(7000, 14000));
                        break;
                }
            }
        };
};

class npc_gurubashi_cauldron_mixer_poison : public CreatureScript
{
    public:
        npc_gurubashi_cauldron_mixer_poison() : CreatureScript("npc_gurubashi_cauldron_mixer_poison") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_gurubashi_cauldron_mixer_poisonAI (creature);
        }

        struct npc_gurubashi_cauldron_mixer_poisonAI : public npc_cauldron_mixerAI
        {
            npc_gurubashi_cauldron_mixer_poisonAI(Creature* creature) : npc_cauldron_mixerAI(creature) { }

            void Reset()
            {
                npc_cauldron_mixerAI::Reset();
                events.ScheduleEvent(EVENT_DRAIN_GREEN_CAULDRON, urand(3000, 7000));
            }

            void EnterCombat(Unit* who)
            {
                events.Reset();
                me->CastSpell(me, 96456, false);
                npc_cauldron_mixerAI::EnterCombat(who);
            }

            void DoExtraEvent(uint32 event_Id)
            {
                if (event_Id== EVENT_DRAIN_GREEN_CAULDRON)
                {
                    me->CastSpell((Unit*)NULL, 96487, false);
                    events.ScheduleEvent(EVENT_DRAIN_GREEN_CAULDRON, urand(7000, 14000));
                }
            }
        };
};

class npc_gurubashi_cauldron_mixer : public CreatureScript
{
    public:
        npc_gurubashi_cauldron_mixer() : CreatureScript("npc_gurubashi_cauldron_mixer") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_gurubashi_cauldron_mixerAI (creature);
        }

        struct npc_gurubashi_cauldron_mixerAI : public npc_cauldron_mixerAI
        {
            npc_gurubashi_cauldron_mixerAI(Creature* creature) : npc_cauldron_mixerAI(creature) { }

            void DoExtraEvent(uint32 /*eventId*/) { }
        };
};

enum
{
    EVENT_POOL_OF_ACRID_TEARS           = 1,
    EVENT_BREATH_OF_HETHISS             = 2,
    EVENT_RETURN_TO_FIGHT               = 3,
    EVENT_CAULDRON_NEUTRALIZER          = 4,
};

class npc_venomguard_destroyer : public CreatureScript
{
    public:
        npc_venomguard_destroyer() : CreatureScript("npc_venomguard_destroyer") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_venomguard_destroyerAI (creature);
        }

        struct npc_venomguard_destroyerAI : public ScriptedAI
        {
            npc_venomguard_destroyerAI(Creature* creature) : ScriptedAI(creature), summons(creature) { }

            EventMap events;
            SummonList summons;

            void Reset()
            {
                events.Reset();
                summons.DespawnAll();
                me->SetReactState(REACT_AGGRESSIVE);
            }

            void JustSummoned(Creature* summoned)
            {
                if (summoned->GetEntry() == 52320)
                {
                    summoned->SetReactState(REACT_PASSIVE);
                    summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                    summoned->CastSpell(summoned, 96756, true);
                }

                summons.Summon(summoned);
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_POOL_OF_ACRID_TEARS, urand(3000, 7000));
                events.ScheduleEvent(EVENT_BREATH_OF_HETHISS, urand(4000, 8000));
                events.ScheduleEvent(EVENT_CAULDRON_NEUTRALIZER, urand(5000, 10000));
            }

            void JustDied(Unit* /*killer*/)
            {
                events.Reset();
                summons.DespawnAll();
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_POOL_OF_ACRID_TEARS:
                            me->CastSpell(me, 96754, false);
                            events.ScheduleEvent(EVENT_POOL_OF_ACRID_TEARS, urand(3000, 7000));
                            break;
                        case EVENT_BREATH_OF_HETHISS:
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 10.0f, true))
                            {
                                me->SetReactState(REACT_PASSIVE);
                                me->AttackStop();
                                me->StopMoving();
                                me->SetFacingTo(me->GetAngle(target));
                                me->CastSpell(target, 96753, false);
                                events.ScheduleEvent(EVENT_RETURN_TO_FIGHT, 3000);
                            }
                            events.ScheduleEvent(EVENT_BREATH_OF_HETHISS, urand(4000, 8000));
                            break;
                        }
                        case EVENT_RETURN_TO_FIGHT:
                            me->SetReactState(REACT_AGGRESSIVE);
                            break;
                        case EVENT_CAULDRON_NEUTRALIZER:
                            me->CastSpell((Unit*)NULL, 97337, false);
                            events.ScheduleEvent(EVENT_CAULDRON_NEUTRALIZER, urand(5000, 15000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

enum
{
    EVENT_SHOOT_VISUAL          = 1,
    EVENT_SHOOT                 = 2,
    EVENT_SHADOWED_SHOT         = 3,
};

class npc_gurubashi_shadow_hunter : public CreatureScript
{
    public:
        npc_gurubashi_shadow_hunter() : CreatureScript("npc_gurubashi_shadow_hunter") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_gurubashi_shadow_hunterAI (creature);
        }

        struct npc_gurubashi_shadow_hunterAI : public ScriptedAI
        {
            npc_gurubashi_shadow_hunterAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void Reset()
            {
                events.Reset();

                if (me->GetUInt32Value(UNIT_NPC_EMOTESTATE) == 376)
                    events.ScheduleEvent(EVENT_SHOOT_VISUAL, urand(5000, 15000));
            }

            void AttackStart(Unit* who)
            {
                if (!who)
                    return;

                me->Attack(who, false);
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.Reset();
                events.ScheduleEvent(EVENT_SHOOT, 500);
                events.ScheduleEvent(EVENT_SHADOWED_SHOT, urand(5000, 15000));
            }

            void JustDied(Unit* /*killer*/)
            {
                events.Reset();
            }

            void UpdateAI(uint32 const diff)
            {
                events.Update(diff);

                if (!UpdateVictim())
                {
                    if (events.ExecuteEvent() == EVENT_SHOOT_VISUAL)
                    {
                        me->CastSpell((Unit*)NULL, 97378, false);
                        events.ScheduleEvent(EVENT_SHOOT_VISUAL, urand(5000, 15000));
                    }

                    return;
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SHOOT:
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                                    me->CastSpell(target, 97239, false);
                                else
                                    ScriptedAI::EnterEvadeMode();

                                events.ScheduleEvent(EVENT_SHOOT, 1500);
                            }
                            break;
                        case EVENT_SHADOWED_SHOT:
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                                    me->CastSpell(target, 96767, false);
                                else
                                    ScriptedAI::EnterEvadeMode();

                                events.ScheduleEvent(EVENT_SHADOWED_SHOT, urand(5000, 15000));
                            }
                            break;
                    }
                }
            }
        };
};

enum
{
    EVENT_FULL_OF_MEAT          = 1,
    EVENT_FISH_SLAP             = 2,
    EVENT_SLAP_CHOP             = 3,
};

class npc_gurubashi_master_chef : public CreatureScript
{
    public:
        npc_gurubashi_master_chef() : CreatureScript("npc_gurubashi_master_chef") { }

    private:
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_gurubashi_master_chefAI (creature);
        }

        struct npc_gurubashi_master_chefAI : public ScriptedAI
        {
            npc_gurubashi_master_chefAI(Creature* creature) : ScriptedAI(creature){}

            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                events.ScheduleEvent(EVENT_FULL_OF_MEAT, urand(4000, 8000));
                events.ScheduleEvent(EVENT_FISH_SLAP, urand(3000, 7000));
                events.ScheduleEvent(EVENT_SLAP_CHOP, urand(7000, 14000));
            }

            void JustDied(Unit* /*killer*/)
            {
                events.Reset();
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FULL_OF_MEAT:
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true, -81252))
                                    me->CastSpell(target, 81252, false);

                                events.ScheduleEvent(EVENT_FULL_OF_MEAT, urand(4000, 8000));
                            }
                            break;
                        case EVENT_FISH_SLAP:
                            me->CastSpell(me->getVictim(), 79554, false);
                            events.ScheduleEvent(EVENT_FISH_SLAP, urand(3000, 7000));
                            break;
                        case EVENT_SLAP_CHOP:
                            me->CastSpell(me->getVictim(), 79175, false);
                            events.ScheduleEvent(EVENT_SLAP_CHOP, urand(10000, 20000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

class spell_slap_chop : public SpellScriptLoader
{
    public:
        spell_slap_chop() : SpellScriptLoader("spell_slap_chop") { }

    private:
        class spell_slap_chop_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_slap_chop_AuraScript)

            void HandleTick(AuraEffect const* /*aurEff*/)
            {
                // TODO: this is hack, need to remove, after trigger spell with target victim will cast on it
                PreventDefaultAction();
                GetCaster()->CastSpell(GetCaster()->getVictim(), GetSpellInfo()->Effects[EFFECT_1].TriggerSpell, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_slap_chop_AuraScript::HandleTick, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript *GetAuraScript() const
        {
            return new spell_slap_chop_AuraScript();
        }
};

void AddSC_zulgurub()
{
    new npc_tiki_torch();
    new npc_berserking_boulder_roller();
    new npc_boulder();
    new npc_tiki_lord_zimwae();
    new npc_tiki_lord_muloa();
    new npc_gub();
    new npc_chosen_of_hethiss();
    new npc_witch_doctor_quin();
    new npc_mortaxx();
    new npc_venomancer_mauri();
    new npc_venomancer_tkulu();
    new npc_gurubashi_cauldron_mixer_frost();
    new npc_gurubashi_cauldron_mixer_fire();
    new npc_gurubashi_cauldron_mixer_poison();
    new npc_gurubashi_cauldron_mixer();
    new npc_venomguard_destroyer();
    new npc_gurubashi_shadow_hunter();
    new npc_gurubashi_master_chef();

    new spell_exploding_boulder();
    new spell_boulder_smash();
    new spell_slap_chop();
}
