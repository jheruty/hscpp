#pragma once

#include <string>

#include "hscpp/module/SwapInfo.h"

namespace hscpp
{
    // Required to be in it's own file to avoid circular dependency with Tracker and ModuleInterface.
    class ITracker
    {
    public:
        virtual uint64_t FreeTrackedObject() = 0;
        virtual std::string GetKey() = 0;
        virtual void CallSwapHandler(SwapInfo& info) = 0;
    };
}