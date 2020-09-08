cmake_minimum_required(VERSION 3.10.0)

project(hscpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(hscpp
    $<$<BOOL:WIN32>:./src/CmdShell_win32.cpp>
    $<$<BOOL:WIN32>:./src/Compiler_win32.cpp>
    ./src/DependencyGraph.cpp
    ./src/FeatureManager.cpp
    ./src/FileParser.cpp
    $<$<BOOL:WIN32>:./src/FileWatcher_win32.cpp>
    ./src/FsPathHasher.cpp
    ./src/Hotswapper.cpp
    ./src/Log.cpp
    ./src/ModuleManager.cpp
    ./src/Platform.cpp
    ./src/Preprocessor.cpp
    ./src/ProtectedFunction.cpp
    ./src/Util.cpp

    ./include/hscpp/module/AllocationResolver.h
    ./include/hscpp/module/Constructors.h
    ./include/hscpp/module/GlobalUserData.h
    ./include/hscpp/module/IAllocator.h
    ./include/hscpp/module/ITracker.h
    ./include/hscpp/module/ModuleInterface.h
    ./include/hscpp/module/ModuleSharedState.h
    ./include/hscpp/module/PreprocessorMacros.h
    ./include/hscpp/module/Serializer.h
    ./include/hscpp/module/SwapInfo.h
    ./include/hscpp/module/Tracker.h

    ./include/hscpp/Callbacks.h
    $<$<BOOL:WIN32>:./include/hscpp/CmdShell_win32.h>
    $<$<BOOL:WIN32>:./include/hscpp/Compiler_win32.h>
    ./include/hscpp/DependencyGraph.h
    ./include/hscpp/Feature.h
    ./include/hscpp/FeatureManager.h
    ./include/hscpp/FileParser.h
    ./include/hscpp/Filesystem.h
    $<$<BOOL:WIN32>:./include/hscpp/FileWatcher_win32.h>
    ./include/hscpp/FsPathHasher.h
    ./include/hscpp/Hotswapper.h
    ./include/hscpp/ICmdShell.h
    ./include/hscpp/ICompiler.h
    ./include/hscpp/IFileWatcher.h
    ./include/hscpp/Log.h
    ./include/hscpp/ModuleManager.h
    ./include/hscpp/Platform.h
    ./include/hscpp/Preprocessor.h
    ./include/hscpp/ProtectedFunction.h
    ./include/hscpp/Util.h
)

target_include_directories(hscpp PUBLIC ./include)

target_compile_definitions(hscpp PRIVATE
    $<$<BOOL:WIN32>:HSCPP_PLATFORM_WIN32>
    $<$<BOOL:UNIX>:HSCPP_PLATFORM_UNIX>
)

option(HSCPP_BUILD_EXAMPLES "Enable building examples." ON)

if (HSCPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
