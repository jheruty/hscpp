add_executable(unit-tests
    Main.cpp
    Test_FileParser.cpp
    Test_FileWatcher.cpp
)

target_link_libraries(unit-tests
    catch
    common
    hscpp
)