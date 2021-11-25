cmake_minimum_required(VERSION 3.18)

project(osu-cheese-loader CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(osu-cheese-loader
    "loader_main.cpp"
    "loader_main.hpp"
)

target_link_libraries(osu-cheese-loader PRIVATE osu-cheese-client) # Force dependency to auto build client on run
target_link_libraries(osu-cheese-loader PRIVATE osu-cheese-framework)
target_include_directories(osu-cheese-loader PRIVATE ${osu-cheese-framework-includes})