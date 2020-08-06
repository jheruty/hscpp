#include "Widget.h"
#include "imgui.h"

Widget::Widget()
{
    auto cb = [this](hscpp::SwapInfo& info) {
        switch (info.Phase())
        {
        case hscpp::SwapPhase::BeforeSwap:
            info.Serialize("Context", m_Context);
            info.Serialize("MemoryManager", m_MemoryManager);
            info.Serialize("Widgets", m_Widgets);
            info.Serialize("Title", m_Title);
            break;
        case hscpp::SwapPhase::AfterSwap:
            info.Unserialize("Context", m_Context);
            info.Unserialize("MemoryManager", m_MemoryManager);
            info.Unserialize("Widgets", m_Widgets);
            info.Unserialize("Title", m_Title);
            break;
        }
    };

    HSCPP_SET_SWAP_HANDLER(cb);
}

void Widget::Init(const std::string& title, Ref<ImGuiContext> context, Ref<MemoryManager> memoryManager)
{
    m_Title = title;
    m_Context = context;
    m_MemoryManager = memoryManager;
}

void Widget::Update()
{
    ImGui::SetCurrentContext(*m_Context);

    if (m_Title.empty())
    {
        m_Title = "<Unnamed Window>";
    }

    ImGui::Begin(m_Title.c_str());

    ImGui::InputText("New widget title", m_InputBuffer.data(), m_InputBuffer.size());

    if (ImGui::Button("Create new widget"))
    {
        Ref<Widget> widget = m_MemoryManager->Allocate<Widget>();
        widget->Init(m_InputBuffer.data(), m_Context, m_MemoryManager);
        m_Widgets.push_back(widget);
    }

    ImGui::End();

    for (auto& widget : m_Widgets)
    {
        widget->Update();
    }
}
