#pragma once

#include <functional>

#include "common/Typedefs.h"

namespace hscpp { namespace test
{
    enum class UpdateLoop
    {
        Running,
        Done,
        Timeout,
    };

    void StartUpdateLoop(Milliseconds timeoutMs, Milliseconds pollMs,
                         const std::function<UpdateLoop(Milliseconds elapsedMs)>& cb);

}}