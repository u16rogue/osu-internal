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
	ImGui::Checkbox("Silent", &silent);
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

			draw->AddLine(last->point, prt.point, 0xFF00FF00);
			last = &prt;
		}
	}

	stext(ImVec2(20.f, 100.f), ("Avg. Velocity: " + std::to_string(velocity)).c_str());

	auto opx_dist = game::pp_viewpos_info->pos.view_to_field().distance(set_point);
	stext(ImVec2(20.f, 112.f), ("p cl2sv dd: " + std::to_string(opx_dist) + " opx").c_str());

	const auto max_p2p_distance = sdk::vec2(0.f, 0.f).distance(sdk::vec2(512.f, 384.f)); // srfgwsvergvserg
	stext(ImVec2(20.f, 124.f), ("p cl2sv d%: " + std::to_string(opx_dist / max_p2p_distance * 100.f) + "%").c_str());

	run_velocity_sampling();
	run_aim_assist();

	// Draw player position
	if (enable)
		draw->AddCircleFilled(set_point.field_to_view(), 8.f, 0xFF00FF00);
}

auto features::aim_assist::on_osu_set_raw_coords(sdk::vec2 * raw_coords) -> void
{
	collect_velocity_sampling(*raw_coords);

	if (enable && !silent && use_set)
		*raw_coords = set_point;

	return;
}

auto features::aim_assist::osu_set_field_coords_rebuilt(sdk::vec2 * out_coords) -> void
{
	if (enable && silent && use_set)
		*out_coords = set_point;

	return;
}

auto features::aim_assist::run_aim_assist() -> void
{
	static bool last_enable_state = false;

	if (enable != last_enable_state)
	{
		last_enable_state = enable;
	}

	if (!enable)
		return;

	if (!game::pp_phitobject || !game::pp_info_player->async_complete || game::pp_info_player->is_replay_mode || !game::p_game_info->is_playing)
		return;

	// check fov

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
