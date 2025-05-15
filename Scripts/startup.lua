-- HelloLua Skyrim Plugin - Example Script
-- This script runs when the game loads and demonstrates common modding tasks

-- Load our utility libraries
local Events = require("events")
local Utils = require("utils")

-- Log initialization message
Log("HelloLua mod initializing...")

-- Print a welcome message to the Skyrim console
PrintToConsole("HelloLua mod successfully loaded!")

-- ===============================================
-- Configuration section - Adjust these values
-- ===============================================
local config = {
    debugMode = true,           -- Enable extended logging
    playerTrackingEnabled = true,  -- Enable player position tracking
    equipWeaponOnStart = false,    -- Auto-equip a weapon on startup
    weatherEffect = nil,           -- Weather to force (nil for no change)
    
    -- Example FormIDs (replace with actual IDs from your game)
    exampleNPC = 0x0001A67D,       -- Hadvar's FormID
    exampleWeapon = 0x0001397E,    -- Iron Sword FormID
}

-- ===============================================
-- Utility functions
-- ===============================================
local function debugLog(message)
    if config.debugMode then
        Log("[DEBUG] " .. message)
    end
end

-- Format actor information into readable text
local function getActorInfo(actorFormID)
    local actor = GetActorFromHandle(actorFormID)
    if not IsActorValid(actor) then
        return "Invalid Actor"
    end
    
    local name = GetFormName(actor) or "Unknown"
    local health = GetActorValue(actor, "Health") or 0
    local isTracked = HelloLua and HelloLua.isActorTracked and HelloLua.isActorTracked(actorFormID)
    
    return string.format("Name: %s, Health: %.1f, Tracked: %s", 
        name, health, isTracked and "Yes" or "No")
end

-- ===============================================
-- Mod features demonstration
-- ===============================================

-- Actor tracking system
local function trackActor(formID)
    local result = TrackActor(formID)
    if result then
        debugLog(string.format("Now tracking actor with FormID: %s", Utils.formatFormID(formID)))
        return true
    else
        Log(string.format("Failed to track actor with FormID: %s", Utils.formatFormID(formID)))
        return false
    end
end

local function incrementActorHitCount(formID, amount)
    amount = amount or 1
    local actor = GetActorFromHandle(formID)
    
    if not IsActorValid(actor) then
        Log(string.format("Actor %s is not valid", Utils.formatFormID(formID)))
        return false
    end
    
    IncrementHitCount(actor, amount)
    local count = GetHitCount(actor)
    
    if count then
        debugLog(string.format("%s now has %d hits", GetFormName(actor) or "Actor", count))
        return true
    else
        return false
    end
end

-- Menu interaction
local function checkGameMenus()
    local commonMenus = {"InventoryMenu", "MagicMenu", "MapMenu", "StatsMenu"}
    for _, menuName in ipairs(commonMenus) do
        local isOpen = Utils.isMenuOpen(menuName)
        debugLog(string.format("Menu '%s' is %s", menuName, isOpen and "OPEN" or "closed"))
    end
end

-- ===============================================
-- Event handlers
-- ===============================================

-- Handler for game load event
Events.register("onGameLoad", function()
    debugLog("Game load event detected - running mod initialization")
    
    -- Example 1: Track player position
    if config.playerTrackingEnabled then
        local playerPos = Utils.getPlayerPosition()
        if playerPos then
            debugLog(string.format("Player position: X=%.2f, Y=%.2f, Z=%.2f", playerPos.x, playerPos.y, playerPos.z))
        else
            Log("Couldn't get player position - player may not be loaded yet")
        end
    end
    
    -- Track example NPC
    if config.exampleNPC then
        trackActor(config.exampleNPC)
        incrementActorHitCount(config.exampleNPC, 1)
        debugLog("Example NPC info: " .. getActorInfo(config.exampleNPC))
    end
    
    -- Check game menus after a short delay to ensure UI is loaded
    Events.setTimeout(function()
        checkGameMenus()
    end, 2.0)
    
    -- Example weather manipulation
    if config.weatherEffect then
        local weather = GetFormFromID(config.weatherEffect)
        if weather then
            debugLog("Forcing weather change...")
            ForceWeather(weather)
        end
    end
    
    PrintToConsole("HelloLua mod is ready!")
end)

-- Handler for equipment changes
Events.register("onEquip", function(actor, item)
    -- This is triggered when an item is equipped
    if actor == GetPlayer() then
        debugLog("Player equipped: " .. (GetFormName(item) or "Unknown Item"))
    end
end)

-- Handler for menu open/close
Events.register("onMenuOpen", function(menuName)
    debugLog("Menu opened: " .. menuName)
end)

Events.register("onMenuClose", function(menuName)
    debugLog("Menu closed: " .. menuName)
end)

-- ===============================================
-- Global API for other scripts to use
-- ===============================================
HelloLua = {
    version = "1.0.0",
    
    -- Actor functions
    trackActor = trackActor,
    incrementHits = incrementActorHitCount,
    getActorInfo = getActorInfo,
    
    -- Check if an actor is being tracked
    isActorTracked = function(formID)
        local actor = GetActorFromHandle(formID)
        return actor and GetHitCount(actor) ~= nil
    end,
    
    -- Utility functions
    checkMenus = checkGameMenus,
    utils = Utils,
    events = Events,
    
    -- Debug functions
    toggleDebug = function(value)
        if value ~= nil then
            config.debugMode = value
        else
            config.debugMode = not config.debugMode
        end
        Log("Debug mode " .. (config.debugMode and "enabled" or "disabled"))
        return config.debugMode
    end,
    
    -- Event system access
    on = Events.register,
    off = Events.unregister,
    trigger = Events.trigger,
    setTimeout = Events.setTimeout,
    setInterval = Events.setInterval,
    clearTimer = Events.clearTimer
}

-- Initialize the events system
Events.initialize()

-- Log success message
Log("HelloLua mod initialization complete - use HelloLua.* functions in console")