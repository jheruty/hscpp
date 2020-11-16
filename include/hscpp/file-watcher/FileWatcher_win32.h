#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <memory>
#include <chrono>

#include "hscpp/Platform.h"
#include "hscpp/file-watcher/IFileWatcher.h"
#include "hscpp/Config.h"

namespace hscpp
{

    class FileWatcher : public IFileWatcher
    {
    public:
        explicit FileWatcher(FileWatcherConfig* pConfig);
        ~FileWatcher();

        bool AddWatch(const fs::path& directoryPath) override;
        bool RemoveWatch(const fs::path& directoryPath) override;
        void ClearAllWatches() override;

        void PollChanges(std::vector<Event>& events) override;

    private:
        struct DirectoryWatch
        {
            // Allows casting of an OVERLAPPED struct to a DirectoryWatch safely.
            OVERLAPPED overlapped = {};

            // Buffer passed into ReadDirectoryChangesW must be aligned on DWORD boundary.
            alignas(sizeof(DWORD)) uint8_t buffer[32 * 1024];

            fs::path directoryPath;
            HANDLE hDirectory = INVALID_HANDLE_VALUE;

            FileWatcher* pFileWatcher = nullptr;
        };

        FileWatcherConfig* m_pConfig = nullptr;

        std::chrono::steady_clock::time_point m_LastPollTime = std::chrono::steady_clock::now();
        bool m_bGatheringEvents = false;

        std::vector<std::unique_ptr<DirectoryWatch>> m_Watchers;
        std::vector<HANDLE> m_DirectoryHandles;

        std::vector<Event> m_PendingEvents;

        void PushPendingEvent(const Event& event);

        static void WINAPI WatchCallback(DWORD error, DWORD nBytesTransferred, LPOVERLAPPED overlapped);
        static bool ReadDirectoryChangesAsync(DirectoryWatch* pWatch);

        void CloseWatch(DirectoryWatch* pWatch);
        void EraseDirectoryHandle(HANDLE hDirectory);
    };
}