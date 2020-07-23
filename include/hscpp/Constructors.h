#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <typeindex>

#include "Swappable.h"

namespace hscpp
{
    //============================================================================
    // IConstructor
    //============================================================================

    class IConstructor
    {
    public:
        virtual ~IConstructor() {};
        virtual ISwappable* Construct() = 0;
    };

    //============================================================================
    // Constructor
    //============================================================================

    template <typename T>
    class Constructor : public IConstructor
    {
    public:
        virtual ISwappable* Construct() override;
    };

    template <typename T>
    ISwappable* hscpp::Constructor<T>::Construct()
    {
        return new T();
    }

    //============================================================================
    // Constructors
    //============================================================================

    class Constructors
    {
    public:
        template <typename T>
        static void RegisterConstructor(const std::string& key);

        static ISwappable* Create(const std::string& key);

    private:
        // Avoid static initialization order issues by placing static variables within functions.
        static std::vector<std::unique_ptr<IConstructor>>& GetConstructors();
        static std::unordered_map<std::string, size_t>& GetConstructorsByKey();
    };

    template <typename T>
    void hscpp::Constructors::RegisterConstructor(const std::string& key)
    {
        GetConstructors().push_back(std::make_unique<Constructor<T>>());
        size_t iConstructor = GetConstructors().size() - 1;

        Swappable<T>::s_Key = key;
        GetConstructorsByKey()[key] = iConstructor;
    }

}

