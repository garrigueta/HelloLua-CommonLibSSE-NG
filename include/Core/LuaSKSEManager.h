#pragma once

#include <string>
#include <vector>
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <mutex>

// Forward declare lua_State to avoid including lua.h in header
struct lua_State;
typedef int (*LuaCFunction)(lua_State* L);

namespace Sample {
    /**
     * The LuaSKSEManager class combines the functionality of LuaManager and SKSEManager.
     * It serves as both the Lua script engine and the interface to Skyrim through SKSE.
     */
#pragma warning(push)
#pragma warning(disable: 4251) // Disable warning about STL classes needing DLL interface
    class __declspec(dllexport) LuaSKSEManager {
    public:
        /**
         * Get the singleton instance of the LuaSKSEManager.
         */
        [[nodiscard]] static LuaSKSEManager* GetSingleton() noexcept;

        // Constructor and destructor
        LuaSKSEManager();
        ~LuaSKSEManager();

        // Lua core functionality
        bool Initialize();
        void Close();
        bool ExecuteScript(const std::string& scriptPath);
        bool ExecuteString(const std::string& luaCode);
        bool RegisterFunction(const char* name, LuaCFunction func);
        void AddPackagePath(const std::string& path);

        // Getter for Lua state
        lua_State* GetLuaState() const { return m_luaState; }

        // SKSE functions
        
        /**
         * Print a message to the Skyrim console.
         * 
         * @param message The message to print.
         */
        void PrintToConsole(const std::string& message);
        
        /**
         * Get the player's current position.
         * 
         * @return The player's position as a NiPoint3.
         */
        RE::NiPoint3 GetPlayerPosition() const;

        // Hit Counter functions
        /**
         * Track an actor for hit counting.
         * 
         * @param target The actor to track.
         * @return true if successful or already tracked, false otherwise.
         */
        bool TrackActor(RE::Actor* target);

        /**
         * Stop tracking an actor.
         * 
         * @param target The actor to untrack.
         * @return true if successful, false otherwise.
         */
        bool UntrackActor(RE::Actor* target);

        /**
         * Increment the hit count for an actor.
         * 
         * @param target The actor whose hit count to increment.
         * @param by The amount to increment by. Defaults to 1.
         */
        void IncrementHitCount(RE::Actor* target, int32_t by = 1);

        /**
         * Get the current hit count for an actor.
         * 
         * @param target The actor whose hit count to get.
         * @return The hit count if the actor is being tracked, std::nullopt otherwise.
         */
        [[nodiscard]] std::optional<int32_t> GetHitCount(RE::Actor* target) const noexcept;

        // Actor Management
        /**
         * Get an Actor from a form ID.
         * 
         * @param formId The form ID of the actor.
         * @return The Actor if found, nullptr otherwise.
         */
        [[nodiscard]] RE::Actor* GetActorFromHandle(uint32_t formId) const;

        /**
         * Check if an actor is valid.
         * 
         * @param actor The actor to check.
         * @return true if the actor is valid, false otherwise.
         */
        [[nodiscard]] bool IsActorValid(RE::Actor* actor) const;

        // Player functions
        /**
         * Get the player character.
         * 
         * @return The player character.
         */
        [[nodiscard]] RE::Actor* GetPlayer() const;

        // Actor value functions
        /**
         * Force an actor value.
         * 
         * @param actor The actor whose value to force.
         * @param avName The name of the actor value.
         * @param value The value to set.
         */
        void ForceActorValue(RE::Actor* actor, const std::string& avName, float value);

        /**
         * Get an actor value.
         * 
         * @param actor The actor whose value to get.
         * @param avName The name of the actor value.
         * @return The actor value.
         */
        [[nodiscard]] float GetActorValue(RE::Actor* actor, const std::string& avName) const;

        // Equipment functions
        /**
         * Equip an item on an actor.
         * 
         * @param actor The actor to equip the item on.
         * @param item The item to equip.
         * @param preventRemoval Whether to prevent the item from being removed.
         * @param silent Whether to suppress equip sounds.
         * @return true if successful, false otherwise.
         */
        bool EquipItem(RE::Actor* actor, RE::TESForm* item, bool preventRemoval, bool silent);

