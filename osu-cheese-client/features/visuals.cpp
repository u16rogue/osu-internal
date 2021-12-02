#include "visuals.hpp"

#include <imgui.h>
#include "../manager/gamefield_manager.hpp"
#include "../manager/beatmap_manager.hpp"
#include "../game.hpp"
#include <string>

auto features::visuals::render() -> void
{
	if (!game::pp_info_player->async_complete || game::pp_info_player->is_replay_mode || !game::p_game_info->is_playing)
		return;

	auto [ho, i] = manager::beatmap::get_coming_hitobject();
	if (!ho)
		return;

	auto draw = ImGui::GetBackgroundDrawList();
	auto [ho_x, ho_y] = manager::game_field::f2v(ho->x, ho->y);
	std::string esptxt; 
	
	if (ho_timer)
		esptxt.append("TIME: " + std::to_string(ho->time - game::p_game_info->beat_time) + "\n");

	if (ho_distance)
		esptxt.append("DST: " + std::to_string(manager::game_field::dist_view2obj(manager::game_field::mouse_x, manager::game_field::mouse_y, *ho)) + "\n");

	if (ho_tracer)
		draw->AddLine(ImVec2 { float(manager::game_field::mouse_x), float(manager::game_field::mouse_y) }, ImVec2 { float(ho_x), float(ho_y) }, 0xFFFFFFFF);

	if (!esptxt.empty())
		draw->AddText(ImVec2 { float(ho_x), float(ho_y) }, 0xFFFFFFFF, esptxt.c_str());
}
