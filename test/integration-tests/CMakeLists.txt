add_executable(integration-tests
    IntegrationTest.cpp
    Main.cpp

    IntegrationTest.h
)

target_link_libraries(integration-tests
    catch
    common
    integration-test-log
    hscpp
)

add_subdirectory(integration-test-log)
add_subdirectory(simple-printer-test)

add_dependencies(integration-tests simple-printer-test)