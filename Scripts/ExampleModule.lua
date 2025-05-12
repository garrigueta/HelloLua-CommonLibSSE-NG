--[[
    Example Lua Module
    
    This example demonstrates how to create a Lua module that registers 
    functions with the HelloLua plugin system.
]]

-- Create a new module named "ExampleModule" with version "1.0.0"
ModuleAPI.createModule("ExampleModule", "1.0.0")

-- Access to native functions
local Log = ModuleAPI.getNativeFunction("Log")
if not Log then
    error("Failed to get native Log function")
end

Log("ExampleModule loaded successfully!")

-- Define a function that will be callable by other Lua scripts
local function CalculateDistance(x1, y1, z1, x2, y2, z2)
    local dx = x2 - x1
    local dy = y2 - y1
    local dz = z2 - z1
    return math.sqrt(dx * dx + dy * dy + dz * dz)
end

-- Register the function with the module system
ModuleAPI.registerFunction("ExampleModule", "CalculateDistance", CalculateDistance, 
    "Calculate distance between two 3D points")

-- Define a function that uses native functions 
local function GetDistanceToPlayer(x, y, z)
    -- Use the native GetPlayerPosition function
    local GetPlayerPosition = ModuleAPI.getNativeFunction("GetPlayerPosition")
    if not GetPlayerPosition then
        error("Failed to get native GetPlayerPosition function")
    end
    
    local playerX, playerY, playerZ = GetPlayerPosition()
    return CalculateDistance(x, y, z, playerX, playerY, playerZ)
end

-- Register the function with the module system
ModuleAPI.registerFunction("ExampleModule", "GetDistanceToPlayer", GetDistanceToPlayer,
    "Calculate distance from a point to the player")

-- Example of registering a function that calls another module's function
local function PrintDistanceToPlayer(x, y, z)
    local distance = GetDistanceToPlayer(x, y, z)
    
    -- Use the native PrintToConsole function
    local PrintToConsole = ModuleAPI.getNativeFunction("PrintToConsole")
    if PrintToConsole then
        PrintToConsole("Distance to player: " .. distance)
    end
    
    return distance
end

-- Register the function with the module system
ModuleAPI.registerFunction("ExampleModule", "PrintDistanceToPlayer", PrintDistanceToPlayer,
    "Print the distance from a point to the player to the console")

-- Example of listing available modules and their functions
local function ListAvailableFunctions()
    local modules = ModuleAPI.listModules()
    
    Log("Available modules:")
    for _, moduleName in ipairs(modules) do
        Log("  - " .. moduleName)
        
        local functions = ModuleAPI.listFunctions(moduleName)
        for _, funcInfo in ipairs(functions) do
            Log("    * " .. funcInfo.name .. ": " .. funcInfo.description)
        end
    end
end

-- Register the function with the module system
ModuleAPI.registerFunction("ExampleModule", "ListAvailableFunctions", ListAvailableFunctions,
    "List all available modules and functions")

-- Call our function to demonstrate it works
Log("Listing available modules and functions:")
ListAvailableFunctions()

-- Return the module table (optional)
return {
    name = "ExampleModule",
    version = "1.0.0",
    
    -- Public functions
    CalculateDistance = CalculateDistance,
    GetDistanceToPlayer = GetDistanceToPlayer,
    PrintDistanceToPlayer = PrintDistanceToPlayer,
    ListAvailableFunctions = ListAvailableFunctions
}
