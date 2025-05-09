-- utils.lua
-- A collection of utility functions for Skyrim Lua modding

-- Create local Utils table that will be returned as the module
local Utils = {}

-- ===============================================
-- String utilities
-- ===============================================

-- Format a form ID as a standard hex string (8 digits with leading zeros)
function Utils.formatFormID(formID)
    return string.format("%08X", formID or 0)
end

-- Split a string by delimiter
function Utils.split(str, delimiter)
    delimiter = delimiter or ","
    local result = {}
    for match in (str..delimiter):gmatch("(.-)"..delimiter) do
        table.insert(result, match)
    end
    return result
end

-- Trim whitespace from beginning and end of string
function Utils.trim(str)
    return str:match("^%s*(.-)%s*$")
end

-- ===============================================
-- Table utilities
-- ===============================================

-- Check if a table contains a value
function Utils.contains(tbl, value)
    for _, v in pairs(tbl) do
        if v == value then
            return true
        end
    end
    return false
end

-- Create a copy of a table (shallow)
function Utils.copyTable(tbl)
    local result = {}
    for k, v in pairs(tbl) do
        result[k] = v
    end
    return result
end

-- Print a table to log (useful for debugging)
function Utils.dumpTable(tbl, indent, seen)
    indent = indent or 0
    seen = seen or {}
    
    -- Avoid infinite recursion
    if seen[tbl] then
        return "recursion"
    end
    seen[tbl] = true
    
    local spacing = string.rep("  ", indent)
    local result = "{\n"
    
    for k, v in pairs(tbl) do
        result = result .. spacing .. "  "
        
        -- Format key
        if type(k) == "string" then
            result = result .. k .. " = "
        else
            result = result .. "[" .. tostring(k) .. "] = "
        end
        
        -- Format value
        if type(v) == "table" then
            result = result .. Utils.dumpTable(v, indent + 1, seen)
        elseif type(v) == "string" then
            result = result .. '"' .. v .. '"'
        else
            result = result .. tostring(v)
        end
        
        result = result .. ",\n"
    end
    
    result = result .. spacing .. "}"
    return result
end

-- Log a table's contents
function Utils.logTable(tbl, name)
    name = name or "table"
    Log("Contents of " .. name .. ":")
    Log(Utils.dumpTable(tbl))
end

-- ===============================================
-- Math utilities
-- ===============================================

-- Get the distance between two positions
function Utils.getDistance(pos1, pos2)
    return math.sqrt((pos1.x - pos2.x)^2 + 
                    (pos1.y - pos2.y)^2 + 
                    (pos1.z - pos2.z)^2)
end

-- Clamp a value between min and max
function Utils.clamp(value, min, max)
    if value < min then return min end
    if value > max then return max end
    return value
end

-- Create a position table from x, y, z values
function Utils.createPosition(x, y, z)
    return { x = x or 0, y = y or 0, z = z or 0 }
end

-- ===============================================
-- Game utilities
-- ===============================================

-- Find the nearest actor of a specific type to the player
function Utils.findNearestActor(actorBaseID, maxDistance)
    maxDistance = maxDistance or 5000.0
    local playerPos = Utils.getPlayerPosition()
    
    if not playerPos then
        Log("Cannot find player position")
        return nil
    end
    
    local baseForm = GetFormFromID(actorBaseID)
    if not baseForm then
        Log("Cannot find form with ID " .. Utils.formatFormID(actorBaseID))
        return nil
    end
    
    local nearestRef = FindClosestReferenceOfType(baseForm, maxDistance)
    if not nearestRef then
        return nil
    end
    
    -- Return the actual actor
    return nearestRef
end

-- Get the player position as a table
function Utils.getPlayerPosition()
    local x, y, z = GetPlayerPosition()
    if not x then
        return nil
    end
    return Utils.createPosition(x, y, z)
end

-- Is the specified menu currently open?
function Utils.isMenuOpen(menuName)
    return IsMenuOpen(menuName) == true
end

-- Check if player is in combat
function Utils.isPlayerInCombat()
    local player = GetPlayer()
    if not player then return false end
    
    -- Note: This is a stub since we don't have direct access to this info
    Log("Utils.isPlayerInCombat() is a stub function")
    return false
end

-- Log when module is loaded
Log("Utils module loaded")

-- Return the Utils table as the module
return Utils