#pragma once

#include "hscpp/module/Tracker.h"
#include "runtime-require-demo/BaseState.h"

// A tracked class can have a base class, as long as the base class is not also tracked.
class Printer : public BaseState
{
    HSCPP_TRACK(Printer, "Printer1")

public:
    hscpp_virtual ~Printer() = default;
    Printer();

    hscpp_virtual void Update();
};
