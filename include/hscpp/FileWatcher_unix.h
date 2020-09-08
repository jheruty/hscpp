#pragma once

#include "hscpp/IFileWatcher.h"

namespace hscpp
{

    class FileWatcher : public IFileWatcher
    {
    public:
        virtual bool AddWatch(const fs::path& directoryPath) override;
        virtual bool RemoveWatch(const fs::path& directoryPath) override;
        virtual void ClearAllWatches() override;

        virtual void SetPollFrequencyMs(int ms) override;
        virtual void PollChanges(std::vector<Event>& events) override;
    };

}