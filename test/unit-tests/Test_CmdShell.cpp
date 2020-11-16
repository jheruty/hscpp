#include <thread>

#include "catch/catch.hpp"
#include "common/Common.h"

#include "hscpp/cmd-shell/ICmdShell.h"
#include "hscpp/Platform.h"
#include "hscpp/Util.h"

namespace hscpp { namespace test
{

    const static fs::path TEST_FILES_PATH = util::GetHscppTestPath() / "unit-tests" / "files" / "test-cmd-shell";

    static void RemoveBlankLines(std::vector<std::string>& vec)
    {
        for (auto it = vec.begin(); it != vec.end();)
        {
            if (it->empty())
            {
                it = vec.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    static void WaitForCmdDone(ICmdShell* pCmdShell, int expectedTaskId)
    {
        auto cb = [&](Milliseconds){
            int taskId = 0;
            ICmdShell::TaskState taskState = pCmdShell->Update(taskId);
            REQUIRE(taskId == expectedTaskId);

            if (taskState == ICmdShell::TaskState::Done)
            {
                return UpdateLoop::Done;
            }

            return UpdateLoop::Running;
        };

        CALL(StartUpdateLoop, Milliseconds(5000), Milliseconds(10), cb);
    }

    TEST_CASE("CmdShell can perform a basic echo.")
    {
        std::unique_ptr<ICmdShell> pCmdShell = platform::CreateCmdShell();
        REQUIRE(pCmdShell->CreateCmdProcess());

        int taskId = 0;
        pCmdShell->StartTask("echo hello", taskId);

        CALL(WaitForCmdDone, pCmdShell.get(), taskId);

        std::vector<std::string> output = pCmdShell->PeekTaskOutput();
        RemoveBlankLines(output);

        REQUIRE(output.size() == 1);
        REQUIRE(output.at(0) == "hello");
    }

    TEST_CASE("CmdShell can cat file in test directory that doesn't end with a newline.")
    {
        fs::path assetsPath = TEST_FILES_PATH / "cat-test";
        fs::path catFilePath = assetsPath / "CatFile.txt";
        fs::path sandboxPath = CALL(InitializeSandbox, assetsPath);

        std::unique_ptr<ICmdShell> pCmdShell = platform::CreateCmdShell();
        REQUIRE(pCmdShell->CreateCmdProcess());

        std::string catExecutable;

        RunWin32([&](){ catExecutable = "type"; });
        RunUnix([&](){ catExecutable = "cat"; });

        std::string cmd = catExecutable + " \"" + catFilePath.u8string() + "\"";
        int taskId = 0;
        pCmdShell->StartTask(cmd, taskId);

        CALL(WaitForCmdDone, pCmdShell.get(), taskId);

        std::vector<std::string> output = pCmdShell->PeekTaskOutput();
        RemoveBlankLines(output);

        REQUIRE(output.size() == 1);
        REQUIRE(output.at(0) == "Hello, CmdShell!");
    }

    TEST_CASE("CmdShell variables are persistent.")
    {
        std::unique_ptr<ICmdShell> pCmdShell = platform::CreateCmdShell();
        REQUIRE(pCmdShell->CreateCmdProcess());

        // Validate that echoing variable is empty.
        int taskId = 28;

        RunWin32([&](){ pCmdShell->StartTask("echo %HSCPP_VAR%", taskId); });
        RunUnix([&](){ pCmdShell->StartTask("echo $HSCPP_VAR", taskId); });

        CALL(WaitForCmdDone, pCmdShell.get(), taskId);

        std::vector<std::string> output = pCmdShell->PeekTaskOutput();
        RemoveBlankLines(output);

        RunWin32([&](){
            REQUIRE(output.size() == 1);
            REQUIRE(output.at(0) == "%HSCPP_VAR%");
        });
        RunUnix([&](){ REQUIRE(output.empty()); });

        RunWin32([&](){ pCmdShell->StartTask("set HSCPP_VAR=HscppVar", taskId); });
        RunUnix([&](){ pCmdShell->StartTask("HSCPP_VAR=HscppVar", taskId); });

        CALL(WaitForCmdDone, pCmdShell.get(), taskId);

        output = pCmdShell->PeekTaskOutput();
        RemoveBlankLines(output);

        REQUIRE(output.empty());

        RunWin32([&](){ pCmdShell->StartTask("echo %HSCPP_VAR%", taskId); });
        RunUnix([&](){ pCmdShell->StartTask("echo $HSCPP_VAR", taskId); });

        CALL(WaitForCmdDone, pCmdShell.get(), taskId);

        output = pCmdShell->PeekTaskOutput();
        RemoveBlankLines(output);

        REQUIRE(output.size() == 1);
        REQUIRE(output.at(0) == "HscppVar");
    }

    TEST_CASE("CmdShell task can be cancelled.")
    {
        std::unique_ptr<ICmdShell> pCmdShell = platform::CreateCmdShell();
        REQUIRE(pCmdShell->CreateCmdProcess());

        int taskId = 2987;
        pCmdShell->StartTask("command_that_will_not_succeed 123454321", taskId);

        ICmdShell::TaskState taskState = ICmdShell::TaskState::Idle;

        auto cb = [&](Milliseconds timeElapsed){
            taskState = pCmdShell->Update(taskId);
            if (taskState == ICmdShell::TaskState::Cancelled)
            {
                return UpdateLoop::Done;
            }

            if (timeElapsed > Milliseconds(30))
            {
                pCmdShell->CancelTask();
            }

            return UpdateLoop::Running;
        };

        CALL(StartUpdateLoop, Milliseconds(5000), Milliseconds(10), cb);

        REQUIRE(taskState == ICmdShell::TaskState::Cancelled);
    }

}}