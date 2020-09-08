#include <assert.h>

#include "hscpp/Preprocessor.h"
#include "hscpp/Log.h"
#include "hscpp/Util.h"

namespace hscpp
{

    void Preprocessor::SetFeatureManager(FeatureManager* pFeatureManager)
    {
        m_pFeatureManager = pFeatureManager;
    }

    void Preprocessor::CreateDependencyGraph(const Input& input)
    {
        if (m_pFeatureManager->IsFeatureEnabled(Feature::DependentCompilation))
        {
            m_DependencyGraph.Clear();

            for (const auto& sourceFilePath : input.sourceFilePaths)
            {
                FileParser::ParseInfo parseInfo = m_FileParser.Parse(sourceFilePath);
                UpdateDependencyGraph(input, parseInfo);
            }
        }
    }

    void Preprocessor::PruneDeletedFilesFromDependencyGraph()
    {
        if (m_pFeatureManager->IsFeatureEnabled(Feature::DependentCompilation))
        {
            m_DependencyGraph.PruneDeletedFiles();
        }
    }

    Preprocessor::Output Preprocessor::Preprocess(const Input& input)
    {
        Reset(input);

        if (m_pFeatureManager->IsFeatureEnabled(Feature::Preprocessor))
        {
            for (const auto& sourceFilePath : input.sourceFilePaths)
            {
                FileParser::ParseInfo parseInfo = m_FileParser.Parse(sourceFilePath);

                AddRequires(input, parseInfo);
                AddPreprocessorDefinitions(parseInfo);

                if (m_pFeatureManager->IsFeatureEnabled(Feature::DependentCompilation))
                {
                    UpdateDependencyGraph(input, parseInfo);
                }
            }

            if (m_pFeatureManager->IsFeatureEnabled(Feature::DependentCompilation))
            {
                for (const auto& sourceFilePath : input.sourceFilePaths)
                {
                    std::vector<fs::path> additionalFilePaths = m_DependencyGraph.ResolveGraph(sourceFilePath);
                    m_SourceFiles.insert(additionalFilePaths.begin(), additionalFilePaths.end());
                }
            }
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
                fs::path canonicalPath = fs::canonical(fullPath, error);

                if (error.value() == ERROR_SUCCESS)
                {
                    switch (require.type)
                    {
                    case FileParser::Require::Type::Source:
                        m_SourceFiles.insert(canonicalPath);
                        break;
                    case FileParser::Require::Type::Include:
                        m_IncludeDirectories.insert(canonicalPath);
                        break;
                    case FileParser::Require::Type::Library:
                        m_Libraries.insert(canonicalPath);
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

        m_DependencyGraph.SetLinkedModules(canonicalFilePath, parseInfo.modules);

        std::vector<fs::path> canonicalIncludePaths;
        for (const auto& includePath : parseInfo.includePaths)
        {
            for (const auto& includeDirectoryPath : input.includeDirectoryPaths)
            {
                // For example, the includePath may be "MathUtil.h", but we want to find the full
                // for creating the dependency graph. Iterate through our include directories to
                // find the folder that contains a matching include.
                fs::path fullIncludePath = includeDirectoryPath / includePath;
                if (fs::exists(fullIncludePath))
                {
                    fullIncludePath = fs::canonical(fullIncludePath, error);
                    if (error.value() == ERROR_SUCCESS)
                    {
                        canonicalIncludePaths.push_back(fullIncludePath);
                    }
                }
            }
        }

        m_DependencyGraph.SetFileDependencies(canonicalFilePath, canonicalIncludePaths);
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