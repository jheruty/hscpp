#pragma once

namespace hscpp
{
    class GlobalUserData
    {
    public:
        static bool IsNull()
        {
            return s_pData == nullptr;
        }

        template <typename T>
        static T* GetAs()
        {
            return static_cast<T*>(s_pData);
        }

    private:
        friend class ModuleInterface;

        inline static void* s_pData = nullptr;
    };
}

