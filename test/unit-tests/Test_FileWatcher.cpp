#include <memory>
#include <thread>
#include <chrono>
#include <fstream>

#include "catch/catch.hpp"
#include "common/Common.h"
#include "hscpp/IFileWatcher.h"
#include "hscpp/Platform.h"
#include "hscpp/Util.h"

namespace hscpp { namespace test
{

    const static fs::path ROOT_PATH = RootTestDirectory() / "unit-tests" / "files" / "test-file-watcher";

    TEST_CASE("FileWatcher can monitor simple directory for changes.", "[FileWatcher]")
    {
        fs::path assetsPath = ROOT_PATH / "simple-test";
        fs::path sandboxPath = CALL(InitializeSandbox, assetsPath);
        fs::path testFilePath = sandboxPath / "src" / "Test.cpp";
        fs::path newFilePath = sandboxPath / "src" / "NewFile.cpp";

        std::unique_ptr<IFileWatcher> pFileWatcher = platform::CreateFileWatcher();
        REQUIRE(pFileWatcher->AddWatch(sandboxPath / "src"));

        std::vector<IFileWatcher::Event> events;
        std::vector<fs::path> canonicalModifiedFilePaths;
        std::vector<fs::path> canonicalRemovedFilePaths;

        auto cb = [&](Milliseconds elapsedMs) {
            pFileWatcher->PollChanges(events);
            return !events.empty()
                   ? UpdateLoop::Done
                   : UpdateLoop::Running;
        };

        SECTION("Modifying a file triggers event.")
        {
            CALL(ModifyFile, testFilePath, {
                { "Body", "int main() {}" },
            });

            fs::path canonicalTestFilePath = CALL(Canonical, testFilePath);

            CALL(StartUpdateLoop, Milliseconds(2000), Milliseconds(10), cb);
            util::SortFileEvents(events, canonicalModifiedFilePaths, canonicalRemovedFilePaths);

            REQUIRE(canonicalModifiedFilePaths.size() == 1);
            REQUIRE(canonicalRemovedFilePaths.empty());
            REQUIRE(canonicalModifiedFilePaths.at(0) == canonicalTestFilePath);
        }

        SECTION("Creating a new file triggers event.")
        {
            CALL(NewFile, newFilePath, "int main() {}");

            fs::path canonicalNewFilePath = CALL(Canonical, newFilePath);

            CALL(StartUpdateLoop, Milliseconds(2000), Milliseconds(10), cb);
            util::SortFileEvents(events, canonicalModifiedFilePaths, canonicalRemovedFilePaths);

            REQUIRE(canonicalModifiedFilePaths.size() == 1);
            REQUIRE(canonicalRemovedFilePaths.empty());
            REQUIRE(canonicalModifiedFilePaths.at(0) == canonicalNewFilePath);
        }

        SECTION("Deleting an existing file triggers event.")
        {
            fs::path canonicalTestFilePath = CALL(Canonical, testFilePath);

            CALL(RemoveFile, testFilePath);
            CALL(StartUpdateLoop, Milliseconds(2000), Milliseconds(10), cb);
            util::SortFileEvents(events, canonicalModifiedFilePaths, canonicalRemovedFilePaths);

            REQUIRE(canonicalModifiedFilePaths.empty());
            REQUIRE(canonicalRemovedFilePaths.size() == 1);
            REQUIRE(canonicalRemovedFilePaths.at(0) == canonicalTestFilePath);
        }
    }

}}