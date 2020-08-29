#include <windows.h>
#include <assert.h>

#include "hscpp/Preprocessor.h"
#include "hscpp/Log.h"
#include "hscpp/Util.h"

namespace hscpp
{

    void Preprocessor::CreateDependencyGraph(const Input& input)
    {
        m_DependencyGraph.Clear();

        for (const auto& file : input.sourceFilePaths)
        {
            FileParser::ParseInfo parseInfo = m_FileParser.Parse(file);
            UpdateDependencyGraph(input, parseInfo);
        }
    }

    Preprocessor::Output Preprocessor::Preprocess(const Input& input)
    {
        Reset(input);

        for (const auto& file : input.sourceFilePaths)
        {
            FileParser::ParseInfo parseInfo = m_FileParser.Parse(file);

            AddRequires(input, parseInfo);
            AddPreprocessorDefinitions(parseInfo);
            UpdateDependencyGraph(input, parseInfo);
        }

        for (const auto& file : input.sourceFilePaths)
        {
            std::vector<fs::path> additionalFilePaths = m_DependencyGraph.ResolveGraph(file);
            m_SourceFiles.insert(additionalFilePaths.begin(), additionalFilePaths.end());
        }

        return CreateOutput();
    }

    void Preprocessor::Reset(const Input& input)
    {
        m_SourceFiles.clear();
        m_IncludeDirectories.clear();
        m_Libraries.clear();
        m_PreprocessorDefinitions.clear();

        m_SourceFiles.insert(input.sourceFilePaths.begin(), input.sourceFilePaths.end());
        m_IncludeDirectories.insert(input.includeDirectoryPaths.begin(), input.includeDirectoryPaths.end());
        m_Libraries.insert(input.libraryPaths.begin(), input.libraryPaths.end());
        m_PreprocessorDefinitions.insert(input.preprocessorDefinitions.begin(), input.preprocessorDefinitions.end());
    }

    hscpp::Preprocessor::Output Preprocessor::CreateOutput()
    {
        Output output;

        output.sourceFilePaths = std::vector<fs::path>(
            m_SourceFiles.begin(), m_SourceFiles.end());
        output.includeDirectoryPaths = std::vector<fs::path>(
            m_IncludeDirectories.begin(), m_IncludeDirectories.end());
        output.libraryPaths = std::vector<fs::path>(
            m_Libraries.begin(), m_Libraries.end());
        output.preprocessorDefinitions = std::vector<std::string>(
            m_PreprocessorDefinitions.begin(), m_PreprocessorDefinitions.end());

        return output;
    }

    void Preprocessor::AddRequires(const Input& input, const FileParser::ParseInfo& parseInfo)
    {
        for (const auto& require : parseInfo.requires)
        {
            for (const auto& path : require.paths)
            {
                // Paths can be either relative or absolute.
                fs::path fullPath = path;
                if (path.is_relative())
                {
                    fullPath = parseInfo.filePath.parent_path() / path;
                }

                InterpolateRequireVariables(input, fullPath);

                std::error_code error;
                fullPath = fs::canonical(fullPath, error);

                if (error.value() == ERROR_SUCCESS)
                {
                    switch (require.type)
                    {
                    case FileParser::Require::Type::Source:
                        m_SourceFiles.insert(fullPath.wstring());
                        break;
                    case FileParser::Require::Type::Include:
                        m_IncludeDirectories.insert(fullPath.wstring());
                        break;
                    case FileParser::Require::Type::Library:
                        m_Libraries.insert(fullPath.wstring());
                        break;
                    default:
                        assert(false);
                        break;
                    }
                }
                else
                {
                    log::Error() << HSCPP_LOG_PREFIX << "Failed to get canonical path of "
                        << fullPath << ". " << log::OsError(error) << log::End();
                }
            }
        }
    }

    void Preprocessor::AddPreprocessorDefinitions(const FileParser::ParseInfo& parseInfo)
    {
        for (const auto& definition : parseInfo.preprocessorDefinitions)
        {
            m_PreprocessorDefinitions.insert(definition);
        }
    }

    void Preprocessor::UpdateDependencyGraph(const Input& input, const FileParser::ParseInfo& parseInfo)
    {
        std::error_code error;
        fs::path canonicalFilePath = fs::canonical(parseInfo.filePath, error);
        
        if (error.value() != ERROR_SUCCESS)
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to get canonical path of "
                << canonicalFilePath << ". " << log::OsError(error) << log::End();
            return;
        }

        for (const auto& module : parseInfo.modules)
        {
            m_DependencyGraph.LinkFileToModule(canonicalFilePath, module);
        }

        std::vector<fs::path> canonicalIncludePaths;
        for (const auto& include : parseInfo.includePaths)
        {
            for (const auto& directory : input.includeDirectoryPaths)
            {
                fs::path fullIncludePath = directory / include;
                if (fs::exists(fullIncludePath))
                {
                    fullIncludePath = fs::canonical(fullIncludePath, error);
                    if (error.value() == ERROR_SUCCESS)
                    {
                        canonicalIncludePaths.push_back(fullIncludePath);
                    }
                }
            }

            m_DependencyGraph.SetFileDependencies(canonicalFilePath, canonicalIncludePaths);
        }
    }

    void Preprocessor::InterpolateRequireVariables(const Input& input, fs::path& path)
    {
        std::string replace = path.u8string();
        for (const auto& var : input.hscppRequireVariables)
        {
            std::string search = "%" + var.first + "%";
            size_t iFound = replace.find(search);
            if (iFound != std::string::npos)
            {
                replace.replace(iFound, search.size(), var.second);
            }
        }

        path = fs::u8path(replace);
    }

}