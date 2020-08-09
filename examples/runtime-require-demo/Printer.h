#pragma once

#include "hscpp/module/Tracker.h"
#include "BaseState.h"

// A tracked class can have a base class, as long as the base class is not also tracked.
class Printer : public BaseState
{
    HSCPP_TRACK(Printer, "Printer1")

public:
    Printer();
    hscpp_virtual void Update();
};