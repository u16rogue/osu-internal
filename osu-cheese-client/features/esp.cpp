#include "esp.hpp"

#include <imgui.h>
#include <sed/macro.hpp>

#include "../game.hpp"
#include <string>

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

	ImGui::Separator();
	ImGui::Checkbox("[DEBUG] Is hit", &dbg_ishit);
	
	ImGui::EndTabItem();
}

auto features::esp::on_wndproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, void * reserved) -> bool
{
	return false;
}

auto features::esp::on_render() -> void
{
	if (!game::pp_info_player->async_complete || !game::pp_phitobject || game::pp_info_player->is_replay_mode || !game::p_game_info->is_playing)
		return;

	auto draw = ImGui::GetBackgroundDrawList();

	if (game::pp_phitobject)
	{
		int x {};
		for (const auto & ho : game::pp_phitobject)//int i = 0; i < game::pp_phitobject.count(); ++i)
		{
			if (dbg_ishit)
				draw->AddCircleFilled(ho->position.field_to_view(), 8.f, ho->is_hit ? 0xFF00FFFF : 0xFFFF00FF);

			// draw->AddCircle(ho->position.field_to_view(), game::pp_phitobject->hitobjectmanager->beatmap->circle_size(), 0xFF00FFFF);
		}
	}

	auto [ho, i] = game::pp_phitobject.get_coming_hitobject(game::p_game_info->beat_time);
	if (!ho)
		return;

	std::string esptxt;

	if (timer)
		esptxt.append("TIME: " + std::to_string(ho->time.start - game::p_game_info->beat_time) + "\n");

	if (distance)
		esptxt.append("DST: " + std::to_string(ho->position.distance(game::pp_viewpos_info->pos.view_to_field())) + "\n");

	if (tracer)
		draw->AddLine(game::pp_viewpos_info->pos, ho->position.field_to_view(), 0xFFFFFFFF);

	if (!esptxt.empty())
		draw->AddText(ho->position.field_to_view(), 0xFFFFFFFF, esptxt.c_str());
}

auto features::esp::on_osu_set_raw_coords(sdk::vec2 * raw_coords) -> void
{
}

auto features::esp::osu_set_field_coords_rebuilt(sdk::vec2 * out_coords) -> void
{
}
