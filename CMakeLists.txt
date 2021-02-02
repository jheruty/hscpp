cmake_minimum_required(VERSION 3.10.0)

project(hscpp)

# Options:
#     - HSCPP_BUILD_EXAMPLES: Build demos in the examples directory.
#     - HSCPP_BUILD_TESTS: Build tests in the test directory.
#     - HSCPP_BUILD_EXTENSION_MEM: Build hscpp::mem extension, creating hscpp-mem library.
#     - HSCPP_USE_GHC_FILESYSTEM: Usg ghc filesystem in place of std::filesystem. This is done
#       implicitly if CMAKE_CXX_STANDARD < 17.
#     - HSCPP_DISABLE: Disable hscpp completely. Hotswapper function calls will have no effect,
#       and no object will be tracked.
option(HSCPP_BUILD_EXAMPLES "Enable building examples." ON)
option(HSCPP_BUILD_TESTS "Enable building tests." ON)
option(HSCPP_BUILD_EXTENSION_MEM "Enable building hscpp::mem extension." ON)
option(HSCPP_USE_GHC_FILESYSTEM "Use ghc filesystem as a substitute of std::filesystem." OFF)
option(HSCPP_DISABLE "Disable hscpp." OFF)

# List of source files compiled in every configuration.
list(APPEND HSCPP_SRC_FILES
    src/compiler/Compiler.cpp
    src/compiler/CompilerCmdLine_gcc.cpp
    src/compiler/CompilerInitializeTask_gcc.cpp
    src/module/Module.cpp
    src/preprocessor/Ast.cpp
    src/preprocessor/DependencyGraph.cpp
    src/preprocessor/Interpreter.cpp
    src/preprocessor/LangError.cpp
    src/preprocessor/Lexer.cpp
    src/preprocessor/Parser.cpp
    src/preprocessor/Preprocessor.cpp
    src/preprocessor/Variant.cpp
    src/preprocessor/VarStore.cpp
    src/Config.cpp
    src/Feature.cpp
    src/FeatureManager.cpp
    src/FsPathHasher.cpp
    src/Log.cpp
    src/ModuleManager.cpp
    src/Platform.cpp
    src/ProtectedFunction.cpp
    src/Util.cpp

    include/hscpp/cmd-shell/ICmdShell.h
    include/hscpp/cmd-shell/ICmdShellTask.h
    include/hscpp/compiler/Compiler.h
    include/hscpp/compiler/CompilerCmdLine_gcc.h
    include/hscpp/compiler/CompilerInitializeTask_gcc.h
    include/hscpp/compiler/ICompiler.h
    include/hscpp/compiler/ICompilerCmdLine.h
    include/hscpp/file-watcher/IFileWatcher.h
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
    include/hscpp/preprocessor/Ast.h
    include/hscpp/preprocessor/DependencyGraph.h
    include/hscpp/preprocessor/Interpreter.h
    include/hscpp/preprocessor/IPreprocessor.h
    include/hscpp/preprocessor/LangError.h
    include/hscpp/preprocessor/Lexer.h
    include/hscpp/preprocessor/Parser.h
    include/hscpp/preprocessor/Preprocessor.h
    include/hscpp/preprocessor/HscppRequire.h
    include/hscpp/preprocessor/Token.h
    include/hscpp/preprocessor/Variant.h
    include/hscpp/preprocessor/VarStore.h
    include/hscpp/Callbacks.h
    include/hscpp/Feature.h
    include/hscpp/FeatureManager.h
    include/hscpp/Filesystem.h
    include/hscpp/FsPathHasher.h
    include/hscpp/Hotswapper.h
    include/hscpp/Log.h
    include/hscpp/ModuleManager.h
    include/hscpp/Platform.h
    include/hscpp/ProtectedFunction.h
    include/hscpp/Util.h
)

# List of compile options in every configuration.
list(APPEND HSCPP_COMPILE_OPTIONS)

# List of compile definitions in every configuration.
list(APPEND HSCPP_COMPILE_DEFINITIONS
    HSCPP_ROOT_PATH="${CMAKE_CURRENT_SOURCE_DIR}"
    HSCPP_COMPILER_PATH="${CMAKE_CXX_COMPILER}"
    HSCPP_BUILD_PATH="${CMAKE_BINARY_DIR}"
)

# List of libraries to link in every configuration.
list(APPEND HSCPP_LINK_LIBRARIES)

# Additional compile definitions in Debug.
list(APPEND HSCPP_COMPILE_DEFINITIONS
    $<$<CONFIG:Debug>:HSCPP_DEBUG>
)

# OS-specific settings.
if (WIN32)
    list(APPEND HSCPP_SRC_FILES
        src/cmd-shell/CmdShell_win32.cpp
        src/compiler/CompilerCmdLine_msvc.cpp
        src/compiler/CompilerInitializeTask_msvc.cpp
        src/file-watcher/FileWatcher_win32.cpp

        include/hscpp/cmd-shell/CmdShell_win32.h
        include/hscpp/compiler/CompilerCmdLine_msvc.h
        include/hscpp/compiler/CompilerInitializeTask_msvc.h
        include/hscpp/file-watcher/FileWatcher_win32.h
    )

    list(APPEND HSCPP_COMPILE_DEFINITIONS
        HSCPP_PLATFORM_WIN32
        _CRT_SECURE_NO_WARNINGS
    )
