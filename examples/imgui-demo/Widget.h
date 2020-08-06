#pragma once

#include "hscpp/Tracker.h"

class Widget
{
    HSCPP_TRACK(Widget, "Widget");

public:
    Widget();
    hscpp_virtual void Update();
};