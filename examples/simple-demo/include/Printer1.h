#pragma once

#include "hscpp/Tracker.h"
#include <stdlib.h>

class Printer1
{
    HSCPP_TRACK(Printer1, "Printer1");

public:
    Printer1();
    ~Printer1();

    virtual void Update();

    int m_Value = rand() % 100;
};