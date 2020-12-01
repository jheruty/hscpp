# How it works

When using hscpp, a high-level understanding of how the system works is helpful, as it explains the library's capabilities and limitations.

At it's core, hscpp works by compiling shared libraries (.dll or .so, depending on the platform), and linking them into a running executable. In hscpp, swappable code must be a `struct` or a `class` that uses the `HSCPP_TRACK` macro in its declaration. Internally, `HSCPP_TRACK` registers instances of that class with hscpp's object tracking system.

When a file is modified, hscpp will recompile that file into a shared library. If that file contains a class using `HSCPP_TRACK`, hscpp will know that new instances of that class should be created using code from the new shared library. In addition, old instances of that class should be deleted, and replaced with new instances using the new code.

The most important thing to note here is that the new shared library is separate from the original code. This means that one cannot share ordinary statics and globals across the program and expect hot-swapping to continue to work (linker errors would ensue). As a workaround, hscpp provides the `GlobalUserData` class for sharing data across all modules.

Another important note is that, even after swapping, the old code is still present in the executable. Only hscpp knows how to map a new object instance to its most recently added constructor. For this reason, one must perform all memory allocations through hscpp, via the `AllocationResolver::Allocate<T>` method.

Finally, after swapping, only virtual methods will be able to call into the newly compiled code. This is because normal function calls will have their address baked into the binary, as opposed to virtual calls which will consult the class' vtable at runtime.

[Next, lets look at how to get started by creating an instance of the `Hotswapper` class.](./2_hotswapper-basics.md)