#pragma once

#include "hscpp/module/Tracker.h"

class Printer
{
    HSCPP_TRACK(Printer, "Printer");

public:
    enum class UpdateResult
    {
        Running,
        Done,
    };

    hscpp_virtual ~Printer() = default;
    Printer();

    hscpp_virtual UpdateResult Update();
    int m_Count = 0;
};