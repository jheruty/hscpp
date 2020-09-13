cmake_minimum_required(VERSION 3.10.0)

add_executable(simple-demo
    src/Main.cpp
    src/Printer.cpp

    include/simple-demo/Printer.h
    include/simple-demo/SimpleDemoData.h
)

target_include_directories(simple-demo PRIVATE include)

target_link_libraries(simple-demo hscpp)
