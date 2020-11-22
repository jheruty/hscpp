#pragma once

#include <unordered_set>

#include "hscpp/preprocessor/IPreprocessor.h"
#include "hscpp/preprocessor/DependencyGraph.h"
#include "hscpp/preprocessor/VarStore.h"
#include "hscpp/preprocessor/Token.h"
#include "hscpp/preprocessor/Lexer.h"
#include "hscpp/preprocessor/Parser.h"
#include "hscpp/preprocessor/Interpreter.h"
#include "hscpp/preprocessor/HscppRequire.h"
#include "hscpp/preprocessor/Variant.h"
#include "hscpp/FsPathHasher.h"

namespace hscpp
{

    class Preprocessor : public IPreprocessor
    {
    public:
        bool Preprocess(const std::vector<fs::path>& canonicalFilePaths, Output& output) override;

        void SetVar(const std::string& name, const Variant& value) override;
        bool RemoveVar(const std::string& name) override;

        void ClearDependencyGraph() override;
        void UpdateDependencyGraph(const std::vector<fs::path>& canonicalModifiedFilePaths,
                const std::vector<fs::path>& canonicalRemovedFilePaths,
                const std::vector<fs::path>& includeDirectoryPaths) override;

    private:
        std::vector<Token> m_Tokens;

        Lexer m_Lexer;
        Parser m_Parser;
        Interpreter m_Interpreter;

        DependencyGraph m_DependencyGraph;
        VarStore m_VarStore;

        std::unordered_set<fs::path, FsPathHasher> m_SourceFilePaths;
        std::unordered_set<fs::path, FsPathHasher> m_IncludeDirectoryPaths;
        std::unordered_set<fs::path, FsPathHasher> m_LibraryPaths;
        std::unordered_set<fs::path, FsPathHasher> m_LibraryDirectoryPaths;
        std::unordered_set<std::string> m_PreprocessorDefinitions;

        void Reset(Output& output);
        void CreateOutput(Output& output);

        void AddDependentFilePaths(std::unordered_set<fs::path, FsPathHasher>& filePaths);

        bool Preprocess(const std::unordered_set<fs::path, FsPathHasher>& filePaths);
        bool Process(const fs::path& filePath, Interpreter::Result& result);

        bool AddHscppRequire(const fs::path& sourceFilePath, const HscppRequire& hscppRequire);
    };

}