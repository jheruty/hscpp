#include <cassert>
#include <cctype>

#include "hscpp/compiler/CompilerInitializeTask_gcc.h"
#include "hscpp/Log.h"

namespace hscpp
{

    CompilerInitializeTask_gcc::CompilerInitializeTask_gcc(CompilerConfig* pConfig)
        : m_pConfig(pConfig)
    {}

    void CompilerInitializeTask_gcc::Start(ICmdShell *pCmdShell,
                                           std::chrono::milliseconds timeout,
                                           const std::function<void(Result)>& doneCb)
    {
        m_pCmdShell = pCmdShell;
        m_DoneCb = doneCb;

        m_StartTime = std::chrono::steady_clock::now();
        m_Timeout = timeout;

        std::string versionCmd = "\"" + m_pConfig->executable.u8string() + "\" --version";
        m_pCmdShell->StartTask(versionCmd, static_cast<int>(CompilerTask::GetVersion));
    }

    void CompilerInitializeTask_gcc::Update()
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
                    TriggerDoneCb(Result::Failure);
                    return;
                }

                break;
            }
            case ICmdShell::TaskState::Idle:
            case ICmdShell::TaskState::Cancelled:
                // Do nothing.
                break;
            case ICmdShell::TaskState::Done:
                HandleTaskComplete(static_cast<CompilerTask>(taskId));
                break;
            case ICmdShell::TaskState::Error:
                log::Error() << HSCPP_LOG_PREFIX
                    << "CmdShell task '" << taskId << "' resulted in error." << log::End();
                TriggerDoneCb(Result::Failure);
                return;
            default:
                assert(false);
                break;
        }
    }

    void CompilerInitializeTask_gcc::TriggerDoneCb(Result result)
    {
        if (m_DoneCb != nullptr)
        {
            m_DoneCb(result);
            m_pCmdShell->Clear();
        }

        m_DoneCb = nullptr;
    }

    void CompilerInitializeTask_gcc::HandleTaskComplete(CompilerTask task)
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
    }

    void CompilerInitializeTask_gcc::HandleGetVersionTaskComplete(const std::vector<std::string>& output)
    {
        if (output.empty())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to get version for compiler '"
                << m_pConfig->executable.u8string() << log::End("'.");

            TriggerDoneCb(Result::Failure);
            return;
        }

        // Very rudimentary verification; assume that a --version string will have at least
        // a letter, a number, and a period associated with it.
        bool bSawLetter = false;
        bool bSawNumber = false;
        bool bSawPeriod = false;
        bool bValidVersion = false;
        for (const auto& line : output)
        {
            for (const char c : line)
            {
                if (std::isdigit(c))
                {
                    bSawNumber = true;
                }

                if (std::isalpha(c))
                {
                    bSawLetter = true;
                }

                if (c == '.')
                {
                    bSawPeriod = true;
                }

                if (bSawLetter && bSawNumber && bSawPeriod)
                {
                    bValidVersion = true;
                    break;
                }
            }
        }

        if (!bValidVersion)
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to get version for compiler '"
                << m_pConfig->executable.u8string() << log::End("'.");

            TriggerDoneCb(Result::Failure);
            return;
        }

        // Since --version verification is not very robust, print out the discovered compiler.
        log::Info() << log::End(); // newline
        log::Info() << HSCPP_LOG_PREFIX << "Found compiler version:" << log::End();
        for (const auto& line : output)
        {
            log::Info() << "    " << line << log::End();
        }
        log::Info() << log::End();

        TriggerDoneCb(Result::Success);
    }

}