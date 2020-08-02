#pragma once

#include "hscpp/Tracker.h"
#include "IUpdateable.h"

class UntrackedPrinter : public IUpdatable
{
    // No HSCCP_TRACK by default. This class can still be allocated using
    // swapper.Allocate<UntrackedPrinter>(), but changes to the class will not be reflected during
    // runtime.
    // HSCPP_TRACK(UntrackedPrinter, "UntrackedPrinter");

public:
    UntrackedPrinter();
    ~UntrackedPrinter();
    
    hscpp_virtual void Init(int value); 
    virtual void Update() override;

private:
    int m_Value = 0;
};
