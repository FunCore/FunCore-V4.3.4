/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "Common.h"
#include "Language.h"
#include "DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Player.h"
#include "GossipDef.h"
#include "UpdateMask.h"
#include "ObjectAccessor.h"
#include "Creature.h"
#include "Pet.h"
#include "ReputationMgr.h"
#include "BattlegroundMgr.h"
#include "Battleground.h"
#include "BattlefieldMgr.h"
#include "Battlefield.h"
#include "ScriptMgr.h"
#include "CreatureAI.h"
#include "PassiveAI.h"
#include "SpellInfo.h"

#define CAST_AI(a, b)   (dynamic_cast<a*>(b))

enum StableResultCode
{
    STABLE_ERR_MONEY        = 0x01,                         // "you don't have enough money"
    STABLE_ERR_STABLE       = 0x06,                         // currently used in most fail cases
    STABLE_SUCCESS_STABLE   = 0x08,                         // stable success
    STABLE_SUCCESS_UNSTABLE = 0x09,                         // unstable/swap success
    STABLE_SUCCESS_BUY_SLOT = 0x0A,                         // buy slot success
    STABLE_ERR_EXOTIC       = 0x0C                          // "you are unable to control exotic creatures"
};

void WorldSession::HandleTabardVendorActivateOpcode(WorldPacket& recvData)
{
    uint64 guid;
    recvData >> guid;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_TABARDDESIGNER);
    if (!unit)
    {
        TC_LOG_DEBUG("network.opcode", "WORLD: HandleTabardVendorActivateOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendTabardVendorActivate(guid);
}

void WorldSession::SendTabardVendorActivate(uint64 guid)
{
    WorldPacket data(MSG_TABARDVENDOR_ACTIVATE, 8);
    data << guid;
    SendPacket(&data);
}

void WorldSession::HandleBankerActivateOpcode(WorldPacket& recvData)
{
    uint64 guid;

    TC_LOG_DEBUG("network.opcode", "WORLD: Received CMSG_BANKER_ACTIVATE");

    recvData >> guid;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_BANKER);
    if (!unit)
    {
        TC_LOG_DEBUG("network.opcode", "WORLD: HandleBankerActivateOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendShowBank(guid);
}

void WorldSession::SendShowBank(uint64 guid)
{
    WorldPacket data(SMSG_SHOW_BANK, 8);
    data << guid;
    SendPacket(&data);
}

void WorldSession::HandleTrainerListOpcode(WorldPacket& recvData)
{
    uint64 guid;

    recvData >> guid;
    SendTrainerList(guid);
}

void WorldSession::SendTrainerList(uint64 guid)
{
    std::string str = GetTrinityString(LANG_NPC_TAINER_HELLO);
    SendTrainerList(guid, str);
}

void WorldSession::SendTrainerList(uint64 guid, const std::string& strTitle)
{
    TC_LOG_DEBUG("network.opcode", "WORLD: SendTrainerList");

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_TRAINER);
    if (!unit)
    {
        TC_LOG_DEBUG("network.opcode", "WORLD: SendTrainerList - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    // trainer list loaded at check;
    if (!unit->isCanTrainingOf(_player, true))
        return;

    CreatureTemplate const* ci = unit->GetCreatureTemplate();

    if (!ci)
    {
        TC_LOG_DEBUG("network.opcode", "WORLD: SendTrainerList - (GUID: %u) NO CREATUREINFO!", GUID_LOPART(guid));
        return;
    }

    TrainerSpellData const* trainer_spells = unit->GetTrainerSpells();
    if (!trainer_spells)
    {
        TC_LOG_DEBUG("network.opcode", "WORLD: SendTrainerList - Training spells not found for creature (GUID: %u Entry: %u)",
            GUID_LOPART(guid), unit->GetEntry());
        return;
    }

    WorldPacket data(SMSG_TRAINER_LIST, 8+4+4+trainer_spells->spellList.size()*38 + strTitle.size()+1);
    data << guid;
    data << uint32(trainer_spells->trainerType);
    data << uint32(1); // different value for each trainer, also found in CMSG_TRAINER_BUY_SPELL

    size_t count_pos = data.wpos();
    data << uint32(trainer_spells->spellList.size());

    // reputation discount
    float fDiscountMod = _player->GetReputationPriceDiscount(unit);
    bool can_learn_primary_prof = GetPlayer()->GetFreePrimaryProfessionPoints() > 0;

    uint32 count = 0;
    for (TrainerSpellMap::const_iterator itr = trainer_spells->spellList.begin(); itr != trainer_spells->spellList.end(); ++itr)
    {
        TrainerSpell const* tSpell = &itr->second;

        bool valid = true;
        bool primary_prof_first_rank = false;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (!tSpell->learnedSpell[i])
                continue;
            if (!_player->IsSpellFitByClassAndRace(tSpell->learnedSpell[i]))
            {
                valid = false;
                break;
            }
            SpellInfo const* learnedSpellInfo = sSpellMgr->GetSpellInfo(tSpell->learnedSpell[i]);
            if (learnedSpellInfo && learnedSpellInfo->IsPrimaryProfessionFirstRank())
                primary_prof_first_rank = true;
        }
        if (!valid)
            continue;

        TrainerSpellState state = _player->GetTrainerSpellState(tSpell);

        data << uint32(tSpell->spell);                      // learned spell (or cast-spell in profession case)
        data << uint8(state == TRAINER_SPELL_GREEN_DISABLED ? TRAINER_SPELL_GREEN : state);
        data << uint32(floor(tSpell->spellCost * fDiscountMod));

        data << uint8(tSpell->reqLevel);
        data << uint32(tSpell->reqSkill);
        data << uint32(tSpell->reqSkillValue);
        //prev + req or req + 0
        uint8 maxReq = 0;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (!tSpell->learnedSpell[i])
                continue;
            if (uint32 prevSpellId = sSpellMgr->GetPrevSpellInChain(tSpell->learnedSpell[i]))
            {
                data << uint32(prevSpellId);
                ++maxReq;
            }
            if (maxReq == 2)
                break;
            SpellsRequiringSpellMapBounds spellsRequired = sSpellMgr->GetSpellsRequiredForSpellBounds(tSpell->learnedSpell[i]);
            for (SpellsRequiringSpellMap::const_iterator itr2 = spellsRequired.first; itr2 != spellsRequired.second && maxReq < 3; ++itr2)
            {
                data << uint32(itr2->second);
                ++maxReq;
            }
            if (maxReq == 2)
                break;
        }
        while (maxReq < 2)
        {
            data << uint32(0);
            ++maxReq;
        }

        data << uint32(primary_prof_first_rank && can_learn_primary_prof ? 1 : 0);
        // primary prof. learn confirmation dialog
        data << uint32(primary_prof_first_rank ? 1 : 0);    // must be equal prev. field to have learn button in enabled state

        ++count;
    }

    data << strTitle;

    data.put<uint32>(count_pos, count);
    SendPacket(&data);
}

void WorldSession::HandleTrainerBuySpellOpcode(WorldPacket& recvData)
{
    uint64 guid;
    uint32 spellId;
    uint32 trainerId;

    recvData >> guid >> trainerId >> spellId;
    TC_LOG_DEBUG("network.opcode", "WORLD: Received CMSG_TRAINER_BUY_SPELL NpcGUID=%u, learn spell id is: %u", uint32(GUID_LOPART(guid)), spellId);

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_TRAINER);
    if (!unit)
    {
        TC_LOG_DEBUG("network.opcode", "WORLD: HandleTrainerBuySpellOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if (!unit->isCanTrainingOf(_player, true))
    {
        SendTrainerBuyFailed(guid, spellId, 0);
        return;
    }

    // check present spell in trainer spell list
    TrainerSpellData const* trainer_spells = unit->GetTrainerSpells();
    if (!trainer_spells)
    {
        SendTrainerBuyFailed(guid, spellId, 0);
        return;
    }

    // not found, cheat?
    TrainerSpell const* trainer_spell = trainer_spells->Find(spellId);
    if (!trainer_spell)
    {
        SendTrainerBuyFailed(guid, spellId, 0);
        return;
    }

    // can't be learn, cheat? Or double learn with lags...
    if (_player->GetTrainerSpellState(trainer_spell) != TRAINER_SPELL_GREEN)
    {
        SendTrainerBuyFailed(guid, spellId, 0);
        return;
    }

    // apply reputation discount
    uint32 nSpellCost = uint32(floor(trainer_spell->spellCost * _player->GetReputationPriceDiscount(unit)));

    // check money requirement
    if (!_player->HasEnoughMoney(uint64(nSpellCost)))
    {
        SendTrainerBuyFailed(guid, spellId, 1);
        return;
    }

    _player->ModifyMoney(-int64(nSpellCost));

    unit->SendPlaySpellVisualKit(179, 0);       // 53 SpellCastDirected
    _player->SendPlaySpellVisualKit(362, 1);    // 113 EmoteSalute

    // learn explicitly or cast explicitly
    if (trainer_spell->IsCastable())
        _player->CastSpell(_player, trainer_spell->spell, true);
    else
        _player->learnSpell(spellId, false);

    WorldPacket data(SMSG_TRAINER_BUY_SUCCEEDED, 12);
    data << uint64(guid);
    data << uint32(spellId);
    SendPacket(&data);
}

void WorldSession::SendTrainerBuyFailed(uint64 guid, uint32 spellId, uint32 reason)
{
    WorldPacket data(SMSG_TRAINER_BUY_FAILED, 16);
    data << uint64(guid);
    data << uint32(spellId);        // should be same as in packet from client
    data << uint32(reason);         // 1 == "Not enough money for trainer service." 0 == "Trainer service %d unavailable."
    SendPacket(&data);
}

void WorldSession::HandleGossipHelloOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network.opcode", "WORLD: Received CMSG_GOSSIP_HELLO");

    uint64 guid;
    recvData >> guid;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_NONE);
    if (!unit)
    {
        TC_LOG_DEBUG("network.opcode", "WORLD: HandleGossipHelloOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // set faction visible if needed
    if (FactionTemplateEntry const* factionTemplateEntry = sFactionTemplateStore.LookupEntry(unit->getFaction()))
        _player->GetReputationMgr().SetVisible(factionTemplateEntry);

    GetPlayer()->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TALK);
    // remove fake death
    //if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
    //    GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if (unit->isArmorer() || unit->isCivilian() || unit->isQuestGiver() || unit->isServiceProvider() || unit->isGuard())
        unit->StopMoving();

    // If spiritguide, no need for gossip menu, just put player into resurrect queue
    if (unit->isSpiritGuide())
    {
        Battleground* bg = _player->GetBattleground();
        if (bg)
        {
            bg->AddPlayerToResurrectQueue(unit->GetGUID(), _player->GetGUID());
            sBattlegroundMgr->SendAreaSpiritHealerQueryOpcode(_player, bg, unit->GetGUID());
            return;
        }
        else if (Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(_player->GetZoneId()))
        {
            bf->AddPlayerToResurrectQueue(unit->GetGUID(), _player->GetGUID());
            bf->SendAreaSpiritHealerQueryOpcode(_player, unit->GetGUID());
            return;
        }
    }

    if (unit->GetAIName() == "TeleporterAI")
    {
        if (!CAST_AI(TeleporterAI, unit->AI())->OnGossipHello(_player, unit))
        {
            //        _player->TalkedToCreature(unit->GetEntry(), unit->GetGUID());
            _player->PrepareGossipMenu(unit, unit->GetCreatureTemplate()->GossipMenuId, true);
            _player->SendPreparedGossip(unit);
        }
    }
    else if (!sScriptMgr->OnGossipHello(_player, unit))
    {
        //        _player->TalkedToCreature(unit->GetEntry(), unit->GetGUID());
        _player->PrepareGossipMenu(unit, unit->GetCreatureTemplate()->GossipMenuId, true);
        _player->SendPreparedGossip(unit);
    }
    unit->AI()->sGossipHello(_player);
}

/*void WorldSession::HandleGossipSelectOptionOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG(LOG_FILTER_PACKETIO, "WORLD: CMSG_GOSSIP_SELECT_OPTION");

    uint32 option;
    uint32 unk;
    uint64 guid;
    std::string code = "";

    recvData >> guid >> unk >> option;

    if (_player->PlayerTalkClass->GossipOptionCoded(option))
    {
        TC_LOG_DEBUG(LOG_FILTER_PACKETIO, "reading string");
        recvData >> code;
        TC_LOG_DEBUG(LOG_FILTER_PACKETIO, "string read: %s", code.c_str());
    }

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_NONE);
    if (!unit)
    {
        TC_LOG_DEBUG(LOG_FILTER_PACKETIO, "WORLD: HandleGossipSelectOptionOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    if (!code.empty())
    {
        if (!Script->GossipSelectWithCode(_player, unit, _player->PlayerTalkClass->GossipOptionSender (option), _player->PlayerTalkClass->GossipOptionAction(option), code.c_str()))
            unit->OnGossipSelect (_player, option);
    }
    else
    {
        if (!Script->OnGossipSelect (_player, unit, _player->PlayerTalkClass->GossipOptionSender (option), _player->PlayerTalkClass->GossipOptionAction (option)))
           unit->OnGossipSelect (_player, option);
    }
}*/

void WorldSession::HandleSpiritHealerActivateOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network.opcode", "WORLD: CMSG_SPIRIT_HEALER_ACTIVATE");

    uint64 guid;

    recvData >> guid;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_SPIRITHEALER);
    if (!unit)
    {
        TC_LOG_DEBUG("network.opcode", "WORLD: HandleSpiritHealerActivateOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendSpiritResurrect();
}

void WorldSession::SendSpiritResurrect()
{
    _player->ResurrectPlayer(0.5f, true);

    _player->DurabilityLossAll(0.25f, true);

    // get corpse nearest graveyard
    WorldSafeLocsEntry const* corpseGrave = NULL;
    Corpse* corpse = _player->GetCorpse();
    if (corpse)
        corpseGrave = sObjectMgr->GetClosestGraveYard(
            corpse->GetPositionX(), corpse->GetPositionY(), corpse->GetPositionZ(), corpse->GetMapId(), _player->GetTeam());

    // now can spawn bones
    _player->SpawnCorpseBones();

    // teleport to nearest from corpse graveyard, if different from nearest to player ghost
    if (corpseGrave)
    {
        WorldSafeLocsEntry const* ghostGrave = sObjectMgr->GetClosestGraveYard(
            _player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetMapId(), _player->GetTeam());

        if (corpseGrave != ghostGrave)
            _player->TeleportTo(corpseGrave->map_id, corpseGrave->x, corpseGrave->y, corpseGrave->z, _player->GetOrientation());
        // or update at original position
        else
            _player->UpdateObjectVisibility();
    }
    // or update at original position
    else
        _player->UpdateObjectVisibility();
}

void WorldSession::HandleBinderActivateOpcode(WorldPacket& recvData)
{
    uint64 npcGUID;
    recvData >> npcGUID;

    if (!GetPlayer()->IsInWorld() || !GetPlayer()->isAlive())
        return;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(npcGUID, UNIT_NPC_FLAG_INNKEEPER);
    if (!unit)
    {
        TC_LOG_DEBUG("network.opcode", "WORLD: HandleBinderActivateOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(npcGUID)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    SendBindPoint(unit);
}

void WorldSession::SendBindPoint(Creature* npc)
{
    // prevent set homebind to instances in any case
    if (GetPlayer()->GetMap()->Instanceable())
        return;

    uint32 bindspell = 3286;

    // send spell for homebinding (3286)
    _player->CastSpell(_player, bindspell, true);

    WorldPacket data(SMSG_TRAINER_BUY_SUCCEEDED, 12);
    data << uint64(npc->GetGUID());
    data << uint32(bindspell);
    SendPacket(&data);

    _player->PlayerTalkClass->SendCloseGossip();
}

void WorldSession::HandleListStabledPetsOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network.opcode", "WORLD: Recv MSG_LIST_STABLED_PETS");
    uint64 npcGUID;

    recvData >> npcGUID;

    if (!CheckStableMaster(npcGUID))
        return;

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    // remove mounts this fix bug where getting pet from stable while mounted deletes pet.
    if (GetPlayer()->IsMounted())
        GetPlayer()->RemoveAurasByType(SPELL_AURA_MOUNTED);

    SendStablePet(npcGUID);
}

void WorldSession::SendStablePet(uint64 guid)
{
    if (!GetPlayer())
        return;

    TC_LOG_DEBUG("network.opcode", "WORLD: Recv MSG_LIST_STABLED_PETS Send.");

    WorldPacket data(MSG_LIST_STABLED_PETS, 200);           // guess size

    data << uint64 (guid);

    size_t wpos = data.wpos();
    data << uint8(0);                                       // place holder for slot show number

    data << uint8(20);

    uint8 num = 0;                                          // counter for place holder

    for (int i= PET_SLOT_HUNTER_FIRST; i < PET_SLOT_STABLE_LAST;i++)
    {
        PetData* t_pet = _player->GetPetDatabySlot(i);

        if (t_pet && t_pet->_state != DATA_REMOVED)
        {
            data << uint32(t_pet->slot);
            data << uint32(t_pet->id);            // petnumber
            data << uint32(t_pet->entry);         // creature entry
            data << uint32(t_pet->level);         // level
            data << t_pet->name;                  // name

            if (t_pet->slot < PET_SLOT_HUNTER_LAST)
                data << uint8(1);                    // 1 = current, 2/3 = in stable (any from 4, 5, ... create problems with proper show)
            else
                data << uint8(2);                    // 1 = current, 2/3 = in stable (any from 4, 5, ... create problems with proper show)

            ++num;
        }

    }

    data.put<uint8>(wpos, num);                             // set real data to placeholder
    SendPacket(&data);
}

void WorldSession::SendStableResult(uint8 res)
{
    WorldPacket data(SMSG_STABLE_RESULT, 1);
    data << uint8(res);
    SendPacket(&data);
}

void WorldSession::HandleStableRevivePet(WorldPacket &/* recvData */)
{
    TC_LOG_DEBUG("network.opcode", "HandleStableRevivePet: Not implemented");
}

void WorldSession::HandleStableSwapPet(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network.opcode", "WORLD: Recv CMSG_STABLE_SWAP_PET.");
    ObjectGuid guid;
    uint32 petId;
    uint8 new_slot;

    recvData >> petId >> new_slot;

    guid[3] = recvData.ReadBit();
    guid[2] = recvData.ReadBit();
    guid[0] = recvData.ReadBit();
    guid[7] = recvData.ReadBit();
    guid[5] = recvData.ReadBit();
    guid[6] = recvData.ReadBit();
    guid[1] = recvData.ReadBit();
    guid[4] = recvData.ReadBit();

    recvData.ReadByteSeq(guid[5]);
    recvData.ReadByteSeq(guid[3]);
    recvData.ReadByteSeq(guid[1]);
    recvData.ReadByteSeq(guid[7]);
    recvData.ReadByteSeq(guid[4]);
    recvData.ReadByteSeq(guid[0]);
    recvData.ReadByteSeq(guid[6]);
    recvData.ReadByteSeq(guid[2]);

    if (!CheckStableMaster(guid))
    {
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    PetData* t_pet = _player->GetPetDatabyGuid(petId);
    if(!t_pet)
    {
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    HandleStableSwapPetCallback(t_pet,new_slot);
}

void WorldSession::HandleStableSwapPetCallback(PetData* t_pet, uint32 newslot)
{
    if (!GetPlayer())
        return;

    uint32 slot = t_pet->slot;

    if (!t_pet->entry)
    {
        SendStableResult(STABLE_ERR_STABLE);
        return;
    }

    if (newslot < PET_SLOT_STABLE_FIRST)
    {
        CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(t_pet->entry);
        if (!creatureInfo || !creatureInfo->isTameable(_player->CanTameExoticPets()))
        {
            // if problem in exotic pet
            if (creatureInfo && creatureInfo->isTameable(true))
                SendStableResult(STABLE_ERR_EXOTIC);
            else
                SendStableResult(STABLE_ERR_STABLE);
            return;
        }
    }

    PetData* t_newslot = _player->GetPetDatabySlot(newslot);

    Pet* p_pet = _player->GetPet();
    if(p_pet && (p_pet->GetGUIDMid() == t_pet->id || (t_newslot && p_pet->GetGUIDMid() == t_newslot->id) ) && newslot >=5)
    {
        _player->RemovePet(p_pet, PetSlot(newslot));
        _player->_currentPetSlot = PetSlot(0);
    }

    if(t_newslot && t_newslot->_state != DATA_REMOVED)
    {
        t_newslot->slot     = slot;
        t_newslot->_state   = DATA_CHANGED;
        t_pet->slot         = newslot;
        t_pet->_state       = DATA_CHANGED;
    }
    else
    {
        t_pet->slot         = newslot;
        t_pet->_state       = DATA_CHANGED;
    }

    SendStableResult(STABLE_SUCCESS_STABLE);
}

void WorldSession::HandleRepairItemOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network.opcode", "WORLD: CMSG_REPAIR_ITEM");

    uint64 npcGUID, itemGUID;
    uint8 guildBank;                                        // new in 2.3.2, bool that means from guild bank money

    recvData >> npcGUID >> itemGUID >> guildBank;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(npcGUID, UNIT_NPC_FLAG_REPAIR);
    if (!unit)
    {
        TC_LOG_DEBUG("network.opcode", "WORLD: HandleRepairItemOpcode - Unit (GUID: %u) not found or you can not interact with him.", uint32(GUID_LOPART(npcGUID)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    // reputation discount
    float discountMod = _player->GetReputationPriceDiscount(unit);

    if (itemGUID)
    {
        TC_LOG_DEBUG("network.opcode", "ITEM: Repair item, itemGUID = %u, npcGUID = %u", GUID_LOPART(itemGUID), GUID_LOPART(npcGUID));

        Item* item = _player->GetItemByGuid(itemGUID);
        if (item)
            _player->DurabilityRepair(item->GetPos(), true, discountMod, guildBank);
    }
    else
    {
        TC_LOG_DEBUG("network.opcode", "ITEM: Repair all items, npcGUID = %u", GUID_LOPART(npcGUID));
        _player->DurabilityRepairAll(true, discountMod, guildBank);
    }
}
