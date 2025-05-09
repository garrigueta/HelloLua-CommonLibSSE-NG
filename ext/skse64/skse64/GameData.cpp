#include "skse64/GameData.h"

// 9615953AF64D4A2EB76F7BDE0F38D30B311B1351+40
RelocAddr <UInt32*> g_gameTime(0x031CC290);

// B8860AA646A4AA64DB8D5C6EDA306FC6B59F1B9B+5B
RelocPtr <DataHandler *> g_dataHandler(0x020F6320);

// 5F417CF4D2EB33C7D6903EA38BB5CDDEF48A6F83+4B
RelocPtr <PersistentFormManager *> g_persistentFormManager(0x020F69C8);

// EC410889481EE7B9DC4B6D7A707FA41B72CDCE48+4
RelocPtr <FaceGen *> g_faceGen(0x020F6A20);

// F22C9BC54C962ED86B36585C0EC0E729DCA18000+26
RelocPtr<MagicFavorites *>	g_MagicFavorites(0x0317ABE0);

// 6C8AEF51A6996BA6DC317E92014FCC446B5DBC17+25
RelocPtr<MenuTopicManager *> g_MenuTopicManager(0x03137778);

// 5ED03BBC1845C7394493F8FAA1DE8C7AEB3E2350+2A
RelocPtr <ActorValueList *> g_actorValueList(0x020F6310);

// aWerewolfSpell
RelocPtr <DefaultObjectList> g_defaultObjectList(0x01FD8F50);

// 68BF756333EF751059E935236F381FA91F62C373+22
RelocPtr <BGSSaveLoadManager *> g_saveLoadManager(0x0317ABF0);

// 3B74AA860AC42F1080FAF792E4BE1D3BF0E0EFEE+47
RelocPtr <MiscStatManager>	g_MiscStatManager(0x030FD318);

// A9D0A72CC9E5F85E2169118F999943FD43AF51EA+95
RelocPtr <EquipManager *> g_equipManager(0x030FC678);

// FBC6C0FB3A858D20081F5FF9CB3E780E59DCE52F+50
RelocPtr <RelationshipRanks> g_relationshipRanks(0x01E66FE8);

RelocAddr<_ChangeActorHeadPart> ChangeActorHeadPart(0x004362B0);
// 
RelocAddr<_GetEitherHandSlot> GetEitherHandSlot(0x00388990);
// 
RelocAddr<_GetRightHandSlot> GetRightHandSlot(0x00388980);
// 
RelocAddr<_GetLeftHandSlot> GetLeftHandSlot(0x00388970);
RelocAddr<_LookupActorValueByName> LookupActorValueByName(0x0043BEA0);
RelocAddr<_UpdatePlayerTints> UpdatePlayerTints(0x00954AE0);
RelocAddr<_GetActorBaseOverlays> GetActorBaseOverlays(0x003C1C40);
RelocAddr<_GetNumActorBaseOverlays> GetNumActorBaseOverlays(0x003C1CD0);

RelocAddr<_ApplyMasksToRenderTarget> ApplyMasksToRenderTarget(0x00435110);

RelocAddr<_UpdateModelSkin> UpdateModelSkin(0x004365E0); // Applies tint to ShaderType 5 nodes
RelocAddr<_UpdateModelHair> UpdateModelHair(0x004366F0); // Applies tint to ShaderType 6 nodes
RelocAddr<_UpdateModelFace> UpdateModelFace(0x00435C50);
RelocAddr<_UpdateHarvestModel> UpdateHarvestModel(0x001E9450);

RelocAddr<_GetRelationshipIndex> GetRelationshipIndex(0x0039DD40);

// 63503D87B68DB4A4C8A2BE534A75246035A76798+82
RelocPtr<FacePresetList> g_facePresetList(0x01FF9470);	// aNosetype

RelocAddr<_HasLOS> HasLOS(0x009BA120);

class LoadedModFinder
{
	const char * m_stringToFind;

public:
	LoadedModFinder(const char * str) : m_stringToFind(str) { }

	bool Accept(ModInfo* modInfo)
	{
		return _stricmp(modInfo->name, m_stringToFind) == 0;
	}
};

