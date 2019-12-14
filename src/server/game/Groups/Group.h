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

#ifndef TRINITYCORE_GROUP_H
#define TRINITYCORE_GROUP_H

#include "DBCEnums.h"
#include "GroupRefManager.h"
#include "LootMgr.h"
#include "QueryResult.h"
#include "SharedDefines.h"

class Battlefield;
class Battleground;
class Creature;
class GroupReference;
class InstanceSave;
class Map;
class Player;
class SpellInfo;
class Unit;
class WorldObject;
class WorldPacket;
class WorldSession;

struct MapEntry;
struct Position;

#define MAXGROUPSIZE 5
#define MAXRAIDSIZE 40
#define MAX_RAID_SUBGROUPS MAXRAIDSIZE/MAXGROUPSIZE
#define TARGETICONCOUNT 8

enum RollVote
{
    PASS              = 0,
    NEED              = 1,
    GREED             = 2,
    DISENCHANT        = 3,
    NOT_EMITED_YET    = 4,
    NOT_VALID         = 5
};

enum GroupMemberOnlineStatus
{
    MEMBER_STATUS_OFFLINE   = 0x0000,
    MEMBER_STATUS_ONLINE    = 0x0001,                       // Lua_UnitIsConnected
    MEMBER_STATUS_PVP       = 0x0002,                       // Lua_UnitIsPVP
    MEMBER_STATUS_DEAD      = 0x0004,                       // Lua_UnitIsDead
    MEMBER_STATUS_GHOST     = 0x0008,                       // Lua_UnitIsGhost
    MEMBER_STATUS_PVP_FFA   = 0x0010,                       // Lua_UnitIsPVPFreeForAll
    MEMBER_STATUS_UNK3      = 0x0020,                       // used in calls from Lua_GetPlayerMapPosition/Lua_GetBattlefieldFlagPosition
    MEMBER_STATUS_AFK       = 0x0040,                       // Lua_UnitIsAFK
    MEMBER_STATUS_DND       = 0x0080                        // Lua_UnitIsDND
};

enum GroupMemberFlags
{
    MEMBER_FLAG_ASSISTANT   = 0x01,
    MEMBER_FLAG_MAINTANK    = 0x02,
    MEMBER_FLAG_MAINASSIST  = 0x04
};

enum GroupMemberAssignment
{
    GROUP_ASSIGN_MAINTANK   = 0,
    GROUP_ASSIGN_MAINASSIST = 1
};

enum GroupType
{
    GROUPTYPE_NORMAL = 0x00,
    GROUPTYPE_BG     = 0x01,
    GROUPTYPE_RAID   = 0x02,
    GROUPTYPE_BGRAID = GROUPTYPE_BG | GROUPTYPE_RAID,       // mask
    GROUPTYPE_LFG_RESTRICTED = 0x04,
    GROUPTYPE_LFG    = 0x08
    // 0x10, leave/change group?, I saw this flag when leaving group and after leaving BG while in group
};

