#pragma once

#include "hscpp/ICmdShellTask.h"
#include "hscpp/Config.h"

namespace hscpp
{

    class CompilerInitializeTask_msvc : public ICmdShellTask
    {
    public:
        void Start(ICmdShell* pCmdShell, std::chrono::milliseconds timeout) override;
        TaskState Update() override;

    private:
        enum class CompilerTask
        {
            GetVsPath,
            SetVcVarsAll,
        };

        ICmdShell* m_pCmdShell = nullptr;

        std::chrono::steady_clock::time_point m_StartTime;
        std::chrono::milliseconds m_Timeout = std::chrono::milliseconds(0);

        void StartVsPathTask();
        TaskState HandleTaskComplete(CompilerTask task);
        TaskState HandleGetVsPathTaskComplete(const std::vector<std::string>& output);
        TaskState HandleSetVcVarsAllTaskComplete(std::vector<std::string> output);
    };

}