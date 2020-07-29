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

        void SetAllocator(std::unique_ptr<IAllocator> pAllocator);
        void SetGlobalUserData(void* pGlobalUserData);

        void* Allocate(const std::string& key, uint64_t id);
        bool PerformRuntimeSwap(const std::filesystem::path& moduleFilepath);

    private:
        std::unordered_map<std::string, std::vector<ITracker*>> m_TrackersByKey;
        std::unique_ptr<IAllocator> m_pAllocator;
        void* m_pGlobalUserData = nullptr; // The library user owns this memory.

        std::unordered_map<std::string, IConstructor*> m_ConstructorsByKey;
    };

}