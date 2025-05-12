--[[
    Lua Module System Test
    
    This script demonstrates how to use the dynamic module system.
]]

-- Access the ExampleModule registered functions
if ExampleModule then
    print("ExampleModule is available in the global namespace")
else
    print("ExampleModule is not available directly - this is expected if modules aren't auto-globalized")
end

-- Load the ExampleModule
local success, exampleModule = pcall(require, "ExampleModule")
if not success then
    error("Failed to load ExampleModule: " .. tostring(exampleModule))
end

-- Now that we have the module, we can use its functions
if exampleModule then
    HelloLua.Log("ExampleModule loaded successfully")
    
    -- Calculate distance between two points
    local distance = exampleModule.CalculateDistance(0, 0, 0, 10, 10, 10)
    HelloLua.Log("Distance between points: " .. distance)
    
    -- Get player position and calculate distance to origin
    local playerX, playerY, playerZ = HelloLua.GetPlayerPosition()
    HelloLua.Log("Player position: " .. playerX .. ", " .. playerY .. ", " .. playerZ)
    
    local distToPlayer = exampleModule.GetDistanceToPlayer(0, 0, 0)
    HelloLua.Log("Distance from origin to player: " .. distToPlayer)
    
    -- Print the distance to the player to the console
    exampleModule.PrintDistanceToPlayer(0, 0, 0)
    
    -- List all registered modules and functions
    exampleModule.ListAvailableFunctions()
else
    HelloLua.Log("Failed to load ExampleModule")
end

-- This demonstrates how to dynamically create and register functions at runtime
-- Create a new module
if ModuleAPI then
    ModuleAPI.createModule("RuntimeModule", "1.0.0")
    
    -- Define and register a function
    local function Greet(name)
        HelloLua.PrintToConsole("Hello, " .. (name or "adventurer") .. "!")
        return "Greeted " .. (name or "adventurer")
    end
    
    ModuleAPI.registerFunction("RuntimeModule", "Greet", Greet, "Greets the specified person")
    HelloLua.Log("RuntimeModule.Greet registered dynamically")
    
    -- Use the function
    local result = ModuleAPI.getNativeFunction("Greet")
    if result then
        result("Dragonborn")
    else
        HelloLua.Log("Failed to get Greet function")
    end
else
    HelloLua.Log("ModuleAPI not available")
end
