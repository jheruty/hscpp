#pragma once

#include <chrono>

#include "hscpp/ICmdShell.h"

namespace hscpp
{
    class ICmdShellTask
    {
    public:
        enum class TaskState
        {
            Running,
            Success,
            Failure,
            Timeout,
        };

        virtual ~ICmdShellTask() = default;

        virtual void Start(ICmdShell* pCmdShell, std::chrono::milliseconds timeout) = 0;
        virtual TaskState Update() = 0;
    };
}