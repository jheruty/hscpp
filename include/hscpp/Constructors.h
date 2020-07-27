#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <typeindex>

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
        virtual void* Construct(uint64_t id) = 0;
    };

    //============================================================================
    // Constructor
    //============================================================================

    template <typename T>
    class Constructor : public IConstructor
    {
    public:
        virtual void* Construct(uint64_t id) override;
    };

    template <typename T>
    void* hscpp::Constructor<T>::Construct(uint64_t id)
    {
        if (ModuleSharedState::s_pAllocator == nullptr)
        {
            return new T();
        }
        else
        {
            uint64_t size = sizeof(std::aligned_storage<sizeof(T)>::type);
            uint8_t* pMem = ModuleSharedState::s_pAllocator->Allocate(size, id);
            T* pT = new (pMem) T;

            return pT;
        }
    }

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
            GetConstructors().push_back(std::make_unique<Constructor<T>>());
            size_t iConstructor = GetConstructors().size() - 1;

            GetConstructorsByKey()[key] = iConstructor;
        }

        static void* Create(const std::string& key, uint64_t id)
        {
            IConstructor* constructor = GetConstructor(key);
            if (constructor != nullptr)
            {
                return constructor->Construct(id);
            }

            return nullptr;
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

        // Avoid static initialization order issues by placing static variables within functions.
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

