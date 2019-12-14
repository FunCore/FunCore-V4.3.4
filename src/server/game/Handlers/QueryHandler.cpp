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
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"
#include "NPCHandler.h"
#include "Pet.h"
#include "MapManager.h"
#include "InfoMgr.h"

void WorldSession::SendNameQueryOpcode(uint64 guid)
{
    Player* player = ObjectAccessor::FindPlayer(guid);
    InfoCharEntry info;
    if(!player)
    {
        strcpy(info.Name, "");
        sInfoMgr->GetCharInfo(GUID_LOPART(guid), info);
    }

    WorldPacket data(SMSG_NAME_QUERY_RESPONSE, (8+1+1+1+1+1+10));
    data.appendPackGUID(guid);
    if (!player && info.Name == "")
    {
        data << uint8(1);                           // name unknown
        SendPacket(&data);
        return;
    }

    data << uint8(0);                               // name known
    data << (player ? player->GetName() : info.Name);                       // played name
    data << uint8(0);                               // realm name - only set for cross realm interaction (such as Battlegrounds)
    data << (player ? player->getRace() : info.Race);
    data << (player ? player->getGender() : info.Gender);
    data << (player ? player->getClass() : info.Class);

    if (DeclinedName const* names = (player ? player->GetDeclinedNames() : NULL))
    {
        data << uint8(1);                           // Name is declined
        for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            data << names->name[i];
    }
    else
        data << uint8(0);                           // Name is not declined

    SendPacket(&data);
}

void WorldSession::HandleNameQueryOpcode(WorldPacket& recvData)
{
    uint64 guid;
    recvData >> guid;

    // This is disable by default to prevent lots of console spam
    // TC_LOG_INFO("network.opcode", "HandleNameQueryOpcode %u", guid);

    SendNameQueryOpcode(guid);
}

void WorldSession::HandleQueryTimeOpcode(WorldPacket & /*recvData*/)
{
    SendQueryTimeResponse();
}

void WorldSession::SendQueryTimeResponse()
{
    WorldPacket data(SMSG_QUERY_TIME_RESPONSE, 4+4);
    data << uint32(time(NULL));
    data << uint32(sWorld->GetNextDailyQuestsResetTime() - time(NULL));
    SendPacket(&data);
}

/// Only _static_ data is sent in this packet !!!
void WorldSession::HandleCreatureQueryOpcode(WorldPacket& recvData)
{
    uint32 entry;
    recvData >> entry;
    uint64 guid;
    recvData >> guid;

    CreatureTemplate const* ci = sObjectMgr->GetCreatureTemplate(entry);
    if (ci)
    {

        std::string Name, SubName;
        Name = ci->Name;
        SubName = ci->SubName;

        int loc_idx = GetSessionDbLocaleIndex();
        if (loc_idx >= 0)
        {
            if (CreatureLocale const* cl = sObjectMgr->GetCreatureLocale(entry))
            {
                ObjectMgr::GetLocaleString(cl->Name, loc_idx, Name);
                ObjectMgr::GetLocaleString(cl->SubName, loc_idx, SubName);
            }
        }
        TC_LOG_DEBUG("network.opcode", "WORLD: CMSG_CREATURE_QUERY '%s' - Entry: %u.", ci->Name.c_str(), entry);
                                                            // guess size
        WorldPacket data(SMSG_CREATURE_QUERY_RESPONSE, 100);
        data << uint32(entry);                              // creature entry
        data << Name;

        for (int i = 0; i < 7; i++)
            data << uint8(0); // name2, ..., name8

        data << SubName;
        data << ci->IconName;                               // "Directions" for guard, string for Icons 2.3.0
        data << uint32(ci->type_flags);                     // flags
        data << uint32(ci->type_flags2);                    // unknown meaning
        data << uint32(ci->type);                           // CreatureType.dbc
        data << uint32(ci->family);                         // CreatureFamily.dbc
        data << uint32(ci->rank);                           // Creature Rank (elite, boss, etc)
        data << uint32(ci->KillCredit[0]);                  // new in 3.1, kill credit
        data << uint32(ci->KillCredit[1]);                  // new in 3.1, kill credit
        data << uint32(ci->Modelid1);                       // Modelid1
        data << uint32(ci->Modelid2);                       // Modelid2
        data << uint32(ci->Modelid3);                       // Modelid3
        data << uint32(ci->Modelid4);                       // Modelid4
        data << float(ci->ModHealth);                       // dmg/hp modifier
        data << float(ci->ModMana);                         // dmg/mana modifier
        data << uint8(ci->RacialLeader);
        for (uint32 i = 0; i < MAX_CREATURE_QUEST_ITEMS; ++i)
            data << uint32(ci->questItems[i]);              // itemId[6], quest drop
        data << uint32(ci->movementId);                     // CreatureMovementInfo.dbc
        data << uint32(ci->expansionUnknown);               // unknown meaning
        SendPacket(&data);
        TC_LOG_DEBUG("network.opcode", "WORLD: Sent SMSG_CREATURE_QUERY_RESPONSE");
    }
    else
    {
        TC_LOG_DEBUG("network.opcode", "WORLD: CMSG_CREATURE_QUERY - NO CREATURE INFO! (GUID: %u, ENTRY: %u)",
            GUID_LOPART(guid), entry);
        WorldPacket data(SMSG_CREATURE_QUERY_RESPONSE, 4);
        data << uint32(entry | 0x80000000);
        SendPacket(&data);
        TC_LOG_DEBUG("network.opcode", "WORLD: Sent SMSG_CREATURE_QUERY_RESPONSE");
    }
}

