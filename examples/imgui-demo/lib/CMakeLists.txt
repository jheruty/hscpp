cmake_minimum_required(VERSION 3.10.0)

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)

# Game projects are likely to have already included these libraries.
if (NOT TARGET gl3w)
    add_subdirectory(gl3w)
endif()

if (NOT TARGET glfw)
    add_subdirectory(glfw)
endif()

if (NOT TARGET imgui)
    add_subdirectory(imgui)
endif()