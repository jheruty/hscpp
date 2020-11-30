# Creating a hot-swappable class

## Class declaration

Example of class declaration:
> HotSwapObject.h
```cpp
#pragma once

#include "hscpp/module/Tracker.h"

class HotSwapObject
{
    HSCPP_TRACK(HotSwapObject, "HotSwapObject");

public:
    hscpp_virtual ~HotSwapObject() = default;
    HotSwapObject();
    hscpp_virtual void Update();
};
```

First, `HSCPP_TRACK` is used to register our class with hscpp. The first argument must be the class name, ignoring any namespaces. The second argument must be a *unique* key, which will identify the object with hscpp's object tracking system. Using the name of the class, with any namespaces prepended, is a simple way to guarantee uniqueness. Keys are limited to 128 characters in length.

Next we see the `hscpp_virtual` keyword. This resolves to `virtual` when `HSCPP_DISABLE` is **not** defined, and to nothing when it is defined. As explained in [the how it works section](./1_how-it-works.md), functions must be virtual for hot-swapping to work as intended. If a function is only made virtual for the purposes for hot-swapping, hscpp_virtual can be used to remove the virtual call overhead in the final build.

For this example, our code will go into the Update method. The hscpp_virtual destructor is only present to suppress warnings.

## Class implementation
> HotSwapObject.cpp
```cpp
#include <iostream>
#include "HotSwapObject.h"

HotSwapObject::HotSwapObject()
{}

void HotSwapObject::Update()
{
    std::cout << "Hello, World!" << std::endl;
}
```

> Main.cpp
```cpp
#include "hscpp/Hotswapper.h"
#include "HotSwapObject.h"

int main()
{
    hscpp::Hotswapper swapper;

    auto path = hscpp::fs::path(__FILE__).parent_path();
    swapper.AddSourceDirectory(path);
    swapper.AddIncludeDirectory(path);

    HotSwapObject* pObj = swapper.Allocate<HotSwapObject>();

    while (true)
    {
        pObj->Update();
        swapper.Update();
    }
}
```

This implementation is close, but modifying `HotSwapObject` will result in a memory exception. Why?

When saving a change in HotSwapObject.cpp, hscpp will recompile its code into a new shared library. It will then delete all old instances of `HotSwapObject`, and replace them with new instances; in our case, we create a single instance in `main()`. When the old instance is deleted, `pObj` will be pointing to a deleted object.

[One way to fix this issue is by using `GlobalUserData`.](./4_global-user-data.md)