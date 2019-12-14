/*
 * Copyright (C) 2013-2014 trinity core og
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

#include "GameObjectAI.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "gilneas.h"

class npc_gilneas_city_guard_phase2 : public CreatureScript
{
public:
    npc_gilneas_city_guard_phase2() : CreatureScript("npc_gilneas_city_guard_phase2") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilneas_city_guard_phase2AI (creature);
    }

    struct npc_gilneas_city_guard_phase2AI : public ScriptedAI
    {
        npc_gilneas_city_guard_phase2AI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            tAnimate   = DELAY_ANIMATE;
            dmgCount   = 0;
            tSound     = DELAY_SOUND;
            playSound  = false;
            tSeek      = urand(1000, 2000);
        }

        void DamageTaken(Unit* who, uint32 &Damage)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
                dmgCount = 0;
            }
            else if (who->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
                dmgCount = 0;
            }
        }

        void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
        {
            if (target->GetEntry() == NPC_RAMPAGING_WORGEN_1)
                dmgCount ++;
        }

        void UpdateAI(const uint32 diff)
        {
            if (tSeek <= diff)
            {
                if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_RAMPAGING_WORGEN_1, 16.0f, true))
                        me->AI()->AttackStart(enemy);
                tSeek = urand(1000, 2000);
            }
            else tSeek -= diff;

            if (!UpdateVictim())
                return;

            if (tSound <= diff)
            {
                me->PlayDistanceSound(SOUND_SWORD_FLESH);
                tSound = DELAY_SOUND;
                playSound = false;
            }

            if (playSound == true)
                tSound -= diff;

            if (dmgCount < 2)
                DoMeleeAttackIfReady();
            else
                if (me->getVictim()->GetTypeId() == TYPEID_PLAYER) dmgCount = 0;
            else
                if (me->getVictim()->isPet()) dmgCount = 0;
            else
            {
                if (tAnimate <= diff)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
                    playSound = true;
                    tAnimate = DELAY_ANIMATE;
                }
                else
                    tAnimate -= diff;
            }
        }

    private :
        uint32 tAnimate;
        uint32 tSound;
        uint32 dmgCount;
        uint32 tSeek;
        bool playSound;
    };
};

class npc_prince_liam_greymane_phase2 : public CreatureScript
{
public:
    npc_prince_liam_greymane_phase2() : CreatureScript("npc_prince_liam_greymane_phase2") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_prince_liam_greymane_phase2AI (creature);
    }

    struct npc_prince_liam_greymane_phase2AI : public ScriptedAI
    {
        npc_prince_liam_greymane_phase2AI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            tAnimate  = DELAY_ANIMATE;
            dmgCount  = 0;
            tSound    = DELAY_SOUND;
            playSound = false;
            tSeek     = urand(1000, 2000);
            doYell    = true;
            tYell     = DELAY_YELL_PRINCE_LIAM_GREYMANE;
        }

        void DamageTaken(Unit * who, uint32 &Damage)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
                dmgCount = 0;
            }
            else if (who->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
                dmgCount = 0;
            }
        }

        void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
        {
            if (target->GetEntry() == NPC_RAMPAGING_WORGEN_1)
                dmgCount ++;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
            {
                if (tSeek <= diff)
                {
                    if (me->isAlive() && !me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f))
                        if (Creature* enemy = me->FindNearestCreature(NPC_RAMPAGING_WORGEN_1, 16.0f, true))
                            me->AI()->AttackStart(enemy);
                    tSeek = urand(1000, 2000);
                }
                else tSeek -= diff;
            }
            else
            {
                if (tSound <= diff)
                {
                    me->PlayDistanceSound(SOUND_SWORD_FLESH);
                    tSound = DELAY_SOUND;
                    playSound = false;
                }

                if (playSound == true)
                    tSound -= diff;

                if (dmgCount < 2)
                    DoMeleeAttackIfReady();
                else
                    if (me->getVictim()->GetTypeId() == TYPEID_PLAYER) dmgCount = 0;
                else
                    if (me->getVictim()->isPet()) dmgCount = 0;
                else
                {
                    if (tAnimate <= diff)
                    {
                        me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
                        playSound = true;
                        tAnimate = DELAY_ANIMATE;
                    }
                    else
                        tAnimate -= diff;
                }
                doYell = false;
            }
        }

    private :
        uint32 tAnimate;
        uint32 tSound;
        uint32 dmgCount;
        uint32 tYell;
        uint32 tSeek;
        bool playSound;
        bool doYell;
    };
};

class npc_rampaging_worgen : public CreatureScript
{
public:
    npc_rampaging_worgen() : CreatureScript("npc_rampaging_worgen") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rampaging_worgenAI (creature);
    }

    struct npc_rampaging_worgenAI : public ScriptedAI
    {
        npc_rampaging_worgenAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tEnrage;
        uint32 dmgCount;
        uint32 tAnimate;
        uint32 tSound;
        bool playSound, willCastEnrage;

        void Reset()
        {
            tEnrage    = 0;
            dmgCount   = 0;
            tAnimate   = DELAY_ANIMATE;
            tSound     = DELAY_SOUND;
            playSound  = false;
            willCastEnrage = urand(0, 1);
        }

        void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
        {
            if (target->GetEntry() == NPC_GILNEAS_CITY_GUARD || target->GetEntry() == NPC_PRINCE_LIAM_GREYMANE)
                dmgCount++;
        }

        void DamageTaken(Unit* who, uint32 &Damage)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
                dmgCount = 0;
            }
            else if (who->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
                dmgCount = 0;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (tEnrage <= diff && willCastEnrage)
            {
                if (me->GetHealthPct() <= 30)
                {
                    me->MonsterTextEmote(-106, 0);
                    DoCast(me, SPELL_ENRAGE);
                    tEnrage = CD_ENRAGE;
                }
            }
            else tEnrage -= diff;

            if (playSound == true)
                tSound -= diff;

            if (tSound <= diff)
            {
                me->PlayDistanceSound(SOUND_SWORD_PLATE);
                tSound = DELAY_SOUND;
                playSound = false;
            }

            if (dmgCount < 2)
                DoMeleeAttackIfReady();
            else
                if (me->getVictim()->GetTypeId() == TYPEID_PLAYER)
                    dmgCount = 0;
            else
                if (me->getVictim()->isPet())
                    dmgCount = 0;
            else
            {
                if (tAnimate <= diff)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_UNARMED);
                    tAnimate = DELAY_ANIMATE;
                    playSound = true;
                }
                else
                    tAnimate -= diff;
            }
        }
    };
};

class npc_rampaging_worgen2 : public CreatureScript
{
public:
    npc_rampaging_worgen2() : CreatureScript("npc_rampaging_worgen2") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rampaging_worgen2AI (creature);
    }

    struct npc_rampaging_worgen2AI : public ScriptedAI
    {
        npc_rampaging_worgen2AI(Creature* creature) : ScriptedAI(creature) {}

        void JustRespawned()
        {
            tEnrage = 0;
            tRun = 500;
            onceRun = true;
            x = me->m_positionX + cos(me->m_orientation)*8;
            y = me->m_positionY + sin(me->m_orientation)*8;
            z = me->m_positionZ;
            willCastEnrage = urand(0, 1);
        }

        void UpdateAI(const uint32 diff)
        {
            if (tRun <= diff && onceRun)
            {
                me->GetMotionMaster()->MoveCharge(x, y, z, 8);
                onceRun = false;
            }
            else
                tRun -= diff;

            if (!UpdateVictim())
                return;

            if (tEnrage <= diff)
            {
                if (me->GetHealthPct() <= 30 && willCastEnrage)
                {
                    me->MonsterTextEmote(-106, 0);
                    DoCast(me, SPELL_ENRAGE);
                    tEnrage = CD_ENRAGE;
                }
            }
            else
                tEnrage -= diff;

            DoMeleeAttackIfReady();
        }

    private :
        uint16 tRun;
        uint16 tEnrage;
        bool onceRun;
        bool willCastEnrage;
        float x, y, z;
    };
};

class go_merchant_square_door : public GameObjectScript
{
public:
    go_merchant_square_door() : GameObjectScript("go_merchant_square_door") {}

    struct go_merchant_square_doorAI : public GameObjectAI
    {
        go_merchant_square_doorAI(GameObject* gameobject) : GameObjectAI(gameobject)
        {
            x = 0; y = 0; z = 0; wx = 0; wy = 0; angle = 0;
            opened = false;
            spawnKind = 0;
            DoorTimer = 1000;
            playerGUID = 0;
        }

        void UpdateAI(uint32 diff)
        {
            if (opened)
            {
                if (tQuestCredit <= diff)
                {
                    opened = false;

                    if (Player *player = Player::GetPlayer(*go, playerGUID))
                        player->CastedCreatureOrGO(195327, 0, 0);

                    if (spawnKind == 3)
                    {
                        if (Creature* spawnedCreature = go->SummonCreature(NPC_RAMPAGING_WORGEN_2, wx, wy, z, angle, TEMPSUMMON_TIMED_DESPAWN, SUMMON1_TTL))
                        {
                            spawnedCreature->SetPhaseMask(6, 1);
                            spawnedCreature->SetReactState(REACT_AGGRESSIVE);
                            if (Player *player = Player::GetPlayer(*go, playerGUID))
                                spawnedCreature->AI()->AttackStart(player);
                        }
                    }
                }
                else tQuestCredit -= diff;
            }
            if (DoorTimer <= diff)
            {
                if (go->GetGoState() == GO_STATE_ACTIVE)
                    go->SetGoState(GO_STATE_READY);
                if (Creature *worgen = go->FindNearestCreature(NPC_RAMPAGING_WORGEN_2, 10))
                    worgen->DespawnOrUnsummon(1);
                DoorTimer = DOOR_TIMER;
            }
            else
                DoorTimer -= diff;
        }

        bool GossipHello(Player* player)
        {
            if (player->GetQuestStatus(QUEST_EVAC_MERC_SQUA) == QUEST_STATUS_INCOMPLETE && go->GetGoState() == GO_STATE_READY)
            {
                playerGUID          = player->GetGUID();
                opened           = true;
                tQuestCredit     = 2500;
                go->SetGoState(GO_STATE_ACTIVE);
                DoorTimer = DOOR_TIMER;
                spawnKind = urand(1, 3);
                angle = go->GetOrientation();
                x = go->GetPositionX() - cos(angle) * 2;
                y = go->GetPositionY() - sin(angle) * 2;
                z = go->GetPositionZ();
                wx = x - cos(angle) * 2;
                wy = y - sin(angle) * 2;

                if (spawnKind < 3)
                {
                    if (Creature* spawnedCreature = go->SummonCreature(NPC_FRIGHTENED_CITIZEN_1, x, y, z, angle, TEMPSUMMON_MANUAL_DESPAWN))
                    {
                        spawnedCreature->SetPhaseMask(6, 1);
                        spawnedCreature->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
                        spawnedCreature->GetMotionMaster()->MovePoint(42, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());
                    }
                }
                else
                {
                    if (Creature* spawnedCreature = go->SummonCreature(NPC_FRIGHTENED_CITIZEN_2, x, y, z, angle, TEMPSUMMON_MANUAL_DESPAWN))
                    {
                        spawnedCreature->SetPhaseMask(6, 1);
                        spawnedCreature->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
                        spawnedCreature->GetMotionMaster()->MovePoint(42, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());
                    }
                }
                return true;
            }
            return false;
        }

    private:
        float x, y, z, wx, wy, angle, tQuestCredit;
        bool opened;
        uint8 spawnKind;
        uint64 playerGUID;
        uint32 DoorTimer;
    };

    GameObjectAI* GetAI(GameObject* gameobject) const
    {
        return new go_merchant_square_doorAI(gameobject);
    }
};

class npc_frightened_citizen : public CreatureScript
{
public:
    npc_frightened_citizen() : CreatureScript("npc_frightened_citizen") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_frightened_citizenAI (creature);
    }

    struct npc_frightened_citizenAI : public ScriptedAI
    {
        npc_frightened_citizenAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            startPath = false;
            mui_moveTimer = 1000;
        }

        void MovementInform(uint32 type, uint32 point)
        {
            if (point == 42)
            {
                startPath = true;
                Talk(0, 0);
            }
            else if (point == 5)
                me->DespawnOrUnsummon(1);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!startPath)
                return;

            if (mui_moveTimer <= diff)
            {
                me->ClearUnitState(UNIT_STATE_IGNORE_PATHFINDING);
                me->GetMotionMaster()->MovePoint(5, FrightenedWay[urand(0, 1)]);
                mui_moveTimer = 10000;
            }
            else
                mui_moveTimer -= diff;
        }

    private :
        bool startPath;
        uint32 mui_moveTimer;
    };
};

class npc_bloodfang_worgen : public CreatureScript
{
public:
    npc_bloodfang_worgen() : CreatureScript("npc_bloodfang_worgen") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodfang_worgenAI (creature);
    }

    struct npc_bloodfang_worgenAI : public ScriptedAI
    {
        npc_bloodfang_worgenAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            tEnrage    = 0;
            dmgCount   = 0;
            tAnimate   = DELAY_ANIMATE;
            tSound     = DELAY_SOUND;
            playSound  = false;
            willCastEnrage = urand(0, 1);
        }

        void DamageDealt(Unit* target, uint32 &damage, DamageEffectType damageType)
        {
            if (target->GetEntry() == NPC_GILNEAN_ROYAL_GUARD || target->GetEntry() == NPC_SERGEANT_CLEESE || target->GetEntry() == NPC_MYRIAM_SPELLWALKER)
                dmgCount++;
        }

        void DamageTaken(Unit* who, uint32 &Damage)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
                dmgCount = 0;
            }
            else if (who->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
                dmgCount = 0;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (tEnrage <= diff && willCastEnrage)
            {
                if (me->GetHealthPct() <= 30)
                {
                    me->MonsterTextEmote(-106, 0);
                    DoCast(me, SPELL_ENRAGE);
                    tEnrage = CD_ENRAGE;
                }
            }
            else tEnrage -= diff;

            if (playSound == true)
                tSound -= diff;

            if (tSound <= diff)
            {
                me->PlayDistanceSound(SOUND_SWORD_PLATE);
                tSound = DELAY_SOUND;
                playSound = false;
            }

            if (dmgCount < 2)
                DoMeleeAttackIfReady();
            else
                if (me->getVictim()->GetTypeId() == TYPEID_PLAYER)
                    dmgCount = 0;
            else
                if (me->getVictim()->isPet())
                    dmgCount = 0;
            else
            {
                if (tAnimate <= diff)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_UNARMED);
                    tAnimate = DELAY_ANIMATE;
                    playSound = true;
                }
                else
                    tAnimate -= diff;
            }
        }

    private :
        uint32 tEnrage;
        uint32 dmgCount;
        uint32 tAnimate;
        uint32 tSound;
        bool playSound;
        bool willCastEnrage;

    };
};

class npc_sergeant_cleese : public CreatureScript
{
public:
    npc_sergeant_cleese() : CreatureScript("npc_sergeant_cleese") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_sergeant_cleeseAI (creature);
    }

    struct npc_sergeant_cleeseAI : public ScriptedAI
    {
        npc_sergeant_cleeseAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            tAnimate   = DELAY_ANIMATE;
            dmgCount   = 0;
            tSound     = DELAY_SOUND;
            playSound  = false;
            tSeek      = urand(1000, 2000);
        }

        void DamageTaken(Unit* who, uint32 &Damage)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
                dmgCount = 0;
            }
            else if (who->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
                dmgCount = 0;
            }
        }

        void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
        {
            if (target->GetEntry() == NPC_BLOODFANG_WORGEN)
                dmgCount ++;
        }

        void UpdateAI(const uint32 diff)
        {
            if (tSeek <= diff)
            {
                if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 10.0f, true))
                        me->AI()->AttackStart(enemy);
                tSeek = urand(1000, 2000);
            }
            else tSeek -= diff;

            if (!UpdateVictim())
                return;

            if (tSound <= diff)
            {
                me->PlayDistanceSound(SOUND_SWORD_FLESH);
                tSound = DELAY_SOUND;
                playSound = false;
            }

            if (playSound == true)
                tSound -= diff;

            if (dmgCount < 2)
                DoMeleeAttackIfReady();
            else
                if (me->getVictim()->GetTypeId() == TYPEID_PLAYER) dmgCount = 0;
            else
                if (me->getVictim()->isPet()) dmgCount = 0;
            else
            {
                if (tAnimate <= diff)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
                    playSound = true;
                    tAnimate = DELAY_ANIMATE;
                }
                else
                    tAnimate -= diff;
            }
        }

    private :
        uint32 tAnimate;
        uint32 tSound;
        uint32 dmgCount;
        uint32 tSeek;
        bool playSound;

    };
};

class npc_gilnean_royal_guard : public CreatureScript
{
public:
    npc_gilnean_royal_guard() : CreatureScript("npc_gilnean_royal_guard") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilnean_royal_guardAI (creature);
    }

    struct npc_gilnean_royal_guardAI : public ScriptedAI
    {
        npc_gilnean_royal_guardAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            tAnimate   = DELAY_ANIMATE;
            dmgCount   = 0;
            tSound     = DELAY_SOUND;
            playSound  = false;
            tSeek      = urand(1000, 2000);
        }

        void DamageTaken(Unit* who, uint32 &Damage)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
                dmgCount = 0;
            }
            else if (who->isPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
                dmgCount = 0;
            }
        }

        void DamageDealt(Unit* target, uint32 &damage, DamageEffectType damageType)
        {
            if (target->GetEntry() == NPC_BLOODFANG_WORGEN)
                dmgCount ++;
        }

        void UpdateAI(const uint32 diff)
        {
            if (tSeek <= diff)
            {
                if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 16.0f, true))
                        me->AI()->AttackStart(enemy);
                tSeek = urand(1000, 2000);
            }
            else tSeek -= diff;

            if (!UpdateVictim())
                return;

            if (tSound <= diff)
            {
                me->PlayDistanceSound(SOUND_SWORD_FLESH);
                tSound = DELAY_SOUND;
                playSound = false;
            }

            if (playSound == true)
                tSound -= diff;

            if (dmgCount < 2)
                DoMeleeAttackIfReady();
            else
                if (me->getVictim()->GetTypeId() == TYPEID_PLAYER) dmgCount = 0;
            else
                if (me->getVictim()->isPet()) dmgCount = 0;
            else
            {
                if (tAnimate <= diff)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
                    playSound = true;
                    tAnimate = DELAY_ANIMATE;
                }
                else
                    tAnimate -= diff;
            }
        }

    private :
        uint32 tAnimate;
        uint32 tSound;
        uint32 dmgCount;
        uint32 tSeek;
        bool playSound;

    };
};

class king_genn_1 : public CreatureScript
{
public:
    king_genn_1() : CreatureScript("king_genn_1") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const*_Quest)
    {
        if (_Quest->GetQuestId() == QUEST_SAVE_KRENNAN_ARANAS)
        {
            if (Creature *horse = player->SummonCreature(35905, player->GetPositionX(), player->GetPositionY(),  player->GetPositionZ(),  player->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
            {
                player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, horse, false);
                CAST_AI(npc_escortAI, (horse->AI()))->Start(false, true, player->GetGUID(), _Quest);
            }
        }
        return true;
    }
};

class npc_greymane_horse : public CreatureScript
{
public:
    npc_greymane_horse() : CreatureScript("npc_greymane_horse") { }

    struct npc_greymane_horseAI : public npc_escortAI
    {
        npc_greymane_horseAI(Creature* creature) : npc_escortAI(creature) {}

        void AttackStart(Unit* /*who*/) {}

        void EnterEvadeMode() {}

        void Reset()
        {
             krennansay = 500;
             AfterJump = false;
             me->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
             loop = true;
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (who->GetTypeId() != TYPEID_PLAYER)
                if (apply)
                    loop = false;
            if (!apply)
            {
                if (who->GetTypeId() != TYPEID_PLAYER)
                {
                    who->ToCreature()->AI()->Talk(0, 0);
                    me->DespawnOrUnsummon(1);
                }
                else if (loop)
                    me->DespawnOrUnsummon(1);
            }
        }

        void MovementInform(uint32 type, uint32 point)
        {
            npc_escortAI::MovementInform(type, point);

            if (type == EFFECT_MOTION_TYPE)
                SetEscortPaused(false);
        }

        void WaypointReached(uint32 i)
        {
            Player* player = GetPlayerForEscort();

            switch(i)
            {
                case 5:
                    me->GetMotionMaster()->MoveJump(-1679.089f,1348.42f,15.31f,25.0f, 15.0f);
                    AfterJump = true;
                    if (me->GetVehicleKit()->HasEmptySeat(1))
                    {
                        SetEscortPaused(true);
                        player->SetClientControl(me, 0);
                        break;
                    }
                    else
                    break;
                case 9:
                    if (loop)
                        SetNextWaypoint(6, false, true);
                    break;
                case 13:
                    player->ExitVehicle();
                    player->SetClientControl(me, 1);
                    break;
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            if (Player* player = GetPlayerForEscort())
               player->FailQuest(QUEST_SAVE_KRENNAN_ARANAS);
        }

        void OnCharmed(bool /*apply*/)
        {
        }

        void UpdateAI(const uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);
            Player* player = GetPlayerForEscort();

            if (AfterJump && (me->IsWithinDist3d(-1679.089f, 1348.42f, 15.31f, 1.0f)))
            {
                Talk(0,player->GetGUID());
                AfterJump = false;
            }

            if (krennansay <=diff)
            {
                if (Creature *krennan = me->FindNearestCreature(3871227, 30, true))
                    krennansay = urand(4000,7000);
            }
            else
                krennansay -= diff;
        }

    private:
        uint32 krennansay;
        bool AfterJump;
        bool loop;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_greymane_horseAI (creature);
    }
};

