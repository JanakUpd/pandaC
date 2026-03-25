cmake_minimum_required(VERSION 3.14)
project(Tests)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark testing" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "Disable benchmark install" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

enable_testing()

add_executable(unit_tests lexer_test.cpp)
target_link_libraries(unit_tests PRIVATE GTest::gtest_main compiler)

add_executable(pandaC_benchmarks
        lexer_benchmark.cpp
        ../src/compiler/lexer.cpp
)

target_include_directories(pandaC_benchmarks PRIVATE ../include/compiler ../include/notifier)

target_link_libraries(pandaC_benchmarks PRIVATE benchmark::benchmark benchmark::benchmark_main)
