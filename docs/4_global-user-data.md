# Using GlobalUserData

## Sharing data across modules

As described in the [how it works section](./1_how-it-works.md), ordinary globals cannot be used with hscpp.

To share data across modules (newly compiled shared libraries), we can use hscpp's `GlobalUserData` interface. For instance:
> Global.h
```cpp
#pragma once

#include "HotSwapObject.h"

class Global
{
public:
    HotSwapObject* pObj = nullptr;
};
```

> Main.cpp
```cpp
#include "hscpp/Hotswapper.h"
#include "HotSwapObject.h"
#include "Global.h"

int main()
{
    Hotswapper swapper;

    Global global;
    swapper.SetGlobalUserData(&global);
    ...
}
```

Anywhere in the codebase, the `global` instance can be accessed with:
```cpp
hscpp::GlobalUserData::GetAs<Global>();
```

## [Fixing our previous example](./3_simple-hotswappable-class.md)

With `GlobalUserData` and the `Global` class, we can amend our previous implementations:

> HotSwapObject.cpp
```cpp
...
#include "Global.h"
...
HotSwapObject::HotSwapObject()
{
    hscpp::GlobalUserData::GetAs<Global>()->pObj = this;
}
...
```

> Main.cpp
```cpp
...
int main()
{
    hscpp::Hotswapper swapper;
    ...
    Global global;
    swapper.SetGlobalUserData(&global);

    global.pObj = swapper.Allocate<HotSwapObject>();

    while (true)
    {
        global.pObj->Update();
        swapper.Update();
    }
}

```

Now, whenever a new object instance is created, the global `pObj` pointer will be updated, and no memory fault will occur.

[Next, lets see how to serialize data and save state between runtime swaps.](./5_manage-state-between-swaps.md)