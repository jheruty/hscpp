#include <windows.h>
#include <assert.h>

#include "hscpp/Preprocessor.h"
#include "hscpp/Log.h"
#include "hscpp/Util.h"

namespace hscpp
{

    Preprocessor::Output Preprocessor::Preprocess(const Input& input)
    {
        Output output;
        output.files = input.files;
        output.includeDirectories = input.includeDirectories;
        output.libraries = input.libraries;
        output.preprocessorDefinitions = input.preprocessorDefinitions;

        // Several files compiled at once may have the same dependencies, so store paths in a set
        // such that common requires are deduplicated.
        std::unordered_set<std::wstring> additionalFiles;
        std::unordered_set<std::wstring> additionalIncludes;
        std::unordered_set<std::wstring> additionalLibraries;

        std::unordered_set<std::string> additionalPreprocessorDefinitions;

        for (const auto& file : input.files)
        {
            FileParser::ParseInfo parseInfo = m_FileParser.Parse(file);

            for (const auto& require : parseInfo.requires)
            {
                for (const auto& path : require.paths)
                {
                    // Paths can be either relative or absolute.
                    std::filesystem::path fullpath = path;
                    if (path.is_relative())
                    {
                        fullpath = file.parent_path() / path;
                    }

                    InterpolateRequireVariables(input, fullpath);

                    std::error_code error;
                    fullpath = std::filesystem::canonical(fullpath, error);

                    if (error.value() == ERROR_SUCCESS)
                    {
                        switch (require.type)
                        {
                        case FileParser::Require::Type::Source:
                            additionalFiles.insert(fullpath.wstring());
                            break;
                        case FileParser::Require::Type::Include:
                            additionalIncludes.insert(fullpath.wstring());
                            break;
                        case FileParser::Require::Type::Library:
                            additionalLibraries.insert(fullpath.wstring());
                            break;
                        default:
                            assert(false);
                            break;
                        }
                    }
                    else
                    {
                        Log::Write(LogLevel::Error, "%s: Failed to get canonical path of %s. [%s]\n",
                            __func__, fullpath.string().c_str(), GetErrorString(error.value()).c_str());
                    }
                }
            }

            for (const auto& definition : parseInfo.preprocessorDefinitions)
            {
                additionalPreprocessorDefinitions.insert(definition);
            }
        }

        output.files.insert(output.files.end(),
            additionalFiles.begin(), additionalFiles.end());
        output.includeDirectories.insert(output.includeDirectories.end(),
            additionalIncludes.begin(), additionalIncludes.end());
        output.libraries.insert(output.libraries.begin(),
            additionalLibraries.begin(), additionalLibraries.end());

        output.preprocessorDefinitions.insert(output.preprocessorDefinitions.end(),
            additionalPreprocessorDefinitions.begin(), additionalPreprocessorDefinitions.end());

        return output;
    }

    void Preprocessor::InterpolateRequireVariables(const Input& input, std::filesystem::path& path)
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

        path = std::filesystem::u8path(replace);
    }

}