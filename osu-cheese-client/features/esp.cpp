#include "esp.hpp"

#include <imgui.h>
#include <sed/macro.hpp>

#include "../game.hpp"
#include "../manager/beatmap_manager.hpp"

auto features::esp::on_tab_render() -> void
{
	if (!ImGui::BeginTabItem("ESP"))
		return;

	ImGui::Checkbox("Hit object timer", &timer);
	OC_IMGUI_HOVER_TXT("Shows a countdown timer towards the next hit object.");
	ImGui::Checkbox("Hit object distance", &distance);
	OC_IMGUI_HOVER_TXT("Shows the distance between the player cursor and the next hit object (also your current FOV value).");
	ImGui::Checkbox("Hit object tracer", &tracer);
	OC_IMGUI_HOVER_TXT("Draws a line from the players cursor to the next hit object.");

	ImGui::EndTabItem();
}

auto features::esp::on_wndproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, void * reserved) -> bool
{
	return false;
}

auto features::esp::on_render() -> void
{
	if (!game::pp_info_player->async_complete || !manager::beatmap::loaded() || game::pp_info_player->is_replay_mode || !game::p_game_info->is_playing)
		return;

	auto [ho, i] = manager::beatmap::get_coming_hitobject();
	if (!ho)
		return;

	auto draw = ImGui::GetBackgroundDrawList();
	std::string esptxt; 

	if (timer)
		esptxt.append("TIME: " + std::to_string(ho->time - game::p_game_info->beat_time) + "\n");

	if (distance)
		esptxt.append("DST: " + std::to_string(ho->coords.distance(manager::game_field::mousepos.view_to_field())) + "\n");

	if (tracer)
		draw->AddLine(manager::game_field::mousepos, ho->coords.field_to_view(), 0xFFFFFFFF);

	if (!esptxt.empty())
		draw->AddText(ho->coords.field_to_view(), 0xFFFFFFFF, esptxt.c_str());
}