enum GroupUpdateFlags
{
    GROUP_UPDATE_FLAG_NONE              = 0x00000000,       // nothing
    GROUP_UPDATE_FLAG_STATUS            = 0x00000001,       // uint16 (GroupMemberStatusFlag)
    GROUP_UPDATE_FLAG_CUR_HP            = 0x00000002,       // uint32 (HP)
    GROUP_UPDATE_FLAG_MAX_HP            = 0x00000004,       // uint32 (HP)
    GROUP_UPDATE_FLAG_POWER_TYPE        = 0x00000008,       // uint8 (PowerType)
    GROUP_UPDATE_FLAG_CUR_POWER         = 0x00000010,       // int16 (power value)
    GROUP_UPDATE_FLAG_MAX_POWER         = 0x00000020,       // int16 (power value)
    GROUP_UPDATE_FLAG_LEVEL             = 0x00000040,       // uint16 (level value)
    GROUP_UPDATE_FLAG_ZONE              = 0x00000080,       // uint16 (zone id)
    GROUP_UPDATE_FLAG_UNK100            = 0x00000100,       // int16 (unk)
    GROUP_UPDATE_FLAG_POSITION          = 0x00000200,       // uint16 (x), uint16 (y), uint16 (z)
    GROUP_UPDATE_FLAG_AURAS             = 0x00000400,       // uint8 (unk), uint64 (mask), uint32 (count), for each bit set: uint32 (spell id) + uint16 (AuraFlags)  (if has flags Scalable -> 3x int32 (bps))
    GROUP_UPDATE_FLAG_PET_GUID          = 0x00000800,       // uint64 (pet guid)
    GROUP_UPDATE_FLAG_PET_NAME          = 0x00001000,       // cstring (name, NULL terminated string)
    GROUP_UPDATE_FLAG_PET_MODEL_ID      = 0x00002000,       // uint16 (model id)
    GROUP_UPDATE_FLAG_PET_CUR_HP        = 0x00004000,       // uint32 (HP)
    GROUP_UPDATE_FLAG_PET_MAX_HP        = 0x00008000,       // uint32 (HP)
    GROUP_UPDATE_FLAG_PET_POWER_TYPE    = 0x00010000,       // uint8 (PowerType)
    GROUP_UPDATE_FLAG_PET_CUR_POWER     = 0x00020000,       // uint16 (power value)
    GROUP_UPDATE_FLAG_PET_MAX_POWER     = 0x00040000,       // uint16 (power value)
    GROUP_UPDATE_FLAG_PET_AURAS         = 0x00080000,       // [see GROUP_UPDATE_FLAG_AURAS]
    GROUP_UPDATE_FLAG_VEHICLE_SEAT      = 0x00100000,       // int32 (vehicle seat id)
    GROUP_UPDATE_FLAG_PHASE             = 0x00200000,       // int32 (unk), uint32 (phase count), for (count) uint16(phaseId)
    GROUP_UPDATE_FLAG_UNK400000         = 0x00400000,
    GROUP_UPDATE_FLAG_UNK800000         = 0x00800000,
    GROUP_UPDATE_FLAG_UNK1000000        = 0x01000000,
    GROUP_UPDATE_FLAG_UNK2000000        = 0x02000000,
    GROUP_UPDATE_FLAG_UNK4000000        = 0x04000000,
    GROUP_UPDATE_FLAG_UNK8000000        = 0x08000000,
    GROUP_UPDATE_FLAG_UNK10000000       = 0x10000000,
    GROUP_UPDATE_FLAG_UNK20000000       = 0x20000000,
    GROUP_UPDATE_FLAG_UNK40000000       = 0x40000000,

    GROUP_UPDATE_PET = GROUP_UPDATE_FLAG_PET_GUID | GROUP_UPDATE_FLAG_PET_NAME | GROUP_UPDATE_FLAG_PET_MODEL_ID |
                       GROUP_UPDATE_FLAG_PET_CUR_HP | GROUP_UPDATE_FLAG_PET_MAX_HP | GROUP_UPDATE_FLAG_PET_POWER_TYPE |
                       GROUP_UPDATE_FLAG_PET_CUR_POWER | GROUP_UPDATE_FLAG_PET_MAX_POWER | GROUP_UPDATE_FLAG_PET_AURAS, // all pet flags
    GROUP_UPDATE_FULL = GROUP_UPDATE_FLAG_STATUS | GROUP_UPDATE_FLAG_CUR_HP | GROUP_UPDATE_FLAG_MAX_HP |
                        GROUP_UPDATE_FLAG_CUR_POWER | GROUP_UPDATE_FLAG_MAX_POWER | GROUP_UPDATE_FLAG_LEVEL |
                        GROUP_UPDATE_FLAG_ZONE | GROUP_UPDATE_FLAG_UNK100 | GROUP_UPDATE_FLAG_POSITION |
                        GROUP_UPDATE_FLAG_AURAS | GROUP_UPDATE_FLAG_PHASE | GROUP_UPDATE_FLAG_POWER_TYPE |
                        GROUP_UPDATE_FLAG_UNK400000 | GROUP_UPDATE_FLAG_UNK800000 | GROUP_UPDATE_FLAG_UNK1000000 |
                        GROUP_UPDATE_FLAG_UNK2000000 | GROUP_UPDATE_FLAG_UNK4000000 | GROUP_UPDATE_FLAG_UNK8000000 |
                        GROUP_UPDATE_FLAG_UNK10000000 | GROUP_UPDATE_FLAG_UNK20000000 | GROUP_UPDATE_FLAG_UNK40000000
};

