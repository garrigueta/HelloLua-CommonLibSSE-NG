#include "Core/PCH.h"
#include "Core/LuaManager.h"
#include "Core/SKSEManager.h"

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

namespace Sample {

    // Singleton instance
    LuaManager* LuaManager::GetSingleton() {
        static LuaManager instance;
        return &instance;
    }

    LuaManager::LuaManager() : m_luaState(nullptr) {
        // Constructor
    }

    LuaManager::~LuaManager() {
        Close();
    }

    bool LuaManager::Initialize() {
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
        luaL_openlibs(m_luaState);

        // Register our custom functions
        RegisterStandardFunctions();
        
        // Register Skyrim-specific functions
        RegisterGameFunctions();

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

    void LuaManager::Close() {
        if (m_luaState) {
            lua_close(m_luaState);
            m_luaState = nullptr;
        }
    }

    bool LuaManager::ExecuteScript(const std::string& scriptPath) {
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

    bool LuaManager::ExecuteString(const std::string& luaCode) {
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

    bool LuaManager::RegisterFunction(const char* name, LuaCFunction func) {
        if (!m_luaState) {
            SKSE::log::error("Cannot register function: Lua state not initialized");
            return false;
        }

        lua_register(m_luaState, name, func);
        return true;
    }

    void LuaManager::AddPackagePath(const std::string& path) {
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

    // -------------------------------------------------------------------------
    // Helper functions to reduce code duplication in Lua function bindings
    // -------------------------------------------------------------------------

    // Helper to get an Actor from Form ID
    static RE::Actor* GetActorParam(lua_State* L, int index) {
        const RE::FormID formId = static_cast<RE::FormID>(luaL_checkinteger(L, index));
        return RE::TESForm::LookupByID<RE::Actor>(formId);
    }

    // Helper to get a Form from Form ID
    static RE::TESForm* GetFormParam(lua_State* L, int index) {
        const RE::FormID formId = static_cast<RE::FormID>(luaL_checkinteger(L, index));
        return RE::TESForm::LookupByID(formId);
    }

    // -------------------------------------------------------------------------
    // Lua function implementations
    // -------------------------------------------------------------------------

    // Example Lua function: Log a message to the SKSE log
    static int LuaLog(lua_State* L) {
        const char* message = luaL_checkstring(L, 1);
        SKSE::log::info("[Lua] {}", message);
        return 0;  // number of return values
    }

    // Example Lua function: Get player's position
    static int GetPlayerPosition(lua_State* L) {
        auto position = Sample::SKSEManager::GetSingleton()->GetPlayerPosition();
        lua_pushnumber(L, position.x);
        lua_pushnumber(L, position.y);
        lua_pushnumber(L, position.z);
        return 3;  // returning x, y, z as 3 values
    }

    // Add function to print to Skyrim's console
    static int PrintToConsole(lua_State* L) {
        const char* message = luaL_checkstring(L, 1);
        Sample::SKSEManager::GetSingleton()->PrintToConsole(message);
        return 0;
    }

    // Example: Access hit counter functionality from Lua
    static int TrackActor(lua_State* L) {
        auto actor = GetActorParam(L, 1);
        if (!actor) {
            lua_pushboolean(L, false);
            return 1;
        }
        
        bool success = Sample::SKSEManager::GetSingleton()->TrackActor(actor);
        lua_pushboolean(L, success);
        return 1;
    }
    
    static int UntrackActor(lua_State* L) {
        auto actor = GetActorParam(L, 1);
        if (!actor) {
            lua_pushboolean(L, false);
            return 1;
        }
        
        bool success = Sample::SKSEManager::GetSingleton()->UntrackActor(actor);
        lua_pushboolean(L, success);
        return 1;
    }
    
    static int IncrementHitCount(lua_State* L) {
        auto actor = GetActorParam(L, 1);
        if (!actor) {
            lua_pushboolean(L, false);
            return 1;
        }
        
        int increment = 1;
        if (lua_gettop(L) >= 2) {
            increment = static_cast<int>(luaL_checkinteger(L, 2));
        }
        
        Sample::SKSEManager::GetSingleton()->IncrementHitCount(actor, increment);
        lua_pushboolean(L, true);
        return 1;
    }
    
    static int GetHitCount(lua_State* L) {
        auto actor = GetActorParam(L, 1);
        if (!actor) {
            lua_pushnil(L);
            return 1;
        }
        
        auto hitCount = Sample::SKSEManager::GetSingleton()->GetHitCount(actor);
        if (hitCount) {
            lua_pushinteger(L, *hitCount);
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    // Actor Management
    static int GetActorByID(lua_State* L) {
        const RE::FormID formId = static_cast<RE::FormID>(luaL_checkinteger(L, 1));
        auto actor = Sample::SKSEManager::GetSingleton()->GetActorFromHandle(formId);
        if (actor) {
            lua_pushinteger(L, actor->GetFormID());
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    static int IsActorValid(lua_State* L) {
        auto actor = GetActorParam(L, 1);
        if (!actor) {
            lua_pushboolean(L, false);
            return 1;
        }
        
        bool isValid = Sample::SKSEManager::GetSingleton()->IsActorValid(actor);
        lua_pushboolean(L, isValid);
        return 1;
    }

    // Player functions
    static int GetPlayerActor(lua_State* L) {
        auto player = Sample::SKSEManager::GetSingleton()->GetPlayer();
        if (player) {
            lua_pushinteger(L, player->GetFormID());
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    // NPC Management
    static int SetActorValue(lua_State* L) {
        auto actor = GetActorParam(L, 1);
        if (!actor) {
            lua_pushboolean(L, false);
            return 1;
        }
        
        const char* avName = luaL_checkstring(L, 2);
        float value = static_cast<float>(luaL_checknumber(L, 3));
        
        Sample::SKSEManager::GetSingleton()->ForceActorValue(actor, avName, value);
        lua_pushboolean(L, true);
        return 1;
    }

    static int GetActorValue(lua_State* L) {
        auto actor = GetActorParam(L, 1);
        if (!actor) {
            lua_pushnumber(L, 0.0);
            return 1;
        }
        
        const char* avName = luaL_checkstring(L, 2);
        float value = Sample::SKSEManager::GetSingleton()->GetActorValue(actor, avName);
        lua_pushnumber(L, value);
        return 1;
    }

    // Equipment functions
    static int EquipItem(lua_State* L) {
        auto actor = GetActorParam(L, 1);
        auto item = GetFormParam(L, 2);
        
        if (!actor || !item) {
            lua_pushboolean(L, false);
            return 1;
        }
        
        bool preventRemoval = lua_toboolean(L, 3);
        bool silent = lua_toboolean(L, 4);
        
        bool success = Sample::SKSEManager::GetSingleton()->EquipItem(actor, item, preventRemoval, silent);
        lua_pushboolean(L, success);
        return 1;
    }

    static int UnequipItem(lua_State* L) {
        auto actor = GetActorParam(L, 1);
        auto item = GetFormParam(L, 2);
        
        if (!actor || !item) {
            lua_pushboolean(L, false);
            return 1;
        }
        
        bool silent = lua_toboolean(L, 3);
        
        bool success = Sample::SKSEManager::GetSingleton()->UnequipItem(actor, item, silent);
        lua_pushboolean(L, success);
        return 1;
    }

    // World interaction
    static int FindClosestReference(lua_State* L) {
        auto form = GetFormParam(L, 1);
        if (!form) {
            lua_pushnil(L);
            return 1;
        }
        
        float searchRadius = static_cast<float>(luaL_checknumber(L, 2));
        
        auto ref = Sample::SKSEManager::GetSingleton()->FindClosestReferenceOfType(form, searchRadius);
        if (ref) {
            lua_pushinteger(L, ref->GetFormID());
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    // Quest and game state
    static int SetQuestStage(lua_State* L) {
        const RE::FormID questId = static_cast<RE::FormID>(luaL_checkinteger(L, 1));
        uint16_t stage = static_cast<uint16_t>(luaL_checkinteger(L, 2));
        
        bool success = Sample::SKSEManager::GetSingleton()->SetQuestStage(questId, stage);
        lua_pushboolean(L, success);
        return 1;
    }

    static int GetQuestStage(lua_State* L) {
        const RE::FormID questId = static_cast<RE::FormID>(luaL_checkinteger(L, 1));
        
        uint16_t stage = Sample::SKSEManager::GetSingleton()->GetQuestStage(questId);
        lua_pushinteger(L, stage);
        return 1;
    }

    static int IsQuestCompleted(lua_State* L) {
        const RE::FormID questId = static_cast<RE::FormID>(luaL_checkinteger(L, 1));
        
        bool completed = Sample::SKSEManager::GetSingleton()->IsQuestCompleted(questId);
        lua_pushboolean(L, completed);
        return 1;
    }

    // Weather and environment
    static int GetCurrentWeather(lua_State* L) {
        auto weather = Sample::SKSEManager::GetSingleton()->GetCurrentWeather();
        if (weather) {
            lua_pushinteger(L, weather->GetFormID());
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    static int ForceWeather(lua_State* L) {
        const RE::FormID weatherId = static_cast<RE::FormID>(luaL_checkinteger(L, 1));
        
        auto weather = RE::TESForm::LookupByID<RE::TESWeather>(weatherId);
        if (!weather) {
            lua_pushboolean(L, false);
            return 1;
        }
        
        Sample::SKSEManager::GetSingleton()->ForceWeather(weather);
        lua_pushboolean(L, true);
        return 1;
    }

    // UI functions
    static int IsMenuOpen(lua_State* L) {
        const char* menuName = luaL_checkstring(L, 1);
        
        bool isOpen = Sample::SKSEManager::GetSingleton()->IsMenuOpen(menuName);
        lua_pushboolean(L, isOpen);
        return 1;
    }

    static int OpenMenu(lua_State* L) {
        const char* menuName = luaL_checkstring(L, 1);
        
        Sample::SKSEManager::GetSingleton()->OpenMenu(menuName);
        return 0;
    }

    static int CloseMenu(lua_State* L) {
        const char* menuName = luaL_checkstring(L, 1);
        
        Sample::SKSEManager::GetSingleton()->CloseMenu(menuName);
        return 0;
    }

    // Forms and objects
    static int GetFormByID(lua_State* L) {
        const RE::FormID formId = static_cast<RE::FormID>(luaL_checkinteger(L, 1));
        
        auto form = Sample::SKSEManager::GetSingleton()->GetFormFromID(formId);
        if (form) {
            lua_pushinteger(L, form->GetFormID());
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    static int GetFormByEditorID(lua_State* L) {
        const char* editorId = luaL_checkstring(L, 1);
        
        auto form = Sample::SKSEManager::GetSingleton()->GetFormFromEditorID(editorId);
        if (form) {
            lua_pushinteger(L, form->GetFormID());
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    // Utility functions
    static int GetActorDistance(lua_State* L) {
        auto actor1 = GetActorParam(L, 1);
        auto actor2 = GetActorParam(L, 2);
        
        if (!actor1 || !actor2) {
            lua_pushnumber(L, -1.0);
            return 1;
        }
        
        float distance = Sample::SKSEManager::GetSingleton()->GetActorDistance(actor1, actor2);
        lua_pushnumber(L, distance);
        return 1;
    }

    static int GetFormName(lua_State* L) {
        auto form = GetFormParam(L, 1);
        if (!form) {
            lua_pushstring(L, "");
            return 1;
        }
        
        std::string name = Sample::SKSEManager::GetSingleton()->GetFormName(form);
        lua_pushstring(L, name.c_str());
        return 1;
    }

    // Add RegisterForOnUpdate functionality
    static int RegisterForOnUpdate(lua_State* L) {
        // Check if a function was passed as parameter
        if (!lua_isfunction(L, 1)) {
            luaL_error(L, "RegisterForOnUpdate requires a function callback as parameter");
            return 0;
        }
        
        // Store the function in the registry to prevent garbage collection
        // and to be able to call it later
        int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
        
        // Store the function reference for later use when update events happen
        Sample::SKSEManager::GetSingleton()->RegisterLuaUpdateCallback(functionRef);
        
        lua_pushboolean(L, true);
        return 1;
    }

    void LuaManager::RegisterStandardFunctions() {
        // Register utility functions
        RegisterFunction("Log", LuaLog);
        RegisterFunction("GetPlayerPosition", GetPlayerPosition);
    }
    
    void LuaManager::RegisterGameFunctions() {
        // Register hit counter related functions
        RegisterFunction("TrackActor", TrackActor);
        RegisterFunction("UntrackActor", UntrackActor);
        RegisterFunction("IncrementHitCount", IncrementHitCount);
        RegisterFunction("GetHitCount", GetHitCount);
        RegisterFunction("PrintToConsole", PrintToConsole);
        
        // Actor Management
        RegisterFunction("GetActorByID", GetActorByID);
        RegisterFunction("IsActorValid", IsActorValid);
        
        // Player functions
        RegisterFunction("GetPlayer", GetPlayerActor);
        
        // NPC Management
        RegisterFunction("SetActorValue", SetActorValue);
        RegisterFunction("GetActorValue", GetActorValue);
        
        // Equipment functions
        RegisterFunction("EquipItem", EquipItem);
        RegisterFunction("UnequipItem", UnequipItem);
        
        // World interaction
        RegisterFunction("FindClosestReference", FindClosestReference);
        
        // Quest and game state
        RegisterFunction("SetQuestStage", SetQuestStage);
        RegisterFunction("GetQuestStage", GetQuestStage);
        RegisterFunction("IsQuestCompleted", IsQuestCompleted);
        
        // Weather and environment
        RegisterFunction("GetCurrentWeather", GetCurrentWeather);
        RegisterFunction("ForceWeather", ForceWeather);
        
        // UI functions
        RegisterFunction("IsMenuOpen", IsMenuOpen);
        RegisterFunction("OpenMenu", OpenMenu);
        RegisterFunction("CloseMenu", CloseMenu);
        
        // Forms and objects
        RegisterFunction("GetFormByID", GetFormByID);
        RegisterFunction("GetFormByEditorID", GetFormByEditorID);
        
        // Utility functions
        RegisterFunction("GetActorDistance", GetActorDistance);
        RegisterFunction("GetFormName", GetFormName);
        
        // Register the update function
        RegisterFunction("RegisterForOnUpdate", RegisterForOnUpdate);
    }
}