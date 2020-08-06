#pragma once

#include "imgui.h"

#include "hscpp/Tracker.h"
#include "hscpp-example-utils/Ref.h"

class Widget
{
    HSCPP_TRACK(Widget, "Widget");

public:
    Widget();

    hscpp_virtual void Init(Ref<ImGuiContext> context);
    hscpp_virtual void Update();

private:
    Ref<ImGuiContext> m_Context;
};