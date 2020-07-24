#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <filesystem>

namespace hscpp
{
    class FileWatcher
    {
    public:
        enum class EventType
        {
            None,
            Added,
            Removed,
            Modified,
        };

        struct Event
        {
            EventType type = EventType::None;
            std::filesystem::path filepath;
        };

        ~FileWatcher();

        bool AddWatch(const std::filesystem::path& directory, bool bRecursive);
        bool RemoveWatch(const std::filesystem::path& directory);
        void ClearAllWatches();

        void SetPollFrequencyMs(int ms);
        void PollChanges(std::vector<Event>& events);

        void PushPendingEvent(const Event& event);

    private:
        struct DirectoryWatch
        {
            // Allows casting of an OVERLAPPED struct to a DirectoryWatch safely.
            OVERLAPPED overlapped = {};

            // Buffer passed into ReadDirectoryChangesW must be aligned on DWORD boundary.
            alignas(sizeof(DWORD)) uint8_t buffer[32 * 1024];

            std::filesystem::path directory;
            HANDLE hDirectory = INVALID_HANDLE_VALUE;
            bool bRecursive = false;

            FileWatcher* pFileWatcher = nullptr;
        };

        std::chrono::milliseconds m_PollFrequency = std::chrono::milliseconds(100);
        std::chrono::steady_clock::time_point m_LastPollTime = std::chrono::steady_clock::now();
        bool m_bGatheringEvents = false;

        std::vector<std::unique_ptr<DirectoryWatch>> m_Watchers;
        std::vector<HANDLE> m_DirectoryHandles;

        std::vector<Event> m_PendingEvents;

        static void WINAPI WatchCallback(DWORD error, DWORD nBytesTransferred, LPOVERLAPPED overlapped);
        static bool ReadDirectoryChangesAsync(DirectoryWatch* pWatch);

        void CloseWatch(DirectoryWatch* pWatch);
        void EraseDirectoryHandle(HANDLE hDirectory);
    };
}