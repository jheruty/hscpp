#pragma once

#include <unordered_map>
#include <memory>
#include <string>

namespace hscpp
{

    struct ISerializedProperty
    {
        virtual ~ISerializedProperty() {};
    };

    template <typename T>
    struct SerializedProperty : public ISerializedProperty
    {
        T value = T();
    };

    class Serializer
    {
    public:
        Serializer() = default;
        Serializer(const Serializer& rhs) = delete;
        Serializer& operator=(const Serializer& rhs) = delete;

        template <typename T>
        void Serialize(const std::string& name, const T& val)
        {
           auto property = std::make_unique<SerializedProperty<T>>();
           property->value = val;

            m_Properties[name] = std::move(property);
        }

        template <typename T>
        bool Unserialize(const std::string& name, T& val)
        {
            auto propertyIt = m_Properties.find(name);
            if (propertyIt != m_Properties.end())
            {
                ISerializedProperty* pProperty = propertyIt->second.get();
                val = static_cast<SerializedProperty<T>*>(pProperty)->value;

                return true;
            }

            return false;
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<ISerializedProperty>> m_Properties;
    };

}