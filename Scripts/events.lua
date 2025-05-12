-- events.lua
-- Event system for Skyrim Lua plugin

-- Create the events table that will be returned as a module
local Events = {}

-- Private variables (not exposed outside this module)
local eventHandlers = {}  -- Table to store event handlers
local timers = {}         -- Table to store timer information
local nextTimerID = 1     -- For generating unique timer IDs

-- ===============================================
-- Core event system functions
-- ===============================================

-- Register an event handler
function Events.register(eventName, handlerFn)
    if not eventHandlers[eventName] then
        eventHandlers[eventName] = {}
    end
    
    table.insert(eventHandlers[eventName], handlerFn)
    Log("Registered handler for event: " .. eventName)
    return handlerFn  -- Return the handler to allow unregistering later
end

-- Unregister an event handler
function Events.unregister(eventName, handlerFn)
    if not eventHandlers[eventName] then
        return false
    end
    
    for i, handler in ipairs(eventHandlers[eventName]) do
        if handler == handlerFn then
            table.remove(eventHandlers[eventName], i)
            Log("Unregistered handler for event: " .. eventName)
            return true
        end
    end
    
    return false
end

-- Trigger an event
function Events.trigger(eventName, ...)
    if not eventHandlers[eventName] then
        return false
    end
    
    local handlers = eventHandlers[eventName]
    for _, handler in ipairs(handlers) do
        local success, error = pcall(handler, ...)
        if not success then
            Log("Error in event handler for " .. eventName .. ": " .. tostring(error))
        end
    end
    
    return true
end

-- ===============================================
-- Timer functionality
-- ===============================================

-- Create a one-time timer (setTimeout equivalent)
function Events.setTimeout(callback, seconds)
    local id = nextTimerID
    nextTimerID = nextTimerID + 1
    
    timers[id] = {
        callback = callback,
        timeLeft = seconds,
        repeating = false
    }
    
    return id
end

-- Create a repeating timer (setInterval equivalent)
function Events.setInterval(callback, seconds)
    local id = nextTimerID
    nextTimerID = nextTimerID + 1
    
    timers[id] = {
        callback = callback,
        timeLeft = seconds,
        repeating = true,
        interval = seconds
    }
    
    return id
end

-- Clear a timer
function Events.clearTimer(id)
    if timers[id] then
        timers[id] = nil
        return true
    end
    return false
end

-- Update timers (call this each frame)
local function updateTimers(deltaTime)
    for id, timer in pairs(timers) do
        timer.timeLeft = timer.timeLeft - deltaTime
        
        if timer.timeLeft <= 0 then
            -- Call the callback
            local success, error = pcall(timer.callback)
            if not success then
                Log("Error in timer callback: " .. tostring(error))
            end
            
            -- Handle repeating timers
            if timer.repeating then
                timer.timeLeft = timer.interval
            else
                timers[id] = nil
            end
        end
    end
end

-- ===============================================
-- Native event hooks - connect to Skyrim events
-- ===============================================

-- Native event handler for OnUpdate (frame update)
local function onUpdateHandler(deltaTime)
    -- Update timers
    updateTimers(deltaTime)
    
    -- Trigger onUpdate event for any scripts that want it
    Events.trigger("onUpdate", deltaTime)
end

-- Initialize the event system and register native hooks
function Events.initialize()
    -- Register native event handlers
    RegisterForOnUpdate(onUpdateHandler)
    
    -- For now, we only have the onUpdate event working
    -- The other events need different registration methods
    Log("Events system initialized with onUpdate event")
    return true
end

-- Log when module is loaded
Log("Events module loaded")

-- Return the Events table as our module
return Events