elseif(APPLE)
    list(APPEND HSCPP_SRC_FILES
        src/cmd-shell/CmdShell_unix.cpp
        src/file-watcher/FileWatcher_apple.cpp

        include/hscpp/cmd-shell/CmdShell_unix.h
        include/hscpp/file-watcher/FileWatcher_apple.h
    )

    list(APPEND HSCPP_COMPILE_DEFINITIONS
        HSCPP_PLATFORM_APPLE
        HSCPP_PLATFORM_UNIX
    )

    find_library(CORE_SERVICES CoreServices)
    list(APPEND HSCPP_LINK_LIBRARIES
        ${CORE_SERVICES}
    )
elseif(UNIX)
    list(APPEND HSCPP_SRC_FILES
        src/cmd-shell/CmdShell_unix.cpp
        src/file-watcher/FileWatcher_unix.cpp

        include/hscpp/cmd-shell/CmdShell_unix.h
        include/hscpp/file-watcher/FileWatcher_unix.h
    )

    list(APPEND HSCPP_COMPILE_DEFINITIONS
        HSCPP_PLATFORM_UNIX
    )

    list(APPEND HSCPP_LINK_LIBRARIES
        dl
        uuid
    )
endif()

# Compiler-specific settings.
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    list(APPEND HSCPP_COMPILE_OPTIONS
        /permissive # Enforce standard compliance.
    )

    list(APPEND HSCPP_COMPILE_DEFINITIONS HSCPP_COMPILER_MSVC)
elseif (NOT CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC") # Exclude clang-cl.
    list(APPEND HSCPP_COMPILE_OPTIONS
        -Wall
        -Wextra
        -Wshadow
        -Wno-unused-function
    )
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") # cl
    list(APPEND HSCPP_COMPILE_DEFINITIONS HSCPP_COMPILER_MSVC)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC") # clang-cl
    list(APPEND HSCPP_COMPILE_OPTIONS
        /EHsc # Use standard exception handling.
    )

    list(APPEND HSCPP_COMPILE_DEFINITIONS HSCPP_COMPILER_CLANG_CL)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang") # clang
    list(APPEND HSCPP_COMPILE_DEFINITIONS
        HSCPP_COMPILER_CLANG
        HSCPP_COMPILER_GCC # clang has a gcc-like interface, and can be treated like g++ when invoked.
    )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU") # g++
    list(APPEND HSCPP_COMPILE_DEFINITIONS HSCPP_COMPILER_GCC)
else()
    message(WARNING "Compiler is not officially supported.")
endif()

# Determine C++ version to use. Default to C++17 if not defined.
if(NOT DEFINED CMAKE_CXX_STANDARD)
    # Set C++17 as default build target.
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
elseif(CMAKE_CXX_STANDARD LESS 11)
    message(FATAL_ERROR "hscpp requires C++11.")
endif()

list(APPEND HSCPP_COMPILE_DEFINITIONS
    HSCPP_CXX_STANDARD=${CMAKE_CXX_STANDARD}
)

message(STATUS "hscpp CMAKE_CXX_STANDARD set to ${CMAKE_CXX_STANDARD}.")

# Fallback to ghc filesystem if using C++ version below C++17, or if user requested it.
if (HSCPP_USE_GHC_FILESYSTEM OR (CMAKE_CXX_STANDARD LESS 17))
    if (CMAKE_CXX_STANDARD LESS 17)
        message(STATUS "hscpp CMAKE_CXX_STANDARD < 17, so using ghc filesystem rather than std::filesystem.")
    else()
        message(STATUS "Using ghc filesystem rather than std::filesystem.")
    endif()

    list(APPEND HSCPP_LINK_LIBRARIES ghc_filesystem)
    list(APPEND HSCPP_COMPILE_DEFINITIONS HSCPP_USE_GHC_FILESYSTEM)

    add_subdirectory(lib/filesystem-1.3.4)

    # Use standard conforming .native() calls on Win32 (wstring instead of UTF-8).
    target_compile_definitions(ghc_filesystem INTERFACE GHC_WIN_WSTRING_STRING_TYPE)
endif()

# Toggle if hscpp is enabled or disabled.
if (HSCPP_DISABLE)
    list(APPEND HSCPP_SRC_FILES src/Hotswapper_disabled.cpp)
    list(APPEND HSCPP_COMPILE_DEFINITIONS HSCPP_DISABLE)
else()
    list(APPEND HSCPP_SRC_FILES src/Hotswapper_enabled.cpp)
endif()

add_library(hscpp ${HSCPP_SRC_FILES})

target_include_directories(hscpp PUBLIC include)
target_compile_options(hscpp PUBLIC ${HSCPP_COMPILE_OPTIONS})
target_compile_definitions(hscpp PUBLIC ${HSCPP_COMPILE_DEFINITIONS})
target_link_libraries(hscpp ${HSCPP_LINK_LIBRARIES})

if (HSCPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (HSCPP_BUILD_TESTS)
    add_subdirectory(test)
endif()

if (HSCPP_BUILD_EXTENSION_MEM)
    add_subdirectory(extensions/mem)
endif()