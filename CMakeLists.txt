cmake_minimum_required(VERSION 2.8)
project(Shinzui)

# -[ Macros and functions
include(cmake/Configure.cmake)
include(cmake/Macros.cmake)

# -[ Configuration
option(SHINZUI_USE_SYSTEM_BOOST "Use system installed Boost libraries" OFF)

# -[ Sources
add_subdirectory(src)
