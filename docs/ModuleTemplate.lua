--[[
    Module Template
    
    This is a template for creating a Lua module for the HelloLua plugin.
    
    To use this template:
    1. Copy this file to Data/SKSE/Plugins/LuaModules/
    2. Rename it to YourModuleName.lua
    3. Replace "ModuleTemplate" with your module name
    4. Replace the example functions with your own
    5. Add your module-specific logic
]]

-- Create a new module - replace "ModuleTemplate" with your module name
ModuleAPI.createModule("ModuleTemplate", "1.0.0")

-- Access native functions you need
local Log = ModuleAPI.getNativeFunction("Log")
local PrintToConsole = ModuleAPI.getNativeFunction("PrintToConsole")

if not Log or not PrintToConsole then
    error("Failed to get required native functions")
end

Log("ModuleTemplate loaded successfully!")

-- Define your functions
local function ExampleFunction(param1, param2)
    -- Your implementation here
    local result = param1 + param2
    return result
end

-- Register your functions with the module system
ModuleAPI.registerFunction("ModuleTemplate", "ExampleFunction", ExampleFunction, 
    "Example function that adds two numbers")

-- You can access functions from other modules if they're loaded
local function UseOtherModule()
    -- Try to require another module
    local success, otherModule = pcall(require, "ExampleModule")
    if success and otherModule then
        -- Use a function from the other module
        local result = otherModule.CalculateDistance(0, 0, 0, 1, 1, 1)
        PrintToConsole("Distance calculated: " .. result)
        return result
    else
        PrintToConsole("ExampleModule not available")
        return nil
    end
end

-- Register the function
ModuleAPI.registerFunction("ModuleTemplate", "UseOtherModule", UseOtherModule, 
    "Example of using functions from another module")

-- You can define local helper functions that aren't exposed
local function HelperFunction()
    -- Internal implementation
    return "This is a helper function"
end

-- Initialization code runs when the module is loaded
Log("ModuleTemplate initialization complete")

-- Return a table for use with require()
return {
    name = "ModuleTemplate",
    version = "1.0.0",
    
    -- Public functions
    ExampleFunction = ExampleFunction,
    UseOtherModule = UseOtherModule
    
    -- Note: HelperFunction is not exported
}
