cmake_minimum_required(VERSION 3.20)
project(LRUCache)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Explicitly list your files instead of GLOB_RECURSE
file(GLOB SOURCES
    "${CMAKE_SOURCE_DIR}/*.cpp"
)

# Exclude anything inside build folder
list(FILTER SOURCES EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/build/.*")

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(LRUCache ${SOURCES})
