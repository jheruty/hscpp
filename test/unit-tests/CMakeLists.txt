add_executable(unit-tests
    Main.cpp
    Test_CmdShell.cpp
    Test_Compiler.cpp
    Test_FeatureManager.cpp
    Test_FileParser.cpp
    Test_FileWatcher.cpp
)

target_link_libraries(unit-tests
    catch
    common
    hscpp
)