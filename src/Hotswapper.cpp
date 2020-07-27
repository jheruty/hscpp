#include <unordered_set>
#include <fstream>
#include <assert.h>

#include "hscpp/Hotswapper.h"
#include "hscpp/Log.h"
#include "hscpp/StringUtil.h"
#include "hscpp/ModuleInterface.h"

namespace hscpp
{

    Hotswapper::Hotswapper()
    {
        Hscpp_GetModuleInterface()->SetTrackersByKey(&m_TrackersByKey);
    }

    void Hotswapper::AddIncludeDirectory(const std::filesystem::path& directory)
    {
        m_FileWatcher.AddWatch(directory, false);
        m_CompileInfo.includeDirectories.push_back(directory);
    }

    void Hotswapper::AddSourceDirectory(const std::filesystem::path& directory, bool bRecursive)
    {
        m_FileWatcher.AddWatch(directory, bRecursive);
    }

    void Hotswapper::Update()
    {
        m_FileWatcher.PollChanges(m_FileEvents);
        if (m_FileEvents.size() > 0)
        {
            m_CompileInfo.files = GetChangedFiles();
            m_Compiler.StartBuild(m_CompileInfo);
        }

        m_Compiler.Update();
        if (m_Compiler.HasCompiledModule())
        {
            PerformRuntimeSwap(m_Compiler.ConsumeModule());
        }
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