class npc_lord_darius_crowley_c2 : public CreatureScript
{
public:
    npc_lord_darius_crowley_c2() : CreatureScript("npc_lord_darius_crowley_c2") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const*_Quest)
    {
        if (_Quest->GetQuestId() == QUEST_SACRIFICES)
        {
            if (Creature *horse = player->SummonCreature(35231, player->GetPositionX(), player->GetPositionY(),  player->GetPositionZ(),  player->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
            {
                player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, horse, false);
                CAST_AI(npc_escortAI, (horse->AI()))->Start(false, true, player->GetGUID(), _Quest);
            }
        }
        return true;
    }
};

class npc_crowley_horse : public CreatureScript
{
public:
    npc_crowley_horse() : CreatureScript("npc_crowley_horse") {}

    struct npc_crowley_horseAI : public npc_escortAI
    {
        npc_crowley_horseAI(Creature* creature) : npc_escortAI(creature) {}

        bool CrowleyOn;
        bool CrowleySpawn;
        bool Run;

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()
        {
            CrowleyOn = false;
            CrowleySpawn = false;
            Run = false;
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (!apply)
                me->DespawnOrUnsummon(1);
        }

        void MovementInform(uint32 type, uint32 point)
        {
            npc_escortAI::MovementInform(type, point);

            if (type == EFFECT_MOTION_TYPE)
                SetEscortPaused(false);
        }

