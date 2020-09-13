cmake_minimum_required(VERSION 3.10.0)

add_executable(memory-allocation-demo
    src/Main.cpp
    src/TrackedPrinter.cpp
    src/UntrackedPrinter.cpp

    include/memory-allocation-demo/IUpdateable.h
    include/memory-allocation-demo/TrackedPrinter.h
    include/memory-allocation-demo/UntrackedPrinter.h
)

target_include_directories(memory-allocation-demo PRIVATE include)

target_link_libraries(memory-allocation-demo
    hscpp
    hscpp-example-utils
)
