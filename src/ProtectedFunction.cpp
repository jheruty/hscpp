#include <windows.h>

#include "hscpp/ProtectedFunction.h"

ProtectedFunction::Result ProtectedFunction::Call(const std::function<void()>& cb)
{
    if (!IsDebuggerPresent())
    {
        // No debugger attached, so there's no way to recover from this exception.
        cb();
        return Result::Success;
    }

    __try
    {
        cb();
        return Result::Success;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return Result::Exception;
    }
}