class Roll : public LootValidatorRef
{
    public:
        Roll(uint64 _guid, LootItem const& li);
        ~Roll();
        void setLoot(Loot* pLoot);
        Loot* getLoot();
        void targetObjectBuildLink();

        uint64 itemGUID;
        uint32 itemid;
        int32  itemRandomPropId;
        uint32 itemRandomSuffix;
        uint8 itemCount;
        typedef std::map<uint64, RollVote> PlayerVote;
        PlayerVote playerVote;                              //vote position correspond with player position (in group)
        uint8 totalPlayersRolling;
        uint8 totalNeed;
        uint8 totalGreed;
        uint8 totalPass;
        uint8 itemSlot;
        uint8 rollVoteMask;
};

struct InstanceGroupBind
{
    InstanceSave* save;
    bool perm;
    bool lfg;
    /* permanent InstanceGroupBinds exist if the leader has a permanent
       PlayerInstanceBind for the same instance. */
    InstanceGroupBind() : save(NULL), perm(false), lfg(false) {}
};

/** request member stats checken **/
/** todo: uninvite people that not accepted invite **/
class Group
{
    public:
        struct MemberSlot
        {
            uint64      guid;
            std::string name;
            uint8       group;
            uint8       flags;
            uint8       roles;
            uint32      guildId;
        };
        typedef std::list<MemberSlot> MemberSlotList;
        typedef MemberSlotList::const_iterator member_citerator;

        typedef UNORDERED_MAP< uint32 /*mapId*/, InstanceGroupBind> BoundInstancesMap;
    protected:
        typedef MemberSlotList::iterator member_witerator;
        typedef std::set<Player*> InvitesList;

        typedef std::vector<Roll*> Rolls;

    public:
        Group();
        ~Group();

        // group manipulation methods
        bool   Create(Player* leader);
        void   LoadGroupFromDB(Field* field);
        void   LoadMemberFromDB(uint32 guidLow, uint8 memberFlags, uint8 subgroup, uint8 roles);
        bool   AddInvite(Player* player);
        void   RemoveInvite(Player* player);
        void   RemoveAllInvites();
        bool   AddLeaderInvite(Player* player);
        bool   AddMember(Player* player);
        bool   RemoveMember(uint64 guid, const RemoveMethod &method = GROUP_REMOVEMETHOD_DEFAULT, uint64 kicker = 0, const char* reason = NULL);
        void   ChangeLeader(uint64 guid);
        void   SetLootMethod(LootMethod method);
        void   SetLooterGuid(uint64 guid);
        void   UpdateLooterGuid(WorldObject* pLootedObject, bool ifneed = false);
        void   SetLootThreshold(ItemQualities threshold);
        void   Disband(bool hideDestroy=false);
        void   SetLfgRoles(uint64 guid, const uint8 roles);
        uint8  GetLfgRoles(uint64 guid);

        // properties accessories
        bool IsFull() const;
        bool isLFGGroup()  const;
        bool isLFRGroup()  const;
        bool isRaidGroup() const;
        bool isBGGroup()   const;
        bool isBFGroup()   const;
        bool IsCreated()   const;
        uint64 GetLeaderGUID() const;
        uint64 GetGUID() const;
        uint32 GetLowGUID() const;
        const char * GetLeaderName() const;
        LootMethod GetLootMethod() const;
        uint64 GetLooterGuid() const;
        ItemQualities GetLootThreshold() const;
        uint8 GetGroupType() { return m_groupType; }

        uint32 GetDbStoreId() const { return m_dbStoreId; };

        // member manipulation methods
        bool IsMember(uint64 guid) const;
        bool IsLeader(uint64 guid) const;
        uint64 GetMemberGUID(const std::string& name);
        bool IsAssistant(uint64 guid) const;

        Player* GetInvited(uint64 guid) const;
        Player* GetInvited(const std::string& name) const;

