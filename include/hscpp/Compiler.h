#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include "hscpp/CmdShell.h"

namespace hscpp
{

    namespace fs = std::filesystem;

    class Compiler
    {
    public:
        struct Input
        {
            fs::path buildDirectoryPath;
            std::vector<fs::path> sourceFilePaths;
            std::vector<fs::path> includeDirectoryPaths;
            std::vector<fs::path> libraryPaths;
            std::vector<std::string> preprocessorDefinitions;
            std::vector<std::string> compileOptions;
            std::vector<std::string> linkOptions;
        };

        Compiler();
        
        bool StartBuild(const Input& info);
        void Update();

        bool IsCompiling();

        bool HasCompiledModule();
        fs::path PopModule();

    private:
        enum class CompilerTask
        {
            GetVsPath,
            SetVcVarsAll,
            Build,
        };

        bool m_Initialized = false;
        CmdShell m_CmdShell;

        size_t m_iCompileOutput = 0;
        fs::path m_CompilingModulePath;
        fs::path m_CompiledModulePath;

        bool CreateClCommandFile(const Input& info);

        void StartVsPathTask();
        bool HandleTaskComplete(CompilerTask task);
        bool HandleGetVsPathTaskComplete(const std::vector<std::string>& output);
        bool HandleSetVcVarsAllTaskComplete(std::vector<std::string> output);
        bool HandleBuildTaskComplete();

    };

}