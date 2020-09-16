#pragma once

namespace hscpp
{
    // Templated to allow static variables in header.
    template <int T = 0>
    class GlobalUserDataImpl
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

        static void* s_pData;
    };

    template <int T>
    void* GlobalUserDataImpl<T>::s_pData = nullptr;

    // Typedef for easier usage; in the future, C++17 inline statics makes the templating of this
    // class unnecessary.
    typedef GlobalUserDataImpl<> GlobalUserData;

}

