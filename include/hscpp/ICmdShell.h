#pragma once

#include <vector>
#include <string>

namespace hscpp
{

    class ICmdShell
    {
    public:
        enum class TaskState
        {
            Idle,
            Running,
            Done,
            Error,
        };

        virtual ~ICmdShell() {};

        virtual bool CreateCmdProcess() = 0;

        virtual void StartTask(const std::string& command, int taskId) = 0;

        virtual TaskState GetTaskState() = 0;
        virtual TaskState Update(int& taskId) = 0;

        virtual const std::vector<std::string>& PeekTaskOutput() = 0;
    };

}