#include <fstream>
#include <sstream>
#include <algorithm>
#include <cassert>

#include "hscpp/preprocessor/Preprocessor.h"
#include "hscpp/preprocessor/Ast.h"
#include "hscpp/Log.h"

namespace hscpp
{

    bool Preprocessor::Preprocess(const std::vector<fs::path>& canonicalFilePaths, IPreprocessor::Output& output)
    {
        Reset(output);

        std::unordered_set<fs::path, FsPathHasher> uniqueFilePaths(
                canonicalFilePaths.begin(), canonicalFilePaths.end());

        // Resolve dependency graph first. This should not be done recursively, as doing so may end
        // up compiling unnecessary dependents.
        AddDependentFilePaths(uniqueFilePaths);

        m_SourceFilePaths = uniqueFilePaths;

        std::unordered_set<fs::path, FsPathHasher> processedFilePaths;

        while (!uniqueFilePaths.empty())
        {
            if (!Preprocess(uniqueFilePaths))
            {
                return false;
            }

            processedFilePaths.insert(uniqueFilePaths.begin(), uniqueFilePaths.end());
            uniqueFilePaths.clear();

            for (const auto& filePath : m_SourceFilePaths)
            {
                if (processedFilePaths.find(filePath) == processedFilePaths.end())
                {
                    uniqueFilePaths.insert(filePath);
                }
            }
        }

        CreateOutput(output);
        return true;
    }

    void Preprocessor::SetVar(const std::string& name, const Variant& value)
    {
        m_VarStore.SetVar(name, value);
    }

    bool Preprocessor::RemoveVar(const std::string& name)
    {
        return m_VarStore.RemoveVar(name);
    }

    void Preprocessor::ClearDependencyGraph()
    {
        m_DependencyGraph.Clear();
    }

    void Preprocessor::UpdateDependencyGraph(const std::vector<fs::path>& canonicalModifiedFilePaths,
            const std::vector<fs::path>& canonicalRemovedFilePaths,
            const std::vector<fs::path>& includeDirectoryPaths)
    {
        for (const auto& filePath : canonicalRemovedFilePaths)
        {
            m_DependencyGraph.RemoveFile(filePath);
        }

        for (const auto& filePath : canonicalModifiedFilePaths)
        {
            Interpreter::Result result;
            if (Process(filePath, result))
            {
                std::vector<fs::path> canonicalIncludePaths;

                for (const auto& include : result.includePaths)
                {
                    fs::path includePath = fs::u8path(include);
                    for (const auto& includeDirectoryPath : includeDirectoryPaths)
                    {
                        // For example, the includePath may be "MathUtil.h", but we want to find the
                        // full path for creating the dependency graph. Iterate through our include
                        // directories to find the folder that contains a matching include.
                        fs::path fullIncludePath = includeDirectoryPath / includePath;
                        if (fs::exists(fullIncludePath))
                        {
                            std::error_code error;
                            fullIncludePath = fs::canonical(fullIncludePath, error);
                            if (error.value() == HSCPP_ERROR_SUCCESS)
                            {
                                canonicalIncludePaths.push_back(fullIncludePath);
                            }
                        }
                    }
                }

                m_DependencyGraph.SetLinkedModules(filePath, result.hscppModules);
                m_DependencyGraph.SetFileDependencies(filePath, canonicalIncludePaths);
            }
        }
    }

    void Preprocessor::Reset(Output& output)
    {
        output = Output();

        m_SourceFilePaths.clear();
        m_IncludeDirectoryPaths.clear();
        m_LibraryPaths.clear();
        m_LibraryDirectoryPaths.clear();
        m_PreprocessorDefinitions.clear();
    }

    void Preprocessor::CreateOutput(Output& output)
    {
        output.sourceFiles = std::vector<fs::path>(
                m_SourceFilePaths.begin(), m_SourceFilePaths.end());
        output.includeDirectories = std::vector<fs::path>(
                m_IncludeDirectoryPaths.begin(), m_IncludeDirectoryPaths.end());
        output.libraries = std::vector<fs::path>(
                m_LibraryPaths.begin(), m_LibraryPaths.end());
        output.libraryDirectories = std::vector<fs::path>(
                m_LibraryDirectoryPaths.begin(), m_LibraryDirectoryPaths.end());
        output.preprocessorDefinitions = std::vector<std::string>(
                m_PreprocessorDefinitions.begin(), m_PreprocessorDefinitions.end());
    }

