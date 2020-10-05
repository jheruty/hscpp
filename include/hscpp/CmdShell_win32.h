#pragma once

#include <Windows.h>
#include <string>
#include <array>
#include <vector>

#include "hscpp/ICmdShell.h"

namespace hscpp
{
    class CmdShell : public ICmdShell
    {
    public:
        ~CmdShell();

        bool CreateCmdProcess();

        void StartTask(const std::string& command, int taskId);
        void CancelTask() override;
        
        TaskState GetTaskState();
        TaskState Update(int& taskId);

        const std::vector<std::string>& PeekTaskOutput();

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