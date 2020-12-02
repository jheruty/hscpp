# Creating a custom memory allocator

## The IAllocator interface

To make a custom memory allocator, the custom allocator must inherit from hscpp's `IAllocator` interface. There are three functions that must be overridden:
- `Hscpp_Allocate(uint64_t size)`:
    - Called when an object is allocated *outside* of a hot-swap. In other words, this object is not replacing an old instance.
- `Hscpp_AllocateSwap(uint64_t previousId, uint64_t size)`:
    - Called when an object is allocated *during* a hot-swap, and is replacing an old instance of the object.
- `Hscpp_FreeSwap(uint8_t* pMemory)`:
    - Called when an object is freed *during* a hot-swap, and will be replaced by a new instance of the object.

 Both Allocate methods return an `AllocationInfo` struct. The struct's `pMemory` member should contain a pointer to the allocated memory, whose size is at least that of the `size` parameter. The `id` member is an optional member, which maps an integral id to the allocated memory.

 When `AllocateSwap` is called, the `previousId` parameter contains the same id returned by `Hscpp_FreeSwap`, when the old object instance is deleted.

 The reason this is useful, is that it enables `AllocateSwap` to allocate brand new memory, but still use the same id as the old object instance. A separate smart pointer object can use these ids to reference memory, instead of using traditional pointers. This allows object instances to be hot-swapped, without needing to do things like update a global pointer, as was done [in a previous example.](./4_global-user-data)

 A sample demonstration of this concept can be found in the [memory-allocation-demo.](../examples/memory-allocation-demo)

## The AllocationResolver

When creating a custom memory allocator, the custom allocator *must* use hscpp's AllocationResolver to ensure the correct constructors are called. The `Hotswapper` provides a `GetAllocationResolver` method to get a pointer to the `AllocationResolver`, allowing it to be passed into a custom memory manager.

The `AllocationResolver` contains an `Allocate<T>()` method, which will always construct a class with its most recently compiled constructor.

[Next we will look at how to specify dependencies with the hscpp preprocessor.](7_preprocessor-requires.md)
