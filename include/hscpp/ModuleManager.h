#pragma once

#include <unordered_map>
#include <memory>
#include <filesystem>

#include "hscpp/ITracker.h"
#include "hscpp/IAllocator.h"
#include "hscpp/Constructors.h"

namespace hscpp
{

    class ModuleManager
    {
    public:
        ModuleManager();

        void SetAllocator(IAllocator* pAllocator);
        IAllocator* GetAllocator();

        void SetGlobalUserData(void* pGlobalUserData);

        AllocationInfo Allocate(const std::string& key);
        bool PerformRuntimeSwap(const std::filesystem::path& moduleFilepath);

    private:
        std::unordered_map<std::string, std::vector<ITracker*>> m_TrackersByKey;
        
        // The library user owns this memory.
        IAllocator* m_pAllocator = nullptr;
        void* m_pGlobalUserData = nullptr;

        std::unordered_map<std::string, IConstructor*> m_ConstructorsByKey;
    };

}