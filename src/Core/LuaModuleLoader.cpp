#include "Core/PCH.h"
#include "Core/LuaModuleLoader.h"
#include "Core/LuaNativeFunctions.h"
#include <SKSE/SKSE.h>
#include <filesystem>
#include <fstream>

// Include Lua headers
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace Sample {

    LuaModuleLoader* LuaModuleLoader::GetSingleton() {
        static LuaModuleLoader instance;
        return &instance;
    }

    // The Lua API for registering a function from a Lua module
    int LuaModuleLoader::LuaRegisterFunction(lua_State* L) {
        // Parameters: moduleName, functionName, luaFunction, [description]
        const char* moduleName = luaL_checkstring(L, 1);
        const char* functionName = luaL_checkstring(L, 2);
        
        // Ensure the third argument is a function
        if (!lua_isfunction(L, 3)) {
            return luaL_error(L, "Expected function as third parameter");
        }
        
        // Get the optional description
        const char* description = lua_isstring(L, 4) ? lua_tostring(L, 4) : "";
        
        // Store the function in the registry
        lua_pushvalue(L, 3);
        int funcRef = luaL_ref(L, LUA_REGISTRYINDEX);
        
        // Create wrapper function that will call the Lua function
        auto wrapper = [funcRef](lua_State* L) -> int {
            // Get the stored function from the registry
            lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);
            
            // Copy all parameters to the function call
            int numArgs = lua_gettop(L) - 1;
            for (int i = 1; i <= numArgs; ++i) {
                lua_pushvalue(L, i);
            }
            
            // Call the Lua function
            lua_call(L, numArgs, LUA_MULTRET);
            
            // Return the number of results
            return lua_gettop(L) - numArgs;
        };
        
        // Register the wrapper function
        LuaFunctionRegistry::GetSingleton()->RegisterFunction(
            moduleName, functionName, 
            static_cast<LuaCFunction>(wrapper), 
            description
        );
        
        // Return true to indicate success
        lua_pushboolean(L, true);
        return 1;
    }

    // The Lua API for creating a new module
    int LuaModuleLoader::LuaCreateModule(lua_State* L) {
        // Parameters: moduleName, moduleVersion
        const char* moduleName = luaL_checkstring(L, 1);
        const char* moduleVersion = lua_isstring(L, 2) ? lua_tostring(L, 2) : "1.0.0";
        
        // Create a Lua module interface
        class LuaScriptModule : public ILuaModuleInterface {
        public:
            LuaScriptModule(const char* name, const char* version) 
                : m_name(name), m_version(version) {}
                
            void RegisterFunctions(LuaFunctionRegistry* registry) override {
                // This is called from RegisterModule, but we register functions
                // directly when they're defined in the Lua script
            }
            
            const char* GetModuleName() const override {
                return m_name.c_str();
            }
            
            const char* GetVersion() const override {
                return m_version.c_str();
            }
            
        private:
            std::string m_name;
            std::string m_version;
        };
        
        // Create and register the module
        auto module = new LuaScriptModule(moduleName, moduleVersion);
        bool success = LuaFunctionRegistry::GetSingleton()->RegisterModule(module);
        
        // Return the success status
        lua_pushboolean(L, success);
        return 1;
    }

    // The Lua API for getting a native C++ function by name
    int LuaModuleLoader::LuaGetNativeFunction(lua_State* L) {
        // Parameters: functionName
        const char* functionName = luaL_checkstring(L, 1);
        
        // Try to get the native function
        auto func = LuaNativeFunctions::GetSingleton()->GetFunctionByName(functionName);
        if (!func) {
            lua_pushnil(L);
            return 1;
        }
        
        // Push the function to Lua
        lua_pushcfunction(L, func);
        return 1;
    }

    // The Lua API for listing all registered modules
    int LuaModuleLoader::LuaListRegisteredModules(lua_State* L) {
        auto modules = LuaFunctionRegistry::GetSingleton()->GetRegisteredModules();
        
        // Create a table to return
        lua_createtable(L, static_cast<int>(modules.size()), 0);
        
        for (size_t i = 0; i < modules.size(); ++i) {
            lua_pushstring(L, modules[i].c_str());
            lua_rawseti(L, -2, static_cast<int>(i + 1));
        }
        
        return 1;
    }

    // The Lua API for listing all functions in a module
    int LuaModuleLoader::LuaListModuleFunctions(lua_State* L) {
        // Parameters: moduleName
        const char* moduleName = luaL_checkstring(L, 1);
        
        auto functions = LuaFunctionRegistry::GetSingleton()->GetModuleFunctions(moduleName);
        
        // Create a table to return
        lua_createtable(L, static_cast<int>(functions.size()), 0);
        
        for (size_t i = 0; i < functions.size(); ++i) {
            // Create a subtable for each function
            lua_createtable(L, 0, 3);
            
            lua_pushstring(L, "name");
            lua_pushstring(L, functions[i].functionName.c_str());
            lua_settable(L, -3);
            
            lua_pushstring(L, "description");
            lua_pushstring(L, functions[i].description.c_str());
            lua_settable(L, -3);
            
            // Add the subtable to the main table
            lua_rawseti(L, -2, static_cast<int>(i + 1));
        }
        
        return 1;
    }

    bool LuaModuleLoader::LoadModuleFromFile(lua_State* L, const std::string& filePath) {
        if (!L) {
            SKSE::log::error("Cannot load module: Lua state not initialized");
            return false;
        }
        
        if (!std::filesystem::exists(filePath)) {
            SKSE::log::error("Module file not found: {}", filePath);
            return false;
        }
        
        // Check if already loaded
        if (IsModuleLoaded(filePath)) {
            SKSE::log::info("Module already loaded: {}", filePath);
            return true;
        }
        
        // Create a metatable with the module registration functions
        lua_newtable(L);
        
        lua_pushstring(L, "registerFunction");
        lua_pushcfunction(L, LuaRegisterFunction);
        lua_settable(L, -3);
        
        lua_pushstring(L, "createModule");
        lua_pushcfunction(L, LuaCreateModule);
        lua_settable(L, -3);
        
        lua_pushstring(L, "getNativeFunction");
        lua_pushcfunction(L, LuaGetNativeFunction);
        lua_settable(L, -3);
        
        lua_pushstring(L, "listModules");
        lua_pushcfunction(L, LuaListRegisteredModules);
        lua_settable(L, -3);
        
        lua_pushstring(L, "listFunctions");
        lua_pushcfunction(L, LuaListModuleFunctions);
        lua_settable(L, -3);
        
        // Set it as global variable "ModuleAPI"
        lua_setglobal(L, "ModuleAPI");
        
        // Use luaL_loadfile and lua_pcall for better error handling
        int loadResult = luaL_loadfile(L, filePath.c_str());
        if (loadResult != 0) {
            SKSE::log::error("Failed to load Lua module: {}", lua_tostring(L, -1));
            lua_pop(L, 1);  // pop error message
            return false;
        }

        int pcallResult = lua_pcall(L, 0, LUA_MULTRET, 0);
        if (pcallResult != 0) {
            SKSE::log::error("Failed to execute Lua module: {}", lua_tostring(L, -1));
            lua_pop(L, 1);  // pop error message
            return false;
        }
        
        // Mark this module as loaded
        m_loadedModules.push_back(filePath);
        SKSE::log::info("Successfully loaded Lua module from: {}", filePath);
        return true;
    }

    int LuaModuleLoader::LoadModulesFromDirectory(lua_State* L, const std::string& dirPath) {
        if (!L) {
            SKSE::log::error("Cannot load modules: Lua state not initialized");
            return 0;
        }
        
        if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
            SKSE::log::error("Directory not found or not a directory: {}", dirPath);
            return 0;
        }
        
        int loadedCount = 0;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".lua") {
                if (LoadModuleFromFile(L, entry.path().string())) {
                    loadedCount++;
                }
            }
        }
        
        SKSE::log::info("Loaded {} Lua modules from directory: {}", loadedCount, dirPath);
        return loadedCount;
    }

    bool LuaModuleLoader::IsModuleLoaded(const std::string& filePath) const {
        return std::find(m_loadedModules.begin(), m_loadedModules.end(), filePath) != m_loadedModules.end();
    }

    std::vector<std::string> LuaModuleLoader::GetLoadedModules() const {
        return m_loadedModules;
    }

}