        void WaypointReached(uint32 i)
        {
            Player* player = GetPlayerForEscort();
            if (!player)
            {
                me->DespawnOrUnsummon(1);
                return;
            }
            Creature *crowley = NULL;
            if (me->GetVehicleKit())
                if (me->GetVehicleKit()->GetPassenger(1))
                    if (me->GetVehicleKit()->GetPassenger(1)->GetTypeId() == TYPEID_UNIT)
                        crowley = me->GetVehicleKit()->GetPassenger(1)->ToCreature();

            switch(i)
            {
                case 0:
                    SetEscortPaused(true);
                    player->SetClientControl(me, 0);
                    if (crowley)
                        crowley->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->GetMotionMaster()->MoveJump(-1714.02f, 1666.37f, 20.57f, 25.0f, 15.0f);
                    break;
                case 3:
                    if (crowley)
                        crowley->AI()->Talk(0);
                    break;
                case 9:
                    SetEscortPaused(true);
                    me->GetMotionMaster()->MoveJump(-1571.32f, 1710.58f, 20.49f, 25.0f, 15.0f);
                    break;
                case 10:
                    if (crowley)
                        crowley->AI()->Talk(1);
                    break;
                case 15:
                    if (crowley)
                        crowley->AI()->Talk(1);
                    break;
                case 19:
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->getThreatManager().resetAllAggro();
                    player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    player->getThreatManager().resetAllAggro();
                    break;
                case 20:
                    player->SetClientControl(me, 1);
                    player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    player->ExitVehicle();
                    break;
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            if (Player* player = GetPlayerForEscort())
               player->FailQuest(QUEST_SACRIFICES);
        }

        void OnCharmed(bool /*apply*/)
        {
        }

        void UpdateAI(const uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);

            if (!CrowleySpawn)
            {
                 DoCast(SPELL_SUMMON_CROWLEY);
                 if (Creature *crowley = me->FindNearestCreature(NPC_DARIUS_CROWLEY, 5, true))
                 {
                     CrowleySpawn = true;
                 }
            }

            if (CrowleySpawn && !CrowleyOn)
            {
                Creature *crowley = me->FindNearestCreature(NPC_DARIUS_CROWLEY, 5, true);
                crowley->CastSpell(me, SPELL_RIDE_HORSE, true);//Mount Crowley in seat 1
                CrowleyOn = true;
            }

            if (!Run)
            {
                me->SetSpeed(MOVE_RUN, CROWLEY_SPEED);
                Run = true;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_crowley_horseAI (creature);
    }
};

class npc_mariam_spellwalker : public CreatureScript
{
public:
    npc_mariam_spellwalker() : CreatureScript("npc_mariam_spellwalker") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_mariam_spellwalkerAI (creature);
    }

    struct npc_mariam_spellwalkerAI : public ScriptedAI
    {
        npc_mariam_spellwalkerAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            dmgCount = 0;
            tSeek = urand(1000, 2000);
        }

        void DamageDealt(Unit* target, uint32 &damage, DamageEffectType damageType)
        {
            if (target->GetEntry() == NPC_BLOODFANG_WORGEN)
                dmgCount++;
        }

        void UpdateAI(const uint32 diff)
        {
            if (tSeek <= diff)
            {
                if ((me->isAlive()) && (!me->isInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 5.0f, true))
                        me->AI()->AttackStart(enemy);
                tSeek = urand(1000, 2000);
            }
            else tSeek -= diff;

            if (!UpdateVictim())
                return;

            if (dmgCount < 2)
                DoSpellAttackIfReady(SPELL_FROSTBOLT_VISUAL_ONLY);
        }

    private :
        uint32 dmgCount;
        uint32 tSeek;

    };
};

class npc_krennan_aranas_c2 : public CreatureScript
{
public:
    npc_krennan_aranas_c2() : CreatureScript("npc_krennan_aranas_c2") { }

    struct npc_krennan_aranas_c2AI : public ScriptedAI
    {
        npc_krennan_aranas_c2AI(Creature* creature) : ScriptedAI(creature) {}

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()
        {
            Say = false;
            Move = true;
            Cast = true;
            KrennanDead = false;
            SayTimer = 500;
        }

        void UpdateAI(const uint32 diff)
        {
            if (Creature *krennan = me->FindNearestCreature(3871227, 50))
                if (!KrennanDead)
                {
                    krennan->DespawnOrUnsummon(1);
                    KrennanDead = true;
                }

            if (Creature *horse = me->FindNearestCreature(35905, 20.0f))
            {
                if (Cast)
                    DoCast(horse, 84275, true);

                if (me->HasAura(84275))
                    Cast = false;
            }

            if (!me->HasAura(84275) && Move)
            {
                me->NearTeleportTo(KRENNAN_END_X, KRENNAN_END_Y, KRENNAN_END_Z, KRENNAN_END_O);
                Say = true;
                Move = false;
                SayTimer = 500;
            }

            if (Say && SayTimer <= diff)
            {
                me->DespawnOrUnsummon(6000);
                Say = false;
            }
            else
                SayTimer -= diff;
        }

