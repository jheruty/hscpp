#include "hscpp/CmdShell_unix.h"

namespace hscpp
{

    bool CmdShell::CreateCmdProcess()
    {
        return false;
    }

    void CmdShell::StartTask(const std::string& command, int taskId)
    {

    }

    ICmdShell::TaskState CmdShell::GetTaskState()
    {
        return ICmdShell::TaskState::Error;
    }

    ICmdShell::TaskState CmdShell::Update(int& taskId)
    {
        return ICmdShell::TaskState::Error;
    }

    const std::vector<std::string>& CmdShell::PeekTaskOutput()
    {
        return m_TaskOutput;
    }


}