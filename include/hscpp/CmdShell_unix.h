#pragma once

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
        std::vector<std::string> m_TaskOutput;
    };

}