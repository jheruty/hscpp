#pragma once

#include <CoreServices/CoreServices.h>

#include <mutex>
#include <thread>
#include <chrono>
#include <unordered_set>

#include "hscpp/file-watcher/IFileWatcher.h"
#include "hscpp/FsPathHasher.h"
#include "hscpp/Config.h"

namespace hscpp
{
    class FileWatcher : public IFileWatcher
    {
    public:
        FileWatcher(FileWatcherConfig* pConfig);
        ~FileWatcher() override;

        bool AddWatch(const fs::path& directoryPath) override;
        bool RemoveWatch(const fs::path& directoryPath) override;
        void ClearAllWatches() override;

        void PollChanges(std::vector<Event>& events) override;

    private:
        enum class RunLoopThreadEvent
        {
            CreateRunLoop,
            StartRunLoop,
            ExitThread,
        };

        FileWatcherConfig* m_pConfig = nullptr;

        std::mutex m_Mutex;
        std::thread m_RunLoopThread;
        int m_MainToRunLoopPipe[2] = {-1, -1};
        int m_RunLoopToMainPipe[2] = {-1, -1};

        std::unordered_set<fs::path, FsPathHasher> m_CanonicalDirectoryPaths;

        std::unique_ptr<FSEventStreamContext> m_pFsContext;
        FSEventStreamRef m_FsStream = nullptr;
        CFRunLoopRef m_CfRunLoop = nullptr;

        std::chrono::steady_clock::time_point m_LastPollTime = std::chrono::steady_clock::now();
        bool m_bGatheringEvents = false;

        std::vector<Event> m_PendingEvents;

        std::vector<fs::path> GetRootDirectories();

        bool CreateFsEventStream();
        bool StopFsEventStream();
        bool ExecuteRunLoopThreadEvent(RunLoopThreadEvent event);
        bool WaitForDoneFromRunThread();
        bool SendDoneToMainThread();
        void CreateRunLoopThread();

        static bool CanonicalPath(const fs::path& path, fs::path& canonicalPath);

        static void FsCallback(ConstFSEventStreamRef stream, void* pInfo, size_t nEvents, void* pEventPaths,
                const FSEventStreamEventFlags* pEventFlags, const FSEventStreamEventId* pEventIds);
    };
}
