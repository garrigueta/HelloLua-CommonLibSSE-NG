#include <Core/LuaSKSEManager.h>
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
     * Initialize the Lua system.
     */
    void InitializeLua() {
        log::trace("Initializing Lua system...");
        if (auto* luaManager = Sample::LuaSKSEManager::GetSingleton()) {
            if (luaManager->Initialize()) {
                log::info("Lua system initialized successfully");
                
                // Execute startup script
                if (luaManager->ExecuteScript("startup.lua")) {
                    log::info("Successfully executed startup script");
                } else {
                    log::warn("Error executing startup script");
                }
            } else {
                log::error("Failed to initialize Lua system");
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
 * Entry point for the HelloLua plugin.
 * This plugin provides a way to run Lua scripts in Skyrim.
 */
SKSEPluginLoad(const LoadInterface* skse) {
    InitializeLogging();

    auto* plugin = PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    log::info("{} {} is loading...", plugin->GetName(), version);

    Init(skse);
    
    // Register serialization callbacks
    auto* serialization = GetSerializationInterface();
    if (serialization) {
        // Use a CRC32 hash of the plugin name as the ID since GetGUID is not available in this version
        const auto pluginName = plugin->GetName();
        const uint32_t pluginID = static_cast<uint32_t>(SKSE::HashUtil::CRC32(pluginName));
        serialization->SetUniqueID(pluginID);
        serialization->SetRevertCallback(Sample::LuaSKSEManager::OnRevert);
        serialization->SetSaveCallback(Sample::LuaSKSEManager::OnGameSaved);
        serialization->SetLoadCallback(Sample::LuaSKSEManager::OnGameLoaded);
    }

    InitializeMessaging();

    log::info("{} has finished loading.", plugin->GetName());
    return true;
}
