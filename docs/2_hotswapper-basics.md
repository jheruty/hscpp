# Creating a Hotswapper

## Setting up the Hotswapper
The `Hotswapper` is the main interface into the hscpp library. To get started, create a `Hotswapper` instance:

```cpp
#include "hscpp/Hotswapper.h"

int main()
{
    hscpp::Hotswapper swapper;
}
```

The `Hotswapper` needs to be told where a project's source files are located, via `AddSourceDirectory` and `AddIncludeDirectory`. For example:

```cpp
{ ...
    swapper.AddSourceDirectory("path/to/src");
    swapper.AddIncludeDirectory("path/to/include");
}
```

This will cause hscpp to watch the "path/to/src" directory for file changes, and search for headers in "path/to/include". If a file within "path/to/src" is changed, a recompilation will be triggered. Note that directories passed to `AddIncludeDirectory` are not monitored for changes. If header file changes should also trigger recompilation, the header folder should additionally be added as a source directory.

In addition to `AddSourceDirectory` and `AddIncludeDirectory`, one can add:
- `AddLibraryDirectory`
    - Add additional directories in which to search for libraries.
- `AddLibrary`
    - Add additional libraries to link against.
- `AddPreprocessorDefinition`
    - Add additional preprocessor definitions.
- `AddCompileOption`
    - Add additional compile option.
- `AddLinkOption`
    - Add additional linker options.
- `AddForceCompiledSourceFile`
    - Add a source file to force-include into every compilation.

All these functions return an integral handle to the appended option, which can be passed into the matching Remove function to delete it.

## Updating the Hotswapper
After creating a Hotswapper, it must be placed in an update loop. For example:
```cpp
{...
    while (true)
    {
        swapper.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
```

Note that the update frequency can be relatively relaxed; here we are only updating every 100ms.

## Allocating memory
As mentioned in the [how it works section](./1_how-it-works.md), all memory should be allocated via hscpp. For example, to allocate a class called `HotSwapObject`, we would use:
```cpp
HotSwapObject* pObj = swapper.GetAllocationResolver()->Allocate<HotSwapObject>();
```

The object will be allocated with `new` and can be freed normally with `delete`. Alternatively, one can provide a [custom memory allocator](./6_custom-memory-allocator.md).

The `AllocationResolver` can be stored directly as a pointer:
```cpp
hscpp::AllocationResolver* pAllocationResolver = swapper.GetAllocationResolver();
```

This allows it to be passed around your system for memory allocations in hot-swappable classes. Due to the way hscpp is structured, a reference to the `Hotswapper` will cause a runtime compilation failure if used in hot-swappable classes.

[Next, lets look at how to create a hot-swappable class.](./3_simple-hotswappable-class.md)