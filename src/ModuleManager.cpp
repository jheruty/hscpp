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

bool hscpp::ModuleManager::PerformRuntimeSwap(const fs::path& modulePath)
{
    void* pModule = platform::LoadModule(modulePath);
    if (pModule == nullptr)
    {
        log::Error() << HSCPP_LOG_PREFIX << "Failed to load module "
             << modulePath << ". " << log::LastOsError() << log::End();
        return false;
    }

    auto GetModuleInterface = platform::GetModuleFunction<ModuleInterface*()>(
            pModule, "Hscpp_GetModuleInterface");
    if (GetModuleInterface == nullptr)
    {
        log::Error() << HSCPP_LOG_PREFIX << "Failed to load Hscpp_GetModuleInterface procedure. "
            << log::LastOsError() << log::End();
        return false;
    }

    ModuleInterface* pModuleInterface = GetModuleInterface();
    if (pModuleInterface == nullptr)
    {
        log::Error() << HSCPP_LOG_PREFIX << "Failed to get pointer to module interface." << log::End();
        return false;
    }

    pModuleInterface->SetIsSwapping(&m_bSwapping);
    pModuleInterface->SetTrackersByKey(&m_TrackersByKey);
    pModuleInterface->SetConstructorsByKey(&m_ConstructorsByKey);
    pModuleInterface->SetAllocator(m_pAllocator);
    pModuleInterface->SetGlobalUserData(m_pGlobalUserData);
    pModuleInterface->PerformRuntimeSwap();

    WarnDuplicateKeys(pModuleInterface);

    log::Build() << HSCPP_LOG_PREFIX << "Successfully performed runtime swap." << log::End();

    return true;
}

void hscpp::ModuleManager::WarnDuplicateKeys(ModuleInterface* pModuleInterface)
{
    auto duplicateKeys = pModuleInterface->GetDuplicateKeys();
    for (const auto& duplicate : duplicateKeys)
    {
        log::Warning() << HSCPP_LOG_PREFIX << "Duplicate HSCPP_TRACK key detected (key="
            << duplicate.key << ", type=" << duplicate.type << log::End(").");
    }
}
