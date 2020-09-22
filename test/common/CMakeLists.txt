add_library(common
    src/FileUtil.cpp

    include/common/Common.h
    include/common/FileUtil.h
    include/common/Macros.h
    include/common/ValidateDataStructures.h
)

target_include_directories(common PUBLIC include)

target_link_libraries(common
    catch
    hscpp
)