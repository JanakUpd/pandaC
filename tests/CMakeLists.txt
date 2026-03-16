cmake_minimum_required(VERSION 3.14)
project(Tests)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(unit_tests matchPattern_test.cpp)

target_link_libraries(unit_tests GTest::gtest_main compiler)
