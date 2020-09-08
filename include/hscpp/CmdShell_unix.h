#pragma once

#include "hscpp/ICmdShell.h"

namespace hscpp
{

    class CmdShell : public ICmdShell
    {
    public:
        virtual bool CreateCmdProcess() override;

        virtual void StartTask(const std::string& command, int taskId) override;

        virtual TaskState GetTaskState() override;
        virtual TaskState Update(int& taskId) override;

        virtual const std::vector<std::string>& PeekTaskOutput() override;

    private:
        std::vector<std::string> m_TaskOutput;
    };

}