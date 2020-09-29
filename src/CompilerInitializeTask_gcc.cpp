#include "hscpp/CompilerInitializeTask_gcc.h"

namespace hscpp
{

    void CompilerInitializeTask_gcc::Start(ICmdShell *pCmdShell, std::chrono::milliseconds timeout)
    {

    }

    ICmdShellTask::TaskState CompilerInitializeTask_gcc::Update()
    {
        return ICmdShellTask::TaskState::Timeout;
    }

}