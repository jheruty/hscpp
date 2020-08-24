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

        for (const auto& file : input.files)
        {
            FileParser::ParseInfo parseInfo = m_FileParser.Parse(file);
            UpdateDependencyGraph(input, parseInfo);
        }
    }

    Preprocessor::Output Preprocessor::Preprocess(const Input& input)
    {
        Reset(input);

        for (const auto& file : input.files)
        {
            FileParser::ParseInfo parseInfo = m_FileParser.Parse(file);

            AddRequires(input, parseInfo);
            AddPreprocessorDefinitions(parseInfo);
            UpdateDependencyGraph(input, parseInfo);
        }

        for (const auto& file : input.files)
        {
            std::vector<fs::path> additionalFiles = m_DependencyGraph.ResolveGraph(file);
            m_SourceFiles.insert(additionalFiles.begin(), additionalFiles.end());
        }

        return CreateOutput();
    }

    void Preprocessor::Reset(const Input& input)
    {
        m_SourceFiles.clear();
        m_IncludeDirectories.clear();
        m_Libraries.clear();
        m_PreprocessorDefinitions.clear();

        m_SourceFiles.insert(input.files.begin(), input.files.end());
        m_IncludeDirectories.insert(input.includeDirectories.begin(), input.includeDirectories.end());
        m_Libraries.insert(input.libraries.begin(), input.libraries.end());
        m_PreprocessorDefinitions.insert(input.preprocessorDefinitions.begin(), input.preprocessorDefinitions.end());
    }

    hscpp::Preprocessor::Output Preprocessor::CreateOutput()
    {
        Output output;

        output.files = std::vector<fs::path>(
            m_SourceFiles.begin(), m_SourceFiles.end());
        output.includeDirectories = std::vector<fs::path>(
            m_IncludeDirectories.begin(), m_IncludeDirectories.end());
        output.libraries = std::vector<fs::path>(
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
                fs::path fullpath = path;
                if (path.is_relative())
                {
                    fullpath = parseInfo.file.parent_path() / path;
                }

                InterpolateRequireVariables(input, fullpath);

                std::error_code error;
                fullpath = fs::canonical(fullpath, error);

                if (error.value() == ERROR_SUCCESS)
                {
                    switch (require.type)
                    {
                    case FileParser::Require::Type::Source:
                        m_SourceFiles.insert(fullpath.wstring());
                        break;
                    case FileParser::Require::Type::Include:
                        m_IncludeDirectories.insert(fullpath.wstring());
                        break;
                    case FileParser::Require::Type::Library:
                        m_Libraries.insert(fullpath.wstring());
                        break;
                    default:
                        assert(false);
                        break;
                    }
                }
                else
                {
                    Log::Write(LogLevel::Error, "%s: Failed to get canonical path of %s. [%s]\n",
                        __func__, fullpath.string().c_str(), util::GetErrorString(error.value()).c_str());
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
        fs::path canoncialFile = fs::canonical(parseInfo.file, error);
        
        if (error.value() != ERROR_SUCCESS)
        {
            Log::Write(LogLevel::Error, "%s: Failed to get canonical path of %s. [%s]\n",
                __func__, canoncialFile.string().c_str(), util::GetErrorString(error.value()).c_str());
            return;
        }

        for (const auto& module : parseInfo.modules)
        {
            m_DependencyGraph.LinkFileToModule(canoncialFile, module);
        }

        std::vector<fs::path> canonicalIncludes;
        for (const auto& include : parseInfo.includes)
        {
            for (const auto& directory : input.includeDirectories)
            {
                fs::path fullIncludePath = directory / include;
                if (fs::exists(fullIncludePath))
                {
                    fullIncludePath = fs::canonical(fullIncludePath, error);
                    if (error.value() == ERROR_SUCCESS)
                    {
                        canonicalIncludes.push_back(fullIncludePath);
                    }
                }
            }

            m_DependencyGraph.SetFileDependencies(canoncialFile, canonicalIncludes);
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