    private :
        bool Say;
        bool Move;
        bool Cast;
        bool KrennanDead;
        uint32 SayTimer;

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_krennan_aranas_c2AI(creature);
    }
};

class spell_keg_placed : public SpellScriptLoader
{
    public:
        spell_keg_placed() : SpellScriptLoader("spell_keg_placed") {}

        class spell_keg_placed_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_keg_placed_AuraScript);

            void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                tick = urand(1, 4);
                tickcount = 0;
            }

            void HandlePeriodic(AuraEffect const* aurEff)
            {
                PreventDefaultAction();
                if (Unit* caster = GetCaster())
                {
                    if (tickcount > tick)
                    {
                        if (caster->GetTypeId() != TYPEID_PLAYER)
                            return;
                        caster->ToPlayer()->KilledMonsterCredit(36233, 0);
                        if (Unit* target = GetTarget())
                            target->Kill(target);
                    }
                    tickcount++;
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_keg_placed_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_keg_placed_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_DUMMY);
            }

        private :
            uint32 tick;
            uint32 tickcount;

        };

        AuraScript* GetAuraScript() const
        {
            return new spell_keg_placed_AuraScript();
        }
};

class npc_bloodfang_stalker_c1 : public CreatureScript
{
public:
    npc_bloodfang_stalker_c1() : CreatureScript("npc_bloodfang_stalker_c1") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodfang_stalker_c1AI (creature);
    }

    struct npc_bloodfang_stalker_c1AI : public ScriptedAI
    {
        npc_bloodfang_stalker_c1AI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            tEnrage    = 0;
            tAnimate   = DELAY_ANIMATE;
            Miss  = false;
            willCastEnrage = urand(0, 1);
            BurningReset = 3000;
            Burning = false;
        }

        void UpdateAI(const uint32 diff)
        {
            if(me->HasAura(SPELL_THROW_TORCH))
                Burning = true;
            else
                Burning = false;

            if (Burning && BurningReset <= diff)
            {
                me->RemoveAllAuras();
                BurningReset = 5000;
                Burning = false;
            }
            else
                BurningReset -= diff;

            if (!UpdateVictim())
                return;

            if (tEnrage <= diff && willCastEnrage)
            {
                if (me->GetHealthPct() <= 30)
                {
                    me->MonsterTextEmote(-106, 0);
                    DoCast(me, SPELL_ENRAGE);
                    tEnrage = CD_ENRAGE;
                }
            }
            else tEnrage -= diff;

            if (me->getVictim()->GetTypeId() == TYPEID_PLAYER)
                Miss = false;
            else if (me->getVictim()->isPet())
                Miss = false;
            else if (me->getVictim()->GetEntry() == NPC_NORTHGATE_REBEL_1)
                if (me->getVictim()->GetHealthPct() < 90)
                    Miss = true;

            if (Miss && tAnimate <= diff)
            {
                me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_UNARMED);
                me->PlayDistanceSound(SOUND_WORGEN_ATTACK);
                tAnimate = DELAY_ANIMATE;
            }
            else
                tAnimate -= diff;

            if (!Miss)
                DoMeleeAttackIfReady();
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            Creature *horse = me->FindNearestCreature(NPC_CROWLEY_HORSE, 100, true);
            if (spell->Id == SPELL_THROW_TORCH)
            {
                Burning = true;
                if (caster->GetTypeId() == TYPEID_PLAYER && caster->ToPlayer()->GetQuestStatus(QUEST_SACRIFICES) == QUEST_STATUS_INCOMPLETE)
                {
                    caster->ToPlayer()->KilledMonsterCredit(NPC_BLOODFANG_STALKER_CREDIT, 0);
                    me->DespawnOrUnsummon(5000);
                }
            }
        }

    private :
        Player* player;
        uint32 tEnrage;
        uint32 tAnimate;
        uint32 BurningReset;
        bool Miss, willCastEnrage, Burning;
    };
};

enum Stocks
{
    QUEST_LAST_STAND = 14222,
    QUEST_LAST_CHANCE_AT_HUMANITY = 14375,

    MOVIE_WORGEN_FORM = 21,

    NPC_LORD_GODFREY_STOCKS = 36330,
    NPC_KRENNAN_ARANAS_STOCKS = 36331,
    NPC_KING_GENN_GREYMANE_STOCKS = 36332,

    SPELL_PLAY_MOVIE = 93477,
    SPELL_FORCE_WORGEN_ALTERED_FORM = 98274,
    SPELL_HIDEOUS_BITE_WOUND = 76642,
    SPELL_CURSE_OF_THE_WORGEN = 68630,
    SPELL_IN_STOCKS = 69196,
    SPELL_LAST_STAND_COMPLETE = 72799,  // Learns Worgen Racials (except for Two Forms which will be learned later via quest)
    SPELL_QUEST_ZONE_SPECIFIC = 68481,
    SPELL_SUMMON_PERSONAL_GODFREY = 68635,
    SPELL_SUMMON_PERSONAL_GREYMANE = 68636,
};

class npc_lord_darius_crowley_c3 : public CreatureScript
{
public:
    npc_lord_darius_crowley_c3() : CreatureScript("npc_lord_darius_crowley_c3") {}

    bool OnQuestReward(Player* player, Creature* creature, const Quest *_Quest, uint32 )
    {
        if (_Quest->GetQuestId() == 14222)
        {
            WorldLocation loc;
            loc.m_mapId       = 654;
            loc.m_positionX   = -1818.4f;
            loc.m_positionY   = 2294.25f;
            loc.m_positionZ   = 42.2135f;
            loc.m_orientation = 3.246661f;

            player->RemoveAurasDueToSpell(SPELL_HIDEOUS_BITE_WOUND);

            player->CastSpell(player, SPELL_LAST_STAND_COMPLETE, true);
            player->CastSpell(player, SPELL_FORCE_WORGEN_ALTERED_FORM, true);
            player->CastSpell(player, SPELL_IN_STOCKS, true);
            player->CastSpell(player, SPELL_CURSE_OF_THE_WORGEN, true);
            player->CastSpell(player, SPELL_PLAY_MOVIE, true);

            player->TeleportTo(loc);
        }
        return true;
    }
};

class spell_curse_of_the_worgen_summon : public SpellScript
{
    PrepareSpellScript(spell_curse_of_the_worgen_summon);

    void Summon()
    {
        if (Creature* krennan = GetCaster()->SummonCreature(NPC_KRENNAN_ARANAS_STOCKS, -1815.9f, 2283.85f, 42.406f, 2.48f))
        {
            krennan->SetOwnerGUID(GetCaster()->GetGUID());
            krennan->ToTempSummon()->SetSummonerGUID(GetCaster()->GetGUID());
        }
    }

    void Register()
    {
        OnCast += SpellCastFn(spell_curse_of_the_worgen_summon::Summon);
    }
};

class PlayerAlreadyStartWorgenZone2
{
public:
    PlayerAlreadyStartWorgenZone2() { }

    bool operator()(Player* player) const
    {
        return (player->GetQuestStatus(14375) != QUEST_STATUS_NONE || player->HasAura(68630));
    }
};

class StocksPlayerScript : public PlayerScript
{
public:
    StocksPlayerScript() : PlayerScript("StocksPlayerScript") {}

    void OnLogin(Player* player)
    {
        if (player->getRace() == RACE_WORGEN && player->GetAreaId() == 4786 && player->GetPhaseMask() == 4096)
            if (player->GetQuestRewardStatus(QUEST_LAST_STAND) && !player->GetQuestRewardStatus(QUEST_LAST_CHANCE_AT_HUMANITY))
            {
                if (player->FindNearestCreature(NPC_KRENNAN_ARANAS_STOCKS, 100.0f))
                {
                    std::list<Unit*> krennan = player->SelectNearbyUnits(NPC_KRENNAN_ARANAS_STOCKS, 100.0f);
                    for (std::list<Unit *>::iterator itr = krennan.begin(); itr != krennan.end(); ++itr)
                        if (Unit* krennan = (*itr))
                        {
                            if (krennan->GetOwnerGUID() == player->GetGUID())
                                return;
                            else if (krennan->GetOwnerGUID() != player->GetGUID())
                                player->CastSpell(player, SPELL_CURSE_OF_THE_WORGEN);
                        }
                }
                else if (!player->FindNearestCreature(NPC_KRENNAN_ARANAS_STOCKS, 100.0f))
                    player->CastSpell(player, SPELL_CURSE_OF_THE_WORGEN);

            }
    }
};

class npc_king_genn_greymane_c2 : public CreatureScript
{
public:
    npc_king_genn_greymane_c2() : CreatureScript("npc_king_genn_greymane_c2") {}

    bool OnQuestReward(Player* player, Creature* creature, Quest const* _Quest, uint32)
    {
        if (_Quest->GetQuestId() == 14375)
        {
            player->RemoveAurasDueToSpell(68630);
            player->RemoveAurasDueToSpell(69196);
            player->CastSpell(player, SPELL_FADE_BACK, true);
            player->CastSpell(player, 72799, true);
            creature->DespawnOrUnsummon(1);
        }
        return true;
    }
};

