#include <algorithm>
#include <assert.h>

#include "hscpp/file-watcher/FileWatcher_win32.h"
#include "hscpp/Log.h"
#include "hscpp/Util.h"

namespace hscpp
{

    const static std::chrono::milliseconds DEBOUNCE_TIME = std::chrono::milliseconds(20);

    FileWatcher::FileWatcher(FileWatcherConfig *pConfig)
        : m_pConfig(pConfig)
    {}

    FileWatcher::~FileWatcher()
    {
        ClearAllWatches();
    }

    bool FileWatcher::AddWatch(const fs::path& directoryPath)
    {
        auto pWatch = std::unique_ptr<DirectoryWatch>(new DirectoryWatch());

        // FILE_FLAG_BACKUP_SEMANTICS is necessary to open a directory.
        HANDLE hDirectory = CreateFileW(
            directoryPath.wstring().c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            NULL);

        if (hDirectory == INVALID_HANDLE_VALUE)
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to add directory " 
                << directoryPath << " to watch. " << log::LastOsError() << log::End();
            return false;
        }

        pWatch->directoryPath = directoryPath;
        pWatch->hDirectory = hDirectory;
        pWatch->pFileWatcher = this;

        if (!ReadDirectoryChangesAsync(pWatch.get()))
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed initial call to ReadDirectoryChangesW. "
                << log::LastOsError() << log::End();

            CloseHandle(hDirectory);
            return false;
        }

        m_DirectoryHandles.push_back(hDirectory);
        m_Watchers.push_back(std::move(pWatch));

        return true;
    }

    bool FileWatcher::RemoveWatch(const fs::path& directoryPath)
    {
        auto watchIt = std::find_if(m_Watchers.begin(), m_Watchers.end(),
            [directoryPath](const std::unique_ptr<DirectoryWatch>& pWatch) {
                return pWatch->directoryPath == directoryPath;
            });

        if (watchIt == m_Watchers.end())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Directory " << directoryPath << "could not be found." << log::End();
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
            if (dt < m_pConfig->latency)
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
            log::Error() << HSCPP_LOG_PREFIX << "ReadDirectoryChangesW failed. "
                << log::OsError(error) << log::End();
            return;
        }

        FILE_NOTIFY_INFORMATION* pNotify = nullptr;
        DirectoryWatch* pWatch = reinterpret_cast<DirectoryWatch*>(overlapped);

        size_t offset = 0;
        do
        {
            pNotify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(&pWatch->buffer[offset]);
            offset += pNotify->NextEntryOffset;

            std::wstring fileName(pNotify->FileName, pNotify->FileNameLength / sizeof(WCHAR));

            Event event;
            event.filePath = pWatch->directoryPath / fileName;

            pWatch->pFileWatcher->PushPendingEvent(event);

        } while (pNotify->NextEntryOffset != 0);

        if (!ReadDirectoryChangesAsync(pWatch))
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed refresh call to ReadDirectoryChangesW. "
                << log::LastOsError() << log::End();
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
        bool bResult = (CancelIoEx(pWatch->hDirectory, &pWatch->overlapped) != 0);
        if (!bResult)
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to cancel IO. "
                << log::LastOsError() << log::End();
            return;
        }

        // Wait for IO to be canceled.
        DWORD nBytesTransferred = 0;
        bResult = (GetOverlappedResult(pWatch->hDirectory, &pWatch->overlapped, &nBytesTransferred, true) != 0);

        // If we were in the middle of an IO operation (like ReadDirectoryChangesW) and call CancelIoEx,
        // GetOverlappedResult returns false, with ERROR_OPERATON_ABORTED.
        if (!bResult && GetLastError() != ERROR_OPERATION_ABORTED)
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to wait on overlapped result. "
                << log::LastOsError() << log::End();
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