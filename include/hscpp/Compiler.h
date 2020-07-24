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
        
        void Compile(const std::vector<std::filesystem::path>& files, const std::vector<std::filesystem::path>& includeDirectories);
        void Update();

    private:
        std::string GetVisualStudioPath();

        CmdShell m_CmdShell;
    };

}