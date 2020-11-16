#pragma once

#include "hscpp/cmd-shell/ICmdShellTask.h"
#include "hscpp/Config.h"

namespace hscpp
{

    class CompilerInitializeTask_gcc : public ICmdShellTask
    {
    public:
        explicit CompilerInitializeTask_gcc(CompilerConfig* pConfig);

        void Start(ICmdShell* pCmdShell,
                   std::chrono::milliseconds timeout,
                   const std::function<void(Result)>& doneCb) override;
        void Update() override;

    private:
        enum class CompilerTask
        {
            GetVersion,
        };

        CompilerConfig* m_pConfig = nullptr;
        ICmdShell* m_pCmdShell = nullptr;
        std::function<void(Result)> m_DoneCb;

        std::chrono::steady_clock::time_point m_StartTime;
        std::chrono::milliseconds m_Timeout = std::chrono::milliseconds(0);

        void TriggerDoneCb(Result result);

        void HandleTaskComplete(CompilerTask task);
        void HandleGetVersionTaskComplete(const std::vector<std::string>& output);
    };

}