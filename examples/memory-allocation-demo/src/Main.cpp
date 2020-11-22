#include <iostream>
#include <thread>

#include "hscpp/Filesystem.h"
#include "hscpp/Hotswapper.h"
#include "hscpp/Util.h"
#include "hscpp-example-utils/Ref.h"
#include "hscpp-example-utils/MemoryManager.h"

#include "memory-allocation-demo/TrackedPrinter.h"
#include "memory-allocation-demo/UntrackedPrinter.h"
#include "memory-allocation-demo/IUpdateable.h"

const static hscpp::fs::path DEMO_PATH = hscpp::util::GetHscppExamplesPath() / "memory-allocation-demo";

int main()
{
    hscpp::Hotswapper swapper;

    auto srcPath = DEMO_PATH / "src";
    auto includePath = DEMO_PATH / "include";

    swapper.AddSourceDirectory(srcPath);
    swapper.AddIncludeDirectory(includePath);

    // After recompiling, the newly compiled module is capable of constructing new versions of of an
    // object. However, only hscpp knows about this new constructor. For this reason, all allocations
    // should go through hscpp, such that the correct constructor is called.
    //
    // The Hotswapper's GetAllocationResolver method returns an hscpp::AllocationResolver, which can
    // be used to allocate memory in a way that will work with hscpp.
    Ref<MemoryManager> memoryManager = MemoryManager::Create(swapper.GetAllocationResolver());

    // You can optionally set a memory allocator. If provided, hscpp will call Allocate, AllocateSwap,
    // and Free, as per the hscpp::IAllocator interface. If no allocator is provided, hscpp uses
    // the standard 'new' and 'delete'.
    swapper.SetAllocator(&memoryManager);

    // In the simple-demo example, we rely on using global memory shared across all modules in order
    // to handle when hscpp deletes an old object and creates a new one. Instead we can use the 'Ref'
    // concept, in which the Ref stores an unchanging id that refers to that object, regardless of the
    // object's memory location. This allows an object to change location in memory without references
    // to the object breaking.
    //
    // For Refs to work, a custom allocator must be provided, which tracks object ids. See the
    // MemoryManager class for details.
    Ref<TrackedPrinter> trackedPrinter = memoryManager->Allocate<TrackedPrinter>();

    int trackedCounter = 0;
    trackedPrinter->Init(++trackedCounter);

    // Untracked objects are allocated the same as tracked objects, but they won't be recompiled
    // when their source code changes.
    Ref<UntrackedPrinter> untrackedPrinter = memoryManager->Allocate<UntrackedPrinter>();

    int untrackedCounter = 0;
    untrackedPrinter->Init(++untrackedCounter);

    std::vector<Ref<IUpdatable>> m_UpdatableObjects;
    m_UpdatableObjects.push_back(trackedPrinter.As<IUpdatable>());
    m_UpdatableObjects.push_back(untrackedPrinter.As<IUpdatable>());

    while (true)
    {
        swapper.Update();

        for (auto& ref : m_UpdatableObjects)
        {
            ref->Update();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }


    for (auto& ref : m_UpdatableObjects)
    {
        memoryManager->Free(ref);
    }

    m_UpdatableObjects.clear();
}
