#include "hscpp/ProtectedFunction.h"

#if defined(HSCPP_PLATFORM_WIN32)

#include <windows.h>

#endif

ProtectedFunction::Result ProtectedFunction::Call(const std::function<void()>& cb)
{
#if defined(HSCPP_PLATFORM_WIN32)
    
    __try
    {
        cb();
        return Result::Success;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return Result::Exception;
    }

#elif defined(HSCPP_PLATFORM_UNIX)

    try
    {
        cb();
        return Result::Success;
    }
    catch(const std::exception& e)
    {
        return Result::Exception;
    }
    
#endif

}
