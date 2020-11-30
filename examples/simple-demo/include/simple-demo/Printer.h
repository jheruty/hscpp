#pragma once

#include <string>

#include "hscpp/module/Tracker.h"

class Printer
{
    // Register this object with hscpp's tracking system. The first argument must be equal to the
    // type of the enclosing class. The second argument must be a unique key, which will be used
    // by hscpp to identify the object's constructor. When HSCPP_DISABLE is defined, this macro
    // will evaluate to nothing.
    HSCPP_TRACK(Printer, "Printer");

public:
    hscpp_virtual ~Printer() = default;

    // A default constructor is required.
    Printer();

    void Init(const std::string& name, int index);

    // Functions must be virtual in order for a swapped object to call the new implementation.
    // The "hscpp_virtual" macro evaluates to "virtual" if HSCPP_DISABLE is not defined. If
    // HSCPP_DISABLE is defined, hscpp_virtual will evaluate to nothing.
    //
    // Note that the use of the "final" keyword may cause hot module reloading to break. This
    // is due to the compiler optimizing the call by devirtualizing it.
    hscpp_virtual void Update();

private:
    std::string m_Name;
    int m_Index = -1;
};