        bool SameSubGroup(uint64 guid1, uint64 guid2) const;
        bool SameSubGroup(uint64 guid1, MemberSlot const* slot2) const;
        bool SameSubGroup(Player const* member1, Player const* member2) const;
        bool HasFreeSlotSubGroup(uint8 subgroup) const;

        MemberSlotList const& GetMemberSlots() const { return m_memberSlots; }
        GroupReference* GetFirstMember() { return m_memberMgr.getFirst(); }
        GroupReference const* GetFirstMember() const { return m_memberMgr.getFirst(); }
        uint32 GetMembersCount() const { return m_memberSlots.size(); }
        uint8 GetGuildMembersCount(uint64 guid) const;

        uint8 GetMemberGroup(uint64 guid) const;

        void ConvertToLFG();
        void ConvertToLFR();
        void ConvertToRaid();
        void ConvertToGroup();

        void SetBattlegroundGroup(Battleground* bg);
        void SetBattlefieldGroup(Battlefield* bf);
        GroupJoinBattlegroundResult CanJoinBattlegroundQueue(Battleground const* bgOrTemplate, BattlegroundQueueTypeId bgQueueTypeId, uint32 MinPlayerCount, uint32 MaxPlayerCount, bool isRated, uint32 arenaSlot);

        void ChangeMembersGroup(uint64 guid, uint8 group);
        void SetTargetIcon(uint8 id, uint64 whoGuid, uint64 targetGuid);
        void SetGroupMemberFlag(uint64 guid, bool apply, GroupMemberFlags flag);
        void SetEveryoneAssistant(Group* group, bool active);
        void RemoveUniqueGroupMemberFlag(GroupMemberFlags flag);

        Difficulty GetDifficulty(bool isRaid) const;
        Difficulty GetDungeonDifficulty() const;
        Difficulty GetRaidDifficulty() const;
        void SetDungeonDifficulty(Difficulty difficulty);
        void SetRaidDifficulty(Difficulty difficulty, uint32 raidId = 0);
        bool InCombatToInstance(uint32 instanceId);
        void ResetInstances(uint8 method, bool isRaid, Player* SendMsgTo);

        // -no description-
        //void SendInit(WorldSession* session);
        void SendTargetIconList(WorldSession* session);
        void SendUpdate();
        void SendUpdateToPlayer(uint64 playerGUID, MemberSlot* slot = NULL);
        void UpdatePlayerOutOfRange(Player* player);
        void LogoutPlayerOutOfRange(Player* player);
                                                            // ignore: GUID of player that will be ignored
        void BroadcastPacket(WorldPacket* packet, bool ignorePlayersInBGRaid, int group = -1, uint64 ignore = 0);
        void BroadcastAddonMessagePacket(WorldPacket* packet, const std::string& prefix, bool ignorePlayersInBGRaid, int group = -1, uint64 ignore = 0);
        void BroadcastReadyCheck(WorldPacket* packet);
        void OfflineReadyCheck();

        /*********************************************************/
        /***                   LOOT SYSTEM                     ***/
        /*********************************************************/

        bool isRollLootActive() const;
        void SendLootStartRoll(uint32 CountDown, uint32 mapid, const Roll &r);
        void SendLootStartRollToPlayer(uint32 countDown, uint32 mapId, Player* p, bool canNeed, Roll const& r, bool canLfrInteract);
        void SendLootRoll(uint64 SourceGuid, uint64 TargetGuid, int8 RollNumber, uint8 RollType, const Roll &r);
        void SendLootRollWon(uint64 SourceGuid, uint64 TargetGuid, int8 RollNumber, uint8 RollType, const Roll &r);
        void SendLootAllPassed(Roll const& roll);
        void SendLooter(Creature* creature, Player* pLooter);
        void GroupLoot(Loot* loot, WorldObject* pLootedObject);
        void NeedBeforeGreed(Loot* loot, WorldObject* pLootedObject);
        void MasterLoot(Loot* loot, WorldObject* pLootedObject);
        Rolls::iterator GetRoll(uint64 Guid);
        void CountTheRoll(Rolls::iterator roll);
        void CountRollVote(uint64 playerGUID, uint64 Guid, uint8 Choise);
        void EndRoll(Loot* loot);

