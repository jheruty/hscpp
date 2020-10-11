#pragma once

#include <vector>

#include "hscpp/Filesystem.h"

namespace hscpp
{

    class ICompiler
    {
    public:
        struct Input
        {
            fs::path buildDirectoryPath;
            std::vector<fs::path> sourceFilePaths;
            std::vector<fs::path> includeDirectoryPaths;
            std::vector<fs::path> libraryDirectoryPaths;
            std::vector<fs::path> libraryPaths;
            std::vector<std::string> preprocessorDefinitions;
            std::vector<std::string> compileOptions;
            std::vector<std::string> linkOptions;
        };

        virtual ~ICompiler() = default;

        virtual bool IsInitialized() = 0;

        virtual bool StartBuild(const Input& info) = 0;
        virtual void Update() = 0;

        virtual bool IsCompiling() = 0;

        virtual bool HasCompiledModule() = 0;
        virtual fs::path PopModule() = 0;
    };

}