#include <unistd.h>
#include <CoreFoundation/CFString.h>

#include <iostream>

#include "hscpp/FileWatcher_apple.h"
#include "hscpp/Log.h"

namespace hscpp
{

    FileWatcher::FileWatcher()
    {
        m_pFsContext = std::make_unique<FSEventStreamContext>();
        m_pFsContext->version = 0; // 0 is the only valid value.
        m_pFsContext->info = this;
        m_pFsContext->retain = nullptr;
        m_pFsContext->release = nullptr;
        m_pFsContext->copyDescription = nullptr;

        m_CfRunLoop = CFRunLoopGetCurrent();

        if (pipe(m_MainToRunLoopPipe) == -1)
        {
            log::Error() << HSCPP_LOG_PREFIX
                << "Failed to create main thread to RunLoop thread pipe." << log::End();
        }

        if (pipe(m_RunLoopToMainPipe) == -1)
        {
            log::Error() << HSCPP_LOG_PREFIX
                << "Failed to create RunLoop thread to main thread pipe." << log::End();
        }
    }

    FileWatcher::~FileWatcher()
    {
        if (!StopCurrentFsEventStream())
        {
            log::Warning() << HSCPP_LOG_PREFIX
                << "Failed to stop FsEventStream on FileWatcher destruction." << log::End();
        }
    }

    bool FileWatcher::AddWatch(const fs::path &directoryPath)
    {
        std::lock_guard lock(m_Mutex);

        fs::path canonicalPath;
        if (!CanonicalPath(directoryPath, canonicalPath))
        {
            return false;
        }

        m_CanonicalDirectoryPaths.insert(canonicalPath);
        return UpdateFsEventStream();
    }

    bool FileWatcher::RemoveWatch(const fs::path &directoryPath)
    {
        std::lock_guard lock(m_Mutex);

        fs::path canonicalPath;
        if (!CanonicalPath(directoryPath, canonicalPath))
        {
            return false;
        }

        size_t nErased = m_CanonicalDirectoryPaths.erase(canonicalPath);
        if (nErased > 0)
        {
            return UpdateFsEventStream();
        }

        return true;
    }

    void FileWatcher::ClearAllWatches()
    {
        std::lock_guard lock(m_Mutex);

        m_CanonicalDirectoryPaths.clear();
        UpdateFsEventStream();
    }

    void FileWatcher::SetPollFrequencyMs(int ms)
    {
        std::lock_guard lock(m_Mutex);

        m_PollFrequency = std::chrono::milliseconds(ms);
    }

    void FileWatcher::PollChanges(std::vector<Event> &events)
    {
        std::lock_guard lock(m_Mutex);

        events.clear();

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

    bool FileWatcher::UpdateFsEventStream()
    {
        if (!StopCurrentFsEventStream())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to stop FsEventStream." << log::End();
            return false;
        }

        if (!m_CanonicalDirectoryPaths.empty())
        {
            std::vector<CFStringRef> cfDirectoryPaths;
            for (const auto& directoryPath : m_CanonicalDirectoryPaths)
            {
                CFStringRef cfDirectoryPath = CFStringCreateWithCString(nullptr,
                        directoryPath.u8string().c_str(), kCFStringEncodingUTF8);

                cfDirectoryPaths.push_back(cfDirectoryPath);
            }

            CFArrayRef cfPaths = CFArrayCreate(nullptr, reinterpret_cast<const void**>(cfDirectoryPaths.data()),
                    cfDirectoryPaths.size(), &kCFTypeArrayCallBacks);


            CFTimeInterval latency = 0.0;
            m_FsStream = FSEventStreamCreate(nullptr, &FileWatcher::FsCallback, m_pFsContext.get(),
                    cfPaths, kFSEventStreamEventIdSinceNow, latency, kFSEventStreamCreateFlagFileEvents);

            if (m_FsStream == nullptr)
            {
                log::Error() << HSCPP_LOG_PREFIX << "Failed to create FsEventStream." << log::End();
                return false;
            }

            CreateRunLoopThread();

            if (!ExecuteRunLoopThreadEvent(RunLoopThreadEvent::CreateRunLoop))
            {
                log::Error() << HSCPP_LOG_PREFIX << "Failed to create RunLoop." << log::End();
                return false;
            }

            FSEventStreamScheduleWithRunLoop(m_FsStream, m_CfRunLoop, kCFRunLoopDefaultMode);

            if (!FSEventStreamStart(m_FsStream))
            {
                log::Error() << HSCPP_LOG_PREFIX << "Failed to start FsEventStream." << log::End();
                return false;
            }

            if (!ExecuteRunLoopThreadEvent(RunLoopThreadEvent::StartRunLoop))
            {
                log::Error() << HSCPP_LOG_PREFIX << "Failed to start RunLoop." << log::End();
                return false;
            }
        }

        return true;
    }

