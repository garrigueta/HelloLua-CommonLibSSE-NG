#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

namespace Sample {
#pragma warning(push)
#pragma warning(disable : 4251)
    /**
     * The SKSEManager class serves as the main interface between Lua scripts and the game.
     * It encapsulates functionality for console output and other SKSE-related features.
     * 
     * Note: This class has been stripped down to only include functionality relevant to the Lua module system.
     */
    class __declspec(dllexport) SKSEManager {
    public:
        /**
         * Get the singleton instance of the SKSEManager.
         */
        [[nodiscard]] static SKSEManager* GetSingleton() noexcept;

        /**
         * Print a message to the Skyrim console.
         * This function is used by Lua modules.
         * 
         * @param message The message to print.
         */
        void PrintToConsole(const std::string& message);

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

        // Actor Management
        /**
         * Get an Actor from its form ID.
         *
         * @param formId The form ID of the actor.
         * @return The actor, or nullptr if not found.
         */
        RE::Actor* GetActorFromHandle(uint32_t formId) const;

        /**
         * Check if an actor reference is valid.
         *
         * @param actor The actor to check.
         * @return true if the actor is valid.
         */
        bool IsActorValid(RE::Actor* actor) const;

        // Player Functions
        /**
         * Get the player character.
         *
         * @return The player singleton.
         */
        RE::Actor* GetPlayer() const;

        // NPC Management
        /**
         * Force an actor value to a specific amount.
         *
         * @param actor The actor to modify.
         * @param avName The name of the actor value (e.g., "health", "stamina").
         * @param value The value to set.
         */
        void ForceActorValue(RE::Actor* actor, const std::string& avName, float value);

        /**
         * Get the current value of an actor value.
         *
         * @param actor The actor to query.
         * @param avName The name of the actor value.
         * @return The current value.
         */
        float GetActorValue(RE::Actor* actor, const std::string& avName) const;

        // Equipment Functions
        /**
         * Equip an item on an actor.
         *
         * @param actor The actor that will equip the item.
         * @param item The item to equip.
         * @param preventRemoval If true, the item cannot be unequipped.
         * @param silent If true, no sound or animation will play.
         * @return true if successful.
         */
        bool EquipItem(RE::Actor* actor, RE::TESForm* item, bool preventRemoval, bool silent);

        /**
         * Unequip an item from an actor.
         *
         * @param actor The actor that will unequip the item.
         * @param item The item to unequip.
         * @param silent If true, no sound or animation will play.
         * @return true if successful.
         */
        bool UnequipItem(RE::Actor* actor, RE::TESForm* item, bool silent);

        // World Interaction
        /**
         * Find the closest reference of a given form type within range of the player.
         *
         * @param formToMatch The base form to search for.
         * @param searchRadius The maximum search radius.
         * @return The closest matching reference, or nullptr if none found.
         */
        RE::TESObjectREFR* FindClosestReferenceOfType(RE::TESForm* formToMatch, float searchRadius) const;

        // Quest and Game State
        /**
         * Set a quest to a specific stage.
         *
         * @param questId The form ID of the quest.
         * @param stage The stage to set.
         * @return true if successful.
         */
        bool SetQuestStage(uint32_t questId, uint16_t stage);

        /**
         * Get the current stage of a quest.
         *
         * @param questId The form ID of the quest.
         * @return The current stage, or 0 if the quest is not found.
         */
        uint16_t GetQuestStage(uint32_t questId) const;

        /**
         * Check if a quest is completed.
         *
         * @param questId The form ID of the quest.
         * @return true if the quest is completed.
         */
        bool IsQuestCompleted(uint32_t questId) const;

        // Weather and Environment
        /**
         * Get the current weather.
         *
         * @return The current weather, or nullptr if not available.
         */
        RE::TESWeather* GetCurrentWeather() const;

        /**
         * Force a specific weather.
         *
         * @param weather The weather to force.
         */
        void ForceWeather(RE::TESWeather* weather);

        // UI Functions
        /**
         * Check if a menu is currently open.
         *
         * @param menuName The name of the menu.
         * @return true if the menu is open.
         */
        bool IsMenuOpen(const std::string& menuName) const;

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

        // Forms and Objects
        /**
         * Get a form from its ID.
         *
         * @param formId The form ID.
         * @return The form, or nullptr if not found.
         */
        RE::TESForm* GetFormFromID(uint32_t formId) const;

        /**
         * Get a form from its editor ID.
         *
         * @param editorId The editor ID.
         * @return The form, or nullptr if not found.
         */
        RE::TESForm* GetFormFromEditorID(const std::string& editorId) const;

        // Utility Functions
        /**
         * Register a Lua function to be called during the game's update loop.
         *
         * @param functionRef The reference to the Lua function in the registry.
         */
        void RegisterLuaUpdateCallback(int functionRef);

        /**
         * Get the distance between two actors.
         *
         * @param actor1 The first actor.
         * @param actor2 The second actor.
         * @return The distance, or -1.0 if either actor is invalid.
         */
        float GetActorDistance(RE::Actor* actor1, RE::Actor* actor2) const;

        /**
         * Get the display name of a form.
         *
         * @param form The form to query.
         * @return The name, or an empty string if the form has no name.
         */
        std::string GetFormName(RE::TESForm* form) const;

        /**
         * The serialization handler for reverting game state.
         */
        static void OnRevert(SKSE::SerializationInterface*);

        /**
         * The serialization handler for saving data to the cosave.
         */
        static void OnGameSaved(SKSE::SerializationInterface* serde);

        /**
         * The serialization handler for loading data from a cosave.
         */
        static void OnGameLoaded(SKSE::SerializationInterface* serde);

    private:
        SKSEManager() = default;
        
        mutable std::recursive_mutex _lock;
        std::unordered_set<RE::Actor*> _trackedActors;
        std::unordered_map<RE::Actor*, int32_t> _hitCounts;
        std::vector<int> _luaUpdateCallbacks;  // Store Lua function references for update callbacks
    };
#pragma warning(pop)
}  // namespace Sample