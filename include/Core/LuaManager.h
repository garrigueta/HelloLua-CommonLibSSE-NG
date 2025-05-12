#pragma once

#include <string>
#include <vector>
#include <unordered_map>

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
        
        // Dynamic module system
        bool LoadModuleFromFile(const std::string& filePath);
        int LoadModulesFromDirectory(const std::string& dirPath);
        bool ReloadModules();
        
        // Function registry access
        void RegisterNativeFunction(const std::string& name, const std::string& category, 
                                   const std::string& description, LuaCFunction func);
        
        // Getter for Lua state
        lua_State* GetLuaState() const { return m_luaState; }
        
        // List loaded modules
        std::vector<std::string> GetLoadedModules() const;

    private:
        // The Lua state
        lua_State* m_luaState;
        
        // Registered script paths
        std::vector<std::string> m_scriptPaths;
        
        // Module directories for auto-loading
        std::vector<std::string> m_moduleDirectories;

        // Function registration
        void RegisterStandardFunctions();
        void RegisterGameFunctions();
        
        // Initialize the module API
        void InitializeModuleSystem();
    };
}