#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <typeindex>
#include <functional>
#include <unordered_set>
#include <typeindex>
#include <vector>

#include "hscpp/module/IAllocator.h"
#include "hscpp/module/ModuleSharedState.h"

namespace hscpp
{
    //============================================================================
    // IConstructor
    //============================================================================

    class IConstructor
    {
    public:
        virtual ~IConstructor() = default;
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
        AllocationInfo Allocate() override
        {
            return Allocate([](uint64_t size) {
                return ModuleSharedState::s_pAllocator->Hscpp_Allocate(size);
                });
        }

        AllocationInfo AllocateSwap(uint64_t id) override
        {
            return Allocate([id](uint64_t size) {
                return ModuleSharedState::s_pAllocator->Hscpp_AllocateSwap(id, size);
                });
        }

    private:
        AllocationInfo Allocate(const std::function<AllocationInfo(uint64_t size)>& allocatorCb)
        {
            if (ModuleSharedState::s_pAllocator == nullptr)
            {
                AllocationInfo info;
                info.pMemory = reinterpret_cast<uint8_t*>(new T());
                return info;
            }
            else
            {
                uint64_t size = sizeof(typename std::aligned_storage<sizeof(T)>::type);
                AllocationInfo info = allocatorCb(size);
                new (info.pMemory) T;

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
        struct DuplicateKey
        {
            std::string key;
            std::string type;
        };

        template <typename T>
        static void RegisterConstructor(const std::string& key)
        {
            TypesByKey()[key].insert(std::type_index(typeid(T)));

            GetConstructorKeys().push_back(key);

            GetConstructors().push_back(std::unique_ptr<Constructor<T>>(new Constructor<T>()));
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

        static std::vector<DuplicateKey> GetDuplicateKeys()
        {
            std::vector<DuplicateKey> duplicates;

            for (const auto& key__types : TypesByKey())
            {
                if (key__types.second.size() > 1)
                {
                    for (const auto& type : key__types.second)
                    {
                        DuplicateKey duplicate;
                        duplicate.key = key__types.first;
                        duplicate.type = type.name();

                        duplicates.push_back(duplicate);
                    }
                }
            }

            return duplicates;
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

        static std::unordered_map<std::string, std::unordered_set<std::type_index>>& TypesByKey()
        {
            static std::unordered_map<std::string, std::unordered_set<std::type_index>> typeByKey;
            return typeByKey;
        }
    };

}

