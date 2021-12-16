cmake_minimum_required(VERSION 3.21)

project(BitmapIndex LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "src/*.cpp")
get_filename_component(SERVER_CPP src/server.cpp ABSOLUTE)
list(REMOVE_ITEM SOURCES ${SERVER_CPP})

add_library(BitmapIndex ${SOURCES})

enable_testing()
add_executable(UnitTest "test/test.cpp")
target_link_libraries(UnitTest BitmapIndex gtest gtest_main)

add_executable(Benchmark "benchmark/benchmark.cpp")
target_link_libraries(Benchmark BitmapIndex benchmark)

add_executable(Server "src/server.cpp")
target_link_libraries(Server BitmapIndex)

add_subdirectory("extern/benchmark" "extern/googletest")
include_directories("src/include" "extern/googletest/include" "extern/benchmark/include")
