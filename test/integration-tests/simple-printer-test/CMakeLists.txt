add_executable(simple-printer-test
    src/Main.cpp
    src/Printer.cpp

    include/simple-printer-test/GlobalData.h
    include/simple-printer-test/Printer.h
)

target_include_directories(simple-printer-test PRIVATE include)

target_link_libraries(simple-printer-test
    integration-test-log
    hscpp
)