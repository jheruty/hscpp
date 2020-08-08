#include "Widget.h"
#include "imgui.h"
#include "Globals.h"

Widget::Widget()
{
    auto cb = [this](hscpp::SwapInfo& info) {
        // As an alternative to switching on info.Phase(), then Serializing and Unserializing, you
        // can use the "Save" function.
        info.Save("Widgets", m_Widgets);
        info.Save("Title", m_Title);
        info.Save("InputBuffer", m_InputBuffer);
    };

    HSCPP_SET_SWAP_HANDLER(cb);
}

Widget::~Widget()
{
    // Don't do this! hscpp will call the Widget's destructor on swap. Freeing an object using
    // HSCPP_TRACK here will result in a double-free.
    //for (auto& ref : m_Widgets)
    //{
    //    Globals::MemoryManager()->Free(ref);
    //}
}

void Widget::Init(const std::string& parent, const std::string& title)
{
    m_Parent = parent;
    m_Title = title;
}

void Widget::Free()
{
    for (auto& ref : m_Widgets)
    {
        Globals::MemoryManager()->Free(ref);
    }
}

void Widget::Update()
{
    // When a module is recompiled, ImGui's static context will be empty. Setting it every frame
    // ensures that the context remains set.
    ImGui::SetCurrentContext(&Globals::ImGuiContext());

    if (m_Title.empty())
    {
        // ImGui needs a unique title per window. For the demo, just use a random number which is
        // likely to be unique.
        m_Title = "Random Title " + std::to_string(rand() % RAND_MAX);
    }

    ImGui::Begin(m_Title.c_str());
    ImGui::Text("Change this text!");
    ImGui::InputText("New widget title", m_InputBuffer.data(), m_InputBuffer.size());

    if (ImGui::Button("Create new widget"))
    {
        // Since we are using Refs, it is safe to store a std::vector to more Refs directly in this
        // object. Create several widgets with different titles, then change the text to demonstrate.
        Ref<Widget> widget = Globals::MemoryManager()->Allocate<Widget>();
        widget->Init(m_Title, m_InputBuffer.data());
        m_Widgets.push_back(widget);
    }

    ImGui::End();

    for (auto& widget : m_Widgets)
    {
        widget->Update();
    }
}
