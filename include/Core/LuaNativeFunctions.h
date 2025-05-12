#pragma once

#include <unordered_map>
#include <string>
#include <functional>
#include <vector>

struct lua_State;
typedef int (*LuaCFunction)(lua_State* L);

namespace Sample {

    // Structure to hold metadata about native functions
    struct NativeFunctionInfo {
        std::string name;
        std::string category;
        std::string description;
        LuaCFunction function;
    };

    // Class to manage the registry of native functions available to Lua
    class LuaNativeFunctions {
    public:
        static LuaNativeFunctions* GetSingleton();

        // Register a native C++ function with metadata
        void RegisterNativeFunction(const std::string& name, 
                                    const std::string& category,
                                    const std::string& description,
                                    LuaCFunction func);

        // Get all registered functions in a specific category
        std::vector<NativeFunctionInfo> GetFunctionsByCategory(const std::string& category) const;

        // Get all registered native functions
        const std::unordered_map<std::string, NativeFunctionInfo>& GetAllRegisteredFunctions() const;

        // Get a function by name
        LuaCFunction GetFunctionByName(const std::string& name) const;

        // Check if a function is registered
        bool IsFunctionRegistered(const std::string& name) const;

    private:
        LuaNativeFunctions() = default;
        ~LuaNativeFunctions() = default;

        // Map of function name to function info
        std::unordered_map<std::string, NativeFunctionInfo> m_registeredFunctions;
    };
}
