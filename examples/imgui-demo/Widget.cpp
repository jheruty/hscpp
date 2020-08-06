#include "Widget.h"
#include "imgui.h"

Widget::Widget()
{
    auto cb = [this](hscpp::SwapInfo& info) {
        switch (info.Phase())
        {
        case hscpp::SwapPhase::BeforeSwap:
            info.Serialize("Context", m_Context);
            break;
        case hscpp::SwapPhase::AfterSwap:
            info.Unserialize("Context", m_Context);
            break;
        }
    };

    HSCPP_SET_SWAP_HANDLER(cb);
}

void Widget::Init(Ref<ImGuiContext> context)
{
    m_Context = context;
}

void Widget::Update()
{
    ImGui::SetCurrentContext(*m_Context);

    ImGui::Begin("Hello");
    ImGui::SetWindowSize({ 100, 100 });
    int Dummy = 0;
    ++Dummy;
    ImGui::End();
}
