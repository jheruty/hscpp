#include <memory>
#include <thread>
#include <chrono>
#include <fstream>

#include "catch/catch.hpp"
#include "common/Common.h"
#include "hscpp/IFileWatcher.h"
#include "hscpp/Platform.h"

namespace hscpp { namespace test
{

    const static fs::path ROOT_PATH = RootTestDirectory() / "unit-tests" / "files" / "test-file-watcher";

    TEST_CASE("FileWatcher can monitor directory for changes.", "[FileWatcher]")
    {
        fs::path assetsPath = ROOT_PATH / "simple-test";
        fs::path sandboxPath = CALL(InitializeSandbox, assetsPath);
        fs::path testFilePath = sandboxPath / "src" / "Test.cpp";
        fs::path newFilePath = sandboxPath / "src" / "NewFile.cpp";

        std::unique_ptr<IFileWatcher> pFileWatcher = platform::CreateFileWatcher();
        REQUIRE(pFileWatcher->AddWatch(sandboxPath / "src"));

        std::vector<IFileWatcher::Event> events;

        auto cb = [&](Milliseconds elapsedMs) {
            pFileWatcher->PollChanges(events);
            return !events.empty()
                   ? UpdateLoop::Done
                   : UpdateLoop::Running;
        };

        SECTION("Modifying a simple file triggers event.")
        {
            CALL(ModifyFile, testFilePath, {
                { "Body", "int main() {}" },
            });

            CALL(StartUpdateLoop, Milliseconds(2000), Milliseconds(10), cb);
        }

        SECTION("Creating a new file triggers event.")
        {
            CALL(NewFile, newFilePath, "int main() {}");
            CALL(StartUpdateLoop, Milliseconds(2000), Milliseconds(10), cb);
        }

        SECTION("Deleting existing file triggers event.")
        {
            CALL(RemoveFile, testFilePath);
            CALL(StartUpdateLoop, Milliseconds(2000), Milliseconds(10), cb);
        }
    }

}}