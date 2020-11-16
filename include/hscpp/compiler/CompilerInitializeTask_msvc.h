#pragma once

#include "hscpp/cmd-shell/ICmdShellTask.h"
#include "hscpp/Config.h"

namespace hscpp
{

    class CompilerInitializeTask_msvc : public ICmdShellTask
    {
    public:
        void Start(ICmdShell* pCmdShell,
                   std::chrono::milliseconds timeout,
                   const std::function<void(Result)>& doneCb) override;
        void Update() override;

    private:
        enum class CompilerTask
        {
            GetVsPath,
            SetVcVarsAll,
        };

        ICmdShell* m_pCmdShell = nullptr;
        std::function<void(Result)> m_DoneCb;

        std::chrono::steady_clock::time_point m_StartTime;
        std::chrono::milliseconds m_Timeout = std::chrono::milliseconds(0);

        void TriggerDoneCb(Result result);

        void StartVsPathTask();
		bool StartVcVarsAllTask(const fs::path& vsPath, const fs::path& vcVarsAllDirectoryPath);

        void HandleTaskComplete(CompilerTask task);
        void HandleGetVsPathTaskComplete(const std::vector<std::string>& output);
        void HandleSetVcVarsAllTaskComplete(std::vector<std::string> output);
    };

}