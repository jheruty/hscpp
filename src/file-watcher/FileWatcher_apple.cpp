#include <unistd.h>
#include <CoreFoundation/CFString.h>

#include <iostream>
#include <algorithm>
#include <unordered_map>

#include "hscpp/file-watcher/FileWatcher_apple.h"
#include "hscpp/Log.h"

namespace hscpp
{

    FileWatcher::FileWatcher(FileWatcherConfig* pConfig)
        : m_pConfig(pConfig)
    {
        m_pFsContext = std::unique_ptr<FSEventStreamContext>(new FSEventStreamContext());
        m_pFsContext->version = 0; // 0 is the only valid value.
        m_pFsContext->info = this;
        m_pFsContext->retain = nullptr;
        m_pFsContext->release = nullptr;
        m_pFsContext->copyDescription = nullptr;

        m_CfRunLoop = CFRunLoopGetCurrent();

        if (pipe(m_MainToRunLoopPipe) == -1)
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to create main thread to RunLoop thread pipe. " <<
                log::LastOsError() << log::End();
        }

        if (pipe(m_RunLoopToMainPipe) == -1)
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to create RunLoop thread to main thread pipe. "
                << log::LastOsError() << log::End();
        }
    }

    FileWatcher::~FileWatcher()
    {
        if (!StopFsEventStream())
        {
            log::Warning() << HSCPP_LOG_PREFIX
                << "Failed to stop FsEventStream on FileWatcher destruction." << log::End();
        }
    }

    bool FileWatcher::AddWatch(const fs::path &directoryPath)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        fs::path canonicalPath;
        if (!CanonicalPath(directoryPath, canonicalPath))
        {
            return false;
        }

        m_CanonicalDirectoryPaths.insert(canonicalPath);
        return CreateFsEventStream();
    }

    bool FileWatcher::RemoveWatch(const fs::path &directoryPath)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        fs::path canonicalPath;
        if (!CanonicalPath(directoryPath, canonicalPath))
        {
            return false;
        }

        size_t nErased = m_CanonicalDirectoryPaths.erase(canonicalPath);
        if (nErased > 0)
        {
            return CreateFsEventStream();
        }

        return true;
    }

    void FileWatcher::ClearAllWatches()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        m_CanonicalDirectoryPaths.clear();
        CreateFsEventStream();
    }

    void FileWatcher::PollChanges(std::vector<Event> &events)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        events.clear();

        // We will gather the events that occur over the next m_PollFrequency ms. This makes it
        // easier to deal with temporary files that occur during saving, as one can be reasonably
        // confident that these files have been created and removed within a sufficiently long
        // m_PollFrequency period.
        if (!m_bGatheringEvents && !m_PendingEvents.empty())
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

    std::vector<fs::path> FileWatcher::GetRootDirectories()
    {
        std::vector<fs::path> rootPaths;

        // Determines whether subPath is a subdirectory within basePath. Assumes paths are canonical
        // and that subPath is longer than basePath.
        auto IsSubPath = [](const fs::path& basePath, const fs::path& subPath){
            auto baseIt = basePath.begin();
            auto subIt = subPath.begin();
            for (; baseIt != basePath.end(); ++baseIt, ++subIt)
            {
                if (*baseIt != *subIt)
                {
                    return false;
                }
            }

            return true;
        };

        // Sort paths from shortest to longest.
        struct SortedPath
        {
            int nSegments = 0; // ex. ~/path/to/dir is 4 segments long.
            fs::path canonicalPath;
        };

        std::vector<SortedPath> sortedPaths;
        for (const auto& canonicalPath : m_CanonicalDirectoryPaths)
        {
            SortedPath sortedPath;
            sortedPath.canonicalPath = canonicalPath;

            for (const auto& segment : canonicalPath)
            {
                HSCPP_UNUSED_VAR(segment);
                sortedPath.nSegments++;
            }

            sortedPaths.push_back(sortedPath);
        }

        std::sort(sortedPaths.begin(), sortedPaths.end(), [](const SortedPath& a, const SortedPath& b) {
           return a.nSegments < b.nSegments;
        });

        // Find root directories.
        std::unordered_set<size_t> overlapped;
        for (size_t i = 0; i < sortedPaths.size(); ++i)
        {
            fs::path basePath = sortedPaths.at(i).canonicalPath;
            if (overlapped.find(i) == overlapped.end())
            {
                // This is not a subdirectory, add it to rootPaths.
                rootPaths.push_back(sortedPaths.at(i).canonicalPath);

                // Check if other directories are subdirectories of basePath. Since directories are
                // sorted by path segment length, one will always be able to find all subdirectories
                // in a single pass.
                for (size_t j = i + 1; j < sortedPaths.size(); ++j)
                {
                    if (overlapped.find(j) == overlapped.end())
                    {
                        // Path was not already marked as a subdirectory; perform check.
                        fs::path subPath = sortedPaths.at(j).canonicalPath;
                        if (IsSubPath(basePath, subPath))
                        {
                            // Mark path as a subdirectory.
                            overlapped.insert(j);
                        }
                    }
                }
            }
        }

        return rootPaths;
    }

    bool FileWatcher::CreateFsEventStream()
    {
        if (!StopFsEventStream())
        {
            log::Error() << HSCPP_LOG_PREFIX << "Failed to stop FsEventStream." << log::End();
            return false;
        }

        if (!m_CanonicalDirectoryPaths.empty())
        {
            // FsEventStreams are recursive, so we only need to watch the root paths. The canonical
            // directory set is used later to determine if a file update came from a watched directory.
            std::vector<fs::path> rootDirectories = GetRootDirectories();

            std::vector<CFStringRef> cfDirectoryPaths;
            for (const auto& directoryPath : rootDirectories)
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

    bool FileWatcher::StopFsEventStream()
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
            m_CfRunLoop = nullptr;
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
            log::Warning() << HSCPP_LOG_PREFIX << "Failed to get canonical path of " << path << log::End(".");
            return false;
        }

        return true;
    }

    void FileWatcher::FsCallback(ConstFSEventStreamRef stream, void *pInfo, size_t nEvents, void *pEventPaths,
            const FSEventStreamEventFlags* pEventFlags, const FSEventStreamEventId* pEventIds)
    {
        HSCPP_UNUSED_PARAM(stream);
        HSCPP_UNUSED_PARAM(pEventIds);

        // pInfo comes from the info field of FSEventStreamContext.
        auto pThis = reinterpret_cast<FileWatcher*>(pInfo);

        std::lock_guard<std::mutex> lock(pThis->m_Mutex);

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

            fs::path canonicalDirectoryPath;
            if (!CanonicalPath(filePath.parent_path(), canonicalDirectoryPath))
            {
                continue;
            }

            if (pThis->m_CanonicalDirectoryPaths.find(canonicalDirectoryPath)
                == pThis->m_CanonicalDirectoryPaths.end())
            {
                // Skip change that occurred in subdirectory.
                continue;
            }

            Event event;
            event.filePath = filePath; // Match behavior of other platforms and set non-canonical path.

            if (pEventFlags[i] & kFSEventStreamEventFlagItemCreated
                || pEventFlags[i] & kFSEventStreamEventFlagItemModified
                || pEventFlags[i] & kFSEventStreamEventFlagItemRemoved
                || pEventFlags[i] & kFSEventStreamEventFlagItemRenamed)
            {
                pThis->m_PendingEvents.push_back(event);
            }
        }
    }

}