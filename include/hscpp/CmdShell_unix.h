#pragma once

#include <array>

#include "hscpp/ICmdShell.h"

namespace hscpp
{

    class CmdShell : public ICmdShell
    {
    public:
        bool CreateCmdProcess() override;

        void StartTask(const std::string& command, int taskId) override;

        TaskState GetTaskState() override;
        TaskState Update(int& taskId) override;

        const std::vector<std::string>& PeekTaskOutput() override;

    private:
        enum class ReadResult
        {
            SuccessfulRead,
            NoData,
            Error,
            Done,
        };

        std::array<char, 512> m_ReadBuffer = { 0 };
        std::string m_LeftoverCmdOutput;

        TaskState m_TaskState = TaskState::Idle;
        int m_TaskId = -1;
        std::vector<std::string> m_TaskOutput;

        FILE* m_pFile = nullptr;
        int m_FileFd = -1;

        ReadResult ReadFromShell();
        void FillOutput();

        void CloseFile();
    };

}