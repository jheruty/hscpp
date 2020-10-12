cmake_minimum_required(VERSION 3.10.0)

# ImGui demo can be difficult to compile on some systems due to reliance on 3rd-party dependencies.
# For example, one must have OpenGL devel on their machine.
option(HSCPP_BUILD_IMGUI_DEMO "Enable build of ImGui demo." ON)

add_subdirectory(hscpp-example-utils)
add_subdirectory(dependent-compilation-demo)
add_subdirectory(memory-allocation-demo)
add_subdirectory(runtime-require-demo)
add_subdirectory(simple-demo)

if (HSCPP_BUILD_IMGUI_DEMO)
    add_subdirectory(imgui-demo)
endif()
