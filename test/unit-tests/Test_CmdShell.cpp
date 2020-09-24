#include "catch/catch.hpp"
#include "common/Common.h"

#include "hscpp/ICmdShell.h"
#include "hscpp/Platform.h"

namespace hscpp { namespace test
{

    TEST_CASE("CmdShell can perform a basic echo.", "[CmdShell]")
    {
        std::unique_ptr<ICmdShell> pCmdShell = platform::CreateCmdShell();

        REQUIRE(pCmdShell->CreateCmdProcess());

        int taskId = 0;
        pCmdShell->StartTask("echo hello", taskId);

        auto cb = [&](Milliseconds timeElapsed){
            ICmdShell::TaskState taskState = pCmdShell->Update(taskId);
            REQUIRE(taskId == 0);

            if (taskState == ICmdShell::TaskState::Done)
            {
                const std::vector<std::string>& output = pCmdShell->PeekTaskOutput();
                REQUIRE(output.size() == 1);
                REQUIRE(output.at(0) == "hello");

                return UpdateLoop::Done;
            }

            return UpdateLoop::Running;
        };

        CALL(StartUpdateLoop, Milliseconds(5000), Milliseconds(10), cb);
    }

}}