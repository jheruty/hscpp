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

        // Temporary hack! Put in header to allow MemoryManager example to work.
        AllocationInfo Allocate(const std::string& key)
        {
            auto constructorIt = m_ConstructorsByKey.find(key);
            if (constructorIt != m_ConstructorsByKey.end())
            {
                return constructorIt->second->Allocate();
            }

            return AllocationInfo();
        }

        bool PerformRuntimeSwap(const std::filesystem::path& moduleFilepath);

    private:
        std::unordered_map<std::string, std::vector<ITracker*>> m_TrackersByKey;
        
        // The library user owns this memory.
        IAllocator* m_pAllocator = nullptr;
        void* m_pGlobalUserData = nullptr;

        std::unordered_map<std::string, IConstructor*> m_ConstructorsByKey;
    };

}