/// Only _static_ data is sent in this packet !!!
void WorldSession::HandleGameObjectQueryOpcode(WorldPacket& recvData)
{
    uint32 entry;
    recvData >> entry;
    uint64 guid;
    recvData >> guid;

    const GameObjectTemplate* info = sObjectMgr->GetGameObjectTemplate(entry);
    if (info)
    {
        std::string Name;
        std::string IconName;
        std::string CastBarCaption;

        Name = info->name;
        IconName = info->IconName;
        CastBarCaption = info->castBarCaption;

        int loc_idx = GetSessionDbLocaleIndex();
        if (loc_idx >= 0)
        {
            if (GameObjectLocale const* gl = sObjectMgr->GetGameObjectLocale(entry))
            {
                ObjectMgr::GetLocaleString(gl->Name, loc_idx, Name);
                ObjectMgr::GetLocaleString(gl->CastBarCaption, loc_idx, CastBarCaption);
            }
        }
        TC_LOG_DEBUG("network.opcode", "WORLD: CMSG_GAMEOBJECT_QUERY '%s' - Entry: %u. ", info->name.c_str(), entry);
        WorldPacket data (SMSG_GAMEOBJECT_QUERY_RESPONSE, 150);
        data << uint32(entry);
        data << uint32(info->type);
        data << uint32(info->displayId);
        data << Name;
        data << uint8(0) << uint8(0) << uint8(0);           // name2, name3, name4
        data << IconName;                                   // 2.0.3, string. Icon name to use instead of default icon for go's (ex: "Attack" makes sword)
        data << CastBarCaption;                             // 2.0.3, string. Text will appear in Cast Bar when using GO (ex: "Collecting")
        data << info->unk1;                                 // 2.0.3, string
        data.append(info->raw.data, MAX_GAMEOBJECT_DATA);
        data << float(info->size);                          // go size
        for (uint32 i = 0; i < MAX_GAMEOBJECT_QUEST_ITEMS; ++i)
            data << uint32(info->questItems[i]);            // itemId[6], quest drop
        data << int32(info->unkInt32);                      // 4.x, unknown
        SendPacket(&data);
        TC_LOG_DEBUG("network.opcode", "WORLD: Sent SMSG_GAMEOBJECT_QUERY_RESPONSE");
    }
    else
    {
        TC_LOG_DEBUG("network.opcode", "WORLD: CMSG_GAMEOBJECT_QUERY - Missing gameobject info for (GUID: %u, ENTRY: %u)",
            GUID_LOPART(guid), entry);
        WorldPacket data (SMSG_GAMEOBJECT_QUERY_RESPONSE, 4);
        data << uint32(entry | 0x80000000);
        SendPacket(&data);
        TC_LOG_DEBUG("network.opcode", "WORLD: Sent SMSG_GAMEOBJECT_QUERY_RESPONSE");
    }
}

