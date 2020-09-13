#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <memory>
#include <chrono>

#include "hscpp/Platform.h"
#include "hscpp/IFileWatcher.h"

namespace hscpp
{

    class FileWatcher : public IFileWatcher
    {
    public:
        ~FileWatcher();

        bool AddWatch(const fs::path& directoryPath) override;
        bool RemoveWatch(const fs::path& directoryPath) override;
        void ClearAllWatches() override;

        void SetPollFrequencyMs(int ms) override;
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

        std::chrono::milliseconds m_PollFrequency = std::chrono::milliseconds(100);
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