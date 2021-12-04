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
	std::string esptxt; 
	
	if (ho_timer)
		esptxt.append("TIME: " + std::to_string(ho->time - game::p_game_info->beat_time) + "\n");

	if (ho_distance)
		esptxt.append("DST: " + std::to_string(ho->coords.distance(manager::game_field::mousepos.view_to_field())) + "\n");

	if (ho_tracer)
		draw->AddLine(manager::game_field::mousepos, ho->coords.field_to_view(), 0xFFFFFFFF);

	if (!esptxt.empty())
		draw->AddText(ho->coords.field_to_view(), 0xFFFFFFFF, esptxt.c_str());
}
