#include <Windows.h>

#include "hscpp/ModuleManager.h"
#include "hscpp/Log.h"
#include "hscpp/Util.h"
#include "hscpp/module/ModuleInterface.h"

hscpp::ModuleManager::ModuleManager()
{
    Hscpp_GetModuleInterface()->SetIsSwapping(&m_bSwapping);
    Hscpp_GetModuleInterface()->SetTrackersByKey(&m_TrackersByKey);
    Hscpp_GetModuleInterface()->SetConstructorsByKey(&m_ConstructorsByKey);

    m_ConstructorsByKey = Hscpp_GetModuleInterface()->GetModuleConstructorsByKey();
    WarnDuplicateKeys(Hscpp_GetModuleInterface());
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

bool hscpp::ModuleManager::PerformRuntimeSwap(const fs::path& moduleFilepath)
{
    HMODULE hModule = LoadLibrary(moduleFilepath.native().c_str());
    if (hModule == nullptr)
    {
        Log::Error() << HSCPP_LOG_PREFIX << "Failed to load module "
            << moduleFilepath << ". " << LastErrorLog() << EndLog();
        return false;
    }

    typedef ModuleInterface* (__cdecl* Hsccp_GetModuleInterfaceProc)(void);
    auto getModuleInterfaceProc = reinterpret_cast<Hsccp_GetModuleInterfaceProc>(
        GetProcAddress(hModule, "Hscpp_GetModuleInterface"));

    if (getModuleInterfaceProc == nullptr)
    {
        Log::Error() << HSCPP_LOG_PREFIX << "Failed to load Hscpp_GetModuleInterface procedure. "
            << LastErrorLog() << EndLog();
        return false;
    }

    ModuleInterface* pModuleInterface = getModuleInterfaceProc();
    if (pModuleInterface == nullptr)
    {
        Log::Error() << HSCPP_LOG_PREFIX << "Failed to get pointer to module interface." << EndLog();
        return false;
    }

    pModuleInterface->SetIsSwapping(&m_bSwapping);
    pModuleInterface->SetTrackersByKey(&m_TrackersByKey);
    pModuleInterface->SetConstructorsByKey(&m_ConstructorsByKey);
    pModuleInterface->SetAllocator(m_pAllocator);
    pModuleInterface->SetGlobalUserData(m_pGlobalUserData);
    pModuleInterface->PerformRuntimeSwap();

    WarnDuplicateKeys(pModuleInterface);

    return true;
}

void hscpp::ModuleManager::WarnDuplicateKeys(ModuleInterface* pModuleInterface)
{
    auto duplicateKeys = pModuleInterface->GetDuplicateKeys();
    for (const auto& duplicate : duplicateKeys)
    {
        Log::Warning() << HSCPP_LOG_PREFIX << "Duplicate HSCPP_TRACK key detected (key="
            << duplicate.key << ", type=" << duplicate.type << EndLog(").");
    }
}
