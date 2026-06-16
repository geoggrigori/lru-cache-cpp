cmake_minimum_required(VERSION 3.14)

project(lru_cache_cpp
    VERSION 1.0.0
    DESCRIPTION "A header-only O(1) LRU cache for C++17"
    LANGUAGES CXX)

# Header-only library exposed as an INTERFACE target.
add_library(lru_cache INTERFACE)
add_library(lru_cache::lru_cache ALIAS lru_cache)

target_include_directories(lru_cache INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

target_compile_features(lru_cache INTERFACE cxx_std_17)

# Tests.
enable_testing()

add_executable(lru_cache_test tests/lru_cache_test.cpp)
target_link_libraries(lru_cache_test PRIVATE lru_cache)
target_include_directories(lru_cache_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
set_target_properties(lru_cache_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

add_test(NAME lru_cache_test COMMAND lru_cache_test)
