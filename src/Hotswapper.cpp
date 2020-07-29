#include <unordered_set>
#include <fstream>
#include <assert.h>

#include "hscpp/Hotswapper.h"
#include "hscpp/Log.h"
#include "hscpp/StringUtil.h"
#include "hscpp/ModuleInterface.h"

namespace hscpp
{

    const static std::string HSCPP_TEMP_DIRECTORY_NAME = "HSCPP_7c9279ff-25af-488c-a634-b6aa68f47a65";

    // Users may not override these file options.
    const static std::unordered_set<std::string> FORBIDDEN_COMPILE_OPTIONS = {
        "Fe", // Name of compiled module.
        "Fo", // Name of directory to place all object files.
    };
    
    const static std::vector<std::string> DEFAULT_COMPILE_OPTIONS = {
        "/std:c++17", // Use C++17 standard.
        "/Z7", // Add full debugging information.
        "/FC", // Print full filepath in diagnostic messages.
        "/MP", // Build with multiple processes.
#ifdef _DEBUG
        // Debug flags.
        "/MDd", // Use multithreaded debug DLL version of run-time library.
        "/LD", // Create debug DLL. 
#else
        // Release flags.
        "/MD", // Use multithreaded release DLL version of run-time library.
        "/Zo", // Enable enhanced debugging for optimized code.
        "/LD", // Create release DLL.
#endif
    };

    Hotswapper::Hotswapper()
    {
        m_CompileOptions = DEFAULT_COMPILE_OPTIONS;

        // Add hotswap-cpp include directory as a default include directory, since parts of the
        // library will need to be compiled into each new module.
        m_IncludeDirectories.push_back(GetHscppIncludePath());
    }

    void Hotswapper::SetAllocator(std::unique_ptr<IAllocator> pAllocator)
    {
        m_ModuleManager.SetAllocator(std::move(pAllocator));
    }

    void Hotswapper::SetGlobalUserData(void* pGlobalUserData)
    {
        m_ModuleManager.SetGlobalUserData(pGlobalUserData);
    }

    void Hotswapper::AddIncludeDirectory(const std::filesystem::path& directory)
    {
        m_FileWatcher.AddWatch(directory, false);
        m_IncludeDirectories.push_back(directory);
    }

    void Hotswapper::AddSourceDirectory(const std::filesystem::path& directory, bool bRecursive)
    {
        m_FileWatcher.AddWatch(directory, bRecursive);
    }

    void Hotswapper::AddCompileOption(const std::string& option)
    {
        m_CompileOptions.push_back(option);
    }

    void Hotswapper::SetCompileOptions(const std::vector<std::string>& options)
    {
        m_CompileOptions = options;
    }

    std::vector<std::string> Hotswapper::GetCompileOptions()
    {
        return m_CompileOptions;
    }

    void Hotswapper::Update()
    {
        m_FileWatcher.PollChanges(m_FileEvents);
        if (m_FileEvents.size() > 0)
        {
            if (CreateBuildDirectory())
            {
                Compiler::CompileInfo info;
                info.buildDirectory = m_BuildDirectory;
                info.files = GetChangedFiles();
                info.includeDirectories = m_IncludeDirectories;
                info.compileOptions = m_CompileOptions;

                m_Compiler.StartBuild(info);
            }
        }

        m_Compiler.Update();
        if (m_Compiler.HasCompiledModule())
        {
            m_ModuleManager.PerformRuntimeSwap(m_Compiler.PopModule());
        }
    }

    bool Hotswapper::CreateHscppTempDirectory()
    {
        std::error_code error;
        std::filesystem::path temp = std::filesystem::temp_directory_path(error);

        if (error.value() != ERROR_SUCCESS)
        {
            Log::Write(LogLevel::Error, "%s: Failed to find temp directory path. [%s]\n",
                __func__, GetErrorString(error.value()).c_str());
            return false;
        }

        std::filesystem::path hscppTemp = temp / HSCPP_TEMP_DIRECTORY_NAME;

        std::filesystem::remove_all(hscppTemp, error);
        if (!std::filesystem::create_directory(hscppTemp, error))
        {
            Log::Write(LogLevel::Error, "%s: Failed to create directory '%s'. [%s]\n",
                __func__, hscppTemp.string().c_str(), GetErrorString(error.value()).c_str());
            return false;
        }

        m_HscppTempDirectory = hscppTemp;

        return true;
    }

    bool Hotswapper::CreateBuildDirectory()
    {
        if (m_HscppTempDirectory.empty())
        {
            if (!CreateHscppTempDirectory())
            {
                return false;
            }
        }

        std::string guid = CreateGuid();
        m_BuildDirectory = m_HscppTempDirectory / guid;

        std::error_code error;
        if (!std::filesystem::create_directory(m_BuildDirectory, error))
        {
            Log::Write(LogLevel::Error, "%s: Failed to create directory '%s'. [%s]\n",
                __func__, m_BuildDirectory.string().c_str(), GetErrorString(error.value()).c_str());
            return false;
        }

        return true;
    }

    std::vector<std::filesystem::path> Hotswapper::GetChangedFiles()
    {
        // When Visual Studio saves, it can create several events for a single file, so use a
        // set to remove these duplicates.
        std::unordered_set<std::wstring> files;
        for (const auto& event : m_FileEvents)
        {
            if (event.type == FileWatcher::EventType::Removed)
            {
                // Removed files don't matter, we simply will not attempt to compile them.
                continue;
            }

            std::error_code error;
            std::filesystem::path canonicalPath = std::filesystem::canonical(event.filepath, error);

            if (error.value() == ERROR_FILE_NOT_FOUND)
            {
                // While saving a file, temporary copies may be created and then removed. Skip them.
                continue;
            }
            else if (error.value() != ERROR_SUCCESS)
            {
                Log::Write(LogLevel::Error, "%s: Failed to get canonical path of %s. [%s]\n",
                    __func__, event.filepath.c_str(), GetLastErrorString().c_str());
                continue;
            }

            switch (event.type)
            {
            case FileWatcher::EventType::Added:
            case FileWatcher::EventType::Modified:
                files.insert(canonicalPath.wstring());
                break;
            default:
                assert(false);
                break;
            }
        }

        return std::vector<std::filesystem::path>(files.begin(), files.end());
    }

}
