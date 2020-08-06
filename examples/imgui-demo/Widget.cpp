#include "Widget.h"
#include "imgui.h"

Widget::Widget()
{
    auto cb = [](hscpp::SwapInfo& info) {
        switch (info.Phase())
        {
        case hscpp::SwapPhase::BeforeSwap:
            info.Serialize("Context", ImGui::GetCurrentContext());
            break;
        case hscpp::SwapPhase::AfterSwap:
            ImGuiContext* pContext = nullptr;
            info.Unserialize("Context", pContext);
            ImGui::SetCurrentContext(pContext);
            break;
        }
    };

    HSCPP_SET_SWAP_HANDLER(cb);
}

void Widget::Update()
{
    ImGui::Begin("Hello");
    ImGui::SetWindowSize({ 100, 100 });
    ImGui::End();
}
