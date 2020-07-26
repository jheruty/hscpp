#pragma once

#include <string>

namespace hscpp
{
    // Required to be in it's own file to avoid circular dependency with Tracker and ModuleInterface.
    class ITracker
    {
    public:
        virtual void FreeTrackedObject() = 0;
        virtual std::string GetKey() = 0;
    };
}