void WorldSession::HandleCorpseQueryOpcode(WorldPacket& /*recvData*/)
{
    TC_LOG_DEBUG("network.opcode", "WORLD: Received MSG_CORPSE_QUERY");

    Corpse* corpse = GetPlayer()->GetCorpse();

    if (!corpse)
    {
        WorldPacket data(MSG_CORPSE_QUERY, 1);
        data << uint8(0);                                   // corpse not found
        SendPacket(&data);
        return;
    }

    uint32 mapid = corpse->GetMapId();
    float x = corpse->GetPositionX();
    float y = corpse->GetPositionY();
    float z = corpse->GetPositionZ();
    uint32 corpsemapid = mapid;

    // if corpse at different map
    if (mapid != _player->GetMapId())
    {
        // search entrance map for proper show entrance
        if (MapEntry const* corpseMapEntry = sMapStore.LookupEntry(mapid))
        {
            if (corpseMapEntry->IsDungeon() && corpseMapEntry->entrance_map >= 0)
            {
                // if corpse map have entrance
                if (Map const* entranceMap = sMapMgr->CreateBaseMap(corpseMapEntry->entrance_map))
                {
                    mapid = corpseMapEntry->entrance_map;
                    x = corpseMapEntry->entrance_x;
                    y = corpseMapEntry->entrance_y;
                    z = entranceMap->GetHeight(GetPlayer()->GetPhaseMask(), x, y, MAX_HEIGHT);
                }
            }
        }
    }

    WorldPacket data(MSG_CORPSE_QUERY, 1+(6*4));
    data << uint8(1);                                       // corpse found
    data << int32(mapid);
    data << float(x);
    data << float(y);
    data << float(z);
    data << int32(corpsemapid);
    data << uint32(0);                                      // unknown
    SendPacket(&data);
}

void WorldSession::HandleNpcTextQueryOpcode(WorldPacket& recvData)
{
    uint32 textID;
    uint64 guid;

    recvData >> textID;
    TC_LOG_DEBUG("network.opcode", "WORLD: CMSG_NPC_TEXT_QUERY ID '%u'", textID);

    recvData >> guid;
    GetPlayer()->SetSelection(guid);

    GossipText const* gossip = sObjectMgr->GetGossipText(textID);

    WorldPacket data(SMSG_NPC_TEXT_UPDATE, 100);          // guess size
    data << textID;

    if (!gossip)
    {
        for (uint32 i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; ++i)
        {
            data << float(0);
            data << "Greetings $N";
            data << "Greetings $N";
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
        }
    }
    else
    {
        std::string text0[MAX_GOSSIP_TEXT_OPTIONS], text1[MAX_GOSSIP_TEXT_OPTIONS];
        LocaleConstant locale = GetSessionDbLocaleIndex();

        for (uint8 i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; ++i)
        {
            uint32 emoteDelay[MAX_GOSSIP_TEXT_EMOTES];
            uint32 emoteIds[MAX_GOSSIP_TEXT_EMOTES];
            BroadcastText const* bct = sObjectMgr->GetBroadcastText(gossip->Options[i].BroadcastTextID);
            if (bct)
            {
                text0[i] = bct->GetText(locale, GENDER_MALE, true);
                text1[i] = bct->GetText(locale, GENDER_FEMALE, true);
                emoteDelay[0] = bct->EmoteDelay0;
                emoteIds[0] = bct->EmoteId0;
                emoteDelay[1] = bct->EmoteDelay1;
                emoteIds[1] = bct->EmoteId1;
                emoteDelay[2] = bct->EmoteDelay2;
                emoteIds[2] = bct->EmoteId2;
            }
            else
            {
                text0[i] = gossip->Options[i].Text_0;
                text1[i] = gossip->Options[i].Text_1;
                for (uint8 j = 0; j < MAX_GOSSIP_TEXT_EMOTES; ++j)
                {
                    emoteDelay[j] = gossip->Options[i].Emotes[j]._Delay;
                    emoteIds[j] = gossip->Options[i].Emotes[j]._Emote;
                }
            }

            if (locale != DEFAULT_LOCALE && !bct)
            {
                if (NpcTextLocale const* npcTextLocale = sObjectMgr->GetNpcTextLocale(textID))
                {
                    ObjectMgr::GetLocaleString(npcTextLocale->Text_0[i], locale, text0[i]);
                    ObjectMgr::GetLocaleString(npcTextLocale->Text_1[i], locale, text1[i]);
                }
            }
            data << gossip->Options[i].Probability;

            if (text0[i].empty())
                data << text1[i];
            else
                data << text0[i];

            if (text1[i].empty())
                data << text0[i];
            else
                data << text1[i];

            data << gossip->Options[i].Language;

            for (uint8 j = 0; j < MAX_GOSSIP_TEXT_EMOTES; ++j)
            {
                data << emoteDelay[j];
                data << emoteIds[j];
            }
        }
    }

    SendPacket(&data);

    TC_LOG_DEBUG("network.opcode", "WORLD: Sent SMSG_NPC_TEXT_UPDATE");
}

