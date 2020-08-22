#include <algorithm>
#include <assert.h>

#include "hscpp/FileWatcher.h"
#include "hscpp/Log.h"
#include "hscpp/StringUtil.h"

namespace hscpp
{

    const static std::chrono::milliseconds DEBOUNCE_TIME = std::chrono::milliseconds(20);

    FileWatcher::~FileWatcher()
    {
        ClearAllWatches();
    }

    bool FileWatcher::AddWatch(const std::filesystem::path& directory)
    {
        auto pWatch = std::make_unique<DirectoryWatch>();

        // FILE_FLAG_BACKUP_SEMANTICS is necessary to open a directory.
        HANDLE hDirectory = CreateFile(
            directory.native().c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            NULL);

        if (hDirectory == INVALID_HANDLE_VALUE)
        {
            Log::Write(LogLevel::Error, "%s: Failed to add directory '%s' to watch. [%s]\n",
                __func__, directory.c_str(), GetLastErrorString().c_str());
            return false;
        }

        pWatch->directory = directory;
        pWatch->hDirectory = hDirectory;
        pWatch->pFileWatcher = this;

        if (!ReadDirectoryChangesAsync(pWatch.get()))
        {
            Log::Write(LogLevel::Error, "%s: Failed initial call to ReadDirectoryChangesW. [%s]\n",
                __func__, GetLastErrorString().c_str());

            CloseHandle(hDirectory);
            return false;
        }

        m_DirectoryHandles.push_back(hDirectory);
        m_Watchers.push_back(std::move(pWatch));

        return true;
    }

    bool FileWatcher::RemoveWatch(const std::filesystem::path& directory)
    {
        auto watchIt = std::find_if(m_Watchers.begin(), m_Watchers.end(),
            [directory](const std::unique_ptr<DirectoryWatch>& pWatch) {
                return pWatch->directory == directory;
            });

        if (watchIt == m_Watchers.end())
        {
            Log::Write(LogLevel::Error, "%s: Directory '%s' could not be found.\n",
                __func__, directory.c_str());
            return false;
        }

        DirectoryWatch* pWatch = watchIt->get();

        EraseDirectoryHandle(pWatch->hDirectory);
        CloseWatch(pWatch);

        m_Watchers.erase(watchIt);

        return true;
    }

    void FileWatcher::ClearAllWatches()
    {
        for (const auto& pWatch : m_Watchers)
        {
            EraseDirectoryHandle(pWatch->hDirectory);
            CloseWatch(pWatch.get());
        }

        m_Watchers.clear();
    }

    void FileWatcher::SetPollFrequencyMs(int ms)
    {
        m_PollFrequency = std::chrono::milliseconds(ms);
    }

    void FileWatcher::PollChanges(std::vector<Event>& events)
    {
        events.clear();

        // Trigger WatchCallback if a file change was detected.
        WaitForMultipleObjectsEx(static_cast<DWORD>(m_DirectoryHandles.size()), m_DirectoryHandles.data(), false, 0, true);

        // We will gather the events that occur over the next m_PollFrequency ms. This makes it
        // easier to deal with temporary files that occur during saving, as one can be reasonably
        // confident that these files have been created and removed within a sufficiently long
        // m_PollFrequency period.
        if (!m_bGatheringEvents && m_PendingEvents.size() > 0)
        {
            // Begin gathering events.
            m_bGatheringEvents = true;
            m_LastPollTime = std::chrono::steady_clock::now();
            
            return;
        }
        else
        {
            // Currently gathering events. Return if not enough time has passed yet.
            auto now = std::chrono::steady_clock::now();
            auto dt = now - m_LastPollTime;
            if (dt < m_PollFrequency)
            {
                return;
            }
        }

        // Done gathering events.
        m_bGatheringEvents = false;
        
        events = m_PendingEvents;
        m_PendingEvents.clear();
    }

    void FileWatcher::PushPendingEvent(const Event& event)
    {
        m_PendingEvents.push_back(event);
    }

