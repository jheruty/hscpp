#pragma once

#include <iostream>

#include "hscpp/Tracker.h"

class TrackedPrinter
{
    HSCPP_TRACK(TrackedPrinter, "Printer");

public:
    TrackedPrinter();
    void Init(int value);

    hscpp_virtual void Update();

private:
    int m_Value = 0;
    int m_Value2 = 0;
};
