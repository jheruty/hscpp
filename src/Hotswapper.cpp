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
    const static std::vector<Compiler::CompileOption> DEFAULT_COMPILE_OPTIONS = {
        {},
    };

    Hotswapper::Hotswapper()
    {
        m_CompileOptions = DEFAULT_COMPILE_OPTIONS;
        Hscpp_GetModuleInterface()->SetTrackersByKey(&m_TrackersByKey);
    }

    void Hotswapper::AddIncludeDirectory(const std::filesystem::path& directory)
    {
        m_FileWatcher.AddWatch(directory, false);
        m_IncludeDirectories.push_back(directory);
    }

    void Hotswapper::RemoveIncludeDirectory(const std::filesystem::path& directory)
    {
        m_FileWatcher.RemoveWatch(directory); // TODO: 
    }

    std::vector<std::filesystem::path> Hotswapper::GetIncludeDirectories()
    {
        return m_IncludeDirectories;
    }

    void Hotswapper::AddSourceDirectory(const std::filesystem::path& directory, bool bRecursive)
    {
        m_FileWatcher.AddWatch(directory, bRecursive);
    }

    void Hotswapper::RemoveSourceDirectory(const std::filesystem::path& directory)
    {

    }

    std::vector<std::filesystem::path> Hotswapper::GetSourceDirectories()
    {
        return m_SourceDirectories;
    }

    void Hotswapper::AddCompileOption(const Compiler::CompileOption& option)
    {

    }

    void Hotswapper::RemoveCompileOption(const Compiler::CompileOption& option)
    {

    }

    std::vector<hscpp::Compiler::CompileOption> Hotswapper::GetCompileOptions()
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
            PerformRuntimeSwap(m_Compiler.PopModule());
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

    bool Hotswapper::PerformRuntimeSwap(const std::filesystem::path& moduleFilepath)
    {
        HMODULE hModule = LoadLibrary(moduleFilepath.native().c_str());
        if (hModule == nullptr)
        {
            Log::Write(LogLevel::Error, "%s: Failed to load module %s. [%s]\n",
                __func__, moduleFilepath.string().c_str(), GetLastErrorString().c_str());
            return false;
        }

        typedef ModuleInterface* (__cdecl* Hsccp_GetModuleInterfaceProc)(void);
        auto getModuleInterfaceProc = reinterpret_cast<Hsccp_GetModuleInterfaceProc>(
            GetProcAddress(hModule, "Hscpp_GetModuleInterface"));

        if (getModuleInterfaceProc == nullptr)
        {
            Log::Write(LogLevel::Error, "%s: Failed to load Hscpp_GetModuleInterface procedure. [%s]\n",
                __func__, GetLastErrorString().c_str());
            return false;
        }

        ModuleInterface* pModuleInterface = getModuleInterfaceProc();
        if (pModuleInterface == nullptr)
        {
            Log::Write(LogLevel::Error, "%s: Failed to get point to module interface.\n", __func__);
            return false;
        }

        pModuleInterface->SetTrackersByKey(&m_TrackersByKey);
        pModuleInterface->PerformRuntimeSwap();

        return true;
    }

}
