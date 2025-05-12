#include "Core/PCH.h"
#include "Core/LuaNativeFunctions.h"
#include <SKSE/SKSE.h>

namespace Sample {

    LuaNativeFunctions* LuaNativeFunctions::GetSingleton() {
        static LuaNativeFunctions instance;
        return &instance;
    }

    void LuaNativeFunctions::RegisterNativeFunction(const std::string& name,
                                                   const std::string& category,
                                                   const std::string& description,
                                                   LuaCFunction func) {
        if (IsFunctionRegistered(name)) {
            SKSE::log::warn("Native function '{}' is already registered. Overwriting.", name);
        }

        NativeFunctionInfo info;
        info.name = name;
        info.category = category;
        info.description = description;
        info.function = func;

        m_registeredFunctions[name] = info;
        SKSE::log::info("Registered native function '{}' in category '{}'", name, category);
    }

    std::vector<NativeFunctionInfo> LuaNativeFunctions::GetFunctionsByCategory(const std::string& category) const {
        std::vector<NativeFunctionInfo> result;
        
        for (const auto& [name, info] : m_registeredFunctions) {
            if (info.category == category) {
                result.push_back(info);
            }
        }
        
        return result;
    }

    const std::unordered_map<std::string, NativeFunctionInfo>& LuaNativeFunctions::GetAllRegisteredFunctions() const {
        return m_registeredFunctions;
    }

    LuaCFunction LuaNativeFunctions::GetFunctionByName(const std::string& name) const {
        auto it = m_registeredFunctions.find(name);
        if (it != m_registeredFunctions.end()) {
            return it->second.function;
        }
        return nullptr;
    }

    bool LuaNativeFunctions::IsFunctionRegistered(const std::string& name) const {
        return m_registeredFunctions.find(name) != m_registeredFunctions.end();
    }

}
