#pragma once

#include <Windows.h>
#include <string>
#include <array>
#include <vector>

#include "hscpp/cmd-shell/ICmdShell.h"

namespace hscpp
{
    class CmdShell : public ICmdShell
    {
    public:
        ~CmdShell();

        bool CreateCmdProcess() override;

        void StartTask(const std::string& command, int taskId) override;
        void CancelTask() override;
        void Clear() override;

        TaskState Update(int& taskId) override;

        const std::vector<std::string>& PeekTaskOutput() override;

    private:
        HANDLE m_hProcess = INVALID_HANDLE_VALUE;
        HANDLE m_hStdoutRead = INVALID_HANDLE_VALUE;
        HANDLE m_hStdinWrite = INVALID_HANDLE_VALUE;

        std::array<char, 512> m_ReadBuffer = { 0 };
        std::string m_LeftoverCmdOutput;

        TaskState m_TaskState = TaskState::Idle;
        int m_TaskId = -1;
        std::vector<std::string> m_TaskOutput;

        bool SendCommand(const std::string& command);
        bool ReadOutputLine(std::string& output);
    };
}