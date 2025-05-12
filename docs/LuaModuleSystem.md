# Dynamic Lua Module System for Skyrim

This document describes the dynamic Lua module system for the HelloLua SKSE plugin. The module system allows you to create and register Lua functions without modifying the C++ code, making the plugin more flexible and extensible.

## Overview

The dynamic module system provides:

1. A way to register native C++ functions with metadata
2. A module API for Lua scripts to register their own functions
3. Auto-loading of Lua modules from designated directories
4. Easy interoperability between Lua modules

## Directory Structure

Lua modules should be placed in:
```
Data/SKSE/Plugins/LuaModules/
```

This directory is automatically scanned when the Lua environment initializes.

## Creating a Lua Module

A Lua module is a `.lua` file that registers functions with the module system. Here's a simple example:

```lua
-- Create a new module named "MyModule" with version "1.0.0"
ModuleAPI.createModule("MyModule", "1.0.0")

-- Define a function
local function SayHello(name)
    local PrintToConsole = ModuleAPI.getNativeFunction("PrintToConsole")
    PrintToConsole("Hello, " .. (name or "world") .. "!")
    return true
end

-- Register the function with the module system
ModuleAPI.registerFunction("MyModule", "SayHello", SayHello, 
    "Prints a greeting to the Skyrim console")

-- You can also return a table to use with require()
return {
    name = "MyModule",
    version = "1.0.0",
    SayHello = SayHello
}
```

## Using the Module API

The ModuleAPI provides several functions for working with modules:

### Creating a Module

```lua
ModuleAPI.createModule(moduleName, moduleVersion)
```

- `moduleName`: String name for your module (must be unique)
- `moduleVersion`: String version identifier (optional)

### Registering Functions

```lua
ModuleAPI.registerFunction(moduleName, functionName, functionObject, description)
```

- `moduleName`: The module this function belongs to
- `functionName`: Name for the function
- `functionObject`: The actual Lua function
- `description`: Description of what the function does (optional)

### Using Native Functions

```lua
local nativeFunc = ModuleAPI.getNativeFunction(functionName)
```

Retrieves a native C++ function that was registered with the system.

### Listing Available Modules and Functions

```lua
-- Get all registered modules
local modules = ModuleAPI.listModules()

-- Get functions in a specific module
local functions = ModuleAPI.listFunctions(moduleName)
```

## Accessing Modules from Other Scripts

You can use the standard Lua `require()` function to access a module:

```lua
-- Load the MyModule module
local myModule = require("MyModule")

-- Use a function from the module
myModule.SayHello("Dragonborn")
```

## Best Practices

1. Always create a module with `ModuleAPI.createModule()` before registering functions
2. Provide clear, descriptive names for your modules and functions
3. Include detailed descriptions of what your functions do
4. Return a table from your module with all public functions for use with `require()`
5. Use semantic versioning for your module versions

## Example

See `Scripts/ExampleModule.lua` for a complete example of creating and using a module.

## Troubleshooting

If you encounter issues with the module system:

1. Check the SKSE logs for error messages
2. Make sure your module is in the correct directory
3. Check that all required functions are properly defined and registered
4. Test with the `ListAvailableFunctions()` function from the example module to see what's available
