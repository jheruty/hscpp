#include "hscpp/CmdShell.h"
#include "hscpp/Log.h"
#include "hscpp/StringUtil.h"

namespace hscpp
{

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

    bool CmdShell::SendCommand(const std::string& command)
    {
        // Terminate command with newline to simulate pressing 'Enter'.
        std::string newlineCommand = command + "\n";

        if (m_hProcess == INVALID_HANDLE_VALUE)
        {
            Log::Write(LogLevel::Error, "%s: Command process is not running.\n", __func__);
            return false;
        }

        int nBytesToWrite = newlineCommand.size();
        int offset = 0;
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
                if (!ReadFile(m_hStdoutRead, m_ReadBuffer.data(), m_ReadBuffer.size(), &nBytesRead, NULL))
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

        // Toss out the carriage return.
        if (output.size() >= 2 && output.at(output.size() - 2) == '\r')
        {
            output.erase(output.begin() + output.size() - 2);
        }

        return true;
    }

}