class npc_trigger_event_first_c3 : public CreatureScript
{
public:
    npc_trigger_event_first_c3() : CreatureScript("npc_trigger_event_first_c3") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_trigger_event_first_c3AI (creature);
    }

    struct npc_trigger_event_first_c3AI : public ScriptedAI
    {
        npc_trigger_event_first_c3AI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            GodfreyGUID = 0;
            GreymaneGUID = 0;
            me->SetVisible(false);

            me->SetPhaseMask(4096, true);
            me->SetWalk(true);

            events.ScheduleEvent(1, 1000);
        }

        void UpdateAI(const uint32 diff)
        {
            Unit* summoner = me->ToTempSummon()->GetSummoner();
            if (!summoner)
                return;

            Player* player = summoner->ToPlayer();
            if (!player)
                return;
            
            /*if (player->IsWatchingMovie())
                return;*/

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case 1:
                        me->GetMotionMaster()->MovePoint(0, -1819.53f, 2291.25f, 42.32689f);
                        events.ScheduleEvent(2, 4900);
                        break;
                    case 2:
                        me->PlayDirectSound(20919, player);
                        me->HandleEmoteCommand(274);
                        Talk(0, player->GetGUID());
                        events.ScheduleEvent(3, 4800);
                        break;
                    case 3:
                        if (Creature* godfrey = me->SummonCreature(NPC_LORD_GODFREY_STOCKS, -1844.040039f, 2289.629883f, 42.406601f, 0.0f, TEMPSUMMON_MANUAL_DESPAWN))
                        {
                            godfrey->SetVisible(false);
                            godfrey->SetOwnerGUID(me->GetOwnerGUID());
                            godfrey->SetPhaseMask(4096, true);
                            godfrey->SetWalk(true);
                            godfrey->GetMotionMaster()->MovePoint(0, -1821.922f, 2295.05f, 42.17052f);

                            events.ScheduleEvent(4, 8400);
                            events.ScheduleEvent(5, 10000);

                            GodfreyGUID = godfrey->GetGUID();
                        }
                        break;
                    case 4:
                        if (Creature* greymane = me->SummonCreature(NPC_KING_GENN_GREYMANE_STOCKS, -1846.709961f, 2288.75f, 42.406601f, 0.698132f, TEMPSUMMON_MANUAL_DESPAWN))
                        {
                            greymane->SetVisible(false);
                            greymane->SetOwnerGUID(me->GetOwnerGUID());
                            greymane->SetPhaseMask(4096, true);
                            greymane->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                            greymane->SetWalk(true);

                            greymane->GetMotionMaster()->MovePoint(0, -1821.09f, 2292.597f, 42.23363f);

                            GreymaneGUID = greymane->GetGUID();

                            events.ScheduleEvent(6, 10500);
                        }
                        break;
                    case 5:
                        if (Creature* godfrey = Unit::GetCreature(*me, GodfreyGUID))
                        {
                            godfrey->PlayDirectSound(19635, player);
                            godfrey->HandleEmoteCommand(274);
                            godfrey->SetFacingToObject(player);

                            godfrey->AI()->Talk(0, player->GetGUID());
                        }
                        break;
                    case 6:
                        if (Creature* greymane = Unit::GetCreature(*me, GreymaneGUID))
                        {
                            if (Creature* godfrey = Unit::GetCreature(*me, GodfreyGUID))
                                greymane->SetFacingToObject(godfrey);
                            greymane->PlayDirectSound(19721, player);
                            greymane->HandleEmoteCommand(1);

                            greymane->AI()->Talk(0, player->GetGUID());

                            events.ScheduleEvent(7, 9200);
                        }
                        break;
                    case 7:
                        if (Creature* greymane = Unit::GetCreature(*me, GreymaneGUID))
                        {
                            greymane->PlayDirectSound(19722, player);
                            greymane->HandleEmoteCommand(1);

                            greymane->AI()->Talk(1, player->GetGUID());

                            events.ScheduleEvent(8, 3600);
                        }
                        break;
                    case 8:
                        if (Creature* greymane = Unit::GetCreature(*me, GreymaneGUID))
                        {
                            greymane->SetFacingToObject(me);
                            greymane->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        uint64 GodfreyGUID;
        uint64 GreymaneGUID;
        EventMap events;
    };
};

class go_mandragore : public GameObjectScript
{
public:
    go_mandragore() : GameObjectScript("go_mandragore") {}

    bool OnQuestReward(Player* player, GameObject * , Quest const* _Quest, uint32)
    {
        if (_Quest->GetQuestId() == 14320)
        {
            player->SendCinematicStart(168);
            WorldPacket data(SMSG_PLAY_SOUND, 4);
            data << uint32(23676);
            player->GetSession()->SendPacket(&data);
        }
        return true;
    }
};

class npc_forsaken_catapult : public CreatureScript
{
public:
    npc_forsaken_catapult() : CreatureScript("npc_forsaken_catapult") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_forsaken_catapultAI(creature);
    }

    struct npc_forsaken_catapultAI : public ScriptedAI
    {
        npc_forsaken_catapultAI(Creature* creature) : ScriptedAI(creature)
        {
            me->setActive(true);
            uiCastTimer = urand(1000, 5000);
            uiRespawnTimer = 10000;
            CanCast = true;
            respawn = false;
        }

        uint32 uiCastTimer;
        uint32 uiJumpTimer;
        uint32 uiRespawnTimer;
        uint32 uiEventTimer = 2000;
        bool CanCast;
        bool respawn;

        void PassengerBoarded(Unit* who, int8 seatId, bool apply)
        {
            if (!apply)
            {
                respawn = true;
                CanCast = false;

                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                me->setFaction(35);
            }
            else
            {
                respawn = false;

                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
                me->SetReactState(REACT_PASSIVE);

                if (who->GetEntry() == 36292)
                {
                    me->setFaction(1735);
                    who->ToCreature()->SetReactState(REACT_PASSIVE);
                    who->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
                }

                if (seatId == 2)
                    CanCast = true;
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (respawn)
                if (uiRespawnTimer <= diff)
                {
                    respawn = false;
                    uiRespawnTimer = 10000;
                    me->DespawnOrUnsummon();
                }
                else
                    uiRespawnTimer -= diff;

            if (CanCast)
                if (uiCastTimer <= diff)
                {
                    uiCastTimer = urand(5000, 20000);
                    if (Unit* target = me->FindNearestCreature(36286, 200.0f))
                        me->CastSpell(target, 68591, true);
                }
                else
                    uiCastTimer -= diff;

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };
};

class spell_round_up_horse : public SpellScriptLoader
{
public:
    spell_round_up_horse() : SpellScriptLoader("spell_round_up_horse") { }

    class spell_round_up_horse_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_round_up_horse_SpellScript);

        bool Validate(SpellInfo const* /*spellInfo*/)
        {
            if (!sSpellMgr->GetSpellInfo(68903))
                return false;
            return true;
        }

        void HandleEffectDummy(SpellEffIndex /*effIndex*/)
        {
            if (GetHitUnit()->GetTypeId() != TYPEID_UNIT || GetCaster()->GetTypeId() != TYPEID_PLAYER || GetCaster()->ToPlayer()->GetQuestStatus(14416) != QUEST_STATUS_INCOMPLETE)
                return ;
            GetHitUnit()->ToCreature()->DespawnOrUnsummon(1);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_round_up_horse_SpellScript::HandleEffectDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_round_up_horse_SpellScript();
    }
};


class npc_trigger_quest_24616 : public CreatureScript
{
public:
    npc_trigger_quest_24616() : CreatureScript("npc_trigger_quest_24616") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_trigger_quest_24616AI(creature);
    }

    struct npc_trigger_quest_24616AI : public ScriptedAI
    {
        npc_trigger_quest_24616AI(Creature* creature) : ScriptedAI(creature)
        {
        }

        void Reset()
        {
            mui_timerAllowSummon = urand(3000, 5000);
            allowSummon = false;
            playerGUID = 0;
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (who->GetTypeId() != TYPEID_PLAYER)
                return ;
            if (who->ToPlayer()->GetQuestStatus(24616) != QUEST_STATUS_INCOMPLETE || me->FindNearestCreature(37953, 100, false) != NULL)
                return ;
            if (me->IsWithinDistInMap(who, 20.0f))
            {
                allowSummon = true;
                playerGUID = who->GetGUID();
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!allowSummon)
                return;
            if (mui_timerAllowSummon <= diff)
            {
                if (Player *player = Unit::GetPlayer(*me, playerGUID))
                    if (me->FindNearestCreature(37953, 100) == NULL)
                        me->CastSpell(player, 70794, true);
                allowSummon = false;
                playerGUID = 0;
                mui_timerAllowSummon = urand(3000, 5000);
            }
            else mui_timerAllowSummon -= diff;
        }

    private :
        uint32 mui_timerAllowSummon;
        bool allowSummon;
        uint64 playerGUID;
    };
};

class npc_krennan_aranas : public CreatureScript
{
public:
    npc_krennan_aranas() : CreatureScript("npc_krennan_aranas") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (uiAction)
        {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->CLOSE_GOSSIP_MENU();
            CAST_AI(npc_krennan_aranas::npc_krennan_aranasAI, creature->AI())->StartBattle(player);
            break;

        case GOSSIP_ACTION_INFO_DEF+2:
            player->CLOSE_GOSSIP_MENU();
            CAST_AI(npc_krennan_aranas::npc_krennan_aranasAI, creature->AI())->EndBattle();
            break;
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_THE_BATTLE_FOR_GILNEAS_CITY) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "start battle !", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        if (player->isGameMaster())
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "(GM ONLY) RESET EVENT!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);

        player->SEND_GOSSIP_MENU(2474, creature->GetGUID());

