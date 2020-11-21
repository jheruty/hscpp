add_library(common
    src/UpdateLoop.cpp
    src/Util.cpp
    src/Validation.cpp

    include/common/Common.h
    include/common/Macros.h
    include/common/Typedefs.h
    include/common/UpdateLoop.h
    include/common/Util.h
    include/common/Validation.h
)

target_include_directories(common PUBLIC include)

target_link_libraries(common
    catch
    hscpp
)