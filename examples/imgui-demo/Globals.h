#pragma once

#include "hscpp-example-utils/Ref.h"
#include "hscpp-example-utils/MemoryManager.h"
#include "hscpp/module/ModuleSharedState.h"

class Globals
{
public:
    static Globals& Instance()
    {
        static Globals* pGlobals = nullptr;
        if (pGlobals == nullptr)
        {
            pGlobals = new Globals();
        }

        return *pGlobals;
    }

    static void Init(Ref<::MemoryManager> memoryManager, Ref<::ImGuiContext> imguiContext)
    {
        Globals& instance = Instance();

        instance.m_MemoryManager = memoryManager;
        instance.m_ImGuiContext = imguiContext;
    }

    static Ref<::MemoryManager> MemoryManager()
    {
        return ResolveGlobals()->m_MemoryManager;
    }

    static Ref<::ImGuiContext> ImGuiContext()
    {
        return ResolveGlobals()->m_ImGuiContext;
    }

private:
    Ref<::MemoryManager> m_MemoryManager;
    Ref<::ImGuiContext> m_ImGuiContext;

    static Globals* ResolveGlobals()
    {
        if (hscpp::ModuleSharedState::s_pGlobalUserData == nullptr)
        {
            return &Globals::Instance();
        }
        else
        {
            return static_cast<Globals*>(hscpp::ModuleSharedState::s_pGlobalUserData);
        }
    }
};