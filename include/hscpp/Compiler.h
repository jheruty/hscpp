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
        struct CompileInfo
        {
            fs::path buildDirectory;
            std::vector<fs::path> files;
            std::vector<fs::path> includeDirectories;
            std::vector<fs::path> libraries;
            std::vector<std::string> preprocessorDefinitions;
            std::vector<std::string> compileOptions;
            std::vector<std::string> linkOptions;
        };

        Compiler();
        
        bool StartBuild(const CompileInfo& info);
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
        fs::path m_CompilingModule;
        fs::path m_CompiledModule;

        bool CreateClCommandFile(const CompileInfo& info);

        void StartVsPathTask();
        bool HandleTaskComplete(CompilerTask task);
        bool HandleGetVsPathTaskComplete(const std::vector<std::string>& output);
        bool HandleSetVcVarsAllTaskComplete(std::vector<std::string> output);
        bool HandleBuildTaskComplete();

    };

}