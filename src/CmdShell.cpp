#include "hscpp/CmdShell.h"
#include "hscpp/Log.h"
#include "hscpp/StringUtil.h"

namespace hscpp
{

    // Unique key we can use to verify a task is done running.
    const static std::string TASK_COMPLETION_KEY = "__hscpp_task_complete(fbdd766e-fa9e-4b12-9304-c9e7af59f44c)__";

    CmdShell::~CmdShell()
    {
        if (m_hProcess != INVALID_HANDLE_VALUE)
        {
            if (!TerminateProcess(m_hProcess, EXIT_SUCCESS))
            {
                Log::Write(LogLevel::Error, "%s: Failed to terminate cmd process. [%s]\n",
                    __func__, GetLastErrorString().c_str());
            }
        }
    }

    bool CmdShell::CreateCmdProcess()
    {
        SECURITY_ATTRIBUTES securityAttrs;
        securityAttrs.nLength = sizeof(SECURITY_ATTRIBUTES);
        securityAttrs.lpSecurityDescriptor = nullptr; // Use default security descriptor.
        securityAttrs.bInheritHandle = true; // Allow child to use handles from parent process.

        HANDLE hStdoutWrite = INVALID_HANDLE_VALUE;
        if (!CreatePipe(&m_hStdoutRead, &hStdoutWrite, &securityAttrs, 0))
        {
            Log::Write(LogLevel::Error, "%s: Failed to create stdout pipe. [%s]\n",
                __func__, GetLastErrorString().c_str());
            return false;
        }

        HANDLE hStdinRead = INVALID_HANDLE_VALUE;
        if (!CreatePipe(&hStdinRead, &m_hStdinWrite, &securityAttrs, 0))
        {
            Log::Write(LogLevel::Error, "%s: Failed to create stdin pipe. [%s]\n",
                __func__, GetLastErrorString().c_str());
            return false;
        }

        PROCESS_INFORMATION processInfo;
        ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

        STARTUPINFO startupInfo;
        ZeroMemory(&startupInfo, sizeof(STARTUPINFO));

        startupInfo.cb = sizeof(STARTUPINFO);
        startupInfo.hStdError = hStdoutWrite;
        startupInfo.hStdOutput = hStdoutWrite;
        startupInfo.hStdInput = hStdinRead;
        startupInfo.dwFlags |= STARTF_USESTDHANDLES;

        std::wstring process = L"cmd";
        std::wstring options = L"";
        options += L"/q "; // Disable echo.
        options += L"/k "; // Carry out command specified by following string.
        options += L"@PROMPT $"; // Remove prompt (ex. C:\Path).

        std::wstring cmdLine = process + L" " + options;

        bool bSuccess = CreateProcess(
            NULL, // Application name.
            const_cast<wchar_t*>(cmdLine.c_str()), // Command line.
            NULL, // Process security attributes.
            NULL,
            true,
            0,
            NULL,
            NULL,
            &startupInfo,
            &processInfo);

        if (!bSuccess)
        {
            Log::Write(LogLevel::Error, "%s: Failed to create cmd process. [%s]\n",
                __func__, GetLastErrorString().c_str());
            return false;
        }

        m_hProcess = processInfo.hProcess;

        CloseHandle(processInfo.hThread);
        CloseHandle(hStdoutWrite);
        CloseHandle(hStdinRead);

        return true;
    }

    void CmdShell::StartTask(const std::string& command, int taskId)
    {
        bool bSuccess = false;

        bSuccess &= SendCommand(command);
        bSuccess &= SendCommand("echo " + TASK_COMPLETION_KEY);

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
    }

    CmdShell::TaskState CmdShell::GetTaskState()
    {
        return m_TaskState;
    }

    CmdShell::TaskState CmdShell::Update(int& taskId)
    {
        taskId = m_TaskId;

        if (m_TaskState == TaskState::Error)
        {
            m_TaskState = TaskState::Idle;
            return TaskState::Error;
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

        if (m_hProcess == INVALID_HANDLE_VALUE)
        {
            Log::Write(LogLevel::Error, "%s: Command process is not running.\n", __func__);
            return false;
        }

        int offset = 0;
        DWORD nBytesToWrite = static_cast<DWORD>(newlineCommand.size());
        const char* pStr = newlineCommand.c_str();

        while (nBytesToWrite > 0)
        {
            DWORD nBytesWritten = 0;
            if (!WriteFile(m_hStdinWrite, pStr + offset, nBytesToWrite, &nBytesWritten, NULL))
            {
                Log::Write(LogLevel::Error, "%s: Failed to write to cmd process. [%s]\n",
                    __func__, GetLastErrorString().c_str());
                return false;
            }

            offset += nBytesWritten;
            nBytesToWrite -= nBytesWritten;
        }

        return true;
    }

    bool CmdShell::ReadOutputLine(std::string& output)
    {
        output.clear();

        if (m_hProcess == INVALID_HANDLE_VALUE)
        {
            Log::Write(LogLevel::Error, "%s: Command process is not running.\n", __func__);
            return false;
        }

        // Only read from file if our leftover buffer does not contain a newline yet.
        size_t iNewline = m_LeftoverCmdOutput.find("\n");
        if (iNewline == std::string::npos)
        {
            // Check that we have data to read to avoid blocking on ReadFile.
            DWORD nBytesAvailable = 0;
            if (!PeekNamedPipe(m_hStdoutRead, NULL, 0, NULL, &nBytesAvailable, NULL))
            {
                Log::Write(LogLevel::Error, "%s: Failed to peek cmd pipe. [%s]\n",
                    __func__, GetLastErrorString().c_str());
                return false;
            }

            if (nBytesAvailable > 0)
            {
                DWORD nBytesRead = 0;
                if (!ReadFile(m_hStdoutRead, m_ReadBuffer.data(),
                    static_cast<DWORD>(m_ReadBuffer.size()), &nBytesRead, NULL))
                {
                    Log::Write(LogLevel::Error, "%s: Failed to read from cmd process. [%s]\n",
                        __func__, GetLastErrorString().c_str());
                    return false;
                }

                m_LeftoverCmdOutput += std::string(m_ReadBuffer.data(), nBytesRead);
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