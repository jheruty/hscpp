#include <Windows.h>

#include "hscpp/ModuleManager.h"
#include "hscpp/ModuleInterface.h"
#include "hscpp/Log.h"
#include "hscpp/StringUtil.h"

namespace hscpp
{

    ModuleManager::ModuleManager()
    {
        Hscpp_GetModuleInterface()->SetTrackersByKey(&m_TrackersByKey);
        m_ConstructorsByKey = Hscpp_GetModuleInterface()->GetConstructorsByKey();
    }

    void ModuleManager::SetAllocator(std::unique_ptr<IAllocator> pAllocator)
    {
        m_pAllocator = std::move(pAllocator);
        Hscpp_GetModuleInterface()->SetAllocator(m_pAllocator.get());
    }

    hscpp::IAllocator* ModuleManager::GetAllocator()
    {
        return m_pAllocator.get();
    }

    void ModuleManager::SetGlobalUserData(void* pGlobalUserData)
    {
        m_pGlobalUserData = pGlobalUserData;
        Hscpp_GetModuleInterface()->SetGlobalUserData(m_pGlobalUserData);
    }

    void* ModuleManager::Allocate(const std::string& key, uint64_t id)
    {
        auto constructorIt = m_ConstructorsByKey.find(key);
        if (constructorIt != m_ConstructorsByKey.end())
        {
            return constructorIt->second->Construct(id);
        }

        return nullptr;
    }

    bool ModuleManager::PerformRuntimeSwap(const std::filesystem::path& moduleFilepath)
    {
        HMODULE hModule = LoadLibrary(moduleFilepath.native().c_str());
        if (hModule == nullptr)
        {
            Log::Write(LogLevel::Error, "%s: Failed to load module %s. [%s]\n",
                __func__, moduleFilepath.string().c_str(), GetLastErrorString().c_str());
            return false;
        }

        typedef ModuleInterface* (__cdecl* Hsccp_GetModuleInterfaceProc)(void);
        auto getModuleInterfaceProc = reinterpret_cast<Hsccp_GetModuleInterfaceProc>(
            GetProcAddress(hModule, "Hscpp_GetModuleInterface"));

        if (getModuleInterfaceProc == nullptr)
        {
            Log::Write(LogLevel::Error, "%s: Failed to load Hscpp_GetModuleInterface procedure. [%s]\n",
                __func__, GetLastErrorString().c_str());
            return false;
        }

        ModuleInterface* pModuleInterface = getModuleInterfaceProc();
        if (pModuleInterface == nullptr)
        {
            Log::Write(LogLevel::Error, "%s: Failed to get point to module interface.\n", __func__);
            return false;
        }

        pModuleInterface->SetGlobalUserData(m_pGlobalUserData);
        pModuleInterface->SetTrackersByKey(&m_TrackersByKey);
        pModuleInterface->SetAllocator(m_pAllocator.get());
        pModuleInterface->PerformRuntimeSwap();

        // Patch our current constructors to include the newly created constructors.
        auto newConstructorsByKey = pModuleInterface->GetConstructorsByKey();
        for (const auto& newConstructor : newConstructorsByKey)
        {
            m_ConstructorsByKey[newConstructor.first] = newConstructor.second;
        }

        return true;
    }

}