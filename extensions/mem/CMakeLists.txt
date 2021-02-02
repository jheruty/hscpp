add_library(hscpp-mem
    src/MemoryManager.cpp

    include/hscpp/mem/IMemoryManager.h
    include/hscpp/mem/MemoryManager.h
    include/hscpp/mem/Ref.h
)

target_include_directories(hscpp-mem
    PUBLIC
        include
    PRIVATE
        ../../include
)