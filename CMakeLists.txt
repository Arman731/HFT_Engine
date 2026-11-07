cmake_minimum_required(VERSION 3.20)

project(HFT_Engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Main application
add_executable(HFT_Engine
    apps/simulator/main.cpp
)

target_include_directories(HFT_Engine PRIVATE include)

# Enable testing
enable_testing()

# Download GoogleTest
include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

FetchContent_MakeAvailable(googletest)

# Test executable
add_executable(HFT_Tests
    tests/unit/order_validation_test.cpp
    tests/unit/matching_engine_test.cpp
)

target_include_directories(HFT_Tests PRIVATE include)

target_link_libraries(HFT_Tests PRIVATE GTest::gtest_main)

include(GoogleTest)

gtest_discover_tests(HFT_Tests)

