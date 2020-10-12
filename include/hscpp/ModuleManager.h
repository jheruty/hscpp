#pragma once

#include <unordered_map>
#include <memory>

#include "hscpp/Platform.h"
#include "hscpp/module/ITracker.h"
#include "hscpp/module/IAllocator.h"
#include "hscpp/module/Constructors.h"
#include "hscpp/module/ModuleInterface.h"

namespace hscpp
{

    class ModuleManager
    {
    public:

        ModuleManager();

        void SetAllocator(IAllocator* pAllocator);
        void SetGlobalUserData(void* pGlobalUserData);

        bool PerformRuntimeSwap(const fs::path& modulePath);

    private:
        bool m_bSwapping = false;
        std::unordered_map<std::string, std::vector<ITracker*>> m_TrackersByKey;
        
        // The library user owns this memory.
        IAllocator* m_pAllocator = nullptr;
        void* m_pGlobalUserData = nullptr;

        std::unordered_map<std::string, IConstructor*> m_ConstructorsByKey;

        void WarnDuplicateKeys(ModuleInterface* pModuleInterface);
    };

}