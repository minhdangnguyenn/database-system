cmake_minimum_required(VERSION 3.20)
project(hashtable)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# std::optional requires C++17; pinned to 20 to match the other projects in
# this repo. Without this, the compiler defaults to C++14 and <optional> is
# not declared at all.
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

# The template definitions live in hashtable.cpp, so the test includes that
# translation unit directly rather than linking against it.
add_executable(tests
    ${CMAKE_SOURCE_DIR}/test_hashtable.cpp
)
