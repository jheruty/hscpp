#pragma once

#include <Windows.h>
#include <string>
#include <array>

namespace hscpp
{
    class CmdShell
    {
    public:
        ~CmdShell();

        bool CreateCmdProcess();

        bool SendCommand(const std::string& command);
        bool ReadOutputLine(std::string& output);

    private:
        HANDLE m_hProcess = INVALID_HANDLE_VALUE;
        HANDLE m_hStdoutRead = INVALID_HANDLE_VALUE;
        HANDLE m_hStdinWrite = INVALID_HANDLE_VALUE;

        std::array<char, 512> m_ReadBuffer = { 0 };
        std::string m_LeftoverCmdOutput;
    };
}