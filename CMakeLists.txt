cmake_minimum_required(VERSION 3.10.0)

project(hscpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(hscpp
    ./src/CmdShell.cpp
    ./src/Compiler.cpp
    ./src/DependencyGraph.cpp
    ./src/FeatureManager.cpp
    ./src/FileParser.cpp
    ./src/FileWatcher.cpp
    ./src/FsPathHasher.cpp
    ./src/Hotswapper.cpp
    ./src/Log.cpp
    ./src/ModuleManager.cpp
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
    ./include/hscpp/CmdShell.h
    ./include/hscpp/Compiler.h
    ./include/hscpp/DependencyGraph.h
    ./include/hscpp/Feature.h
    ./include/hscpp/FeatureManager.h
    ./include/hscpp/FileParser.h
    ./include/hscpp/FileWatcher.h
    ./include/hscpp/FsPathHasher.h
    ./include/hscpp/Hotswapper.h
    ./include/hscpp/Log.h
    ./include/hscpp/ModuleManager.h
    ./include/hscpp/Platform.h
    ./include/hscpp/Preprocessor.h
    ./include/hscpp/ProtectedFunction.h
    ./include/hscpp/Util.h
)

target_include_directories(hscpp PUBLIC ./include)

option(BUILD_EXAMPLES "Enable building examples." OFF)

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
