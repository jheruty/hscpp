cmake_minimum_required(VERSION 3.10.0)

add_library(gl3w src/gl3w.c)

target_include_directories(gl3w PUBLIC include)