        return true;
    }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_krennan_aranasAI(pCreature);
    }

    struct npc_krennan_aranasAI : public ScriptedAI
    {
        npc_krennan_aranasAI(Creature *c) : ScriptedAI(c) {}

        uint64 princeGUID;

        void Reset()
        {
            princeGUID = 0;
            if (Creature* newPrince = me->SummonCreature(NPC_PRINCE_LIAM_GREYMANE_BATTLE, -1408.661f, 1260.017f, 36.51123f, 1.79f, TEMPSUMMON_DEAD_DESPAWN, 180000))
                princeGUID = newPrince->GetGUID();
        }

        void StartBattle(Player* pl)
        {
            if (Creature* prince = Unit::GetCreature(*me, princeGUID))
            {
                if (me->GetDistance2d(prince->GetPositionX(), prince->GetPositionY()) > 50)
                    me->MonsterSay("event already started please wait a minute.", LANG_UNIVERSAL, 0);
                else
                {
                    prince->AI()->DoAction(ACTION_START_EVENT);
                    Talk(0, pl->GetGUID());
                }
            }
            else
            {
                princeGUID = 0;
                if (Creature* newPrince = me->SummonCreature(NPC_PRINCE_LIAM_GREYMANE_BATTLE, -1408.661f, 1260.017f, 36.51123f, 1.79f, TEMPSUMMON_DEAD_DESPAWN, 180000))
                {
                    princeGUID = newPrince->GetGUID();
                    newPrince->AI()->DoAction(ACTION_START_EVENT);
                    Talk(0, pl->GetGUID());
                }
            }
        }

        void EndBattle()
        {
            if (Creature* prince = Unit::GetCreature(*me, princeGUID))
                prince->DespawnOrUnsummon(1);
            princeGUID = 0;
            if (Creature* newPrince = me->SummonCreature(NPC_PRINCE_LIAM_GREYMANE_BATTLE, -1408.661f, 1260.017f, 36.51123f, 1.79f, TEMPSUMMON_DEAD_DESPAWN, 180000))
                princeGUID = newPrince->GetGUID();
        }
    };
};

class npc_prince_liam_greymane_gilneas : public CreatureScript
{
public:
    npc_prince_liam_greymane_gilneas() : CreatureScript("npc_prince_liam_greymane_gilneas") {}

    struct npc_prince_liam_greymane_gilneasAI : public npc_escortAI
    {
        npc_prince_liam_greymane_gilneasAI(Creature *c) : npc_escortAI(c) , summons(me)
        {
        }

        void Reset()
        {
            me->CastSpell(me, 72069, true);

            startEvent = false;
            intro = true;
            canFollow = false;
            phase = 1;
            intoTimer = 5000;
            checkTimer = 3000;
            summons.DespawnAll();
            float summon_x = me->GetPositionX() - 6;
            float summon_y = me->GetPositionY() - 4;

            int j = 0;
            int row = 0;

            for (int i = 0; i < 70; i++)
            {
                if (j > 6)
                {
                    j = 0;
                    row++;
                    summon_y -= 3;
                    summon_x = me->GetPositionX() - 6;
                }

                if (Creature * milit = me->SummonCreature(38221, (summon_x+cos(5.42f)*row), (summon_y-sin(5.42f)*j), 36.51123f, 1.79f, TEMPSUMMON_MANUAL_DESPAWN))
                {
                    militiamans[i] = milit->GetGUID();
                    if (i % 2 == 0)
                        milit->AI()->SetData(DATA_GROUP, GROUP_1);
                    else
                        milit->AI()->SetData(DATA_GROUP, GROUP_1); // need to be improved...
                }
                summon_x += 2;
                j++;
            }
            me->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
            mui_moveTimer = 1000;
        }

        void JustDied(Unit* /*killer*/)
        {
            summons.DespawnAll();
        }

        bool CanFollow() { return canFollow; }

        void EnterCombat(Unit* pWho)
        {
            Talk(10);
        }

        void moveGroup(uint32 group, float x, float y, float z)
        {
            for (int i = 0; i < 70; i++)
                if (Creature *milit = Unit::GetCreature(*me, militiamans[i]))
                    if (milit->AI()->GetData(DATA_GROUP) == group)
                    {
                        milit->SetHomePosition(x, y, z, milit->GetOrientation());
                        milit->GetMotionMaster()->MovePoint(0, x, y, z);
                    }
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/)
        {
            summons.Despawn(summon);
        }

        void WaypointReached(uint32 point)
        {
            RefreshMovementMilitary(GROUP_1);
            switch (point)
            {
            case 1:
                moveGroup(GROUP_2, -1487.67f, 1345.69f, 35.56f);
                break;
            case 3:
                moveGroup(GROUP_2, -1551.90f, 1327.45f, 35.56f);
                break;
            case 5:
                Talk(7);
                if (Creature *c = me->SummonCreature(38426, -1549.20f, 1285.96f, 11.78f, 3.47f, TEMPSUMMON_MANUAL_DESPAWN))
                    c->AI()->DoAction(ACTION_START_EVENT);
                SetEscortPaused(true);
                break;
            case 6:
                moveGroup(GROUP_2, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                Talk(8);
                break;
            case 7 :
                moveGroup(GROUP_2, -1657.01f, 1337.70f, 15.135f);
                break;
            case 8: //crowley
                moveGroup(GROUP_2, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                Talk(9);
                break;
            case 10:
            case 11:
            case 12:
                moveGroup(GROUP_2, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                break;
            case 13:
                if (Creature *abo = me->SummonCreature(38331, -1710.466f, 1407.905f, 21.75196f, -1.89314f, TEMPSUMMON_DEAD_DESPAWN, 60000))
                {
                    abo->GetMotionMaster()->MoveJump(-1710.466f, 1407.905f, 21.75196f, 20.0f, 20.0f);
                    if (Creature *darius = me->SummonCreature(38415, -1771.81f, 1341.33f, 19.75, 0.81314f, TEMPSUMMON_MANUAL_DESPAWN))
                        darius->AI()->SetGUID(abo->GetGUID());
                }
                break;
            case 14 :
                moveGroup(GROUP_2, -1761.24f, 1512.72f, 26.22f);
                break;
            case 15:
                for (int i = 0; i < 70; i++)
                    if (Creature *milit = Unit::GetCreature(*me, militiamans[i]))
                        milit->AI()->SetData(DATA_GROUP, GROUP_1);
                break;
            case 19:
                if (Creature *c = me->FindNearestCreature(38470, 100, true))
                    c->AI()->SetData(0, 1);
                summons.DespawnAll();
                me->DespawnOrUnsummon(1);
                break;
            }
        }

        void RefreshMovementMilitary(uint32 group)
        {
            float x, y, z;
            for (int i = 0; i < 70; i++)
            {
                float distance = urand(0, 7);
                float angle = frand(-3 * M_PI / 4, 3 * M_PI / 2);
                me->GetNearPoint(me, x, y, z, me->GetObjectSize(), distance, angle);
                if (Creature *milit = Unit::GetCreature(*me, militiamans[i]))
                    if (milit->AI()->GetData(DATA_GROUP) == group)
                        milit->SetHomePosition(x, y, z, me->GetOrientation());
            }
        }

        void DoAction(const int32 info)
        {
            switch (info)
            {
                case ACTION_START_EVENT:
                    startEvent = true;
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!startEvent)
                return;
            npc_escortAI::UpdateAI(diff);

            if (canFollow)
            {
                if (mui_moveTimer <= diff)
                {
                    RefreshMovementMilitary(GROUP_1);
                    mui_moveTimer = 10000;
                }
                else
                    mui_moveTimer -= diff;
            }

            if (intro)
            {
                if (intoTimer <= diff)
                {
                    switch (phase)
                    {
                    case 1:
                        Talk(0);
                        intoTimer = 10000;
                        break;
                    case 2:
                        Talk(1);
                        intoTimer = 10000;
                        break;
                    case 3:
                        Talk(2);
                        intoTimer = 12000;
                        break;
                    case 4:
                        Talk(3);
                        intoTimer = 10000;
                        break;
                    case 5:
                        Talk(4);
                        intoTimer = 12000;
                        break;
                    case 6:
                        Talk(5);
                        intoTimer = 5500;
                        break;
                    case 7:
                        Talk(6);
                        intoTimer = 1500;
                        Start(true, true);
                        SetDespawnAtEnd(true);
                        break;
                    case 8:
                        me->PlayDistanceSound(22584);
                        for (int i = 0; i < 70; i++)
                            if (Creature *milit = Unit::GetCreature(*me, militiamans[i]))
                            {
                                milit->HandleEmoteCommand(25);
                                float distance = urand(7, 15);
                                float angle = frand(-3 * M_PI / 4, 3 * M_PI / 4);
                                milit->GetMotionMaster()->MoveFollow(me, distance, angle, MOTION_SLOT_ACTIVE);
                            }
                        intro = false;
                        canFollow = true;
                        break;
                    }
                    phase++;
                } else intoTimer -= diff;
            }

            if (checkTimer <= diff)
            {
                if (!me->HasAura(72069))
                    me->CastSpell(me, 72069, true);
                checkTimer = 3000;
            } else checkTimer -= diff;

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

    private :
        bool intro;
        bool canFollow;
        uint32 phase;
        uint32 intoTimer;
        uint32 checkTimer;
        uint64 militiamans[70];
        uint32 mui_moveTimer;
        bool startEvent;
        SummonList summons;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_prince_liam_greymane_gilneasAI(creature);
    }
};

class npc_lorna_crowley_event : public CreatureScript
{
public:
    npc_lorna_crowley_event() : CreatureScript("npc_lorna_crowley_event") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_lorna_crowley_eventAI(pCreature);
    }

    struct npc_lorna_crowley_eventAI : public ScriptedAI
    {
        npc_lorna_crowley_eventAI(Creature *c) : ScriptedAI(c) {}

        void Reset()
        {
            eventInProgress = false;
            mui_moveTimer = 5000;
            mui_attack = 8000;
        }

        void MovementInform(uint32 type, uint32 point)
        {
            if (!eventInProgress)
                Talk(0);
            eventInProgress = true;
            me->DespawnOrUnsummon(60000);
        }

        void DoAction(const int32 info)
        {
            switch (info)
            {
                case ACTION_START_EVENT:
                    float summon_x = me->GetPositionX();
                    float summon_y = me->GetPositionY();
                    float summon_z = me->GetPositionZ();
                    for (int i = 0; i < 12; i++)
                        if (Creature * milit = me->SummonCreature(38425, summon_x, summon_y, summon_z, 0.0f, TEMPSUMMON_MANUAL_DESPAWN))
                        {
                            if (i == 0 || i == 1 || i == 2)
                                citizenGUID[i] = milit->GetGUID();
                            if (i != 0)
                                if (Creature *firstCiti = Unit::GetCreature(*me, citizenGUID[0]))
                                    milit->GetMotionMaster()->MoveFollow(firstCiti, urand(0, 5), frand(-3 * M_PI / 8, 3 * M_PI / 8), MOTION_SLOT_ACTIVE);
                        }
                    if (Creature *firstCiti = Unit::GetCreature(*me, citizenGUID[0]))
                        firstCiti->GetMotionMaster()->MovePoint(0, -1571.79f, 1318.833f, 35.56271f);
                    me->GetMotionMaster()->MovePoint(0, -1571.79f, 1318.833f, 35.56271f);
                    if (Creature *c = me->SummonCreature(38424, -1542.548f, 1290.957f, 6.239825f, 3.490021f, TEMPSUMMON_MANUAL_DESPAWN))
                    {
                        c->GetMotionMaster()->MovePoint(0, -1571.79f, 1318.833f, 35.56271f);
                        canonGUID = c->GetGUID();
                    }
                    if (Creature *c = me->SummonCreature(38424, -1541.036f, 1283.645f, 6.35803f,  3.343305f, TEMPSUMMON_MANUAL_DESPAWN))
                        c->GetMotionMaster()->MovePoint(0, -1571.79f, 1318.833f, 35.56271f);
                    if (Creature *c = me->SummonCreature(38424, -1543.06f,  1287.436f, 7.076612f, 3.446335f, TEMPSUMMON_MANUAL_DESPAWN))
                        c->GetMotionMaster()->MovePoint(0, -1571.79f, 1318.833f, 35.56271f);
                    break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!eventInProgress)
                return;

            if (mui_moveTimer <= diff)
            {
                if (Creature *canon = Unit::GetCreature(*me, canonGUID))
                {
                    if (Creature *firstCiti = Unit::GetCreature(*me, citizenGUID[0]))
                        firstCiti->GetMotionMaster()->MovePoint(1, -1606.97f, 1313.59f, 18.99f);
                    if (Creature *firstCiti = Unit::GetCreature(*me, citizenGUID[1]))
                        firstCiti->GetMotionMaster()->MovePoint(1, -1610.97f, 1310.59f, 18.11f);
                    me->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, canon, false);
                    canon->GetMotionMaster()->MovePoint(1, -1609.97f, 1312.59f, 18.11f);
                    if (me->ToTempSummon())
                        if (Unit *general = me->ToTempSummon()->GetSummoner())
                        {
                            me->GetMotionMaster()->MoveFollow(general, urand(0, 5), frand(-3 * M_PI / 8, 3 * M_PI / 8), MOTION_SLOT_ACTIVE);
                            if (general->ToCreature())
                                CAST_AI(npc_escortAI, (general->ToCreature()->AI()))->SetEscortPaused(false);
                        }
                }
                mui_moveTimer = 3600000;
            }
            else  mui_moveTimer -= diff;

            if (mui_attack <= diff)
            {
                if (Creature *c = me->FindNearestCreature(38420, 200))
                    if (Creature *canon = Unit::GetCreature(*me, canonGUID))
                        canon->CastSpell(c, 72013, true);
                mui_attack = 5000;
            }
            else mui_attack -= diff;
        }

    private:
        bool eventInProgress;
        uint32 mui_moveTimer;
        uint32 mui_attack;
        uint64 canonGUID;
        uint64 citizenGUID[2];
    };
};

class npc_gilnean_militia : public CreatureScript
{
public:
    npc_gilnean_militia() : CreatureScript("npc_gilnean_militia") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_gilnean_militiaAI(pCreature);
    }

