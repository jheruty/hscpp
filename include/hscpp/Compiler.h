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
        struct CompileInfo
        {
            std::filesystem::path buildDirectory;
            std::vector<std::filesystem::path> files;
            std::vector<std::filesystem::path> includeDirectories;
            std::vector<std::filesystem::path> libraries;
            std::vector<std::string> preprocessorDefinitions;
            std::vector<std::string> compileOptions;
            std::vector<std::string> linkOptions;
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
        std::filesystem::path m_CompilingModule;
        std::filesystem::path m_CompiledModule;

        bool CreateClCommandFile(const CompileInfo& info);

        void StartVsPathTask();
        bool HandleTaskComplete(CompilerTask task);
        bool HandleGetVsPathTaskComplete(const std::vector<std::string>& output);
        bool HandleSetVcVarsAllTaskComplete(std::vector<std::string> output);
        bool HandleBuildTaskComplete();

    };

}