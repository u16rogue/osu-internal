cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(osu-cheese-client SHARED
	"client_main.cpp"
	"hooks.hpp"
	"hooks.cpp"
	"game.hpp"
	"game.cpp"
	"sdk/info_struct.hpp"
	"sdk/info_player.hpp"
	"sdk/osu_vec.hpp"
	"sdk/osu_vec.cpp"
	"manager/gamefield_manager.hpp"
	"manager/gamefield_manager.cpp"
	"utils/beatmap.hpp"
	"utils/beatmap.cpp"
	"sdk/osu_file.hpp"
	"menu.hpp"
	"menu.cpp"
	"manager/beatmap_manager.hpp"
	"manager/beatmap_manager.cpp"
	"features/aim_assist.hpp"
	"features/aim_assist.cpp"
	"features/relax.hpp"
	"features/relax.cpp"
	"features/esp.hpp"
	"features/esp.cpp"
	"features/features.hpp"
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
