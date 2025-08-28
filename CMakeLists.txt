# This CMakeLists.txt was ChatGPT'ed

cmake_minimum_required(VERSION 3.20)
project(byo-redis LANGUAGES CXX VERSION 0.1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTING "Build unit tests" ON)
option(ENABLE_ASAN "Enable AddressSanitizer" ON)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" ON)
option(ENABLE_TIDY "Enable clang-tidy if available" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warnings
if(MSVC)
  add_compile_options(/W4 /permissive-)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wnon-virtual-dtor)
endif()

# Optional ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

# Sanitizers
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Sanitizers)

# Library (core)
add_library(byoredis_core
  src/client.cpp
  src/server.cpp
)
target_include_directories(byoredis_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_features(byoredis_core PUBLIC cxx_std_23)
enable_sanitizers(byoredis_core)

# Executable
add_executable(byoredis src/main.cpp)
target_link_libraries(byoredis PRIVATE byoredis_core)
enable_sanitizers(byoredis)

# Tests
include(CTest)
if(BUILD_TESTING)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  )
  # For MSVC: avoid overriding parent’s /W flags
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  add_subdirectory(tests)
endif()
