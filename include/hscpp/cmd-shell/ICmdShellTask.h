#pragma once

#include <chrono>
#include <functional>

#include "hscpp/cmd-shell/ICmdShell.h"

namespace hscpp
{
    class ICmdShellTask
    {
    public:
        enum class Result
        {
            Success,
            Failure,
        };

        virtual ~ICmdShellTask() = default;

        virtual void Start(ICmdShell* pCmdShell,
                           std::chrono::milliseconds timeout,
                           const std::function<void(Result)>& doneCb) = 0;
        virtual void Update() = 0;
    };
}