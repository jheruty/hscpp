cmake_minimum_required(VERSION 3.10.0)

project(hscpp)

list(APPEND SRC_FILES
    src/Compiler_win32.cpp
    src/Compiler_clang.cpp
    src/Compiler_gcc.cpp
    src/DependencyGraph.cpp
    src/Feature.cpp
    src/FeatureManager.cpp
    src/FileParser.cpp
    src/FsPathHasher.cpp
    src/Hotswapper.cpp
    src/Log.cpp
    src/ModuleManager.cpp
    src/Platform.cpp
    src/Preprocessor.cpp
    src/ProtectedFunction.cpp
    src/Util.cpp

    include/hscpp/module/AllocationResolver.h
    include/hscpp/module/CompileTimeString.h
    include/hscpp/module/Constructors.h
    include/hscpp/module/GlobalUserData.h
    include/hscpp/module/IAllocator.h
    include/hscpp/module/ITracker.h
    include/hscpp/module/ModuleInterface.h
    include/hscpp/module/ModuleSharedState.h
    include/hscpp/module/PreprocessorMacros.h
    include/hscpp/module/Serializer.h
    include/hscpp/module/SwapInfo.h
    include/hscpp/module/Tracker.h

    include/hscpp/Compiler_win32.h
    include/hscpp/Compiler_clang.h
    include/hscpp/Compiler_gcc.h
    include/hscpp/Callbacks.h
    include/hscpp/DependencyGraph.h
    include/hscpp/Feature.h
    include/hscpp/FeatureManager.h
    include/hscpp/FileParser.h
    include/hscpp/Filesystem.h
    include/hscpp/FsPathHasher.h
    include/hscpp/Hotswapper.h
    include/hscpp/ICmdShell.h
    include/hscpp/ICompiler.h
    include/hscpp/IFileWatcher.h
    include/hscpp/Log.h
    include/hscpp/ModuleManager.h
    include/hscpp/Platform.h
    include/hscpp/Preprocessor.h
    include/hscpp/ProtectedFunction.h
    include/hscpp/Util.h
)

list(APPEND COMPILE_DEFINITIONS)
list(APPEND LINK_LIBRARIES)

if (WIN32)
    list(APPEND SRC_FILES
        src/CmdShell_win32.cpp
        src/Compiler_win32.cpp
        src/FileWatcher_win32.cpp

        include/hscpp/CmdShell_win32.h
        include/hscpp/FileWatcher_win32.h
    )

    list(APPEND COMPILE_DEFINITIONS
        HSCPP_PLATFORM_WIN32
    )
elseif(APPLE)
    list(APPEND SRC_FILES
        src/CmdShell_unix.cpp
        src/Compiler_unix.cpp
        src/FileWatcher_apple.cpp

        include/hscpp/CmdShell_unix.h
        include/hscpp/FileWatcher_apple.h
    )

    list(APPEND COMPILE_DEFINITIONS
        HSCPP_PLATFORM_APPLE
        HSCPP_PLATFORM_UNIX
    )

    find_library(CORE_SERVICES CoreServices)
    list(APPEND LINK_LIBRARIES
        ${CORE_SERVICES}
    )
elseif(UNIX)
    list(APPEND SRC_FILES
        src/CmdShell_unix.cpp
        src/Compiler_unix.cpp
        src/FileWatcher_unix.cpp

        include/hscpp/CmdShell_unix.h
        include/hscpp/Compiler_unix.h
        include/hscpp/FileWatcher_unix.h
    )

    list(APPEND COMPILE_DEFINITIONS
        HSCPP_PLATFORM_UNIX
    )

    list(APPEND LINK_LIBRARIES
        dl
        uuid
    )
endif()

option(HSCPP_BUILD_EXAMPLES "Enable building examples." ON)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    # Set C++17 as default build target.
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
elseif(CMAKE_CXX_STANDARD LESS 11)
    message(FATAL_ERROR "hotswap-cpp requires C++11.")
endif()

message(STATUS "hscpp CMAKE_CXX_STANDARD set to ${CMAKE_CXX_STANDARD}.")

if (CMAKE_CXX_STANDARD LESS 17)
    message(STATUS "hscpp CMAKE_CXX_STANDARD < 17, so using ghc filesystem rather than std::filesystem.")

    list(APPEND LINK_LIBRARIES ghc_filesystem)
    list(APPEND COMPILE_DEFINITIONS HSCPP_USE_GHC_FILESYSTEM)

    add_subdirectory(lib/filesystem-1.3.4)
endif()

add_library(hscpp ${SRC_FILES})

target_include_directories(hscpp PUBLIC include)
target_compile_definitions(hscpp PUBLIC ${COMPILE_DEFINITIONS})
target_link_libraries(hscpp ${LINK_LIBRARIES})

if (HSCPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()