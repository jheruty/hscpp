#include "hscpp/Compiler_clang.h"
#include "hscpp/Platform.h"
#include "hscpp/Log.h"

namespace hscpp
{

    Compiler_clang::Compiler_clang()
    {
        m_pCmdShell = platform::CreateCmdShell();
        if (!m_pCmdShell->CreateCmdProcess())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to create command process." << log::End();
        }

        m_pCmdShell->StartTask("clang --version", 0);
    }

    bool Compiler_clang::IsInitialized()
    {
        return false;
    }

    bool Compiler_clang::StartBuild(const Input& info)
    {


        return false;
    }

    void Compiler_clang::Update()
    {
        int taskId = 0;
        ICmdShell::TaskState state = m_pCmdShell->Update(taskId);
        if (state == ICmdShell::TaskState::Done)
        {
            auto& output = m_pCmdShell->PeekTaskOutput();
            int dummy = 0;
        }
    }

    bool Compiler_clang::IsCompiling()
    {
        return false;
    }

    bool Compiler_clang::HasCompiledModule()
    {
        return false;
    }

    fs::path Compiler_clang::PopModule()
    {
        return fs::path();
    }


}