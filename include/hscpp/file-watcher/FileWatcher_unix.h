#pragma once

#include <limits.h>
#include <sys/inotify.h>

#include <chrono>
#include <unordered_map>
#include <array>
#include <vector>

#include "hscpp/Platform.h"
#include "hscpp/file-watcher/IFileWatcher.h"
#include "hscpp/Config.h"

namespace hscpp
{

    class FileWatcher : public IFileWatcher
    {
    public:
        FileWatcher(FileWatcherConfig* pConfig);

        bool AddWatch(const fs::path& directoryPath) override;
        bool RemoveWatch(const fs::path& directoryPath) override;
        void ClearAllWatches() override;

        void PollChanges(std::vector<Event>& events) override;

    private:
        struct DirectoryWatch
        {
            int wd;
            fs::path directoryPath;
        };

        FileWatcherConfig* m_pConfig = nullptr;

        std::chrono::steady_clock::time_point m_LastPollTime = std::chrono::steady_clock::now();
        bool m_bGatheringEvents = false;

        int m_NotifyFd = -1;
        std::unordered_map<int, fs::path> m_DirectoryPathsByWd;

        std::vector<Event> m_PendingEvents;

        std::array<char, 32 * (sizeof(struct inotify_event) + NAME_MAX + 1)> m_NotifyBuffer;

        void PollChanges();
        void HandleNotifyEvent(struct inotify_event* pNotifyEvent);

        void CloseWatch(int wd);
    };

}