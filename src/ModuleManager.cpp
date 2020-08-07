#include <Windows.h>

#include "hscpp/ModuleManager.h"
#include "hscpp/Log.h"
#include "hscpp/StringUtil.h"
#include "hscpp/module/ModuleInterface.h"

hscpp::ModuleManager::ModuleManager()
{
    Hscpp_GetModuleInterface()->SetTrackersByKey(&m_TrackersByKey);
    Hscpp_GetModuleInterface()->SetConstructorsByKey(&m_ConstructorsByKey);

    m_ConstructorsByKey = Hscpp_GetModuleInterface()->GetModuleConstructorsByKey();
}

void hscpp::ModuleManager::SetAllocator(IAllocator* pAllocator)
{
    m_pAllocator = pAllocator;
    Hscpp_GetModuleInterface()->SetAllocator(pAllocator);
}

void hscpp::ModuleManager::SetGlobalUserData(void* pGlobalUserData)
{
    m_pGlobalUserData = pGlobalUserData;
    Hscpp_GetModuleInterface()->SetGlobalUserData(m_pGlobalUserData);
}

bool hscpp::ModuleManager::PerformRuntimeSwap(const std::filesystem::path& moduleFilepath)
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

    pModuleInterface->SetTrackersByKey(&m_TrackersByKey);
    pModuleInterface->SetConstructorsByKey(&m_ConstructorsByKey);
    pModuleInterface->SetAllocator(m_pAllocator);
    pModuleInterface->SetGlobalUserData(m_pGlobalUserData);
    pModuleInterface->PerformRuntimeSwap();

    return true;
}
