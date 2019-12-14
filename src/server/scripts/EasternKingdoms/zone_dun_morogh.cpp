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

#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "ScriptMgr.h"

/*
    Gnome Starting Zone
*/

/*
 *  @Npc   : Sanitron 500 (46185)
 *  @Quest : Decontamination (27635)
 *  @Descr : Board the Sanitron 500 to begin the decontamination process.
 */

enum NpcSanotron500
{
    SAY_CONTAMINATION_START     = 0,
    SAY_CONTAMINATION_3         = 1,
    SAY_CONTAMINATION_OVERLOAD  = 2,

    SAY_UGH_NOT_THIS            = 0,
    SAY_OH_NO                   = 1,

    NPC_DECONTAMINATION_BUNNY   = 46165,
    NPC_CLEAN_CANNON            = 46208,
    NPC_SAFE_TECHNICAN          = 46230,

    SPELL_DECONTAMINATION_STAGE_1       = 86075,
    SPELL_DECONTAMINATION_STAGE_2       = 86084,
    SPELL_DECONTAMINATION_STAGE_3       = 86086,
    SPELL_DECONTAMINATION_STAGE_2_ALT   = 86098,
    SPELL_DECONTAMINATION               = 86106,
    SPELL_CLEAN_CANNON_CLEAN_BURST      = 86080,
    SPELL_IRRADIATED                    = 80653,
    SPELL_EXPLOSION                     = 30934,
};

class DistanceSelector
{
    public:
        DistanceSelector(Unit* source, uint32 const distance) : _source(source), _distance(distance) {}

        bool operator()(Creature* creature)
        {
            return _source->GetDistance(creature) > _distance;
        }

    private:
        Unit* _source;
        uint32 const _distance;
};

class npc_sanotron_500 : public CreatureScript
{
public:
    npc_sanotron_500() : CreatureScript("npc_sanotron_500") { }

    struct npc_sanotron_500AI : public npc_escortAI
    {
        npc_sanotron_500AI(Creature* creature) : npc_escortAI(creature) {}

        void Reset()
        {
            Position const& pos = me->GetHomePosition();
            me->NearTeleportTo(pos.m_positionX, pos.m_positionY, pos.m_positionZ, pos.m_orientation);
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (who->GetTypeId() != TYPEID_PLAYER || !apply)
                return;

            Start(false, true, who->GetGUID());
            Talk(SAY_CONTAMINATION_START);
        }

        void OnCharmed(bool /*apply*/) {}

        void WaypointReached(uint32 point)
        {
            Player* player = GetPlayerForEscort();

            switch(point)
            {
            case 2:
                HandleStop(NPC_DECONTAMINATION_BUNNY, 10, SPELL_DECONTAMINATION_STAGE_1);
                break;
            case 3:
                TechnicanTalk(SAY_UGH_NOT_THIS);
                HandleStop(NPC_CLEAN_CANNON, 35, SPELL_CLEAN_CANNON_CLEAN_BURST);
                me->CastSpell(player, SPELL_DECONTAMINATION_STAGE_2, true);
                player->RemoveAura(SPELL_IRRADIATED);
                break;
            case 4:
                Talk(SAY_CONTAMINATION_3);
                HandleStop(NPC_DECONTAMINATION_BUNNY, 10, SPELL_DECONTAMINATION_STAGE_3);
                break;
            case 5:
                Talk(SAY_CONTAMINATION_OVERLOAD);
                DoCast(SPELL_EXPLOSION);
                me->DespawnOrUnsummon(1500);
                TechnicanTalk(SAY_OH_NO);
                break;
            }
        }

    private:
        void HandleStop(uint32 const entry, uint32 const distance, uint32 const spellId)
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            std::list<Creature*> creatures;
            GetCreatureListWithEntryInGrid(creatures, me, entry, float(distance + 3));
            creatures.remove_if(DistanceSelector(me, distance));

            for (std::list<Creature*>::const_iterator itr = creatures.begin(); itr != creatures.end(); ++itr)
                (*itr)->CastSpell(player, spellId, true);
        }

        void TechnicanTalk(uint32 const groupId)
        {
            if (Creature* technican = me->FindNearestCreature(NPC_SAFE_TECHNICAN, 20.f))
                technican->AI()->Talk(groupId);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_sanotron_500AI (creature);
    }
};

enum Misc
{
    NPC_COLDRIDGE_DEFENDER      = 37177,
    NPC_ROCKJAW_INVADER         = 37070
};

class npc_coldridge_defender : public CreatureScript
{
public:
    npc_coldridge_defender() : CreatureScript("npc_coldridge_defender") { }

    struct npc_coldridge_defenderAI : public ScriptedAI
    {
        npc_coldridge_defenderAI(Creature* creature) : ScriptedAI(creature)
        {
            me->HandleEmoteCommand(EMOTE_STATE_READY1H);
        }

        EventMap events;

        void EnterCombat(Unit * who)
        {
            me->AddUnitState(UNIT_STATE_ROOT);
            me->HandleEmoteCommand(EMOTE_STATE_READY1H);
        }

        void Reset()
        {
            me->HandleEmoteCommand(EMOTE_STATE_READY1H);
        }

        void DamageTaken(Unit* attacker, uint32& damage)
        {
            if (attacker->GetEntry() == NPC_ROCKJAW_INVADER && ((me->GetHealth() - damage) <= me->GetHealth() / 2))
                return;
        }

        void UpdateAI(const uint32 diff)
        {
            DoMeleeAttackIfReady();
        }
    };
    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_coldridge_defenderAI (creature);
    }
};

class npc_rockjaw_defender : public CreatureScript
{
public:
    npc_rockjaw_defender() : CreatureScript("npc_rockjaw_defender") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rockjaw_defenderAI (creature);
    }

    struct npc_rockjaw_defenderAI : public ScriptedAI
    {
        npc_rockjaw_defenderAI(Creature* creature) : ScriptedAI(creature) { }

        void DamageTaken(Unit* who, uint32& damage)
        {
            if (who->GetEntry() == NPC_COLDRIDGE_DEFENDER && ((me->GetHealth() - damage) <= me->GetHealth() / 2))
                return;

            if (who->GetTypeId() == TYPEID_PLAYER || who->isPet())
            {
                if (Creature* guard = me->FindNearestCreature(NPC_COLDRIDGE_DEFENDER, 6.0f, true))
                {
                    guard->getThreatManager().resetAllAggro();
                    guard->CombatStop(true);
                }

                me->getThreatManager().resetAllAggro();
                me->GetMotionMaster()->MoveChase(who);
                me->AI()->AttackStart(who);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                if (Creature* guard = me->FindNearestCreature(NPC_COLDRIDGE_DEFENDER, 6.0f, true))
                    me->AI()->AttackStart(guard);

                DoMeleeAttackIfReady();
        }
    };
};


void AddSC_dun_morogh()
{
    new npc_sanotron_500();
    new npc_coldridge_defender();
    new npc_rockjaw_defender();
}
