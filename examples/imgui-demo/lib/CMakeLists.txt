cmake_minimum_required(VERSION 3.10.0)

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)

add_subdirectory(gl3w)
add_subdirectory(glfw)
add_subdirectory(imgui)
