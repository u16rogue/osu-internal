cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(osu-cheese-client SHARED
	"client_main.cpp"
	"client_main.hpp"
	"hooks.hpp"
	"hooks.cpp"
)

set_target_properties(osu-cheese-client PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(osu-cheese-client PRIVATE
	osu-cheese-framework
)

target_include_directories(osu-cheese-client PRIVATE
	${osu-cheese-framework-includes}
)
