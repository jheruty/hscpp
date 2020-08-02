#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <typeindex>
#include <functional>

#include "hscpp/IAllocator.h"
#include "hscpp/ModuleSharedState.h"

namespace hscpp
{
    //============================================================================
    // IConstructor
    //============================================================================

    class IConstructor
    {
    public:
        virtual ~IConstructor() {};
        virtual AllocationInfo Allocate() = 0;
        virtual AllocationInfo AllocateSwap(uint64_t id) = 0;
    };

    //============================================================================
    // Constructor
    //============================================================================

    template <typename T>
    class Constructor : public IConstructor
    {
    public:
        virtual AllocationInfo Allocate() override
        {
            return Allocate([](uint64_t size) {
                return ModuleSharedState::s_pAllocator->Hscpp_Allocate(size);
                });
        }

        virtual AllocationInfo AllocateSwap(uint64_t id) override
        {
            return Allocate([id](uint64_t size) {
                return ModuleSharedState::s_pAllocator->Hscpp_AllocateSwap(id, size);
                });
        }

    private:
        AllocationInfo Allocate(const std::function<AllocationInfo(uint64_t size)> allocatorCb)
        {
            if (ModuleSharedState::s_pAllocator == nullptr)
            {
                AllocationInfo info;
                info.pMemory = reinterpret_cast<uint8_t*>(new T());
                return info;
            }
            else
            {
                uint64_t size = sizeof(std::aligned_storage<sizeof(T)>::type);
                AllocationInfo info = allocatorCb(size);
                T* pT = new (info.pMemory) T;

                return info;
            }
        }
    };

    //============================================================================
    // Constructors
    //============================================================================

    // All functions implemented inline, so that this is compiled into a hotswapped module simply
    // by including Registration.h
    class Constructors
    {
    public:
        template <typename T>
        static void RegisterConstructor(const std::string& key)
        {
            GetConstructorKeys().push_back(key);

            GetConstructors().push_back(std::make_unique<Constructor<T>>());
            size_t iConstructor = GetConstructors().size() - 1;

            GetConstructorsByKey()[key] = iConstructor;
        }

        static size_t GetNumberOfKeys()
        {
            return GetConstructorKeys().size();
        }

        static std::string GetKey(size_t iKey)
        {
            return GetConstructorKeys().at(iKey);
        }

        static IConstructor* GetConstructor(const std::string& key)
        {
            auto constructorIt = GetConstructorsByKey().find(key);
            if (constructorIt != GetConstructorsByKey().end())
            {
                return GetConstructors().at(constructorIt->second).get();
            }

            return nullptr;
        }

    private:
        // Avoid static initialization order issues by placing static variables within functions.
        static std::vector<std::string>& GetConstructorKeys()
        {
            static std::vector<std::string> keys;
            return keys;
        }

        static std::vector<std::unique_ptr<IConstructor>>& GetConstructors()
        {
            static std::vector<std::unique_ptr<hscpp::IConstructor>> constructors;
            return constructors;
        }

        static std::unordered_map<std::string, size_t>& GetConstructorsByKey()
        {
            static std::unordered_map<std::string, size_t> iConstructorByKey;
            return iConstructorByKey;
        }
    };

}

