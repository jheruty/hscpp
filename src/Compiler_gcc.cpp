#include "hscpp/Compiler_gcc.h"
#include "hscpp/Platform.h"
#include "hscpp/Log.h"

namespace hscpp
{

    Compiler_gcc::Compiler_gcc()
    {
        m_pCmdShell = platform::CreateCmdShell();
        if (!m_pCmdShell->CreateCmdProcess())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to create command process." << log::End();
        }

        m_pCmdShell->StartTask("clang --version", 0);
    }

    bool Compiler_gcc::IsInitialized()
    {
        return false;
    }

    bool Compiler_gcc::StartBuild(const Input& info)
    {


        return false;
    }

    void Compiler_gcc::Update()
    {
        int taskId = 0;
        ICmdShell::TaskState state = m_pCmdShell->Update(taskId);
        if (state == ICmdShell::TaskState::Done)
        {
            auto& output = m_pCmdShell->PeekTaskOutput();
            int dummy = 0;
        }
    }

    bool Compiler_gcc::IsCompiling()
    {
        return false;
    }

    bool Compiler_gcc::HasCompiledModule()
    {
        return false;
    }

    fs::path Compiler_gcc::PopModule()
    {
        return fs::path();
    }


}