    struct npc_gilnean_militiaAI : public ScriptedAI
    {
        npc_gilnean_militiaAI(Creature* pCreature) : ScriptedAI(pCreature) { }

        void Reset()
        {
            mui_spell1 = urand(1200, 5100);
            mui_spell2 = urand(2100, 5400);
            me->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
        }

        void AttackStart(Unit* who)
        {
            if (me->Attack(who, true))
                DoStartNoMovement(who);
        }

        void SetData(uint32 uiType, uint32 uiData)
        {
            if (uiType == DATA_GROUP)
                group = uiData;
        }

        uint32 GetData(uint32 uiType) const
        {
            if (uiType == DATA_GROUP)
                return group;
            return 0;
        }

        void JustReachedHome()
        {
            if (group == GROUP_1)
                if (!UpdateVictim())
                    if (me->ToTempSummon())
                        if (Unit *p = me->ToTempSummon()->GetSummoner())
                        {
                            float distance = urand(7, 15);
                            float angle = frand(-3 * M_PI / 4, 3 * M_PI / 2);
                            float x, y, z;
                            me->GetNearPoint(p, x, y, z, p->GetObjectSize(), distance, angle);
                            me->GetMotionMaster()->MoveFollow(p, distance, angle, MOTION_SLOT_ACTIVE);
                        }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;
            if (mui_spell1 <= diff)
            {
                DoCastVictim(15572);
                mui_spell1 = 10000 + urand(1200, 5100);
            }
            else
                mui_spell1 -= diff;

            if (mui_spell2 <= diff)
            {
                DoCastVictim(47168);
                mui_spell2 = 10000 + urand(2100, 5400);
            }
            else
                mui_spell2 -= diff;

            victim = me->getVictim();
            if (victim && me->GetDistance2d(victim->GetPositionX(), victim->GetPositionY()) > 7)
                DoSpellAttackIfReady(6660);
            else
                DoMeleeAttackIfReady();
        }

    private :
        uint32 mui_spell1;
        uint32 mui_spell2;
        Unit *victim;
        uint32 group;
    };
};

class npc_lord_darius_crowley_gilneas : public CreatureScript
{
public:
    npc_lord_darius_crowley_gilneas() : CreatureScript("npc_lord_darius_crowley_gilneas") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_lord_darius_crowley_gilneasAI(pCreature);
    }

    struct npc_lord_darius_crowley_gilneasAI : public ScriptedAI
    {
        npc_lord_darius_crowley_gilneasAI(Creature* pCreature) : ScriptedAI(pCreature) { }

        void Reset()
        {
            float angle = 0;
            for (int i = 0; i < 30; i++)
            {
                if (Creature *summon = me->SummonCreature(38348, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_MANUAL_DESPAWN))
                {
                    summon->GetMotionMaster()->MovePoint(0, -1710.466f + cos(angle) * 15, 1407.905f + sin(angle) * 15, 21.75196f);
                    if (i >= 0 && i <= 3)
                        worgenGUID[i] = summon->GetGUID();
                }
                angle += M_PI / 15;
            }
            me->GetMotionMaster()->MovePoint(0, -1710.466f, 1407.905f, 21.75196f);
            aboGUID = 0;
            mui_jumpBoss = 3000;
            moveJumpBoss = true;
        }

        void SetGUID(uint64 guid, int32 type = 0)
        {
            aboGUID = guid;
        }

        void EnterCombat(Unit* pWho)
        {
            if (pWho->GetEntry() == 38348)
                me->CastSpell(pWho, 71921, true);
            Talk(0);
        }

        void UpdateAI(const uint32 diff)
        {
            if (moveJumpBoss)
            {
                if (mui_jumpBoss <= diff)
                {
                    if (Creature *abo = Unit::GetCreature(*me, aboGUID))
                        for (int i = 0; i <= 3; i++)
                            if (Creature *worgen = Unit::GetCreature(*me, worgenGUID[i]))
                                worgen->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, i + 1, abo, false);
                    moveJumpBoss = false;
                    mui_jumpBoss = 3600000;
                }
                else mui_jumpBoss -= diff;
            }
            if (!UpdateVictim())
                return;
            DoMeleeAttackIfReady();
        }

    private:
        uint64 aboGUID;
        uint32 mui_jumpBoss;
        bool moveJumpBoss;
        uint64 worgenGUID[4];
    };
};

