#include <unordered_set>
#include <fstream>

#include "hscpp/Hotswapper.h"

namespace hscpp
{

    void Hotswapper::AddIncludeDirectory(const std::string& directory)
    {
        m_FileWatcher.AddWatch(directory, false);
        m_IncludeDirectories.push_back(directory);
    }

    void Hotswapper::AddSourceDirectory(const std::string& directory, bool bRecursive)
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

    std::vector<std::string> Hotswapper::GetChangedFiles()
    {
        // When Visual Studio saves, it can create several events for a single file, so use a
        // set to remove these duplicates.
        std::unordered_set<std::string> files;
        for (const auto& event : m_FileEvents)
        {
            switch (event.type)
            {
            // Removed files don't matter, we simply will not attempt to compile them.
            case FileWatcher::EventType::Added:
            case FileWatcher::EventType::Modified:
                files.insert(event.FullPath());
            }
        }

        // While saving a file, temporary copies may be created and then removed. Remove the
        // file from our list if it no longer exists.
        for (auto fileIt = files.begin(); fileIt != files.end();)
        {
            std::ifstream checkExistence(fileIt->c_str());
            if (!checkExistence.good())
            {
                fileIt = files.erase(fileIt);
            }
            else
            {
                ++fileIt;
            }
        }

        return std::vector<std::string>(files.begin(), files.end());
    }

}
