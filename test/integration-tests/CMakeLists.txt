add_executable(integration-tests
    Main.cpp
)

target_link_libraries(integration-tests
    catch
    hscpp
)