class npc_worgen_warrior : public CreatureScript
{
public:
    npc_worgen_warrior() : CreatureScript("npc_worgen_warrior") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_worgen_warriorAI(pCreature);
    }

    struct npc_worgen_warriorAI : public ScriptedAI
    {
        npc_worgen_warriorAI(Creature* pCreature) : ScriptedAI(pCreature) { }

        void Reset()
        {
        }

        void EnterCombat(Unit* pWho)
        {
            if (pWho->GetEntry() == 38348)
            {
                me->CastSpell(pWho, 71921, true);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };
};

class npc_lady_sylvanas_gilneas : public CreatureScript
{
public:
    npc_lady_sylvanas_gilneas() : CreatureScript("npc_lady_sylvanas_gilneas") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_lady_sylvanas_gilneasAI(pCreature);
    }

    struct npc_lady_sylvanas_gilneasAI : public ScriptedAI
    {
        npc_lady_sylvanas_gilneasAI(Creature* pCreature) : ScriptedAI(pCreature) { }

        bool outEvent;
        uint32 phase;
        uint32 phaseTimer;

        uint64 liamGUID;
        uint64 gennGUID;

        void Reset()
        {
            outEvent = false;
            phase = 1;
            phaseTimer = 300;
            liamGUID = 0;
            gennGUID = 0;
        }

        void EnterCombat(Unit* pWho)
        {

        }

        void DoAction(const int32 type)
        {
            if (type == ACTION_START_EVENT)
                outEvent = true;
        }

        void DamageTaken(Unit* who, uint32 &damage)
        {
            if (me->GetHealthPct() <= 20)
            {
                DoAction(ACTION_START_EVENT);
                damage = 0;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (outEvent)
            {
                if (phaseTimer <= diff)
                {
                    switch (phase)
                    {
                    case 1:
                    {
                        std::list<Player *> players = me->GetPlayersInRange(200, true);
                        for (std::list<Player *>::const_iterator it = players.begin(); it != players.end(); it++)
                            (*it)->KilledMonsterCredit(38854, 0);
                        phaseTimer = 1000;
                        DoCastAOE(72113);
                        break;
                    }
                    case 2:
                        me->SetReactState(REACT_PASSIVE);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        phaseTimer = 2000;
                        break;
                    case 3:
                        if (Creature *genn = me->FindNearestCreature(38470, 100))
                        {
                            gennGUID = genn->GetGUID();
                            genn->CastSpell(genn, 86141, true);
                        }
                        phaseTimer = 2000;
                        break;
                    case 4:
                        if (Creature *liam = me->SummonCreature(38474, -1634.634f, 1631.859f, 21.21159f, 4.694936f, TEMPSUMMON_TIMED_DESPAWN, 60000))
                        {
                            liamGUID = liam->GetGUID();
                            liam->AI()->Talk(0);
                            liam->SetReactState(REACT_PASSIVE);
                        }
                        phaseTimer = 2000;
                        break;
                    case 5:
                        if (Creature *liam = Unit::GetCreature(*me, liamGUID))
                        {
                            liam->GetMotionMaster()->MovePoint(0, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                            liam->AI()->Talk(1);
                        }
                        phaseTimer = 6000;
                        break;
                    case 7:
                        if (Creature *liam = Unit::GetCreature(*me, liamGUID))
                        {
                            liam->AI()->Talk(2);
                            me->CastSpell(liam, 72116, true);
                            me->Kill(liam);

                        }
                        if (Creature *genn = Unit::GetCreature(*me, gennGUID))
                        {
                            genn->RemoveAurasDueToSpell(86141);
                            genn->RemoveAurasDueToSpell(72113);
                            genn->CastSpell(genn, 68442, true);
                        }
                        phaseTimer = 2000;
                        break;
                    case 8:
                        Talk(0);
                        phaseTimer = 2000;
                        break;
                    case 9:
                        Talk(1);
                        phaseTimer = 3500;
                        break;
                    case 10:
                        Talk(2);
                        phaseTimer = 2500;
                        break;
                    case 11:
                        me->GetMotionMaster()->MovePoint(0, -1634.634f, 1631.859f, 21.21159f);
                        phaseTimer = 2500;
                        break;
                    case 12:
                        if (Creature *genn = Unit::GetCreature(*me, gennGUID))
                            genn->DespawnOrUnsummon(1);
                        outEvent = false;
                        me->DespawnOrUnsummon(1);
                        break;
                    }
                    phase++;
                } else phaseTimer -= diff;
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };
};

class npc_lorna_crowley_c2 : public CreatureScript
{
public:
    npc_lorna_crowley_c2() : CreatureScript("npc_lorna_crowley_c2") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == 24902)
            CAST_AI(npc_lorna_crowley_c2::npc_lorna_crowley_c2AI, creature->AI())->StartEncounter(player);
        return true;
    }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_lorna_crowley_c2AI(pCreature);
    }

    struct npc_lorna_crowley_c2AI : public ScriptedAI
    {
        npc_lorna_crowley_c2AI(Creature *c) : ScriptedAI(c) {}

        uint64 tobiasGUID;

        void Reset()
        {
            tobiasGUID = 0;
        }

        void StartEncounter(Player* pl)
        {
            if (Creature* tobias = Unit::GetCreature(*me, tobiasGUID))
                pl->TeleportTo(MAP_GILNEAS, tobias->GetPositionX(), tobias->GetPositionY(), tobias->GetPositionZ(), 5.42f);
            else
            {
                if (Creature* newTobias = me->SummonCreature(NPC_TOBIAS_MISTMANTLE, -1662.616f, 1591.453f, 23.20896f, 3.740811f, TEMPSUMMON_DEAD_DESPAWN, 180000))
                {
                    newTobias->SetPhaseMask(pl->GetPhaseMask(), true);
                    tobiasGUID = newTobias->GetGUID();
                }
            }
        }
    };
};

class npc_chauve_souris_capture : public CreatureScript
{
public:
    npc_chauve_souris_capture() : CreatureScript("npc_chauve_souris_capture") { }

    struct npc_chauve_souris_captureAI : public npc_escortAI
    {
        npc_chauve_souris_captureAI(Creature* creature) : npc_escortAI(creature) {}

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()
        {
            _checkQuest = 1000;
            _checkDespawn = 1000;
            isBoarded = false;
            isBoarded2 = false;
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (apply)
            {
                if (who->GetTypeId() == TYPEID_PLAYER)
                    if (who->ToPlayer()->GetQuestStatus(24920) != QUEST_STATUS_INCOMPLETE)
                    {
                        who->ExitVehicle();
                        return;
                    }
                me->SetCanFly(true);
                me->SetSpeed(MOVE_FLIGHT, 3.0f);
                isBoarded = true;
                Start(false, true, who->GetGUID(), NULL, false, true);
            }
        }

        void WaypointReached(uint32 i)
        {
            me->SetCanFly(true);
        }

        void JustDied(Unit* /*killer*/)
        {
        }

        void OnCharmed(bool /*apply*/)
        {
        }


        void UpdateAI(const uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);
        }

        void UpdateEscortAI(const uint32 diff)
        {
            if (isBoarded)
            {
                if (isBoarded2)
                {
                    if (_checkDespawn <= diff)
                    {
                        me->DespawnOrUnsummon(1000);
                        _checkDespawn = 1000;
                    }
                    else
                        _checkDespawn -= diff;
                }
                else
                {
                    if (_checkQuest <= diff)
                    {
                        if (me->GetVehicleKit())
                            if (Unit *u = me->GetVehicleKit()->GetPassenger(0))
                                if (u->GetTypeId() == TYPEID_PLAYER)
                                {
                                    Player *player = u->ToPlayer();
                                    if (player->GetQuestStatus(24920) == QUEST_STATUS_COMPLETE)
                                    {
                                        isBoarded2 = true;
                                        _checkDespawn = 70000;
                                        SetEscortPaused(true);
                                        me->GetMotionMaster()->MovePoint(4242, me->GetHomePosition());
                                    }
                                }
                        _checkQuest = 1000;
                    }
                    else
                        _checkQuest -= diff;
                }
            }
        }

    private :
        uint32 _checkQuest;
        uint32 _checkDespawn;
        bool isBoarded;
        bool isBoarded2;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_chauve_souris_captureAI (creature);
    }
};

void AddSC_gilneas()
{
    new npc_gilneas_city_guard_phase2();
    new npc_prince_liam_greymane_phase2();
    new npc_rampaging_worgen();
    new npc_rampaging_worgen2();
    new go_merchant_square_door();
    new npc_sergeant_cleese();
    new npc_bloodfang_worgen();
    new npc_frightened_citizen();
    new npc_gilnean_royal_guard();
    new npc_mariam_spellwalker();
    new npc_lord_darius_crowley_c3();
    new king_genn_1();
    new npc_crowley_horse();
    new spell_keg_placed();
    new npc_greymane_horse();
    new npc_krennan_aranas_c2();
    new npc_bloodfang_stalker_c1();
    new npc_lord_darius_crowley_c2();

    new StocksPlayerScript();
    new npc_trigger_event_first_c3();
    new npc_king_genn_greymane_c2();

    new go_mandragore();
    new spell_round_up_horse();
    new npc_trigger_quest_24616();

    new npc_krennan_aranas();
    new npc_prince_liam_greymane_gilneas();
    new npc_gilnean_militia();
    new npc_lord_darius_crowley_gilneas();
    new npc_worgen_warrior();
    new npc_lady_sylvanas_gilneas();
    new npc_lorna_crowley_c2();

    new npc_lorna_crowley_event();
    new npc_chauve_souris_capture();

    new SpellScriptLoaderEx<spell_curse_of_the_worgen_summon>("spell_curse_of_the_worgen_summon");

    new npc_forsaken_catapult();
}
