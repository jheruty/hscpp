cmake_minimum_required(VERSION 3.10.0)

add_executable(runtime-require-demo
    src/BaseState.cpp
    src/BaseStateUtil.cpp
    src/Main.cpp
    src/Printer.cpp
    src/PrintHello.cpp
    src/PrintVariant.cpp

    include/runtime-require-demo/BaseState.h
    include/runtime-require-demo/BaseStateUtil.h
    include/runtime-require-demo/Printer.h
    include/runtime-require-demo/PrintHello.h
    include/runtime-require-demo/PrintVariant.h
)

target_include_directories(runtime-require-demo PRIVATE include)

target_compile_definitions(runtime-require-demo PRIVATE PREPROCESSOR_DEMO)

target_link_libraries(runtime-require-demo
    hscpp
    hscpp-example-utils
)
