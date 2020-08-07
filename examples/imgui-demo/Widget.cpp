#include "Widget.h"
#include "imgui.h"
#include "Globals.h"

Widget::Widget()
{
    auto cb = [this](hscpp::SwapInfo& info) {
        switch (info.Phase())
        {
        case hscpp::SwapPhase::BeforeSwap:
            info.Serialize("Widgets", m_Widgets);
            info.Serialize("Title", m_Title);
            info.Serialize("InputBuffer", m_InputBuffer);
            break;
        case hscpp::SwapPhase::AfterSwap:
            info.Unserialize("Widgets", m_Widgets);
            info.Unserialize("Title", m_Title);
            info.Unserialize("InputBuffer", m_InputBuffer);
            break;
        }
    };

    HSCPP_SET_SWAP_HANDLER(cb);
}

void Widget::Init(const std::string& title)
{
    m_Title = title;
}

void Widget::Update()
{
    ImGui::SetCurrentContext(&Globals::ImGuiContext());

    if (m_Title.empty())
    {
        m_Title = "<Unnamed Window>";
    }

    ImGui::Begin(m_Title.c_str());

    ImGui::InputText("New widget title", m_InputBuffer.data(), m_InputBuffer.size());

    if (ImGui::Button("Create new widget"))
    {
        Ref<Widget> widget = Globals::MemoryManager()->Allocate<Widget>();
        widget->Init(m_InputBuffer.data());
        m_Widgets.push_back(widget);
    }

    ImGui::End();

    for (auto& widget : m_Widgets)
    {
        widget->Update();
    }
}
