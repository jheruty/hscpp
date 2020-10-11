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
            Cancelled,
            Error,
        };

        virtual ~ICmdShell() = default;

        virtual bool CreateCmdProcess() = 0;

        virtual void StartTask(const std::string& command, int taskId) = 0;
        virtual void CancelTask() = 0;
        virtual void Clear() = 0;

        virtual TaskState Update(int& taskId) = 0;

        virtual const std::vector<std::string>& PeekTaskOutput() = 0;
    };

}