cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(osu-cheese-framework STATIC
	"sed/windows/smart_handle.hpp"
	"sed/windows/smart_handle.cpp"
	"sed/windows/suspend_guard.hpp"
	"sed/windows/suspend_guard.cpp"

	"sed/console.hpp"
	"sed/console.cpp"
	"sed/memory.hpp"
	"sed/memory.cpp"
)

set_target_properties(osu-cheese-framework PROPERTIES LINKER_LANGUAGE CXX)
set(osu-cheese-framework-includes "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)