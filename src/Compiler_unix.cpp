#include "hscpp/Compiler_unix.h"
#include "hscpp/Platform.h"
#include "hscpp/Log.h"

namespace hscpp
{

    Compiler::Compiler()
    {
        m_pCmdShell = platform::CreateCmdShell();
        if (!m_pCmdShell->CreateCmdProcess())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to create command process." << log::End();
        }

        m_pCmdShell->StartTask("clang --version", 0);
    }

    bool Compiler::StartBuild(const Input& info)
    {


        return false;
    }

    void Compiler::Update()
    {
        int taskId = 0;
        ICmdShell::TaskState state = m_pCmdShell->Update(taskId);
        if (state == ICmdShell::TaskState::Done)
        {
            auto& output = m_pCmdShell->PeekTaskOutput();
            int dummy = 0;
        }
    }

    bool Compiler::IsCompiling()
    {
        return false;
    }

    bool Compiler::HasCompiledModule()
    {
        return false;
    }

    fs::path Compiler::PopModule()
    {
        return fs::path();
    }


}