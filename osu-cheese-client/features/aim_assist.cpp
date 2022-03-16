#include "aim_assist.hpp"

#include <imgui.h>
#include <sed/macro.hpp>

#include "../game.hpp"
#include "../sdk/osu_vec.hpp"

#include "../manager/gamefield_manager.hpp"

#include <algorithm>
#include <string>

auto features::aim_assist::on_tab_render() -> void
{
	if (!ImGui::BeginTabItem("Aim assist"))
		return;

	ImGui::Checkbox("Enabled", &enable);
	ImGui::InputInt("[DEBUG] Max tick sample", &max_tick_sample);

	ImGui::EndTabItem();
}

auto features::aim_assist::on_wndproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, void * reserved) -> bool
{
	return false;
}

auto features::aim_assist::on_render() -> void
{
	if (!enable)
		return;

	auto draw = ImGui::GetBackgroundDrawList();

	const auto stext = [&](const ImVec2 & pos, const char * str) -> void
	{
		draw->AddText(ImVec2(pos.x + 1.f, pos.y + 1.f), 0xFF000000, str);
		draw->AddText(pos, 0xFFFFFFFF, str);
	};

	// HACK: DEBUG ! REMOVE !
	if (point_records)
	{
		point_record * last = nullptr;
		for (auto & prt : *point_records)
		{
			if (prt.tick < GetTickCount() - max_tick_sample)
				continue;

			if (!last)
			{
				last = &prt;
				continue;
			}

			draw->AddLine(last->point, prt.point, 0xFF0000FF);
			last = &prt;
		}
	}

	stext(ImVec2(20.f, 100.f), ("Avg. Velocity: " + std::to_string(velocity)).c_str());

	run_velocity_sampling();

	if (!game::pp_phitobject || !game::pp_info_player->async_complete || game::pp_info_player->is_replay_mode)
		return;

}

auto features::aim_assist::on_osu_set_raw_coords(sdk::vec2 * raw_coords) -> void
{
	collect_velocity_sampling(*raw_coords);

	return;
}

auto features::aim_assist::osu_set_field_coords_rebuilt(sdk::vec2 * out_coords) -> void
{
	return;
}

auto features::aim_assist::run_aim_assist() -> void
{
	if (!enable)
		return;

	if (!game::pp_phitobject || !game::pp_info_player->async_complete || game::pp_info_player->is_replay_mode || !game::p_game_info->is_playing)
		return;

	

}

auto features::aim_assist::collect_velocity_sampling(const sdk::vec2 & cpoint) -> void
{
	// retarded initialization bug fix
	if (!point_records)
	{
		std::deque<point_record> asdf {};
		point_records = std::move(asdf);
	}

	const auto current_tick = GetTickCount();
	point_records->push_back({ cpoint, current_tick });

	// cleanup old ticks
	for (;;)
	{
		if (point_records->empty() || point_records->front().tick > current_tick - max_tick_sample)
			break;

		point_records->pop_front();
	}
}

auto features::aim_assist::run_velocity_sampling() -> void
{
	if (point_records)
	{
		int count {};
		float total {};
		point_record * last = nullptr;
		for (auto & prt : *point_records)
		{
			if (prt.tick < GetTickCount() - max_tick_sample)
				continue;

			if (!last)
			{
				last = &prt;
				continue;
			}

			total += last->point.view_to_field().distance(prt.point.view_to_field());
			last = &prt;
			++count;
		}

		velocity = total ? total / count : 0.f;
	}
}
