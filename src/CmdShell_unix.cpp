#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <cstdio>
#include <assert.h>

#include "hscpp/CmdShell_unix.h"
#include "hscpp/Log.h"

namespace hscpp
{

    bool CmdShell::CreateCmdProcess()
    {
        // popen will be called on the start of a new task.
        return true;
    }

    void CmdShell::StartTask(const std::string& command, int taskId)
    {
        m_TaskId = taskId;
        m_TaskOutput.clear();
        m_LeftoverCmdOutput.clear();

        // Stop any running command.
        if (m_pFile != nullptr)
        {
            CloseFile();
        }

        // Start the command.
        m_pFile = popen(command.c_str(), "r");
        if (m_pFile == nullptr)
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to call popen with command '"
                << command << "'. " << log::LastOsError() << log::End();

            m_TaskState = TaskState::Error;
            return;
        }

        // Get FILE* fd.
        m_FileFd = fileno(m_pFile);
        if (m_FileFd == -1)
        {
            log::Error() << "Failed to get popen fd. " << log::LastOsError() << log::End();

            m_TaskState = TaskState::Error;
            return;
        }

        m_TaskState = TaskState::Running;
    }

    ICmdShell::TaskState CmdShell::GetTaskState()
    {
        return m_TaskState;
    }

    ICmdShell::TaskState CmdShell::Update(int& taskId)
    {
        taskId = m_TaskId;

        if (m_TaskState == TaskState::Error)
        {
            m_TaskState = TaskState::Idle;
            return TaskState::Error;
        }
        else if (m_TaskState == TaskState::Idle)
        {
            return TaskState::Idle;
        }

        // Read as many output lines as possible from the cmd subprocess.
        ReadResult result = ReadFromShell();
        while (result == ReadResult::SuccessfulRead)
        {
            result = ReadFromShell();
        }

        // Fill out m_TaskOutput.
        FillOutput();

        if (result == ReadResult::Error)
        {
            m_TaskState = TaskState::Idle;
            return TaskState::Error;
        }
        else if (result == ReadResult::Done)
        {
            m_TaskState = TaskState::Idle;
            return TaskState::Done;
        }

        return m_TaskState;
    }

    const std::vector<std::string>& CmdShell::PeekTaskOutput()
    {
        return m_TaskOutput;
    }

    CmdShell::ReadResult CmdShell::ReadFromShell()
    {
        // Only read from file if our leftover buffer does not contain a newline yet.
        const int nFds = 1;

        struct pollfd fds[nFds];
        fds->fd = m_FileFd;
        fds->events = POLLIN;

        int ret = poll(fds, nFds, 0);
        if (ret > 0)
        {
            for (int i = 0; i < nFds; ++i)
            {
                if (fds[i].events & POLLIN)
                {
                    ssize_t nBytesRead = read(fds[i].fd, m_ReadBuffer.data(), m_ReadBuffer.size());

                    if (nBytesRead > 0)
                    {
                        m_LeftoverCmdOutput += std::string(m_ReadBuffer.data(), nBytesRead);
                        return ReadResult::SuccessfulRead;
                    }
                    else if (nBytesRead == 0)
                    {
                        // read returns 0, signaling EOF and that command is done executing.
                        CloseFile();

                        // If output does not end with a newline, append one manually, so that
                        // FillOutput chunks output correctly.
                        if (!m_LeftoverCmdOutput.empty()
                            && m_LeftoverCmdOutput.at(m_LeftoverCmdOutput.size() - 1) != '\n')
                        {
                            m_LeftoverCmdOutput += "\n";
                        }

                        return ReadResult::Done;
                    }
                    else if (nBytesRead == -1)
                    {
                        log::Error() << HSCPP_LOG_PREFIX << "Failed to read file fd. "
                                     << log::LastOsError() << log::End();
                        return ReadResult::Error;
                    }
                }
            }
        }

        return ReadResult::NoData;
    }

    void CmdShell::FillOutput()
    {
        // Get string up to next newline; do not include newline in output.
        size_t iNewline = m_LeftoverCmdOutput.find("\n");
        while (iNewline != std::string::npos)
        {
            m_TaskOutput.push_back(m_LeftoverCmdOutput.substr(0, iNewline));
            m_LeftoverCmdOutput = m_LeftoverCmdOutput.substr(iNewline + 1);

            iNewline = m_LeftoverCmdOutput.find("\n");
        }
    }

    void CmdShell::CloseFile()
    {
        if (pclose(m_pFile) == -1)
        {
            log::Warning() << HSCPP_LOG_PREFIX << "Failed to call pclose. "
                << log::LastOsError() << log::End();
        }

        m_pFile = nullptr;
        m_FileFd = -1;
    }

}