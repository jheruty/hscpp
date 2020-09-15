#pragma once

#include <vector>

#include "hscpp/Filesystem.h"

namespace hscpp
{

    class IFileWatcher
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
            fs::path filePath;
        };

        virtual ~IFileWatcher() = default;

        virtual bool AddWatch(const fs::path& directoryPath) = 0;
        virtual bool RemoveWatch(const fs::path& directoryPath) = 0;
        virtual void ClearAllWatches() = 0;

        virtual void SetPollFrequencyMs(int ms) = 0;
        virtual void PollChanges(std::vector<Event>& events) = 0;
    };

}