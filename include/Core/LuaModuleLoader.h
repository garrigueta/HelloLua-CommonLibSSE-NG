#pragma once

#include "Core/LuaFunctionRegistry.h"
#include <string>
#include <memory>
#include <vector>

struct lua_State;

namespace Sample {
    
    // Class responsible for loading and executing Lua modules from files
    class LuaModuleLoader {
    public:
        static LuaModuleLoader* GetSingleton();

        // Load a Lua module from file
        bool LoadModuleFromFile(lua_State* L, const std::string& filePath);

        // Load all Lua modules from a directory (recursive)
        int LoadModulesFromDirectory(lua_State* L, const std::string& dirPath);

        // Check if a module file has been loaded
        bool IsModuleLoaded(const std::string& filePath) const;

        // Get a list of all loaded modules
        std::vector<std::string> GetLoadedModules() const;

    private:
        LuaModuleLoader() = default;
        ~LuaModuleLoader() = default;

        // Helper functions for module registration
        static int LuaRegisterFunction(lua_State* L);
        static int LuaCreateModule(lua_State* L);
        static int LuaGetNativeFunction(lua_State* L);
        static int LuaListRegisteredModules(lua_State* L);
        static int LuaListModuleFunctions(lua_State* L);

        // Registry of loaded module paths
        std::vector<std::string> m_loadedModules;
    };
}
