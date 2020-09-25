#include "catch/catch.hpp"
#include "common/Common.h"

#include "hscpp/Platform.h"
#include "hscpp/ICompiler.h"

namespace hscpp { namespace test
{

    const static fs::path ROOT_PATH = RootTestDirectory() / "unit-tests" / "files" / "test-compiler";
    const static fs::path BUILD_DIRECTORY_NAME = "build";

    static fs::path CreateBuildDirectory(const fs::path& sandboxPath)
    {
        fs::path buildDirectoryPath = sandboxPath / BUILD_DIRECTORY_NAME;
        REQUIRE(fs::create_directory(buildDirectoryPath));

        return buildDirectoryPath;
    }

    static void WaitForInitialize(ICompiler* pCompiler)
    {
        auto cb = [&](Milliseconds elapsedTime)
        {
            pCompiler->Update();
            if (pCompiler->IsInitialized())
            {
                return UpdateLoop::Done;
            }

            return UpdateLoop::Running;
        };

        CALL(StartUpdateLoop, Milliseconds(10000), Milliseconds(10), cb);

        REQUIRE(pCompiler->IsInitialized());
    }

    static fs::path CompileUpdateLoop(ICompiler* pCompiler)
    {
        auto cb = [&](Milliseconds elapsedTime)
        {
            pCompiler->Update();
            if (pCompiler->HasCompiledModule())
            {
                return UpdateLoop::Done;
            }

            return UpdateLoop::Running;
        };

        CALL(StartUpdateLoop, Milliseconds(10000), Milliseconds(10), cb);

        REQUIRE(pCompiler->HasCompiledModule());

        fs::path modulePath = pCompiler->PopModule();
        REQUIRE(fs::exists(modulePath));

        return modulePath;
    }

    TEST_CASE("Compiler can compile a basic library.", "[Compiler]")
    {
        fs::path assetsPath = ROOT_PATH / "simple-test";
        fs::path sandboxPath = CALL(InitializeSandbox, assetsPath);

        fs::path includeDirectoryPath = sandboxPath;
        fs::path buildDirectoryPath = CALL(CreateBuildDirectory, sandboxPath);
        fs::path filePath = sandboxPath / "Lib.cpp";

        std::unique_ptr<ICompiler> pCompiler = platform::CreateCompiler();

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

    }

}}