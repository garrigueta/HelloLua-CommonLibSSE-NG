#pragma once

#include <string>
#include <vector>
#include <functional>

struct lua_State;
typedef int (*LuaCFunction)(lua_State* L);

namespace Sample {
    // Forward declarations
    class LuaFunctionRegistry;

    // Interface for modules that register functions with the Lua system
    class ILuaModuleInterface {
    public:
        virtual ~ILuaModuleInterface() = default;
        
        // Called when the module should register its functions
        virtual void RegisterFunctions(LuaFunctionRegistry* registry) = 0;
        
        // Name of the module
        virtual const char* GetModuleName() const = 0;
        
        // Module version
        virtual const char* GetVersion() const = 0;
    };

    // Lua module registration system
    class LuaFunctionRegistry {
    public:
        static LuaFunctionRegistry* GetSingleton();

        // Register a Lua module
        bool RegisterModule(ILuaModuleInterface* module);

        // Unregister a Lua module by name
        bool UnregisterModule(const char* moduleName);

        // Register a function that can be called from Lua
        bool RegisterFunction(const char* moduleName, const char* funcName, LuaCFunction func, const char* description = "");

        // Check if a module is registered
        bool IsModuleRegistered(const char* moduleName);

        // Get all registered module names
        std::vector<std::string> GetRegisteredModules() const;

        // Apply all registered functions to a Lua state
        void ApplyToLuaState(lua_State* L);

        // Structure to hold function metadata
        struct FunctionInfo {
            std::string moduleName;
            std::string functionName;
            std::string description;
            LuaCFunction function;
        };

        // Get all registered functions for a module
        std::vector<FunctionInfo> GetModuleFunctions(const char* moduleName) const;

    private:
        LuaFunctionRegistry() = default;
        ~LuaFunctionRegistry() = default;

        // Map of module name to registered module interface
        std::unordered_map<std::string, ILuaModuleInterface*> m_modules;

        // Map of module name to function infos
        std::unordered_map<std::string, std::vector<FunctionInfo>> m_functions;
    };
}
