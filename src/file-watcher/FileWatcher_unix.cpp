#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include "hscpp/file-watcher/FileWatcher_unix.h"
#include "hscpp/Log.h"

namespace hscpp
{

    FileWatcher::FileWatcher(FileWatcherConfig* pConfig)
        : m_pConfig(pConfig)
    {}

    bool FileWatcher::AddWatch(const fs::path& directoryPath)
    {
        // Create the inotify fd, if it is not already initialized.
        if (m_NotifyFd == -1)
        {
            m_NotifyFd = inotify_init();
            if (m_NotifyFd == -1)
            {
                log::Error() << HSCPP_LOG_PREFIX << "Failed inotify_init call."
                    << log::LastOsError() << log::End();
                return false;
            }

            int flags = fcntl(m_NotifyFd, F_GETFL, 0);
            if (flags == -1)
            {
                log::Error() << HSCPP_LOG_PREFIX << "Failed to get flags from directory watch fd. "
                    << log::LastOsError() << log::End();
                return false;
            }

            if (fcntl(m_NotifyFd, F_SETFL, flags | O_NONBLOCK) == -1)
            {
                log::Error() << HSCPP_LOG_PREFIX << "Failed to set directory watch fd to nonblocking. "
                    << log::LastOsError() << log::End();
                return false;
            }
        }

        int mask = IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO;
        int wd = inotify_add_watch(m_NotifyFd, directoryPath.u8string().c_str(), mask);
        if (wd == -1)
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to add directory "
                 << directoryPath << " to watch. " << log::LastOsError() << log::End();
            return false;
        }

        m_DirectoryPathsByWd[wd] = directoryPath;

        return true;
    }

    bool FileWatcher::RemoveWatch(const fs::path& directoryPath)
    {
        auto watchIt = std::find_if(m_DirectoryPathsByWd.begin(), m_DirectoryPathsByWd.end(),
            [directoryPath](const std::pair<int, fs::path>& pair) {
                return directoryPath == pair.second;
            });

        if (watchIt == m_DirectoryPathsByWd.end())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Directory " << directoryPath << "could not be found." << log::End();
            return false;
        }

        CloseWatch(watchIt->first);

        m_DirectoryPathsByWd.erase(watchIt);
        return true;
    }

    void FileWatcher::ClearAllWatches()
    {
        for (const auto& wd__path : m_DirectoryPathsByWd)
        {
            CloseWatch(wd__path.first);
        }

        m_DirectoryPathsByWd.clear();
    }

    void FileWatcher::PollChanges(std::vector<Event>& events)
    {
        events.clear();

        // Check for changes and update m_PendingEvents.
        PollChanges();

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

    void FileWatcher::PollChanges()
    {
        const int nFds = 1;

        struct pollfd fds[nFds];
        fds->fd = m_NotifyFd;
        fds->events = POLLIN;

        int ret = poll(fds, nFds, 0);
        if (ret > 0)
        {
            for (int i = 0; i < nFds; ++i)
            {
                ssize_t nBytes = read(fds[i].fd, m_NotifyBuffer.data(), m_NotifyBuffer.size());
                if (nBytes <= 0)
                {
                    log::Error() << HSCPP_LOG_PREFIX << "Failed to read notify fd. "
                        << log::LastOsError() << log::End();
                    return;
                }

                for (char* pData = m_NotifyBuffer.data(); pData < m_NotifyBuffer.data() + nBytes;)
                {
                    struct inotify_event* pNotifyEvent = reinterpret_cast<inotify_event*>(pData);
                    HandleNotifyEvent(pNotifyEvent);

                    pData += sizeof(struct inotify_event) + pNotifyEvent->len;
                }
            }
        }
    }

    void FileWatcher::HandleNotifyEvent(struct inotify_event *pNotifyEvent)
    {
        if (pNotifyEvent->mask & IN_ISDIR)
        {
            // Ignore directories.
            return;
        }

        fs::path directoryPath = m_DirectoryPathsByWd[pNotifyEvent->wd];

        Event event;
        event.filePath = directoryPath / fs::u8path(pNotifyEvent->name);

        if (pNotifyEvent->mask & IN_CREATE
            || pNotifyEvent->mask & IN_MOVED_TO
            || pNotifyEvent->mask & IN_MODIFY
            || pNotifyEvent->mask & IN_DELETE
            || pNotifyEvent->mask & IN_MOVED_FROM)
        {
            m_PendingEvents.push_back(event);
        }
    }

    void FileWatcher::CloseWatch(int wd)
    {
        if (inotify_rm_watch(m_NotifyFd, wd) == -1)
        {
            log::Warning() << HSCPP_LOG_PREFIX << "Failed to remove watch from inotify."
                << log::LastOsError() << log::End();
        }
    }

}