        /**
         * Unequip an item from an actor.
         * 
         * @param actor The actor to unequip the item from.
         * @param item The item to unequip.
         * @param silent Whether to suppress unequip sounds.
         * @return true if successful, false otherwise.
         */
        bool UnequipItem(RE::Actor* actor, RE::TESForm* item, bool silent);

        // World interaction
        /**
         * Find the closest reference of a specific form type.
         * 
         * @param formToMatch The form type to match.
         * @param searchRadius The radius to search within.
         * @return The closest reference if found, nullptr otherwise.
         */
        [[nodiscard]] RE::TESObjectREFR* FindClosestReferenceOfType(RE::TESForm* formToMatch, float searchRadius) const;

        // Quest and game state
        /**
         * Set a quest stage.
         * 
         * @param questId The quest ID.
         * @param stage The stage to set.
         * @return true if successful, false otherwise.
         */
        bool SetQuestStage(uint32_t questId, uint16_t stage);

        /**
         * Get the current stage of a quest.
         * 
         * @param questId The quest ID.
         * @return The current stage.
         */
        [[nodiscard]] uint16_t GetQuestStage(uint32_t questId) const;

        /**
         * Check if a quest is completed.
         * 
         * @param questId The quest ID.
         * @return true if the quest is completed, false otherwise.
         */
        [[nodiscard]] bool IsQuestCompleted(uint32_t questId) const;

        // Weather and environment
        /**
         * Get the current weather.
         * 
         * @return The current weather.
         */
        [[nodiscard]] RE::TESWeather* GetCurrentWeather() const;

        /**
         * Force a specific weather.
         * 
         * @param weather The weather to force.
         */
        void ForceWeather(RE::TESWeather* weather);

        // UI functions
        /**
         * Check if a menu is open.
         * 
         * @param menuName The name of the menu.
         * @return true if the menu is open, false otherwise.
         */
        [[nodiscard]] bool IsMenuOpen(const std::string& menuName) const;

        /**
         * Open a menu.
         * 
         * @param menuName The name of the menu to open.
         */
        void OpenMenu(const std::string& menuName);

        /**
         * Close a menu.
         * 
         * @param menuName The name of the menu to close.
         */
        void CloseMenu(const std::string& menuName);

        // Forms and objects
        /**
         * Get a form from its ID.
         * 
         * @param formId The form ID.
         * @return The form if found, nullptr otherwise.
         */
        [[nodiscard]] RE::TESForm* GetFormFromID(uint32_t formId) const;

        /**
         * Get a form from its editor ID.
         * 
         * @param editorId The editor ID.
         * @return The form if found, nullptr otherwise.
         */
        [[nodiscard]] RE::TESForm* GetFormFromEditorID(const std::string& editorId) const;

        // Utility functions
        /**
         * Get the distance between two actors.
         * 
         * @param actor1 The first actor.
         * @param actor2 The second actor.
         * @return The distance between the actors.
         */
        [[nodiscard]] float GetActorDistance(RE::Actor* actor1, RE::Actor* actor2) const;

        /**
         * Get the name of a form.
         * 
         * @param form The form.
         * @return The form's name.
         */
        [[nodiscard]] std::string GetFormName(RE::TESForm* form) const;

        /**
         * Register a Lua update callback.
         * 
         * @param functionRef The Lua function reference.
         */
        void RegisterLuaUpdateCallback(int functionRef);

        // Serialization methods
        static void OnRevert(SKSE::SerializationInterface*);
        static void OnGameSaved(SKSE::SerializationInterface* serde);
        static void OnGameLoaded(SKSE::SerializationInterface* serde);

    private:
        // The Lua state
        lua_State* m_luaState;
        
        // Registered script paths
        std::vector<std::string> m_scriptPaths;

        // Function registration
        void RegisterStandardFunctions();
        void RegisterGameFunctions();

        // Hit counter storage
        mutable std::recursive_mutex _lock;
        std::unordered_set<RE::Actor*> _trackedActors;
        std::unordered_map<RE::Actor*, int32_t> _hitCounts;
        std::vector<int> _luaUpdateCallbacks;
    };
#pragma warning(pop)
}
