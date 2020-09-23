add_library(common
    src/FileUtil.cpp
    src/UpdateLoop.cpp

    include/common/Common.h
    include/common/FileUtil.h
    include/common/Macros.h
    include/common/Typedefs.h
    include/common/ValidateDataStructures.h
    include/common/UpdateLoop.h
)

target_include_directories(common PUBLIC include)

target_link_libraries(common
    catch
    hscpp
)