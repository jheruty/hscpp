#pragma once

#include <unordered_map>
#include <memory>
#include <filesystem>

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

        bool PerformRuntimeSwap(const std::filesystem::path& moduleFilepath);

    private:
        std::unordered_map<std::string, std::vector<ITracker*>> m_TrackersByKey;
        
        // The library user owns this memory.
        IAllocator* m_pAllocator = nullptr;
        void* m_pGlobalUserData = nullptr;

        std::unordered_map<std::string, IConstructor*> m_ConstructorsByKey;
    };

}