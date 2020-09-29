#pragma once

#include "hscpp/ICmdShellTask.h"
#include "hscpp/Config.h"

namespace hscpp
{

    class CompilerInitializeTask_gcc : public ICmdShellTask
    {
    public:
        CompilerInitializeTask_gcc(const CompilerConfig& config);

        void Start(ICmdShell* pCmdShell, std::chrono::milliseconds timeout) override;
        TaskState Update() override;

    private:
        enum class CompilerTask
        {
            GetVersion,
        };

        ICmdShell* m_pCmdShell = nullptr;

        std::chrono::steady_clock::time_point m_StartTime;
        std::chrono::milliseconds m_Timeout = std::chrono::milliseconds(0);

        CompilerConfig m_Config;

        TaskState HandleTaskComplete(CompilerTask task);
        TaskState HandleGetVersionTaskComplete(const std::vector<std::string>& output);
    };

}