cmake_minimum_required(VERSION 3.10.0)

add_executable(dependent-compilation-demo
    src/Main.cpp
    src/MathUtil.cpp
    src/MathUtilExtra.cpp
    src/Printer1.cpp
    src/Printer2.cpp
    src/StringUtil.cpp

    include/dependent-compilation-demo/MathUtil.h
    include/dependent-compilation-demo/Printer1.h
    include/dependent-compilation-demo/Printer2.h
    include/dependent-compilation-demo/StringUtil.h
)

target_include_directories(dependent-compilation-demo PRIVATE include)

target_link_libraries(dependent-compilation-demo hscpp)