    void WINAPI FileWatcher::WatchCallback(DWORD error, DWORD nBytesTransferred, LPOVERLAPPED overlapped)
    {
        UNREFERENCED_PARAMETER(nBytesTransferred);

        if (error != ERROR_SUCCESS)
        {
            Log::Write(LogLevel::Error, "%s: ReadDirectoryChangesW failed. [%s]\n",
                __func__, GetErrorString(error).c_str());
            return;
        }

        FILE_NOTIFY_INFORMATION* pNotify = nullptr;
        DirectoryWatch* pWatch = reinterpret_cast<DirectoryWatch*>(overlapped);

        size_t offset = 0;
        do
        {
            pNotify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(&pWatch->buffer[offset]);
            offset += pNotify->NextEntryOffset;

            std::wstring filename(pNotify->FileName, pNotify->FileNameLength / sizeof(WCHAR));

            Event event;
            event.filepath = pWatch->directory / filename;

            // Check that this is a regular file, to ignore updates to directories. Pass in an
            // std::error_code to suppress exceptions. It is possible temporary files have been
            // deleted since the notification.
            std::error_code stdError;
            if (std::filesystem::is_regular_file(event.filepath, stdError))
            {
                switch (pNotify->Action)
                {
                case FILE_ACTION_ADDED:
                case FILE_ACTION_RENAMED_NEW_NAME:
                    event.type = EventType::Added;
                    break;
                case FILE_ACTION_REMOVED:
                case FILE_ACTION_RENAMED_OLD_NAME:
                    event.type = EventType::Removed;
                    break;
                case FILE_ACTION_MODIFIED:
                    event.type = EventType::Modified;
                    break;
                default:
                    assert(false);
                    break;
                }

                pWatch->pFileWatcher->PushPendingEvent(event);
            }

        } while (pNotify->NextEntryOffset != 0);

        if (!ReadDirectoryChangesAsync(pWatch))
        {
            Log::Write(LogLevel::Error, "%s: Failed refresh call to ReadDirectoryChangesW. [%s]\n",
                __func__, GetLastErrorString().c_str());
            return;
        }
    }

    bool FileWatcher::ReadDirectoryChangesAsync(DirectoryWatch* pWatch)
    {
        // OVERLAPPED struct must be zero-initialized before calling ReadDirectoryChangesW.
        pWatch->overlapped = {};

        return ReadDirectoryChangesW(
            pWatch->hDirectory,
            pWatch->buffer,
            sizeof(pWatch->buffer),
            false,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SIZE,
            NULL,
            &pWatch->overlapped,
            WatchCallback) != 0;
    }

    void FileWatcher::CloseWatch(DirectoryWatch* pWatch)
    {
        bool bResult = CancelIoEx(pWatch->hDirectory, &pWatch->overlapped);
        if (!bResult)
        {
            Log::Write(LogLevel::Error, "%s: Failed to cancel IO. [%s]\n",
                __func__, GetLastErrorString().c_str());
            return;
        }

        // Wait for IO to be canceled.
        DWORD nBytesTransferred = 0;
        bResult = GetOverlappedResult(pWatch->hDirectory, &pWatch->overlapped, &nBytesTransferred, true);

        // If we were in the middle of an IO operation (like ReadDirectoryChangesW) and call CancelIoEx,
        // GetOverlappedResult returns false, with ERROR_OPERATON_ABORTED.
        if (!bResult && GetLastError() != ERROR_OPERATION_ABORTED)
        {
            Log::Write(LogLevel::Error, "%s: Failed to wait on overlapped result. [%s]\n",
                __func__, GetLastErrorString().c_str());
            return;
        }

        CloseHandle(pWatch->hDirectory);
    }

    void FileWatcher::EraseDirectoryHandle(HANDLE hDirectory)
    {
        auto directoryIt = std::find_if(m_DirectoryHandles.begin(),
            m_DirectoryHandles.end(), [hDirectory](HANDLE h) {
                return hDirectory == h;
            });
        m_DirectoryHandles.erase(directoryIt);
    }

}