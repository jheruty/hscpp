list(APPEND HSCPP_UNIT_TEST_SRC_FILES
    Main.cpp
    Test_CmdShell.cpp
    Test_Compiler.cpp
    Test_DependencyGraph.cpp
    Test_FeatureManager.cpp
    Test_FileWatcher.cpp
    Test_Interpreter.cpp
    Test_Lexer.cpp
    Test_Parser.cpp
    Test_Preprocessor.cpp
    Test_SwapInfo.cpp
    Test_VarStore.cpp
)

list(APPEND HSCPP_UNIT_TEST_LINK_LIBRARIES
    catch
    common
    hscpp
)

if (HSCPP_BUILD_EXTENSION_MEM)
    list(APPEND HSCPP_UNIT_TEST_SRC_FILES extensions/mem/Test_MemoryManager.cpp)
    list(APPEND HSCPP_UNIT_TEST_LINK_LIBRARIES hscpp-mem)
endif()

add_executable(unit-tests ${HSCPP_UNIT_TEST_SRC_FILES})
target_link_libraries(unit-tests ${HSCPP_UNIT_TEST_LINK_LIBRARIES})