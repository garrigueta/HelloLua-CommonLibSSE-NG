#include "Core/PCH.h"
#include "Core/LuaSKSEManager.h"

// Include Lua headers with proper extern "C" block to ensure correct linkage
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <filesystem>
#include <fstream>
#include <sstream>
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

using namespace RE;
using namespace Sample;
using namespace SKSE;

namespace {
    inline const auto TrackedActorsRecord = _byteswap_ulong('TACT');
    inline const auto HitCountsRecord = _byteswap_ulong('HITC');
    
    // Helper to get an Actor from Form ID for Lua functions
    RE::Actor* GetActorParam(lua_State* L, int index) {
        const RE::FormID formId = static_cast<RE::FormID>(luaL_checkinteger(L, index));
        return RE::TESForm::LookupByID<RE::Actor>(formId);
    }

    // Helper to get a Form from Form ID for Lua functions
    RE::TESForm* GetFormParam(lua_State* L, int index) {
        const RE::FormID formId = static_cast<RE::FormID>(luaL_checkinteger(L, index));
        return RE::TESForm::LookupByID(formId);
    }
      // Lua function implementations
    
    // Log a message to the SKSE log
    int LuaLog(lua_State* L) {
        const char* message = luaL_checkstring(L, 1);
        SKSE::log::info("[Lua] {}", message);
        return 0;  // number of return values
    }

    // Get player's position
    int GetPlayerPosition(lua_State* L) {
        auto position = Sample::LuaSKSEManager::GetSingleton()->GetPlayerPosition();
        lua_pushnumber(L, position.x);
        lua_pushnumber(L, position.y);
        lua_pushnumber(L, position.z);
        return 3;  // returning x, y, z as 3 values
    }

    // Add function to print to Skyrim's console
    int PrintToConsole(lua_State* L) {
        const char* message = luaL_checkstring(L, 1);
        Sample::LuaSKSEManager::GetSingleton()->PrintToConsole(message);
        return 0;
    }

    // Hit counter functionality for Lua
    int TrackActor(lua_State* L) {
        auto actor = GetActorParam(L, 1);
        if (!actor) {
            lua_pushboolean(L, false);
            return 1;
        }
        
        bool success = Sample::LuaSKSEManager::GetSingleton()->TrackActor(actor);
        lua_pushboolean(L, success);
        return 1;
    }
    
    int UntrackActor(lua_State* L) {
        auto actor = GetActorParam(L, 1);
        if (!actor) {
            lua_pushboolean(L, false);
            return 1;
        }
        
        bool success = Sample::LuaSKSEManager::GetSingleton()->UntrackActor(actor);
        lua_pushboolean(L, success);
        return 1;
    }
    
    int IncrementHitCount(lua_State* L) {
        auto actor = GetActorParam(L, 1);
        if (!actor) {
            lua_pushboolean(L, false);
            return 1;
        }
        
        int increment = 1;
        if (lua_gettop(L) >= 2) {
            increment = static_cast<int>(luaL_checkinteger(L, 2));
        }
        
        Sample::LuaSKSEManager::GetSingleton()->IncrementHitCount(actor, increment);
        lua_pushboolean(L, true);
        return 1;
    }
    
