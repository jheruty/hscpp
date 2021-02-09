#pragma once

#include <unordered_map>
#include <memory>
#include <string>

namespace hscpp
{

    struct ISerializedProperty
    {
        virtual ~ISerializedProperty() = default;
    };

    template <typename T>
    struct SerializedCopyProperty : public ISerializedProperty
    {
        T value = T();
    };

    template <typename T>
    struct SerializedMoveProperty : public ISerializedProperty
    {
        std::unique_ptr<T> value;
    };

    class Serializer
    {
    public:
        Serializer() = default;
        Serializer(const Serializer& rhs) = delete;
        Serializer& operator=(const Serializer& rhs) = delete;

        template <typename T>
        void SerializeCopy(const std::string& name, const T& val)
        {
           auto pProperty = std::unique_ptr<SerializedCopyProperty<T>>(
               new SerializedCopyProperty<T>());
           pProperty->value = val;

            m_Properties[name] = std::move(pProperty);
        }

        template <typename T>
        bool UnserializeCopy(const std::string& name, T& val)
        {
            auto propertyIt = m_Properties.find(name);
            if (propertyIt != m_Properties.end())
            {
                ISerializedProperty* pProperty = propertyIt->second.get();
                val = static_cast<SerializedCopyProperty<T>*>(pProperty)->value;

                return true;
            }

            return false;
        }

        template <typename T>
        void SerializeMove(const std::string& name, T&& val)
        {
            auto pProperty = std::unique_ptr<SerializedMoveProperty<T>>(
                new SerializedMoveProperty<T>());
            pProperty->value = std::unique_ptr<T>(new T(std::move(val)));

            m_Properties[name] = std::move(pProperty);
        }

        template <typename T>
        bool UnserializeMove(const std::string& name, T& val)
        {
            auto propertyIt = m_Properties.find(name);
            if (propertyIt != m_Properties.end())
            {
                ISerializedProperty* pProperty = propertyIt->second.get();
                val = std::move(*static_cast<SerializedMoveProperty<T>*>(pProperty)->value);

                return true;
            }

            return false;
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<ISerializedProperty>> m_Properties;
    };

}