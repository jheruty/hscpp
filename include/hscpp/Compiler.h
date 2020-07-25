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
            std::vector<std::filesystem::path> files;
            std::vector<std::filesystem::path> includeDirectories;
            std::vector<CompileOption> compileOptions;
        };

        Compiler();
        
        bool Compile(const CompileInfo& info);
        void Update();

    private:
        enum class CompilerTask
        {
            GetVsPath,
            SetVcVarsAll,
        };

        bool m_Initialized = false;
        CmdShell m_CmdShell;

        CompileInfo m_CompileInfo;
        std::filesystem::path m_BuildDirectory;

        bool CreateBuildDirectory();
        bool CreateClCommandFile();

        void StartVsPathTask();
        bool HandleTaskComplete(CompilerTask task);
        bool HandleGetVsPathTaskComplete(const std::vector<std::string>& output);
        bool HandleSetVcVarsAllTaskComplete(std::vector<std::string> output);

    };

}