#include "hscpp/FileWatcher_unix.h"

namespace hscpp
{
    bool FileWatcher::AddWatch(const fs::path& directoryPath)
    {
        return true;
    }

    bool FileWatcher::RemoveWatch(const fs::path& directoryPath)
    {
        return true;
    }

    void FileWatcher::ClearAllWatches()
    {

    }

    void FileWatcher::SetPollFrequencyMs(int ms)
    {

    }

    void FileWatcher::PollChanges(std::vector<Event>& events)
    {

    }

}