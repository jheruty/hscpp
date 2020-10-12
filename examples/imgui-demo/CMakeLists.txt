cmake_minimum_required(VERSION 3.10.0)

add_executable(imgui-demo
    src/Main.cpp
    src/Widget.cpp

    include/imgui-demo/Globals.h
    include/imgui-demo/Widget.h
)

target_include_directories(imgui-demo PRIVATE include)

target_link_libraries(imgui-demo
    hscpp
    hscpp-example-utils
    imgui
)

add_subdirectory(lib)