const ModInfo * DataHandler::LookupModByName(const char * modName)
{
	return modList.modInfoList.Find(LoadedModFinder(modName));
}

DataHandler* DataHandler::GetSingleton()
{
	return *(g_dataHandler.GetPtr());
}

MiscStatManager * MiscStatManager::GetSingleton(void)
{
	return g_MiscStatManager.GetPtr(); 
}

MiscStatManager::MiscStat * MiscStatManager::Get(const char * name)
{
	for (UInt32 i = 0; i < m_stats.count; i++)
	{
		MiscStat	* stat = &m_stats[i];

		if (!_stricmp(name, stat->name))
		{
			return stat;
		}
	}

	return NULL;
}

EquipManager * EquipManager::GetSingleton(void)
{
	return *g_equipManager;
}

PersistentFormManager * PersistentFormManager::GetSingleton(void)
{
	return *g_persistentFormManager;
}

FaceGen * FaceGen::GetSingleton(void)
{
	return *g_faceGen;
}

MagicFavorites * MagicFavorites::GetSingleton()
{
	return *(g_MagicFavorites.GetPtr());
}

void MagicFavorites::ClearHotkey(SInt8 idx)
{
	if (idx < 0 || idx >= hotkeys.count)
		return;

	hotkeys[idx] = NULL;
}

void MagicFavorites::SetHotkey(TESForm * form, SInt8 idx)
{
	if (idx < 0 || idx >= hotkeys.count)
		return;

	SInt8 oldIdx = hotkeys.GetItemIndex(form);
	if (idx == oldIdx)
		return;

	if (IsFavorited(form))
	{
		hotkeys[oldIdx] = NULL;
		hotkeys[idx] = form;
	}
}

TESForm * MagicFavorites::GetSpell(SInt8 idx)
{
	TESForm * form = NULL;
	hotkeys.GetNthItem(idx, form);
	return form;
}

bool MagicFavorites::IsFavorited(TESForm * form)
{
	SInt32 indexOut = -1;
	if (GetSortIndex(spells, form, indexOut) && indexOut != -1)
		return true;
	else
		return false;
}

MenuTopicManager * MenuTopicManager::GetSingleton(void)
{
	return *(g_MenuTopicManager.GetPtr());
}

NiPointer<TESObjectREFR> MenuTopicManager::GetDialogueTarget()
{
	
	NiPointer<TESObjectREFR> refr;
	if (talkingHandle == (*g_invalidRefHandle) || talkingHandle == 0)
		return NULL;

	LookupREFRByHandle(talkingHandle, refr);
	return refr;
}

ActorValueList * ActorValueList::GetSingleton(void)
{
	return *g_actorValueList;
}

ActorValueInfo * ActorValueList::GetActorValue(UInt32 id)
{
	return (id < kNumActorValues) ? actorValues[id] : NULL;
}

UInt32 ActorValueList::ResolveActorValueByName(const char * name)
{
	UInt32 id = LookupActorValueByName(name);
	if (id >= kNumActorValues)
		return 255;

	return id;
}

DefaultObjectList * DefaultObjectList::GetSingleton(void)
{
	return g_defaultObjectList.GetPtr();
}

void BGSSaveLoadManager::Save(const char * name)
{
	CALL_MEMBER_FN(this, Save_Internal)(2, 0, name);
}

void BGSSaveLoadManager::Load(const char * name)
{
	CALL_MEMBER_FN(this, Load_Internal)(name, -1, 0, 1);
}

BGSSaveLoadManager * BGSSaveLoadManager::GetSingleton(void)
{
	return *g_saveLoadManager;
}

RelationshipRanks * RelationshipRanks::GetSingleton(void)
{
	return g_relationshipRanks;
}

SInt32 RelationshipRanks::GetRelationshipRank(TESForm * form1, TESForm * form2)
{
	RelationshipRanks * ranks = RelationshipRanks::GetSingleton();
	if (form1 && form2) {
		return ranks->value[GetRelationshipIndex(form1, form2)];
	}

	return ranks->value[kRelationshipAcquaintance];
}

FacePresetList * FacePresetList::GetSingleton(void)
{
	return g_facePresetList;
}
