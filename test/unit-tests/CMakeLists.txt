add_executable(unit-tests
    Main.cpp
)

target_link_libraries(unit-tests
    catch
    hscpp
)