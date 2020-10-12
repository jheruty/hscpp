cmake_minimum_required(VERSION 3.10.0)

project(hscpp-example-utils)

add_library(hscpp-example-utils
    src/MemoryManager.cpp
    src/Variant.cpp

    include/hscpp-example-utils/IMemoryManager.h
    include/hscpp-example-utils/MemoryManager.h
    include/hscpp-example-utils/Ref.h
    include/hscpp-example-utils/Variant.h
)

target_link_libraries(hscpp-example-utils hscpp)

target_include_directories(hscpp-example-utils PUBLIC include)