        // related to disenchant rolls
        void ResetMaxEnchantingLevel();

        void LinkMember(GroupReference* pRef);
        void DelinkMember(uint64 guid);

        InstanceGroupBind* BindToInstance(InstanceSave* save, bool permanent, bool load = false);
        void UnbindInstance(uint32 mapid, uint8 difficulty, bool unload = false, bool isLfgId = false);
        void ProcessBoundSwitch(InstanceSave *save, Difficulty previousDifficulty, Difficulty newDifficulty, std::list<uint32 > &mapToErase, Player *leader = NULL);
        void SwitchBoundInstance(uint32 /*mapid*/, Difficulty previousDifficulty, Difficulty newDifficulty);
        InstanceGroupBind* GetBoundInstance(Player* player, bool isLfgId = false);
        InstanceGroupBind* GetBoundInstance(Map* aMap, bool isLfgId = false);
        InstanceGroupBind* GetBoundInstance(MapEntry const* mapEntry, bool isLfgId = false);
        InstanceGroupBind* GetBoundInstance(Difficulty difficulty, uint32 mapId, bool isLfgId = false);
        BoundInstancesMap& GetBoundInstances(Difficulty difficulty);

        //RaidMarker System
        void SetMarker(uint8 slot, Position const& destTarget, Unit* caster, SpellInfo const* spell);
        void RemoveMarker(uint8 slot);
        void SendRaidMarkerChanged();

        // FG: evil hacks
        void BroadcastGroupUpdate(void);

        // Use for guild system
        bool IsGuildGroupFor(Player *player);
        uint32 GetMembersCountOfGuild(uint32 guildId);
        uint32 GetNeededMembersOfSameGuild(uint8 arenaType, Map const *map);
        float GetGuildXpRateForPlayer(Player *player);
        void UpdateGuildFor(uint64 guid, uint32 guildId);
        bool MemberLevelIsInRange(uint32 levelMin, uint32 levelMax);
        void AddBoundSwithCoolDown() { m_boundSwitchCooldown = time(NULL); }
        bool HasBoundSwitchCoolDown(uint32 t) { return ((time(NULL) - m_boundSwitchCooldown) < t); }
        void SetRaidRoles(uint64 guid, int32 role) { m_raidRoles[guid] = role; }
        int32 GetRaidRoles(uint64 guid) { return m_raidRoles[guid]; }
        bool IsNonPermanentLFGGroup() const;


    protected:
        bool _setMembersGroup(uint64 guid, uint8 group);
        void _homebindIfInstance(Player* player);

        void _initRaidSubGroupsCounter();
        member_citerator _getMemberCSlot(uint64 Guid) const;
        member_witerator _getMemberWSlot(uint64 Guid);
        void SubGroupCounterIncrease(uint8 subgroup);
        void SubGroupCounterDecrease(uint8 subgroup);
        void ToggleGroupMemberFlag(member_witerator slot, uint8 flag, bool apply);

        MemberSlotList      m_memberSlots;
        GroupRefManager     m_memberMgr;
        InvitesList         m_invitees;
        uint64              m_leaderGuid;
        std::string         m_leaderName;
        GroupType           m_groupType;
        Difficulty          m_dungeonDifficulty;
        Difficulty          m_raidDifficulty;
        Battleground*       m_bgGroup;
        Battlefield*        m_bfGroup;
        uint64              m_markers[6];
        uint64              m_targetIcons[TARGETICONCOUNT];
        LootMethod          m_lootMethod;
        ItemQualities       m_lootThreshold;
        uint64              m_looterGuid;
        Rolls               RollId;
        BoundInstancesMap   m_boundInstances[MAX_DIFFICULTY];
        uint8*              m_subGroupsCounts;
        uint64              m_guid;
        uint32              m_counter;                      // used only in SMSG_GROUP_LIST
        uint32              m_maxEnchantingLevel;
        uint32              m_dbStoreId;                    // Represents the ID used in database (Can be reused by other groups if group was disbanded)
        time_t              m_boundSwitchCooldown;
        std::map<uint64, int32 >    m_raidRoles;
};
#endif
