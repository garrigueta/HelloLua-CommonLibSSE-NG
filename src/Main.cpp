#include <Core/LuaManager.h>
#include "Core/SKSEManager.h"

#include <stddef.h>

using namespace Sample;
using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

namespace {
    /**
     * Setup logging.
     */
    void InitializeLogging() {
        auto path = log_directory();
        if (!path) {
            report_and_fail("Unable to lookup SKSE logs directory.");
        }
        *path /= PluginDeclaration::GetSingleton()->GetName();
        *path += L".log";

        std::shared_ptr<spdlog::logger> log;
        if (IsDebuggerPresent()) {
            log = std::make_shared<spdlog::logger>(
                "Global", std::make_shared<spdlog::sinks::msvc_sink_mt>());
        } else {
            log = std::make_shared<spdlog::logger>(
                "Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));
        }

        spdlog::set_default_logger(std::move(log));
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v");
    }

    /**
     * Initialize the Lua module system.
     */
    void InitializeLua() {
        log::trace("Initializing Lua module system...");
        if (auto* luaManager = Sample::LuaManager::GetSingleton()) {
            if (luaManager->Initialize()) {
                log::info("Lua module system initialized successfully");
                
                // Execute our test modules script to demonstrate the module system
                if (luaManager->ExecuteScript("test_modules.lua")) {
                    log::info("Successfully executed test modules script");
                } else {
                    log::warn("Error executing test modules script");
                }
            } else {
                log::error("Failed to initialize Lua module system");
            }
        }
    }

    /**
     * Register to listen for SKSE messages.
     */
    void InitializeMessaging() {
        if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) {
            switch (message->type) {
                case MessagingInterface::kDataLoaded: // All ESM/ESL/ESP plugins have loaded, main menu is now active.
                    // It is now safe to access form data and initialize Lua
                    InitializeLua();
                    break;

                default:
                    break;
            }
        })) {
            stl::report_and_fail("Unable to register message listener.");
        }
    }
}

/**
 * Entry point for the Lua Module System plugin.
 * This plugin provides a dynamic way to register and use Lua functions in Skyrim.
 */
SKSEPluginLoad(const LoadInterface* skse) {
    InitializeLogging();

    auto* plugin = PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    log::info("{} {} is loading...", plugin->GetName(), version);

    Init(skse);
    InitializeMessaging();

    log::info("{} has finished loading.", plugin->GetName());
    return true;
}