    int GetHitCount(lua_State* L) {
        auto actor = GetActorParam(L, 1);
        if (!actor) {
            lua_pushnil(L);
            return 1;
        }
        
        auto hitCount = Sample::LuaSKSEManager::GetSingleton()->GetHitCount(actor);
        if (hitCount) {
            lua_pushinteger(L, *hitCount);
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    // Actor Management
    int GetActorByID(lua_State* L) {
        const RE::FormID formId = static_cast<RE::FormID>(luaL_checkinteger(L, 1));
        auto actor = Sample::LuaSKSEManager::GetSingleton()->GetActorFromHandle(formId);
        if (actor) {
            lua_pushinteger(L, actor->GetFormID());
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    int IsActorValid(lua_State* L) {
        auto actor = GetActorParam(L, 1);
        if (!actor) {
            lua_pushboolean(L, false);
            return 1;
        }
        
        bool isValid = Sample::LuaSKSEManager::GetSingleton()->IsActorValid(actor);
        lua_pushboolean(L, isValid);
        return 1;
    }

    // Player functions
    int GetPlayerActor(lua_State* L) {
        auto player = Sample::LuaSKSEManager::GetSingleton()->GetPlayer();
        if (player) {
            lua_pushinteger(L, player->GetFormID());
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    // NPC Management
    int SetActorValue(lua_State* L) {
        auto actor = GetActorParam(L, 1);
        if (!actor) {
            lua_pushboolean(L, false);
            return 1;
        }
        
        const char* avName = luaL_checkstring(L, 2);
        float value = static_cast<float>(luaL_checknumber(L, 3));
        
        Sample::LuaSKSEManager::GetSingleton()->ForceActorValue(actor, avName, value);
        lua_pushboolean(L, true);
        return 1;
    }

    int GetActorValue(lua_State* L) {
        auto actor = GetActorParam(L, 1);
        if (!actor) {
            lua_pushnumber(L, 0.0);
            return 1;
        }
        
        const char* avName = luaL_checkstring(L, 2);
        float value = Sample::LuaSKSEManager::GetSingleton()->GetActorValue(actor, avName);
        lua_pushnumber(L, value);
        return 1;
    }

    // Equipment functions
    int EquipItem(lua_State* L) {
        auto actor = GetActorParam(L, 1);
        auto item = GetFormParam(L, 2);
        
        if (!actor || !item) {
            lua_pushboolean(L, false);
            return 1;
        }
        
        bool preventRemoval = lua_toboolean(L, 3);
        bool silent = lua_toboolean(L, 4);
        
        bool success = Sample::LuaSKSEManager::GetSingleton()->EquipItem(actor, item, preventRemoval, silent);
        lua_pushboolean(L, success);
        return 1;
    }

    int UnequipItem(lua_State* L) {
        auto actor = GetActorParam(L, 1);
        auto item = GetFormParam(L, 2);
        
        if (!actor || !item) {
            lua_pushboolean(L, false);
            return 1;
        }
        
        bool silent = lua_toboolean(L, 3);
        
        bool success = Sample::LuaSKSEManager::GetSingleton()->UnequipItem(actor, item, silent);
        lua_pushboolean(L, success);
        return 1;
    }

    // World interaction
    int FindClosestReference(lua_State* L) {
        auto form = GetFormParam(L, 1);
        if (!form) {
            lua_pushnil(L);
            return 1;
        }
        
        float searchRadius = static_cast<float>(luaL_checknumber(L, 2));
        
        auto ref = Sample::LuaSKSEManager::GetSingleton()->FindClosestReferenceOfType(form, searchRadius);
        if (ref) {
            lua_pushinteger(L, ref->GetFormID());
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    // Quest and game state
    int SetQuestStage(lua_State* L) {
        const RE::FormID questId = static_cast<RE::FormID>(luaL_checkinteger(L, 1));
        uint16_t stage = static_cast<uint16_t>(luaL_checkinteger(L, 2));
        
        bool success = Sample::LuaSKSEManager::GetSingleton()->SetQuestStage(questId, stage);
        lua_pushboolean(L, success);
        return 1;
    }

    int GetQuestStage(lua_State* L) {
        const RE::FormID questId = static_cast<RE::FormID>(luaL_checkinteger(L, 1));
        
        uint16_t stage = Sample::LuaSKSEManager::GetSingleton()->GetQuestStage(questId);
        lua_pushinteger(L, stage);
        return 1;
    }

    int IsQuestCompleted(lua_State* L) {
        const RE::FormID questId = static_cast<RE::FormID>(luaL_checkinteger(L, 1));
        
        bool completed = Sample::LuaSKSEManager::GetSingleton()->IsQuestCompleted(questId);
        lua_pushboolean(L, completed);
        return 1;
    }

    // Weather and environment
    int GetCurrentWeather(lua_State* L) {
        auto weather = Sample::LuaSKSEManager::GetSingleton()->GetCurrentWeather();
        if (weather) {
            lua_pushinteger(L, weather->GetFormID());
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    int ForceWeather(lua_State* L) {
        const RE::FormID weatherId = static_cast<RE::FormID>(luaL_checkinteger(L, 1));
        
        auto weather = RE::TESForm::LookupByID<RE::TESWeather>(weatherId);
        if (!weather) {
            lua_pushboolean(L, false);
            return 1;
        }
        
        Sample::LuaSKSEManager::GetSingleton()->ForceWeather(weather);
        lua_pushboolean(L, true);
        return 1;
    }

    // UI functions
    int IsMenuOpen(lua_State* L) {
        const char* menuName = luaL_checkstring(L, 1);
        
        bool isOpen = Sample::LuaSKSEManager::GetSingleton()->IsMenuOpen(menuName);
        lua_pushboolean(L, isOpen);
        return 1;
    }

    int OpenMenu(lua_State* L) {
        const char* menuName = luaL_checkstring(L, 1);
        
        Sample::LuaSKSEManager::GetSingleton()->OpenMenu(menuName);
        return 0;
    }

    int CloseMenu(lua_State* L) {
        const char* menuName = luaL_checkstring(L, 1);
        
        Sample::LuaSKSEManager::GetSingleton()->CloseMenu(menuName);
        return 0;
    }

    // Forms and objects
    int GetFormByID(lua_State* L) {
        const RE::FormID formId = static_cast<RE::FormID>(luaL_checkinteger(L, 1));
        
        auto form = Sample::LuaSKSEManager::GetSingleton()->GetFormFromID(formId);
        if (form) {
            lua_pushinteger(L, form->GetFormID());
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    int GetFormByEditorID(lua_State* L) {
        const char* editorId = luaL_checkstring(L, 1);
        
        auto form = Sample::LuaSKSEManager::GetSingleton()->GetFormFromEditorID(editorId);
        if (form) {
            lua_pushinteger(L, form->GetFormID());
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    // Utility functions
    int GetActorDistance(lua_State* L) {
        auto actor1 = GetActorParam(L, 1);
        auto actor2 = GetActorParam(L, 2);
        
        if (!actor1 || !actor2) {
            lua_pushnumber(L, -1.0);
            return 1;
        }
        
        float distance = Sample::LuaSKSEManager::GetSingleton()->GetActorDistance(actor1, actor2);
        lua_pushnumber(L, distance);
        return 1;
    }

    int GetFormName(lua_State* L) {
        auto form = GetFormParam(L, 1);
        if (!form) {
            lua_pushstring(L, "");
            return 1;
        }
        
        std::string name = Sample::LuaSKSEManager::GetSingleton()->GetFormName(form);
        lua_pushstring(L, name.c_str());
        return 1;
    }

    // Add RegisterForOnUpdate functionality
    int RegisterForOnUpdate(lua_State* L) {
        // Check if a function was passed as parameter
        if (!lua_isfunction(L, 1)) {
            luaL_error(L, "RegisterForOnUpdate requires a function callback as parameter");
            return 0;
        }
        
        // Store the function in the registry to prevent garbage collection
        // and to be able to call it later
        int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
        
        // Store the function reference for later use when update events happen
        Sample::LuaSKSEManager::GetSingleton()->RegisterLuaUpdateCallback(functionRef);
        
        lua_pushboolean(L, true);
        return 1;
    }
}

namespace Sample {

    // Singleton instance
    LuaSKSEManager* LuaSKSEManager::GetSingleton() noexcept {
        static LuaSKSEManager instance;
        return &instance;
    }

    LuaSKSEManager::LuaSKSEManager() : m_luaState(nullptr) {
        // Constructor
    }

    LuaSKSEManager::~LuaSKSEManager() {
        Close();
    }

    bool LuaSKSEManager::Initialize() {
        // Close any existing Lua state
        if (m_luaState) {
            Close();
        }

        // Create a new Lua state
        m_luaState = luaL_newstate();
        if (!m_luaState) {
            SKSE::log::error("Failed to create Lua state");
            return false;
        }

        // Open standard libraries
        luaL_openlibs(m_luaState);        // Register all our functions
        RegisterStandardFunctions();
        
        // RegisterGameFunctions() is kept for compatibility but does nothing now

        // Set up paths for scripts
        std::string pluginsPath = "SKSE/Plugins/Scripts/";  // Removed "Data/" prefix
        std::string dataPath;
        
        // Set a default data path - simplified from the original code that used GetCompileIndex
        const auto directory = "Data\\";
        dataPath = directory;
        
        // Add script paths
        AddPackagePath(dataPath + pluginsPath + "?.lua");
        AddPackagePath(dataPath + pluginsPath + "?/init.lua");

        SKSE::log::info("Lua environment initialized successfully");
        return true;
    }

    void LuaSKSEManager::Close() {
        if (m_luaState) {
            lua_close(m_luaState);
            m_luaState = nullptr;
        }
    }

    bool LuaSKSEManager::ExecuteScript(const std::string& scriptPath) {
        if (!m_luaState) {
            SKSE::log::error("Cannot execute script: Lua state not initialized");
            return false;
        }

        std::string fullPath = "Data/SKSE/Plugins/Scripts/" + scriptPath;
        if (!std::filesystem::exists(fullPath)) {
            SKSE::log::error("Script file not found: {}", fullPath);
            return false;
        }

        // Use luaL_loadfile and lua_pcall instead of luaL_dofile for better error handling
        if (luaL_loadfile(m_luaState, fullPath.c_str()) != 0) {
            SKSE::log::error("Failed to load Lua script: {}", lua_tostring(m_luaState, -1));
            lua_pop(m_luaState, 1);  // pop error message
            return false;
        }

        if (lua_pcall(m_luaState, 0, LUA_MULTRET, 0) != 0) {
            SKSE::log::error("Failed to execute Lua script: {}", lua_tostring(m_luaState, -1));
            lua_pop(m_luaState, 1);  // pop error message
            return false;
        }

        return true;
    }

    bool LuaSKSEManager::ExecuteString(const std::string& luaCode) {
        if (!m_luaState) {
            SKSE::log::error("Cannot execute string: Lua state not initialized");
            return false;
        }

        // Use luaL_loadstring and lua_pcall instead of luaL_dostring for Lua 5.1 compatibility
        int loadResult = luaL_loadstring(m_luaState, luaCode.c_str());
        if (loadResult != 0) {
            SKSE::log::error("Failed to load Lua string: {}", lua_tostring(m_luaState, -1));
            lua_pop(m_luaState, 1);  // pop error message
            return false;
        }

        int pcallResult = lua_pcall(m_luaState, 0, LUA_MULTRET, 0);
        if (pcallResult != 0) {
            SKSE::log::error("Failed to execute Lua string: {}", lua_tostring(m_luaState, -1));
            lua_pop(m_luaState, 1);  // pop error message
            return false;
        }

        return true;
    }

    bool LuaSKSEManager::RegisterFunction(const char* name, LuaCFunction func) {
        if (!m_luaState) {
            SKSE::log::error("Cannot register function: Lua state not initialized");
            return false;
        }

        lua_register(m_luaState, name, func);
        return true;
    }

    void LuaSKSEManager::AddPackagePath(const std::string& path) {
        if (!m_luaState) {
            return;
        }

        // Get the current package.path
        lua_getglobal(m_luaState, "package");
        lua_getfield(m_luaState, -1, "path");
        std::string currentPath = lua_tostring(m_luaState, -1);
        
        // Add the new path
        currentPath += ";" + path;
        
        // Update package.path
        lua_pop(m_luaState, 1);
        lua_pushstring(m_luaState, currentPath.c_str());
        lua_setfield(m_luaState, -2, "path");
        lua_pop(m_luaState, 1);

        m_scriptPaths.push_back(path);
    }

    // Implement standard SKSE functions    void LuaSKSEManager::RegisterStandardFunctions() {
        // Create a HelloLua table to organize our functions
        lua_newtable(m_luaState);
        
        // Define all free functions we want to register
        const struct {
            const char* name;
            LuaCFunction func;
        } allFuncs[] = {
            // Standard functions
            {"Log", LuaLog},
            {"PrintToConsole", PrintToConsole},
            
            // Hit counter related functions
            {"TrackActor", TrackActor},
            {"UntrackActor", UntrackActor},
            {"IncrementHitCount", IncrementHitCount},
            {"GetHitCount", GetHitCount},
            
            // Actor Management
            {"GetActorByID", GetActorByID},
            {"IsActorValid", IsActorValid},
            {"GetPlayer", GetPlayerActor},
            
            // Player functions
            {"GetPlayerPosition", GetPlayerPosition},
            
            // NPC Management
            {"SetActorValue", SetActorValue},
            {"GetActorValue", GetActorValue},
            
            // Equipment functions
            {"EquipItem", EquipItem},
            {"UnequipItem", UnequipItem},
            
            // World interaction
            {"FindClosestReference", FindClosestReference},
            
            // Quest and game state
            {"SetQuestStage", SetQuestStage},
            {"GetQuestStage", GetQuestStage},
            {"IsQuestCompleted", IsQuestCompleted},
            
            // Weather and environment
            {"GetCurrentWeather", GetCurrentWeather},
            {"ForceWeather", ForceWeather},
            
            // UI functions
            {"IsMenuOpen", IsMenuOpen},
            {"OpenMenu", OpenMenu},
            {"CloseMenu", CloseMenu},
            
            // Forms and objects
            {"GetFormByID", GetFormByID},
            {"GetFormByEditorID", GetFormByEditorID},
            {"GetFormName", GetFormName},
            
            // Utility functions
            {"GetActorDistance", GetActorDistance},
            {"RegisterForOnUpdate", RegisterForOnUpdate},
            
            {nullptr, nullptr} // Sentinel to mark end of array
        };
        
        // Register all functions
        for (int i = 0; allFuncs[i].name != nullptr; ++i) {
            // Add to HelloLua table
            lua_pushstring(m_luaState, allFuncs[i].name);
            lua_pushcfunction(m_luaState, allFuncs[i].func);
            lua_settable(m_luaState, -3);
            
            // Also register as global for backward compatibility
            RegisterFunction(allFuncs[i].name, allFuncs[i].func);
        }
        
        // Set the HelloLua table as a global variable
        lua_setglobal(m_luaState, "HelloLua");
    }
      // This method is kept for backwards compatibility with existing code
    void LuaSKSEManager::RegisterGameFunctions() {
        // All functions are now registered in RegisterStandardFunctions
    }

    // SKSE Functions Implementation
    void LuaSKSEManager::PrintToConsole(const std::string& message) {
        if (RE::ConsoleLog::GetSingleton()) {
            RE::ConsoleLog::GetSingleton()->Print(message.c_str());
        } else {
            SKSE::log::warn("Failed to print to console: Console not available");
        }
    }

    NiPoint3 LuaSKSEManager::GetPlayerPosition() const {
        if (auto* player = RE::PlayerCharacter::GetSingleton()) {
            return player->GetPosition();
        }
        return RE::NiPoint3();  // Return default position (0,0,0) if player not found
    }

    // Hit Counter functions
    bool LuaSKSEManager::TrackActor(Actor* target) {
        if (!target) {
            return false;
        }
        std::unique_lock lock(_lock);
        return _trackedActors.emplace(target).second;
    }

    bool LuaSKSEManager::UntrackActor(Actor* target) {
        if (!target) {
            return false;
        }
        std::unique_lock lock(_lock);
        return _trackedActors.erase(target);
    }

    void LuaSKSEManager::IncrementHitCount(Actor* target, int32_t by) {
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

    std::optional<int32_t> LuaSKSEManager::GetHitCount(Actor* target) const noexcept {
        std::unique_lock lock(_lock);
        auto result = _hitCounts.find(target);
        if (result == _hitCounts.end()) {
            return {};
        }
        return result->second;
    }

    // Actor Management
    Actor* LuaSKSEManager::GetActorFromHandle(uint32_t formId) const {
        return TESForm::LookupByID<Actor>(formId);
    }

    bool LuaSKSEManager::IsActorValid(Actor* actor) const {
        return actor && !actor->IsDeleted() && actor->GetBaseObject();
    }

    // Player functions
    Actor* LuaSKSEManager::GetPlayer() const {
        return RE::PlayerCharacter::GetSingleton();
    }    // NPC Management
    void LuaSKSEManager::ForceActorValue(Actor* actor, const std::string& avName, float value) {
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
        
        // TODO: Implement proper actor value setting
        // actor->SetActorValue(av, value);
    }

    float LuaSKSEManager::GetActorValue(Actor* actor, const std::string& avName) const {
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
        
        // TODO: Implement proper actor value retrieval
        // return actor->GetActorValue(av);
        return 0.0f;
    }

    // Equipment functions
    bool LuaSKSEManager::EquipItem(Actor* actor, TESForm* item, bool preventRemoval, bool silent) {
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

    bool LuaSKSEManager::UnequipItem(Actor* actor, TESForm* item, bool silent) {
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
    }    // World interaction
    TESObjectREFR* LuaSKSEManager::FindClosestReferenceOfType(TESForm* formToMatch, float searchRadius) const {
        // TODO: Implement proper reference search with spatial awareness
        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player || !formToMatch) {
            return nullptr;
        }
        
        // For now just return player if matching
        if (player->GetBaseObject() == formToMatch) {
            return player;
        }
        
        return nullptr;
    }    // Quest and game state
    bool LuaSKSEManager::SetQuestStage(uint32_t questId, uint16_t stage) {
        // TODO: Implement proper quest stage setting
        auto quest = TESForm::LookupByID<TESQuest>(questId);
        if (!quest) {
            return false;
        }
        
        // Implementation needed
        return false;
    }

    uint16_t LuaSKSEManager::GetQuestStage(uint32_t questId) const {
        // TODO: Implement proper quest stage retrieval
        auto quest = TESForm::LookupByID<TESQuest>(questId);
        if (!quest) {
            return 0;
        }
        
        // Implementation needed
        return 0;
    }

    bool LuaSKSEManager::IsQuestCompleted(uint32_t questId) const {
        auto quest = TESForm::LookupByID<TESQuest>(questId);
        if (!quest) {
            return false;
        }
        return quest->IsCompleted();
    }

    // Weather and environment
    TESWeather* LuaSKSEManager::GetCurrentWeather() const {
        auto sky = RE::Sky::GetSingleton();
        if (!sky) {
            return nullptr;
        }
        return sky->currentWeather;
    }

    void LuaSKSEManager::ForceWeather(TESWeather* weather) {
        if (!weather) {
            return;
        }
        auto sky = RE::Sky::GetSingleton();
        if (sky) {
            sky->ForceWeather(weather, true);
        }
    }

    // UI functions
    bool LuaSKSEManager::IsMenuOpen(const std::string& menuName) const {
        auto ui = RE::UI::GetSingleton();
        if (!ui) {
            return false;
        }
        return ui->IsMenuOpen(menuName);
    }    void LuaSKSEManager::OpenMenu(const std::string& menuName) {
        // TODO: Implement proper menu opening
        auto ui = RE::UI::GetSingleton();
        if (ui) {
            // ui->OpenMenu(menuName);
        }
    }

    void LuaSKSEManager::CloseMenu(const std::string& menuName) {
        // TODO: Implement proper menu closing
        auto ui = RE::UI::GetSingleton();
        if (ui) {
            // ui->CloseMenu(menuName);
        }
    }

    // Forms and objects
    TESForm* LuaSKSEManager::GetFormFromID(uint32_t formId) const {
        return TESForm::LookupByID(formId);
    }    TESForm* LuaSKSEManager::GetFormFromEditorID(const std::string& editorId) const {
        // TODO: Implement proper lookup by editor ID
        // Would require accessing the game's form lookup tables
        return nullptr;
    }

    // Utility functions
    float LuaSKSEManager::GetActorDistance(Actor* actor1, Actor* actor2) const {
        if (!actor1 || !actor2) {
            return -1.0f;
        }
        return actor1->GetPosition().GetDistance(actor2->GetPosition());
    }

    std::string LuaSKSEManager::GetFormName(TESForm* form) const {
        if (!form) {
            return "";
        }
        
        auto fullName = form->As<TESFullName>();
        if (fullName) {
            return fullName->GetFullName();
        }
        return "";
    }

    void LuaSKSEManager::RegisterLuaUpdateCallback(int functionRef) {
        std::unique_lock lock(_lock);
        _luaUpdateCallbacks.push_back(functionRef);
        SKSE::log::info("Registered Lua update callback with reference ID: {}", functionRef);
    }

    // Serialization methods
    void LuaSKSEManager::OnRevert(SerializationInterface*) {
        std::unique_lock lock(GetSingleton()->_lock);
        GetSingleton()->_hitCounts.clear();
        GetSingleton()->_trackedActors.clear();
        SKSE::log::info("LuaSKSEManager state reverted.");
    }

    void LuaSKSEManager::OnGameSaved(SerializationInterface* serde) {
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

    void LuaSKSEManager::OnGameLoaded(SerializationInterface* serde) {
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
                        continue;
                    }
                    auto* actor = TESForm::LookupByID<Actor>(newActorFormID);
                    if (actor) {
                        GetSingleton()->_trackedActors.emplace(actor);
                    }
                }
            } else {
                log::warn("Unknown record type in cosave.");
            }
        }
    }
}
