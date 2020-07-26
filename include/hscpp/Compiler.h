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

        bool HasCompiledModule();
        std::filesystem::path ReadCompiledModule();

    private:
        enum class CompilerTask
        {
            GetVsPath,
            SetVcVarsAll,
            Compile,
        };

        bool m_Initialized = false;
        CmdShell m_CmdShell;

        CompileInfo m_CompileInfo;
        std::filesystem::path m_BuildDirectory;

        size_t m_iCompileOutput = 0;
        std::filesystem::path m_CompiledModule;

        bool CreateBuildDirectory();
        bool CreateClCommandFile();

        void StartVsPathTask();
        bool HandleTaskComplete(CompilerTask task);
        bool HandleGetVsPathTaskComplete(const std::vector<std::string>& output);
        bool HandleSetVcVarsAllTaskComplete(std::vector<std::string> output);
        bool HandleCompileTaskComplete();

    };

}