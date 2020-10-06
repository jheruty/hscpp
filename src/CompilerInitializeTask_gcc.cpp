#include <cassert>

#include "hscpp/CompilerInitializeTask_gcc.h"
#include "hscpp/Log.h"

namespace hscpp
{

    CompilerInitializeTask_gcc::CompilerInitializeTask_gcc(const CompilerConfig& config)
        : m_Config(config)
    {}

    void CompilerInitializeTask_gcc::Start(ICmdShell *pCmdShell, std::chrono::milliseconds timeout)
    {
        m_pCmdShell = pCmdShell;

        m_StartTime = std::chrono::steady_clock::now();
        m_Timeout = timeout;

        std::string versionCmd = "\"" + m_Config.executable.u8string() + "\" --version";
        m_pCmdShell->StartTask(versionCmd, static_cast<int>(CompilerTask::GetVersion));
    }

    ICmdShellTask::TaskState CompilerInitializeTask_gcc::Update()
    {
        int taskId = 0;
        ICmdShell::TaskState state = m_pCmdShell->Update(taskId);

        switch (state)
        {
            case ICmdShell::TaskState::Running:
            {
                auto now = std::chrono::steady_clock::now();
                if (now - m_StartTime > m_Timeout)
                {
                    m_pCmdShell->CancelTask();
                    return ICmdShellTask::TaskState::Timeout;
                }

                break;
            }
            case ICmdShell::TaskState::Idle:
            case ICmdShell::TaskState::Cancelled:
                // Do nothing.
                break;
            case ICmdShell::TaskState::Done:
                return HandleTaskComplete(static_cast<CompilerTask>(taskId));
            case ICmdShell::TaskState::Error:
                log::Error() << HSCPP_LOG_PREFIX << "CmdShell task '"
                             << taskId << "' resulted in error." << log::End();
                return ICmdShellTask::TaskState::Failure;
            default:
                assert(false);
                break;
        }

        return ICmdShellTask::TaskState::Running;
    }

    ICmdShellTask::TaskState CompilerInitializeTask_gcc::HandleTaskComplete(CompilerTask task)
    {
        const std::vector<std::string>& output = m_pCmdShell->PeekTaskOutput();

        switch (task)
        {
            case CompilerTask::GetVersion:
                return HandleGetVersionTaskComplete(output);
            default:
                assert(false);
                break;
        }

        return ICmdShellTask::TaskState::Failure;
    }

    ICmdShellTask::TaskState CompilerInitializeTask_gcc::HandleGetVersionTaskComplete(
            const std::vector<std::string> &output)
    {
        if (output.empty())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to get version for compiler '"
                 << m_Config.executable.u8string() << log::End("'.");
            return ICmdShellTask::TaskState::Failure;
        }

        log::Info() << log::End();
        log::Info() << HSCPP_LOG_PREFIX << "Found compiler version:" << log::End();
        for (const auto& line : output)
        {
            log::Info() << "    " << line << log::End();
        }
        log::Info() << log::End();

        return ICmdShellTask::TaskState::Success;
    }

}