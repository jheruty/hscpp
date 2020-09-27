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

        template <typename U>
        static U* GetAs()
        {
            return static_cast<U*>(s_pData);
        }

    private:
        friend class ModuleInterface;

        static void* s_pData;
    };

}

