#include <SKSE/SKSE.h>
#include <Core/SKSEManager.h>

using namespace RE;
using namespace Sample;
using namespace SKSE;

namespace {
    inline const auto TrackedActorsRecord = _byteswap_ulong('TACT');
    inline const auto HitCountsRecord = _byteswap_ulong('HITC');
}

// Basic functionality methods
SKSEManager* SKSEManager::GetSingleton() noexcept {
    static SKSEManager instance;
    return &instance;
}

bool SKSEManager::TrackActor(Actor* target) {
    if (!target) {
        return false;
    }
    std::unique_lock lock(_lock);
    return _trackedActors.emplace(target).second;
}

bool SKSEManager::UntrackActor(Actor* target) {
    if (!target) {
        return false;
    }
    std::unique_lock lock(_lock);
    return _trackedActors.erase(target);
}

void SKSEManager::IncrementHitCount(Actor* target, int32_t by) {
    if (!target) {
        return;
    }
    std::unique_lock lock(_lock);
    if (!_trackedActors.contains(target)) {
        return;
    }
    auto result = _hitCounts.try_emplace(target, by);
    if (!result.second) {
        result.first->second += by;
    }
}

std::optional<int32_t> SKSEManager::GetHitCount(Actor* target) const noexcept {
    std::unique_lock lock(_lock);
    auto result = _hitCounts.find(target);
    if (result == _hitCounts.end()) {
        return {};
    }
    return result->second;
}

void SKSEManager::PrintToConsole(const std::string& message) {
    if (RE::ConsoleLog::GetSingleton()) {
        RE::ConsoleLog::GetSingleton()->Print(message.c_str());
    } else {
        SKSE::log::warn("Failed to print to console: Console not available");
    }
}

// Actor Management
Actor* SKSEManager::GetActorFromHandle(uint32_t formId) const {
    return TESForm::LookupByID<Actor>(formId);
}

bool SKSEManager::IsActorValid(Actor* actor) const {
    return actor && !actor->IsDeleted() && actor->GetBaseObject();
}

// Player functions
Actor* SKSEManager::GetPlayer() const {
    return RE::PlayerCharacter::GetSingleton();
}

// NPC Management - Stub implementations
void SKSEManager::ForceActorValue(Actor* actor, const std::string& avName, float value) {
    if (!actor) {
        return;
    }
    
    auto avList = RE::ActorValueList::GetSingleton();
    if (!avList) {
        SKSE::log::error("ActorValueList singleton not available");
        return;
    }
    
    auto av = avList->LookupActorValueByName(avName);
    if (av == RE::ActorValue::kNone) {
        SKSE::log::error("Invalid actor value name: {}", avName);
        return;
    }
    
    // Log that we're not actually setting anything
    SKSE::log::info("ForceActorValue called for {}, but is stubbed. Value would be {}", avName, value);
}

float SKSEManager::GetActorValue(Actor* actor, const std::string& avName) const {
    if (!actor) {
        return 0.0f;
    }
    
    auto avList = RE::ActorValueList::GetSingleton();
    if (!avList) {
        SKSE::log::error("ActorValueList singleton not available");
        return 0.0f;
    }
    
    auto av = avList->LookupActorValueByName(avName);
    if (av == RE::ActorValue::kNone) {
        SKSE::log::error("Invalid actor value name: {}", avName);
        return 0.0f;
    }
    
    // Return a default value
    SKSE::log::info("GetActorValue called for {}, but is stubbed. Returning 0.0", avName);
    return 0.0f;
}

// Equipment functions - No changes needed here
bool SKSEManager::EquipItem(Actor* actor, TESForm* item, bool preventRemoval, bool silent) {
    if (!actor || !item) {
        return false;
    }
    
    auto boundObj = item->As<TESBoundObject>();
    if (!boundObj) {
        SKSE::log::error("Form is not a bound object and cannot be equipped");
        return false;
    }
    
    if (!item->Is(FormType::Armor) && !item->Is(FormType::Weapon) && 
        !item->Is(FormType::Ammo) && !item->Is(FormType::Light) && 
        !item->Is(FormType::Misc)) {
        return false;
    }
    
    auto equipManager = RE::ActorEquipManager::GetSingleton();
    if (!equipManager) {
        return false;
    }
    
    equipManager->EquipObject(actor, boundObj, nullptr, 1, nullptr, preventRemoval, false, silent);
    return true;
}

