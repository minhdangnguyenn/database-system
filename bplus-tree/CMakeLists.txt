cmake_minimum_required(VERSION 3.20)
project(dbms)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─── Include headers from include/ folder ───
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

# ─── Collect all source files from src/ ───
file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# ─── Main executable (benchmark) ───
# add_executable(dbms
#     ${SOURCES}
#     ${CMAKE_SOURCE_DIR}/benchmark/benchmark.cpp
# )

# ─── Tests executable ───
add_executable(tests
     ${SOURCES}
     ${CMAKE_SOURCE_DIR}/tests/include/test-lru.h
     ${CMAKE_SOURCE_DIR}/tests/include/test-buffer-pool.h
     ${CMAKE_SOURCE_DIR}/tests/test-lru.cpp
     ${CMAKE_SOURCE_DIR}/tests/test-buffer-pool.cpp
     ${CMAKE_SOURCE_DIR}/tests/main.cpp
)
