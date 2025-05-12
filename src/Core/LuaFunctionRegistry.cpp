#include "Core/PCH.h"
#include "Core/LuaFunctionRegistry.h"
#include <SKSE/SKSE.h>

// Include Lua headers
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace Sample {

    LuaFunctionRegistry* LuaFunctionRegistry::GetSingleton() {
        static LuaFunctionRegistry instance;
        return &instance;
    }

    bool LuaFunctionRegistry::RegisterModule(ILuaModuleInterface* module) {
        if (!module) {
            SKSE::log::error("Attempted to register a null module");
            return false;
        }

        const char* moduleName = module->GetModuleName();
        if (m_modules.find(moduleName) != m_modules.end()) {
            SKSE::log::warn("Module '{}' already registered. Overwriting.", moduleName);
        }

        m_modules[moduleName] = module;
        
        // Initialize the function list for this module if it doesn't exist
        if (m_functions.find(moduleName) == m_functions.end()) {
            m_functions[moduleName] = std::vector<FunctionInfo>();
        }

        // Let the module register its functions
        module->RegisterFunctions(this);
        
        SKSE::log::info("Registered Lua module: {} (version {})", moduleName, module->GetVersion());
        return true;
    }

    bool LuaFunctionRegistry::UnregisterModule(const char* moduleName) {
        if (!moduleName) {
            return false;
        }

        auto it = m_modules.find(moduleName);
        if (it == m_modules.end()) {
            return false;
        }

        // Remove the module from the registry
        m_modules.erase(it);
        
        // Remove all functions registered by this module
        m_functions.erase(moduleName);

        SKSE::log::info("Unregistered Lua module: {}", moduleName);
        return true;
    }

    bool LuaFunctionRegistry::RegisterFunction(const char* moduleName, const char* funcName, LuaCFunction func, const char* description) {
        if (!moduleName || !funcName || !func) {
            return false;
        }

        // Make sure the module exists in our map
        if (m_modules.find(moduleName) == m_modules.end()) {
            SKSE::log::error("Cannot register function '{}': Module '{}' not registered", funcName, moduleName);
            return false;
        }

        FunctionInfo info;
        info.moduleName = moduleName;
        info.functionName = funcName;
        info.description = description ? description : "";
        info.function = func;

        m_functions[moduleName].push_back(info);

        SKSE::log::info("Registered function '{}.{}'", moduleName, funcName);
        return true;
    }

    bool LuaFunctionRegistry::IsModuleRegistered(const char* moduleName) {
        if (!moduleName) {
            return false;
        }
        return m_modules.find(moduleName) != m_modules.end();
    }

    std::vector<std::string> LuaFunctionRegistry::GetRegisteredModules() const {
        std::vector<std::string> result;
        for (const auto& [name, _] : m_modules) {
            result.push_back(name);
        }
        return result;
    }

    std::vector<LuaFunctionRegistry::FunctionInfo> LuaFunctionRegistry::GetModuleFunctions(const char* moduleName) const {
        if (!moduleName) {
            return {};
        }

        auto it = m_functions.find(moduleName);
        if (it == m_functions.end()) {
            return {};
        }

        return it->second;
    }

    void LuaFunctionRegistry::ApplyToLuaState(lua_State* L) {
        if (!L) {
            SKSE::log::error("Cannot apply functions to null Lua state");
            return;
        }

        // First, create the module tables
        for (const auto& [moduleName, _] : m_modules) {
            lua_newtable(L);
            lua_setglobal(L, moduleName.c_str());
        }

        // Then register all functions in their module tables
        for (const auto& [moduleName, functions] : m_functions) {
            // Get the module table
            lua_getglobal(L, moduleName.c_str());
            
            // Register each function
            for (const auto& funcInfo : functions) {
                lua_pushstring(L, funcInfo.functionName.c_str());
                lua_pushcfunction(L, funcInfo.function);
                lua_settable(L, -3);
                
                // Also register in _G for backward compatibility
                lua_pushcfunction(L, funcInfo.function);
                lua_setglobal(L, funcInfo.functionName.c_str());
            }
            
            // Pop the module table
            lua_pop(L, 1);
        }
    }
}
