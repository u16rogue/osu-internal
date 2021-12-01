cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(osu-cheese-client SHARED
	"client_main.cpp"
	"client_main.hpp"
	"hooks.hpp"
	"hooks.cpp"
	"game.hpp"
	"game.cpp"
	"sdk/info_struct.hpp"
	"sdk/info_player.hpp"
	"sdk/osu_vec.hpp"
	"sdk/gamefield.hpp"
	"sdk/gamefield.cpp"
	"utils/beatmap.hpp"
	"utils/beatmap.cpp"
	"sdk/osu_file.hpp"
	"features/assist.hpp"
	"features/assist.cpp"	
)

set_target_properties(osu-cheese-client PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(osu-cheese-client PRIVATE
	osu-cheese-framework
	imgui
	gl3w
)

target_include_directories(osu-cheese-client PRIVATE
	${osu-cheese-framework-includes}
	${imgui-includes}
	${gl3w-includes}
)
