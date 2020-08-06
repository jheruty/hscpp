#pragma once

#include <vector>

#include "imgui.h"

#include "hscpp/Tracker.h"
#include "hscpp-example-utils/Ref.h"
#include "hscpp-example-utils/MemoryManager.h"

class Widget
{
    HSCPP_TRACK(Widget, "Widget");

public:
    Widget();

    hscpp_virtual void Init(const std::string& title, Ref<ImGuiContext> context, Ref<MemoryManager> memoryManager);
    hscpp_virtual void Update();

private:
    std::string m_Title;
    Ref<ImGuiContext> m_Context;
    Ref<MemoryManager> m_MemoryManager;

    std::array<char, 128> m_InputBuffer = {};
    std::vector<Ref<Widget>> m_Widgets;
};