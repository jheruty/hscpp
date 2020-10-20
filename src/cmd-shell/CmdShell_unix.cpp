#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <signal.h>

#include <cstdio>
#include <cassert>

#include "hscpp/cmd-shell/CmdShell_unix.h"
#include "hscpp/Log.h"

namespace hscpp
{

    // Unique key we can use to verify a task is done running.
    const static std::string TASK_COMPLETION_KEY = "\"__hscpp_task_complete(fbdd766e-fa9e-4b12-9304-c9e7af59f44c)__\"";

    CmdShell::~CmdShell()
    {
        if (m_ShellPid != -1)
        {
            if (kill(m_ShellPid, SIGKILL) == -1)
            {
                log::Warning() << HSCPP_LOG_PREFIX << "Failed to terminate CmdShell process. "
                               << log::LastOsError() << log::End();
            }
        }
    }

    bool CmdShell::CreateCmdProcess()
    {
        if (pipe(m_ReadPipe) == -1)
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to create read pipe." << log::End();
            return false;
        }

        if (pipe(m_WritePipe) == -1)
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to create read pipe." << log::End();
            return false;
        }

        m_ShellPid = fork();
        if (m_ShellPid == -1)
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to fork subprocess." << log::End();
            return false;
        }

        if (m_ShellPid == 0)
        {
            // This is the child process. Link up the read/write pipes with stdin/stdout.
            dup2(m_WritePipe[0], STDIN_FILENO);
            dup2(m_ReadPipe[1], STDOUT_FILENO);
            dup2(m_ReadPipe[1], STDERR_FILENO);

            execl("/bin/sh", "sh", nullptr);

            // Should never get here.
            exit(1);
        }

        close(m_WritePipe[0]);
        close(m_ReadPipe[1]);

        return true;
    }

    void CmdShell::StartTask(const std::string& command, int taskId)
    {
        Clear();

        bool bSuccess = true;

        bSuccess &= SendCommand(command);

        // Single quote to avoid interpolation.
        // Prefix with newline to ensure it ends up on its own line.
        bSuccess &= SendCommand("echo '\n" + TASK_COMPLETION_KEY + "'");

        if (!bSuccess)
        {
            m_TaskState = TaskState::Error;
        }
        else
        {
            m_TaskState = TaskState::Running;
        }

        m_TaskId = taskId;
        m_TaskOutput.clear();
        m_LeftoverCmdOutput.clear();
    }

    void CmdShell::CancelTask()
    {
        m_TaskState = TaskState::Cancelled;
    }

    void CmdShell::Clear()
    {
        m_TaskId = -1;
        m_TaskOutput.clear();
        m_LeftoverCmdOutput.clear();

        m_TaskState = TaskState::Idle;
    }

    ICmdShell::TaskState CmdShell::Update(int& taskId)
    {
        taskId = m_TaskId;

        if (m_TaskState == TaskState::Error)
        {
            m_TaskState = TaskState::Idle;
            return TaskState::Error;
        }
        else if (m_TaskState == TaskState::Cancelled)
        {
            m_TaskState = TaskState::Idle;
            return TaskState::Cancelled;
        }
        else if (m_TaskState == TaskState::Idle)
        {
            return TaskState::Idle;
        }

        // Read as many output lines as possible from the cmd subprocess.
        bool bDoneReading = false;
        do
        {
            std::string line;
            if (!ReadOutputLine(line))
            {
                m_TaskState = TaskState::Idle;
                return TaskState::Error;
            }

            if (line.empty())
            {
                bDoneReading = true;
            }
            else
            {
                // Remove trailing /n.
                if (line.size() >= 1
                    && line.at(line.size() - 1) == '\n')
                {
                    line.pop_back();
                }

                m_TaskOutput.push_back(line);
            }
        } while (!bDoneReading);

        // Check if the completion key is in the output. If so, our second 'echo' command has run,
        // so we know the task is complete.
        int iCompletionKey = -1;
        for (size_t i = 0; i < m_TaskOutput.size(); ++i)
        {
            if (m_TaskOutput.at(i).find(TASK_COMPLETION_KEY) != std::string::npos)
            {
                iCompletionKey = static_cast<int>(i);
                break;
            }
        }

        if (iCompletionKey != -1)
        {
            // Remove completion key from task output.
            m_TaskOutput.resize(iCompletionKey);

            m_TaskState = TaskState::Idle;
            return TaskState::Done;
        }

        return m_TaskState;
    }

    const std::vector<std::string>& CmdShell::PeekTaskOutput()
    {
        return m_TaskOutput;
    }

    bool CmdShell::SendCommand(const std::string& command)
    {
        // Terminate command with newline to simulate pressing 'Enter'.
        std::string newlineCommand = command + "\n";

        if (m_ShellPid == -1)
        {
            log::Error() << HSCPP_LOG_PREFIX << "CmdShell process is not running." << log::End();
            return false;
        }

        int offset = 0;
        ssize_t nBytesToWrite = static_cast<ssize_t>(newlineCommand.size());
        const char* pStr = newlineCommand.c_str();

        while (nBytesToWrite > 0)
        {
            ssize_t nBytesWritten = write(m_WritePipe[1], pStr + offset, nBytesToWrite);
            if (nBytesWritten == -1)
            {
                log::Error() << HSCPP_LOG_PREFIX << "Failed to write to CmdShell process. "
                             << log::LastOsError() << log::End();
                return false;
            }

            offset += nBytesWritten;
            nBytesToWrite -= nBytesWritten;
        }

        return true;
    }

    bool CmdShell::ReadOutputLine(std::string& output)
    {
        // Only read from process if our leftover buffer does not contain a newline yet.
        size_t iNewline = m_LeftoverCmdOutput.find("\n");
        if (iNewline == std::string::npos)
        {
            const int nFds = 1;

            struct pollfd fds[nFds];
            fds->fd = m_ReadPipe[0];
            fds->events = POLLIN;

            int ret = poll(fds, nFds, 0);
            if (ret > 0)
            {
                for (int i = 0; i < nFds; ++i)
                {
                    ssize_t nBytesRead = read(fds[i].fd, m_ReadBuffer.data(), m_ReadBuffer.size());
                    if (nBytesRead == -1)
                    {
                        log::Error() << HSCPP_LOG_PREFIX << "Failed to read from CmdShell process. "
                                     << log::LastOsError() << log::End();
                        return false;
                    }

                    m_LeftoverCmdOutput += std::string(m_ReadBuffer.data(), nBytesRead);
                }
            }
        }

        // Get string up to next newline.
        iNewline = m_LeftoverCmdOutput.find("\n");
        if (iNewline != std::string::npos)
        {
            output = m_LeftoverCmdOutput.substr(0, iNewline + 1);
            m_LeftoverCmdOutput = m_LeftoverCmdOutput.substr(iNewline + 1);
        }

        // Success, note that no data may have been available, in which case output is empty.
        return true;
    }

}