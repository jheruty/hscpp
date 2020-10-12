#pragma once

#include <vector>
#include <array>

#include "imgui.h"

#include "hscpp/module/Tracker.h"
#include "hscpp-example-utils/Ref.h"
#include "hscpp-example-utils/MemoryManager.h"

class Widget
{
    HSCPP_TRACK(Widget, "Widget");

public:
    Widget();
    hscpp_virtual ~Widget();

    hscpp_virtual void Init(const std::string& parent, const std::string& title);
    hscpp_virtual void Update();

private:
    std::string m_Parent;
    std::string m_Title;

    std::array<char, 128> m_InputBuffer = {};
    std::vector<Ref<Widget>> m_Widgets;
};