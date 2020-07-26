#pragma once

#include <string.h>

class ITracker
{
public:
    virtual void FreeTrackedObject() = 0;
    virtual std::string GetKey() = 0;
};
