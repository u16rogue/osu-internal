cmake_minimum_required(VERSION 3.18)

project(osu-cheese CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Libraries
add_subdirectory("framework")
add_subdirectory("dependencies/minhook")

# Projects
option(BUILD_CLIENT "Build's the client library (DLL)." ON)
option(BUILD_LOADER "Build's the loader executable, this depends on the CLIENT project." ON)

# Force client if loader build is enabled
if (BUILD_LOADER)
    set(BUILD_CLIENT ON)
endif ()

if (BUILD_LOADER)
    add_subdirectory("osu-cheese-client")
endif ()

if (BUILD_LOADER)
    add_subdirectory("osu-cheese-loader")
endif ()