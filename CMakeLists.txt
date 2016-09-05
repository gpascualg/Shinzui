cmake_minimum_required(VERSION 2.8)
project(Shinzui)

# -[ Macros and functions
include(cmake/Configure.cmake)
include(cmake/Macros.cmake)
ResetAllTargets()

# -[ Configuration
option(SHINZUI_USE_SYSTEM_BOOST "Use system installed Boost libraries" OFF)
option(SHINZUI_BUILD_TESTS "Build Shinzui unit tests" ON)
option(SHINZUI_DEBUG "Build in debug mode" OFF)

# -[ Build type
set(BUILD_TYPE_STR "Choose the type of
    build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug
    Release RelWithDebInfo MinSizeRel.")

if (SHINZUI_DEBUG)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING ${BUILD_TYPE_STR} FORCE)
elseif (CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING ${BUILD_TYPE_STR} FORCE)
else ()
    # Should we force Release?
    set(CMAKE_BUILD_TYPE Release CACHE STRING ${BUILD_TYPE_STR} FORCE)
endif()

# -[ C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# -[ Sources
add_subdirectory(src)

# -[ Tests
add_subdirectory(tests)