bool SKSEManager::UnequipItem(Actor* actor, TESForm* item, bool silent) {
    if (!actor || !item) {
        return false;
    }
    
    auto boundObj = item->As<TESBoundObject>();
    if (!boundObj) {
        SKSE::log::error("Form is not a bound object and cannot be unequipped");
        return false;
    }
    
    auto equipManager = RE::ActorEquipManager::GetSingleton();
    if (!equipManager) {
        return false;
    }
    
    equipManager->UnequipObject(actor, boundObj, nullptr, 1, nullptr, false, false, silent);
    return true;
}

// World interaction - Super simplified approach
TESObjectREFR* SKSEManager::FindClosestReferenceOfType(TESForm* formToMatch, float searchRadius) const {
    // Mark the parameter as unused to avoid compiler warning
    (void)searchRadius;
    
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player || !formToMatch) {
        return nullptr;
    }
    
    // Return player if matching, otherwise null
    SKSE::log::info("FindClosestReferenceOfType is a stub function");
    if (player->GetBaseObject() == formToMatch) {
        return player;
    }
    
    return nullptr;
}

// Quest and game state - Stub implementations
bool SKSEManager::SetQuestStage(uint32_t questId, uint16_t stage) {
    SKSE::log::info("SetQuestStage called with quest ID {:X}, stage {}, but is stubbed", questId, stage);
    return false;
}

uint16_t SKSEManager::GetQuestStage(uint32_t questId) const {
    SKSE::log::info("GetQuestStage called with quest ID {:X}, but is stubbed", questId);
    return 0;
}

bool SKSEManager::IsQuestCompleted(uint32_t questId) const {
    auto quest = TESForm::LookupByID<TESQuest>(questId);
    if (!quest) {
        return false;
    }
    return quest->IsCompleted();
}

// Weather and environment
TESWeather* SKSEManager::GetCurrentWeather() const {
    auto sky = RE::Sky::GetSingleton();
    if (!sky) {
        return nullptr;
    }
    return sky->currentWeather;
}

void SKSEManager::ForceWeather(TESWeather* weather) {
    if (!weather) {
        return;
    }
    auto sky = RE::Sky::GetSingleton();
    if (sky) {
        sky->ForceWeather(weather, true);
    }
}

// UI functions - Stub implementations
bool SKSEManager::IsMenuOpen(const std::string& menuName) const {
    auto ui = RE::UI::GetSingleton();
    if (!ui) {
        return false;
    }
    return ui->IsMenuOpen(menuName);
}

void SKSEManager::OpenMenu(const std::string& menuName) {
    SKSE::log::info("OpenMenu called with menu name {}, but is stubbed", menuName);
}

void SKSEManager::CloseMenu(const std::string& menuName) {
    SKSE::log::info("CloseMenu called with menu name {}, but is stubbed", menuName);
}

// Forms and objects - Stub implementations
TESForm* SKSEManager::GetFormFromID(uint32_t formId) const {
    return TESForm::LookupByID(formId);
}

TESForm* SKSEManager::GetFormFromEditorID(const std::string& editorId) const {
    SKSE::log::info("GetFormFromEditorID called with editor ID {}, but is stubbed", editorId);
    return nullptr;
}

// Utility functions - No changes needed
float SKSEManager::GetActorDistance(Actor* actor1, Actor* actor2) const {
    if (!actor1 || !actor2) {
        return -1.0f;
    }
    return actor1->GetPosition().GetDistance(actor2->GetPosition());
}

std::string SKSEManager::GetFormName(TESForm* form) const {
    if (!form) {
        return "";
    }
    
    auto fullName = form->As<TESFullName>();
    if (fullName) {
        return fullName->GetFullName();
    }
    return "";
}

