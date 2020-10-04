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

    static void WaitForCmdDone(ICmdShell* pCmdShell, int expectedTaskId)
    {
        auto cb = [&](Milliseconds timeElapsed){
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

    TEST_CASE("CmdShell can perform a basic echo.", "[CmdShell]")
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
#if defined(HSCPP_PLATFORM_WIN32)
        catExecutable = "type";
#else
        catExecutable = "cat";
#endif

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

#if defined(HSCPP_PLATFORM_WIN32)
        pCmdShell->StartTask("echo %HSCPP_VAR%", taskId);
#else
        pCmdShell->StartTask("echo $HSCPP_VAR", taskId);
#endif

        CALL(WaitForCmdDone, pCmdShell.get(), taskId);

        std::vector<std::string> output = pCmdShell->PeekTaskOutput();
        RemoveBlankLines(output);

#if defined(HSCPP_PLATFORM_WIN32)
        REQUIRE(output.size() == 1);
        REQUIRE(output.at(0) == "%HSCPP_VAR%");
#else
        REQUIRE(output.empty());
#endif

#if defined(HSCPP_PLATFORM_WIN32)
        // Set variable.
        pCmdShell->StartTask("set HSCPP_VAR=HscppVar", taskId);
#else
        // Set variable.
        pCmdShell->StartTask("HSCPP_VAR=HscppVar", taskId);
#endif

        CALL(WaitForCmdDone, pCmdShell.get(), taskId);

        output = pCmdShell->PeekTaskOutput();
        RemoveBlankLines(output);

        REQUIRE(output.empty());

#if defined(HSCPP_PLATFORM_WIN32)
        pCmdShell->StartTask("echo %HSCPP_VAR%", taskId);
#else
        // Validate that variable is set.
        pCmdShell->StartTask("echo $HSCPP_VAR", taskId);
#endif

        CALL(WaitForCmdDone, pCmdShell.get(), taskId);

        output = pCmdShell->PeekTaskOutput();
        RemoveBlankLines(output);

        REQUIRE(output.size() == 1);
        REQUIRE(output.at(0) == "HscppVar");
    }

}}