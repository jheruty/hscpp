#include "catch/catch.hpp"
#include "common/Common.h"

#include "hscpp/Platform.h"
#include "hscpp/compiler/ICompiler.h"
#include "hscpp/Util.h"
#include "hscpp/Log.h"
#include "hscpp/Config.h"

namespace hscpp { namespace test
{

    const static fs::path TEST_FILES_PATH = util::GetHscppTestPath() / "unit-tests" / "files" / "test-compiler";
    const static fs::path BUILD_DIRECTORY_PATH = util::GetHscppTestPath() / "test-module-builds";

    static bool bCreatedBuildDirectory = false;

    static fs::path CreateBuildDirectory()
    {
        // Modules will be loaded by the program, so they cannot be deleted while it is running. Get
        // around this by creating a separate build directory, which houses directories with unique
        // names for each module build.
        if (!bCreatedBuildDirectory)
        {
            REQUIRE_NOTHROW(fs::remove_all(BUILD_DIRECTORY_PATH));
            REQUIRE(fs::create_directory(BUILD_DIRECTORY_PATH));

            bCreatedBuildDirectory = true;
        }

        std::string buildFolderName = "build-" + platform::CreateGuid();
        fs::path buildDirectoryPath = BUILD_DIRECTORY_PATH / buildFolderName;
        REQUIRE(fs::create_directory(buildDirectoryPath));

        return buildDirectoryPath;
    }

    static void WaitForInitialize(ICompiler* pCompiler)
    {
        auto cb = [&](Milliseconds)
        {
            pCompiler->Update();
            if (pCompiler->IsInitialized())
            {
                return UpdateLoop::Done;
            }

            return UpdateLoop::Running;
        };

        CALL(StartUpdateLoop, Milliseconds(15000), Milliseconds(10), cb);

        REQUIRE(pCompiler->IsInitialized());
    }

    static fs::path CompileUpdateLoop(ICompiler* pCompiler)
    {
        auto cb = [&](Milliseconds)
        {
            pCompiler->Update();
            if (pCompiler->HasCompiledModule())
            {
                return UpdateLoop::Done;
            }

            return UpdateLoop::Running;
        };

        CALL(StartUpdateLoop, Milliseconds(30000), Milliseconds(10), cb);

        REQUIRE(pCompiler->HasCompiledModule());

        fs::path modulePath = pCompiler->PopModule();
        REQUIRE(fs::exists(modulePath));

        return modulePath;
    }

    TEST_CASE("Compiler can compile a basic library.")
    {
        fs::path assetsPath = TEST_FILES_PATH / "simple-test";
        fs::path sandboxPath = CALL(InitializeSandbox, assetsPath);

        fs::path includeDirectoryPath = sandboxPath;
        fs::path buildDirectoryPath = CALL(CreateBuildDirectory);
        fs::path filePath = sandboxPath / "Lib.cpp";

        auto pConfig = std::unique_ptr<Config>(new Config());
        std::unique_ptr<ICompiler> pCompiler = platform::CreateCompiler(&pConfig->compiler);

        CALL(WaitForInitialize, pCompiler.get());

        ICompiler::Input compileInput;
        compileInput.buildDirectoryPath = buildDirectoryPath;
        compileInput.sourceFilePaths.push_back(filePath);
        compileInput.includeDirectoryPaths.push_back(includeDirectoryPath);
        compileInput.compileOptions = platform::GetDefaultCompileOptions();
        compileInput.preprocessorDefinitions = platform::GetDefaultPreprocessorDefinitions();

        REQUIRE(pCompiler->StartBuild(compileInput));
        REQUIRE(pCompiler->IsCompiling());

        fs::path modulePath = CALL(CompileUpdateLoop, pCompiler.get());

        void* pModule = platform::LoadModule(modulePath);
        REQUIRE(pModule != nullptr);

        auto SetValueTo12 = platform::GetModuleFunction<void(int&)>(pModule, "SetValueTo12");
        REQUIRE(SetValueTo12 != nullptr);

        int val = 0;
        SetValueTo12(val);

        REQUIRE(val == 12);
    }

}}