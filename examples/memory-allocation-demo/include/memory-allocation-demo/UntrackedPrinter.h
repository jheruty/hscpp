#pragma once

#include "hscpp/module/Tracker.h"
#include "memory-allocation-demo/IUpdateable.h"

class UntrackedPrinter : public IUpdatable
{
    // No HSCCP_TRACK by default. This class can still be allocated using
    // swapper.GetAllocationResolver()->Allocate(), but changes to the class will not be
    // reflected at runtime.
    //
    // HSCPP_TRACK(UntrackedPrinter, "UntrackedPrinter");

public:
    UntrackedPrinter();
    ~UntrackedPrinter();

    // hscpp_virtual is just the 'virtual' keyword, so it can be safely used even in untracked
    // objects when hscpp is enabled.
    hscpp_virtual void Init(int value);
    virtual void Update() override;

private:
    int m_Value = 0;
};
