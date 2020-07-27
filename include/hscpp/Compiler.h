#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include "hscpp/CmdShell.h"

namespace hscpp
{

    class Compiler
    {
    public:
        struct CompileOption
        {
            std::string option;
            std::string arg;
        };

        struct CompileInfo
        {
            std::filesystem::path buildDirectory;
            std::vector<std::filesystem::path> files;
            std::vector<std::filesystem::path> includeDirectories;
            std::vector<CompileOption> compileOptions;
        };

        Compiler();
        
        bool StartBuild(const CompileInfo& info);
        void Update();

        bool HasCompiledModule();
        std::filesystem::path PopModule();

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
        std::filesystem::path m_CompiledModule;

        bool CreateClCommandFile(const CompileInfo& info);

        void StartVsPathTask();
        bool HandleTaskComplete(CompilerTask task);
        bool HandleGetVsPathTaskComplete(const std::vector<std::string>& output);
        bool HandleSetVcVarsAllTaskComplete(std::vector<std::string> output);
        bool HandleBuildTaskComplete();

    };

}