# Managing state between hot-swaps

When a class is hot-swapped, a brand new instance of that class is created, calling its default constructor. This means that all of that class' data is reset to default. If it is desirable to save state between swaps, one can make use of the `Hscpp_SetSwapHandler` macro, which should be placed in the default constructor.

For example:
> HotSwapObject.cpp
```cpp
HotSwapObject::HotSwapObject()
{
    ...
    auto cb = [this](hscpp::SwapInfo& info) {
        switch (info.Phase())
        {
            case hscpp::SwapPhase::BeforeSwap:
                info.Serialize("Message", m_Message);
                break;
            case hscpp::SwapPhase::AfterSwap:
                info.Unserialize("Message", m_Message);
                break;
        }
    };

    Hscpp_SetSwapHandler(cb);
}
```

m_Message can be any type, so long as it is copyable. Every time hscpp performs a hot-swap, this callback will be run *twice*. First, it will be run by the old instance that is about to be deleted (BeforeSwap), and then it will be run by the new instance that will replace it (AfterSwap). The `SwapInfo` parameter can be used to transfer state between both instances.

Note that this callback can be used for general system plumbing, such as deregistering an old object from a system before it is deleted, and then registering the new one after it has been created.

Since it is so common to serialize state data, the above can be abbreviated with:
```cpp
HotSwapObject::HotSwapObject()
{
    ...
    auto cb = [this](hscpp::SwapInfo& info) {
        info.Save("Message", m_Message);
    };

    Hscpp_SetSwapHandler(cb);
}
```

In some situations, some constructor code should be skipped if the object is being created as part of a hot-swap. In these cases, one can check if the object is currently being swapped with the `Hscpp_IsSwapping` macro.

[Next, lets see how we can create a custom memory allocator.](./6_custom-memory-allocator.md)