    void Preprocessor::AddDependentFilePaths(std::unordered_set<fs::path, FsPathHasher>& filePaths)
    {
        std::unordered_set<fs::path, FsPathHasher> dependentPaths;
        for (const auto& filePath : filePaths)
        {
            std::vector<fs::path> dependentFilePaths = m_DependencyGraph.ResolveGraph(filePath);
            dependentPaths.insert(dependentFilePaths.begin(), dependentFilePaths.end());
        }

        filePaths.insert(dependentPaths.begin(), dependentPaths.end());
    }

    bool Preprocessor::Preprocess(const std::unordered_set<fs::path, FsPathHasher>& filePaths)
    {
        for (const auto& filePath : filePaths)
        {
            Interpreter::Result result;
            if (!Process(filePath, result))
            {
                log::Error() << HSCPP_LOG_PREFIX << "Failed to process file " << filePath << log::End(".");
                return false;
            }

            for (const auto& hscppRequire : result.hscppRequires)
            {
                if (!AddHscppRequire(filePath, hscppRequire))
                {
                    log::Error() << HSCPP_LOG_PREFIX << "Failed to process " << hscppRequire.name
                        << " in file " << filePath << ". (Line: " << hscppRequire.line << ")" << log::End();
                    return false;
                }
            }

            for (const auto& hscppMessage : result.hscppMessages)
            {
                log::Build() << HSCPP_LOG_PREFIX << hscppMessage << log::End();
            }
        }

        return true;
    }

    bool Preprocessor::Process(const fs::path& filePath, Interpreter::Result& result)
    {
        std::ifstream ifs(filePath.u8string());
        if (!ifs.is_open())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to open file "
                << filePath << log::LastOsError() << log::End(".");
            return false;
        }

        std::stringstream ss;
        ss << ifs.rdbuf();

        if (!m_Lexer.Lex(ss.str(), m_Tokens))
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to lex " << filePath << log::End(".");
            log::Error() << m_Lexer.GetLastError().ToString() << log::End();
            return false;
        }

        std::unique_ptr<Stmt> pRootStmt;
        if (!m_Parser.Parse(m_Tokens, pRootStmt))
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to parse " << filePath << log::End(".");
            log::Error() << m_Parser.GetLastError().ToString() << log::End();
            return false;
        }

        if (!m_Interpreter.Evaluate(*pRootStmt, m_VarStore, result))
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to interpret " << filePath << log::End(".");
            log::Error() << m_Interpreter.GetLastError().ToString() << log::End();
            return false;
        }

        return true;
    }

    bool Preprocessor::AddHscppRequire(const fs::path& sourceFilePath, const HscppRequire& hscppRequire)
    {
        for (const auto& value : hscppRequire.values)
        {
            switch (hscppRequire.type)
            {
                case HscppRequire::Type::Source:
                case HscppRequire::Type::IncludeDir:
                case HscppRequire::Type::Library:
                case HscppRequire::Type::LibraryDir:
                {
                    // If path is relative, it should be relative to the path of the source file.
                    fs::path path = fs::u8path(value);

                    fs::path fullPath;
                    if (path.is_relative())
                    {
                        fullPath = sourceFilePath.parent_path() / path;
                    }
                    else
                    {
                        fullPath = path;
                    }

                    std::error_code error;
                    fs::path canonicalPath = fs::canonical(fullPath, error);

                    if (error.value() != HSCPP_ERROR_SUCCESS)
                    {
                        log::Error() << HSCPP_LOG_PREFIX << "Unable to get canonical path of " << fullPath
                            << " within " << hscppRequire.name << ". (Line: " << hscppRequire.line << ")" << log::End();
                        return false;
                    }

                    switch (hscppRequire.type)
                    {
                        case HscppRequire::Type::Source:
                            m_SourceFilePaths.insert(canonicalPath);
                            break;
                        case HscppRequire::Type::IncludeDir:
                            m_IncludeDirectoryPaths.insert(canonicalPath);
                            break;
                        case HscppRequire::Type::Library:
                            m_LibraryPaths.insert(canonicalPath);
                            break;
                        case HscppRequire::Type::LibraryDir:
                            m_LibraryDirectoryPaths.insert(canonicalPath);
                            break;
                        default:
                            assert(false);
                            return false;
                    }
                }
                    break;
                case HscppRequire::Type::PreprocessorDef:
                    m_PreprocessorDefinitions.insert(value);
                    break;
                default:
                    assert(false);
                    return false;
            }
        }

        return true;
    }

}