/// Only _static_ data is sent in this packet !!!
void WorldSession::HandlePageTextQueryOpcode(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network.opcode", "WORLD: Received CMSG_PAGE_TEXT_QUERY");

    uint32 pageID;
    recvData >> pageID;
    recvData.read_skip<uint64>();                          // guid

    while (pageID)
    {
        PageText const* pageText = sObjectMgr->GetPageText(pageID);
                                                            // guess size
        WorldPacket data(SMSG_PAGE_TEXT_QUERY_RESPONSE, 50);
        data << pageID;

        if (!pageText)
        {
            data << "Item page missing.";
            data << uint32(0);
            pageID = 0;
        }
        else
        {
            std::string Text = pageText->Text;

            int loc_idx = GetSessionDbLocaleIndex();
            if (loc_idx >= 0)
                if (PageTextLocale const* player = sObjectMgr->GetPageTextLocale(pageID))
                    ObjectMgr::GetLocaleString(player->Text, loc_idx, Text);

            data << Text;
            data << uint32(pageText->NextPage);
            pageID = pageText->NextPage;
        }
        SendPacket(&data);

        TC_LOG_DEBUG("network.opcode", "WORLD: Sent SMSG_PAGE_TEXT_QUERY_RESPONSE");
    }
}

void WorldSession::HandleCorpseMapPositionQuery(WorldPacket& recvData)
{
    TC_LOG_DEBUG("network.opcode", "WORLD: Recv CMSG_CORPSE_MAP_POSITION_QUERY");

    uint32 transportGuidLow;
    recvData >> transportGuidLow;

    WorldPacket data(SMSG_CORPSE_MAP_POSITION_QUERY_RESPONSE, 4+4+4+4);
    data << float(0);
    data << float(0);
    data << float(0);
    data << float(0);
    SendPacket(&data);
}

