#include "aim_assist.hpp"

#include <imgui.h>
#include <sed/macro.hpp>

#include "../game.hpp"
#include "../sdk/osu_vec.hpp"

#include "../manager/gamefield_manager.hpp"

#include <algorithm>

auto features::aim_assist::on_tab_render() -> void
{
	if (!ImGui::BeginTabItem("Aim assist"))
		return;
}

auto features::aim_assist::on_wndproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, void * reserved) -> bool
{
	return false;
}

auto features::aim_assist::on_render() -> void
{
	if (!enable || !game::pp_phitobject || !game::pp_info_player->async_complete || game::pp_info_player->is_replay_mode)
		return;

}

auto features::aim_assist::on_osu_set_raw_coords(sdk::vec2 * raw_coords) -> void
{
	return;
}

auto features::aim_assist::osu_set_field_coords_rebuilt(sdk::vec2 * out_coords) -> void
{
	return;
}

auto features::aim_assist::run_aim_assist(sdk::vec2 * pcoords) -> void
{
	if (!enable || !game::pp_phitobject || !game::pp_info_player->async_complete || game::pp_info_player->is_replay_mode || !game::p_game_info->is_playing)
		return;
}