    bool FileWatcher::StopCurrentFsEventStream()
    {
        if (m_FsStream != nullptr)
        {
            FSEventStreamStop(m_FsStream);
            FSEventStreamInvalidate(m_FsStream);
            FSEventStreamRelease(m_FsStream);

            CFRunLoopStop(m_CfRunLoop);

            if (!ExecuteRunLoopThreadEvent(RunLoopThreadEvent::ExitThread))
            {
                log::Error() << HSCPP_LOG_PREFIX << "Failed to exit RunLoop thread." << log::End();
                return false;
            }

            m_RunLoopThread.join();
            m_FsStream = nullptr;
        }

        return true;
    }

    bool FileWatcher::ExecuteRunLoopThreadEvent(RunLoopThreadEvent event)
    {
        ssize_t nBytesSent = write(m_MainToRunLoopPipe[1], &event, sizeof(event));
        if (nBytesSent != sizeof(event))
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to send data to RunLoop thread." << log::End();
            return false;
        }

        return WaitForDoneFromRunThread();
    }

    bool FileWatcher::SendDoneToMainThread()
    {
        char msg[] = {1};
        ssize_t nBytesSent = write(m_RunLoopToMainPipe[1], msg, sizeof(msg));
        if (nBytesSent != sizeof(msg))
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to send done message to main thread." << log::End();
            return false;
        }

        return true;
    }

    bool FileWatcher::WaitForDoneFromRunThread()
    {
        char msg[1];
        ssize_t nBytesRead = read(m_RunLoopToMainPipe[0], msg, sizeof(msg));
        if (nBytesRead != 1)
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to recv data from run thread." << log::End();
            return false;
        }

        return true;
    }

    void FileWatcher::CreateRunLoopThread()
    {
        m_RunLoopThread = std::thread([this](){
            bool bDone = false;
            while (!bDone)
            {
                RunLoopThreadEvent event;
                ssize_t nBytesRead = read(m_MainToRunLoopPipe[0], &event, sizeof(event));

                if (nBytesRead != sizeof(event))
                {
                    log::Error() << HSCPP_LOG_PREFIX << "Failed to recv data from main thread." << log::End();
                    continue;
                }

                switch (event)
                {
                    case RunLoopThreadEvent::CreateRunLoop:
                        m_CfRunLoop = CFRunLoopGetCurrent();
                        SendDoneToMainThread();
                        break;
                    case RunLoopThreadEvent::StartRunLoop:
                        SendDoneToMainThread(); // CFRunLoopRun will block, so send done before calling.
                        CFRunLoopRun();
                        break;
                    case RunLoopThreadEvent::ExitThread:
                        bDone = true;
                        SendDoneToMainThread();
                        break;
                    default:
                        assert(false);
                        break;
                }
            }
        });
    }

    bool FileWatcher::CanonicalPath(const fs::path& path, fs::path& canonicalPath)
    {
        std::error_code error;
        canonicalPath = fs::canonical(path, error);

        if (error.value() != HSCPP_ERROR_SUCCESS)
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to get canonical path of " << path << log::End(".");
            return false;
        }

        return true;
    }

    void FileWatcher::FsCallback(ConstFSEventStreamRef stream, void *pInfo, size_t nEvents, void *pEventPaths,
            const FSEventStreamEventFlags* pEventFlags, const FSEventStreamEventId* pEventIds)
    {
        // pInfo comes from the info field of FSEventStreamContext.
        FileWatcher* pThis = reinterpret_cast<FileWatcher*>(pInfo);

        std::lock_guard lock(pThis->m_Mutex);

        for (size_t i = 0; i < nEvents; ++i)
        {
            if (pEventFlags[i] & kFSEventStreamEventFlagItemIsDir)
            {
                // Ignore directories.
                continue;
            }

            // FSEventStreams are recursive, but the hscpp API is not. Validate that the change
            // happened within the watched directory, and not one of its subdirectories.
            fs::path filePath = fs::u8path(static_cast<char**>(pEventPaths)[i]);
            fs::path canonicalFilePath;

            if (!CanonicalPath(filePath, canonicalFilePath))
            {
                continue;
            }

            fs::path canonicalDirectoryPath = canonicalFilePath.parent_path();

            if (pThis->m_CanonicalDirectoryPaths.find(canonicalDirectoryPath)
                == pThis->m_CanonicalDirectoryPaths.end())
            {
                // Skip change that occurred in subdirectory.
                continue;
            }

            Event event;
            event.filePath = filePath; // Match behavior of other platforms and set non-canonical path.

            if (pEventFlags[i] & kFSEventStreamEventFlagItemCreated)
            {
                event.type = EventType::Added;
                pThis->m_PendingEvents.push_back(event);
            }
            else if (pEventFlags[i] & kFSEventStreamEventFlagItemModified)
            {
                event.type = EventType::Modified;
                pThis->m_PendingEvents.push_back(event);
            }
            else if (pEventFlags[i] & kFSEventStreamEventFlagItemRemoved)
            {
                event.type = EventType::Removed;
                pThis->m_PendingEvents.push_back(event);
            }
        }
    }

}