void WorldSession::HandleQuestPOIQuery(WorldPacket& recvData)
{
    uint32 count;
    recvData >> count; // quest count, max=25

    if (count >= MAX_QUEST_LOG_SIZE)
    {
        recvData.rfinish();
        return;
    }

    WorldPacket data(SMSG_QUEST_POI_QUERY_RESPONSE, 4+(4+4)*count);
    data << uint32(count); // count

    for (uint32 i = 0; i < count; ++i)
    {
        uint32 questId;
        recvData >> questId; // quest id

        bool questOk = false;

        uint16 questSlot = _player->FindQuestSlot(questId);

        if (questSlot != MAX_QUEST_LOG_SIZE)
            questOk =_player->GetQuestSlotQuestId(questSlot) == questId;

        if (questOk)
        {
            QuestPOIVector const* POI = sObjectMgr->GetQuestPOIVector(questId);

            if (POI)
            {
                data << uint32(questId); // quest ID
                data << uint32(POI->size()); // POI count

                for (QuestPOIVector::const_iterator itr = POI->begin(); itr != POI->end(); ++itr)
                {
                    data << uint32(itr->Id);                // POI index
                    data << int32(itr->ObjectiveIndex);     // objective index
                    data << uint32(itr->MapId);             // mapid
                    data << uint32(itr->AreaId);            // areaid
                    data << uint32(itr->Unk2);              // unknown
                    data << uint32(itr->Unk3);              // unknown
                    data << uint32(itr->Unk4);              // unknown
                    data << uint32(itr->points.size());     // POI points count

                    for (std::vector<QuestPOIPoint>::const_iterator itr2 = itr->points.begin(); itr2 != itr->points.end(); ++itr2)
                    {
                        data << int32(itr2->x); // POI point x
                        data << int32(itr2->y); // POI point y
                    }
                }
            }
            else
            {
                data << uint32(questId); // quest ID
                data << uint32(0); // POI count
            }
        }
        else
        {
            data << uint32(questId); // quest ID
            data << uint32(0); // POI count
        }
    }

    SendPacket(&data);
}

void WorldSession::HandleQuestNpcQueryOpcode(WorldPacket& recvData)
{
    std::set<uint32> quests;
    std::map<uint32, std::set<uint32> > questIdsToNpcOrGoIds;
    uint32 questCount = recvData.ReadBits(24);
    uint32 involvedCount = 0;

    for (uint8 i = 0; i < questCount; ++i)
    {
        uint32 questId;
        recvData >> questId;
        std::set<uint32> npcOrGoIds;
        quests.insert(questId);

        if (std::set<uint32> const* npcs = sObjectMgr->GetInvolvedNpcsForQuest(questId))
        {
            for (std::set<uint32>::const_iterator itr = npcs->begin(); itr != npcs->end(); ++itr)
            {
                npcOrGoIds.insert(*itr);
                ++involvedCount;
            }
        }

        if (std::set<uint32> const* gos = sObjectMgr->GetInvolvedGosForQuest(questId))
        {
            for (std::set<uint32>::const_iterator itr = gos->begin(); itr != gos->end(); ++itr)
            {
                npcOrGoIds.insert(*itr | 0x80000000);
                ++involvedCount;
            }
        }

        questIdsToNpcOrGoIds.insert(std::pair<uint32, std::set<uint32> >(questId, npcOrGoIds));

        TC_LOG_DEBUG("network.opcode", "WORLD: Recv CMSG_QUEST_NPC_QUERY with count : %u and questId %u ", questCount, questId);
    }

    WorldPacket sendData(SMSG_QUEST_NPC_QUERY_RESPONSE, 4 + 4 + (4 * questCount) + (4 * involvedCount));

    sendData.WriteBits<uint8>(quests.size(), 23);
    for (std::map<uint32, std::set<uint32> >::iterator qToNItr = questIdsToNpcOrGoIds.begin(); qToNItr != questIdsToNpcOrGoIds.end(); ++qToNItr)
    {
        std::set<uint32> npcsOrgos = qToNItr->second;
        sendData.WriteBits<uint8>(npcsOrgos.size(), 24);
    }

    for (std::map<uint32, std::set<uint32> >::iterator qToNpcsOrGosItr = questIdsToNpcOrGoIds.begin(); qToNpcsOrGosItr != questIdsToNpcOrGoIds.end(); ++qToNpcsOrGosItr)
    {
        std::set<uint32> npcsOrGos = qToNpcsOrGosItr->second;
        sendData << qToNpcsOrGosItr->first;
        for (std::set<uint32>::iterator nOgItr = npcsOrGos.begin(); nOgItr != npcsOrGos.end(); ++nOgItr)
            sendData << (*nOgItr);
    }

    SendPacket(&sendData);
}