RE::NiPoint3 SKSEManager::GetPlayerPosition() const {
    if (auto* player = RE::PlayerCharacter::GetSingleton()) {
        return player->GetPosition();
    }
    return RE::NiPoint3();  // Return default position (0,0,0) if player not found
}

// Serialization methods - No changes needed
void SKSEManager::OnRevert(SerializationInterface*) {
    std::unique_lock lock(GetSingleton()->_lock);
    GetSingleton()->_hitCounts.clear();
    GetSingleton()->_trackedActors.clear();
    SKSE::log::info("SKSEManager state reverted.");
}

void SKSEManager::OnGameSaved(SerializationInterface* serde) {
    std::unique_lock lock(GetSingleton()->_lock);
    if (!serde->OpenRecord(HitCountsRecord, 0)) {
        log::error("Unable to open record to write cosave data.");
        return;
    }

    auto hitCountsSize = GetSingleton()->_hitCounts.size();
    serde->WriteRecordData(&hitCountsSize, sizeof(hitCountsSize));
    for (auto& count : GetSingleton()->_hitCounts) {
        serde->WriteRecordData(&count.first->formID, sizeof(count.first->formID));
        serde->WriteRecordData(&count.second, sizeof(count.second));
    }

    if (!serde->OpenRecord(TrackedActorsRecord, 0)) {
        log::error("Unable to open record to write cosave data.");
        return;
    }

    auto trackedActorsCount = GetSingleton()->_trackedActors.size();
    serde->WriteRecordData(&trackedActorsCount, sizeof(trackedActorsCount));
    for (auto* actor : GetSingleton()->_trackedActors) {
        serde->WriteRecordData(&actor->formID, sizeof(actor->formID));
    }
}

void SKSEManager::OnGameLoaded(SerializationInterface* serde) {
    std::uint32_t type;
    std::uint32_t size;
    std::uint32_t version;
    
    while (serde->GetNextRecordInfo(type, version, size)) {
        if (type == HitCountsRecord) {
            std::size_t hitCountsSize;
            serde->ReadRecordData(&hitCountsSize, sizeof(hitCountsSize));
            for (; hitCountsSize > 0; --hitCountsSize) {
                RE::FormID actorFormID;
                serde->ReadRecordData(&actorFormID, sizeof(actorFormID));
                RE::FormID newActorFormID;
                if (!serde->ResolveFormID(actorFormID, newActorFormID)) {
                    log::warn("Actor ID {:X} could not be found after loading the save.", actorFormID);
                    continue;
                }
                int32_t hitCount;
                serde->ReadRecordData(&hitCount, sizeof(hitCount));
                auto* actor = TESForm::LookupByID<Actor>(newActorFormID);
                if (actor) {
                    GetSingleton()->_hitCounts.try_emplace(actor, hitCount);
                } else {
                    log::warn("Actor ID {:X} could not be found after loading the save.", newActorFormID);
                }
            }
        } else if (type == TrackedActorsRecord) {
            std::size_t trackedActorsSize;
            serde->ReadRecordData(&trackedActorsSize, sizeof(trackedActorsSize));
            for (; trackedActorsSize > 0; --trackedActorsSize) {
                RE::FormID actorFormID;
                serde->ReadRecordData(&actorFormID, sizeof(actorFormID));
                RE::FormID newActorFormID;
                if (!serde->ResolveFormID(actorFormID, newActorFormID)) {
                    log::warn("Actor ID {:X} could not be found after loading the save.", actorFormID);
                    continue;
                }
                auto* actor = TESForm::LookupByID<Actor>(newActorFormID);
                if (actor) {
                    GetSingleton()->_trackedActors.emplace(actor);
                } else {
                    log::warn("Actor ID {:X} could not be found after loading the save.", newActorFormID);
                }
            }
        } else {
            log::warn("Unknown record type in cosave.");
        }
    }
}

void SKSEManager::RegisterLuaUpdateCallback(int functionRef) {
    std::unique_lock lock(_lock);
    _luaUpdateCallbacks.push_back(functionRef);
    SKSE::log::info("Registered Lua update callback with reference ID: {}", functionRef);
}