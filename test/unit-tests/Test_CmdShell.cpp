#include "catch/catch.hpp"
#include "common/Common.h"

#include "hscpp/ICmdShell.h"
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
                std::vector<std::string> output = pCmdShell->PeekTaskOutput();
                RemoveBlankLines(output);

                REQUIRE(output.size() == 1);
                REQUIRE(output.at(0) == "hello");

                return UpdateLoop::Done;
            }

            return UpdateLoop::Running;
        };

        CALL(StartUpdateLoop, Milliseconds(5000), Milliseconds(10), cb);
    }

    TEST_CASE("CmdShell can cat file in test directory that doesn't end with a newline.")
    {
        fs::path assetsPath = TEST_FILES_PATH / "cat-test";
        fs::path catFilePath = assetsPath / "CatFile.txt";
        fs::path sandboxPath = CALL(InitializeSandbox, assetsPath);

        std::unique_ptr<ICmdShell> pCmdShell = platform::CreateCmdShell();

        REQUIRE(pCmdShell->CreateCmdProcess());

        std::string catExecutable;
#if defined(HSCPP_PLATFORM_WIN32)
        catExecutable = "type";
#else
        catExecutable = "cat";
#endif

        std::string cmd = catExecutable + " \"" + catFilePath.u8string() + "\"";
        int taskId = 0;
        pCmdShell->StartTask(cmd, taskId);

        auto cb = [&](Milliseconds timeElapsed){
            ICmdShell::TaskState taskState = pCmdShell->Update(taskId);
            REQUIRE(taskId == 0);

            if (taskState == ICmdShell::TaskState::Done)
            {
                std::vector<std::string> output = pCmdShell->PeekTaskOutput();
                RemoveBlankLines(output);

                REQUIRE(output.size() == 1);
                REQUIRE(output.at(0) == "Hello, CmdShell!");

                return UpdateLoop::Done;
            }

            return UpdateLoop::Running;
        };

        CALL(StartUpdateLoop, Milliseconds(5000), Milliseconds(10), cb);
    }

}}