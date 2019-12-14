#ifndef _INFOMGR_H_
#define _INFOMGR_H_

#include "Common.h"
#include "ArenaTeam.h"
#include <ace/Singleton.h>
#include "ObjectMgr.h"

#define MAX_INFOCHAR_NAME_LENGTH (MAX_CHARTER_NAME * 2 + 1) // should be MAX_CHARACTER_NAME + 1 but i saw longer name in db... idk

struct InfoCharEntry
{
    uint32 Guid;
    char Name[MAX_INFOCHAR_NAME_LENGTH];
    uint8 Class;
    uint8 Race;
    uint8 Gender;
    uint8 Level;
    uint16 Zone;
    uint32 Guild;
    uint32 Account;
    uint32 Group;
    uint32 ArenaTeam[MAX_NORMAL_ARENA_SLOT];
    uint32 MMR[MAX_ARENA_SLOT];
    uint16 RbgWon;
    uint16 RbgPlayed;
    uint16 RBGRating;
};

struct InfoBGEntry
{
    uint8 slot;
    uint32 instanceID;
    uint32 team1;
    uint32 team2;
    uint16 MMR1;
    uint16 MMR2;
};

enum SlotStoreType
{
    SLOT_2VS2,
    SLOT_2VS2_TOP,
    SLOT_3VS3,
    SLOT_3VS3_TOP,
    SLOT_SOLO_QUEUE
};

class InfoMgr
{
public:
    //~InfoMgr();
    // Misc
    void Initialize();
    void UnloadAll();

    // Characters
    void UpdateCharBase(uint32 guid, std::string name, uint8 gender = GENDER_NONE, uint8 race = 0, uint8 cclass = 0, uint32 account = 0, uint8 level = 0, uint16 zone = 0, uint8 XPfactor = 0);
    void UpdateCharGuild(uint32 guid, uint32 guild);
    void UpdateCharGroup(uint32 guid, uint32 group);
    void UpdateCharArenaTeam(uint32 guid, uint32 team, uint8 slot);
    void UpdateCharMMR(uint32 guid, uint8 slot, uint16 mmr);
    void UpdateCharRBGstats(uint32 guid, uint16 win, uint16 played, uint16 rating);
    void UpdateCharLevel(uint32 guid, uint8 level);
    void RemoveCharInfo(uint32 guid);
    bool GetCharInfo(uint32 guid, InfoCharEntry &info);
    bool GetCharInfo(std::string name, InfoCharEntry &info);
    uint32 GetCharCount() { return m_charInfos.size(); }

    // Accounts
    void IncreaseAccountCharCount(uint32 id);
    void DecreaseAccountCharCount(uint32 id);
    uint8 GetAccountCharCount(uint32 id);

    // Pets
    /*void FillPetList(uint32 ownerId, SavedPetList &petList);
    void SavePet(SavedPet* pet, Player* owner);
    void DeletePet(SavedPet* pet, Player* owner);
    void DeleteAllPetsFromOwner(uint32 ownerGuid);*/
    // Battlegrounds
    void AddBGInfo(SlotStoreType Slot, uint32 instanceID, uint32 team1, uint32 team2, uint16 mmr1, uint16 mmr2);
    typedef std::map<uint32, InfoBGEntry> BGInfoMap;
    void RemoveBGInfo(SlotStoreType Slot, uint32 instanceID);
    InfoBGEntry GetBgInfo(SlotStoreType Slot, uint32 InstanceID);
    BGInfoMap GetBgStore(SlotStoreType Slot)
    {
        ACE_Guard<ACE_Thread_Mutex> g(m_BgInfoMutex);
        return m_bgStore[Slot];
    }

private:
    // Characters
    void _RemoveName(std::string name);
    void _AddName(std::string name, InfoCharEntry *info);
    typedef std::map<std::string, InfoCharEntry*> CharInfoNameMap;
    typedef std::map<uint32, InfoCharEntry*> CharInfoMap;
    typedef CharInfoNameMap::iterator CharInfoNameItr;
    typedef CharInfoMap::iterator CharInfoItr;
    CharInfoNameMap m_charInfosName;
    CharInfoMap m_charInfos;
    ACE_Thread_Mutex m_charMutex;

    // Accounts
    void SetAccountCharCount(uint32 id, uint8 count);
    typedef std::map<uint32, uint8> AccountCharCountMap;
    typedef AccountCharCountMap::iterator AccountCharCountMapItr;
    AccountCharCountMap m_accountCharCounts;
    ACE_Thread_Mutex m_accountMutex;

    // Pets
    /*typedef std::map<uint32, SavedPet*> PetsMap;
    typedef PetsMap::iterator PetsMapItr;
    typedef std::set<SavedPet*> PetsSet;
    typedef PetsSet::iterator PetsSetItr;
    typedef std::map<uint32, PetsSet*> PetsToOwnerMap;
    typedef PetsToOwnerMap::iterator PetsToOwnerMapItr;
    PetsMap petsMap;
    PetsToOwnerMap petsToOwner;
    ACE_Thread_Mutex petsMutex;
    void UpdatePet(uint32 id, uint32 entry, uint32 ownerId, uint32 modelId, uint32 createdBySpell, uint32 type, uint8 reacteState, std::string name, uint8 renamed, uint8 slot,
        uint32 health, uint32 mana, uint32 happiness, uint32 saveTime, std::string actionbars, uint8 level, uint32 experience);
    void UpdatePetAuras(uint32 guid, uint64 casterGuid, uint32 spellId, uint8 effectMask, uint8 recalcMask, uint32 stacks, int32 amount_0, int32 amount_1, int32 amount_2, int32 baseAmount_0, int32 baseAmount_1, int32 baseAmount_2, int32 maxDuration, int32 remDuration, uint8 charges);
    void UpdatePetSpells(uint32 id, uint32 spellId, uint8 active);
    void UpdatePetSpellCooldowns(uint32 id, uint32 spellId, uint32 time);*/

    // Battlegrounds
    typedef std::map<uint8, BGInfoMap> BGStoreMap;
    BGInfoMap m_bgInfos;
    BGStoreMap m_bgStore;
    ACE_Thread_Mutex m_BgInfoMutex;
};

#define sInfoMgr ACE_Singleton<InfoMgr, ACE_Null_Mutex>::instance()

#endif
