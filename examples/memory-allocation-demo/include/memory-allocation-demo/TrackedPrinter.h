#pragma once

#include "hscpp/module/Tracker.h"
#include "memory-allocation-demo/IUpdateable.h"

class TrackedPrinter : public IUpdatable
{
    HSCPP_TRACK(TrackedPrinter, "TrackedPrinter");

public:
    TrackedPrinter();
    ~TrackedPrinter();

    hscpp_virtual void Init(int value);
    virtual void Update() override;

private:
    int m_Value = 0;
};
