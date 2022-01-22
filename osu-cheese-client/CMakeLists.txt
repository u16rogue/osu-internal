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
	"menu.hpp"
	"menu.cpp"
	"features/aim_assist.hpp"
	"features/aim_assist.cpp"
	"features/relax.hpp"
	"features/relax.cpp"
	"features/esp.hpp"
	"features/esp.cpp"
	"features/features.hpp"
	"sdk/position_info.hpp"
	"sdk/raw_info.hpp"
	"sdk/wnd_info.hpp"
	"oc_client.hpp"
	"oc_client.cpp"
	"sdk/player_keys.hpp"
	"sdk/hitobject.hpp"
	"sdk/hitobject.cpp"
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
