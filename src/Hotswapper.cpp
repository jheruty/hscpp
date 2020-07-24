#include <unordered_set>
#include <fstream>
#include <assert.h>

#include "hscpp/Hotswapper.h"
#include "hscpp/Log.h"
#include "hscpp/StringUtil.h"

namespace hscpp
{

    void Hotswapper::AddIncludeDirectory(const std::filesystem::path& directory)
    {
        m_FileWatcher.AddWatch(directory, false);
        m_IncludeDirectories.push_back(directory);
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
            auto files = GetChangedFiles();
            m_Compiler.Compile(files, m_IncludeDirectories);
        }

        m_Compiler.Update();
    }

    std::vector<std::filesystem::path> Hotswapper::GetChangedFiles()
    {
        // When Visual Studio saves, it can create several events for a single file, so use a
        // set to remove these duplicates.
        std::unordered_set<std::string> files;
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
                files.insert(canonicalPath.u8string());
                break;
            default:
                assert(false);
                break;
            }
        }

        return std::vector<std::filesystem::path>(files.begin(), files.end());
    }

}
