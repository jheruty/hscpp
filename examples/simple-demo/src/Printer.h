#pragma once

#include <string>

#include "hscpp/Tracker.h"

class Printer
{
    // Register this object with HSCPP's tracking system. The type must be equal to the
    // type of the enclosing class, and the key must be unique per tracked object. When
    // HSCPP_DISABLE is defined, this macro evaluates to nothing.
    HSCPP_TRACK(Printer, "Printer");

public:
    // A default constructor is required. 
    Printer();
    Printer(const std::string& name, int index);

    // Functions must be virtual in order for a swapped object to call the new implementation.
    // The "hscpp_func" macro evaluates to "virtual" if HSCPP_DISABLE is not defined. If
    // HSCPP_DISABLE is defined, hscpp_func will evaluate to nothing. Similarly, hscpp_final
    // will do the same thing for "final".
    hscpp_virtual void Update();

private:
    std::string m_Name;
    int m_Index = -1;
};