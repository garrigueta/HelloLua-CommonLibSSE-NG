#pragma once

#include <string>
#include <vector>

// Forward declare lua_State to avoid including lua.h in header
struct lua_State;
typedef int (*LuaCFunction)(lua_State* L);

namespace Sample {
    class LuaManager {
    public:
        // Singleton access
        static LuaManager* GetSingleton();

        // Constructor and destructor
        LuaManager();
        ~LuaManager();

        // Core functionality
        bool Initialize();
        void Close();
        bool ExecuteScript(const std::string& scriptPath);
        bool ExecuteString(const std::string& luaCode);
        bool RegisterFunction(const char* name, LuaCFunction func);
        void AddPackagePath(const std::string& path);

    private:
        // The Lua state
        lua_State* m_luaState;
        
        // Registered script paths
        std::vector<std::string> m_scriptPaths;

        // Function registration
        void RegisterStandardFunctions();
        void RegisterGameFunctions();
    };
}