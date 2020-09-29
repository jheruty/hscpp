#pragma once

#include "hscpp/ICmdShellTask.h"

namespace hscpp
{

    class CompilerInitializeTask_gcc : public ICmdShellTask
    {
    public:
        void Start(ICmdShell* pCmdShell, std::chrono::milliseconds timeout) override;
        TaskState Update() override;
    };

}