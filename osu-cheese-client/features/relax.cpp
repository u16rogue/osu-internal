#include "relax.hpp"

#include <imgui.h>

#include <sed/macro.hpp>
#include "../sdk/osu_vec.hpp"
#include "../sdk/osu_file.hpp"
#include "../manager/beatmap_manager.hpp"
#include "../game.hpp"

auto features::relax::on_tab_render() -> void
{
	if (!ImGui::BeginTabItem("Relax"))
		return;

	ImGui::Checkbox("Relax", &enable);
	OC_IMGUI_HOVER_TXT("Auto click hit objects.");
	ImGui::SliderFloat("Edge field", &edge, 0.f, 300.f);
	OC_IMGUI_HOVER_TXT("Edge field area where relax is enabled. (0 = Global)");
	ImGui::SliderInt("Timing offset", &offset, -10000, 10000);
	OC_IMGUI_HOVER_TXT("Hit timing offset from the hit object's time.");
	ImGui::Checkbox("Randomize timing offset", &offsetrand);
	OC_IMGUI_HOVER_TXT("Randomizes the timing offset and uses the current timing offset as the range.");

	ImGui::EndTabItem();
}

auto features::relax::on_wndproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, void * reserved) -> bool
{
	static const sdk::hit_object * filter_ho = nullptr;

	if (Msg == WM_MOUSEMOVE || !enable || !manager::beatmap::loaded() || !game::pp_info_player->async_complete || game::pp_info_player->is_replay_mode || !game::p_game_info->is_playing)
		return false;

	auto [ho, i] = manager::beatmap::get_coming_hitobject();
	if (!ho)
		return false;

	if (offset >= 0)
		--ho;

	if (ho->time + offset <= game::p_game_info->beat_time || ho == filter_ho)
		return false;

	filter_ho = ho;

	INPUT inp[2] { 0 };
	inp[0].type = INPUT_MOUSE;
	inp[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

	inp[1].type = INPUT_MOUSE;
	inp[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

	SendInput(2, inp, sizeof(INPUT));

	return false;
}

auto features::relax::on_render() -> void
{
}
