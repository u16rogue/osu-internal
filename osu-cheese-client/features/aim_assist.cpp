#include "aim_assist.hpp"

#include <imgui.h>
#include <sed/macro.hpp>

#include "../game.hpp"
#include "../sdk/osu_vec.hpp"

#include "../manager/gamefield_manager.hpp"

#include <format>
#include <algorithm>
#include <string>

auto features::aim_assist::on_tab_render() -> void
{
	if (!ImGui::BeginTabItem("Aim assist"))
		return;

	ImGui::Checkbox("Enabled", &enable);
	ImGui::Checkbox("Silent", &silent);
	ImGui::InputInt("[DEBUG] max_tick_sample", &max_tick_sample);
	ImGui::InputInt("[DEBUG] max_reach_time_offset (late and over)", &max_reach_time_offset);
	ImGui::SliderFloat("opx Distance", &distance_fov, 0.f, 300.f);
	ImGui::SliderFloat("directional_fov", &directional_fov, 0.f, 180.f);
	ImGui::InputInt("[DEBUG] count_direction_sampling", &count_direction_sampling);

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

	run_sampling();
	move_aim_assist();
	check_aim_assist();

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

	// fov test
	#if 0
	if (game::pp_phitobject)
	{
		for (const auto & phit : game::pp_phitobject)
		{
			draw->AddCircleFilled(phit->position.field_to_view(), 4.f, (game::pp_viewpos_info->pos.view_to_field().distance(phit->position)) > distance_fov ? 0xFF00FFFF : 0xFF00FF00);
		}
	}
	#endif

	const sdk::vec2 & curpnt = use_set ? set_point : game::pp_viewpos_info->pos.view_to_field();

	stext(ImVec2(20.f, 100.f), ("Avg. Velocity: " + std::to_string(velocity)).c_str());

	auto opx_dist = game::pp_viewpos_info->pos.view_to_field().distance(curpnt);
	stext(ImVec2(20.f, 112.f), ("p cl2sv dd: " + std::to_string(opx_dist) + " opx").c_str());

	const auto max_p2p_distance = sdk::vec2(0.f, 0.f).distance(sdk::vec2(512.f, 384.f)); // srfgwsvergvserg
	stext(ImVec2(20.f, 124.f), ("p cl2sv d%: " + std::to_string(opx_dist / max_p2p_distance * 100.f) + "%").c_str());

	// Draw player position
	if (enable)
	{
		draw->AddCircleFilled(curpnt.field_to_view(), 4.f, 0xFF00FF00);
		draw->AddCircleFilled(target_point.field_to_view(), 4.f, 0xFF00FFFF);
		draw->AddCircle(game::pp_viewpos_info->pos, distance_fov * manager::game_field::field_ratio, 0xFFFFFFFF); // opx distance visualization

		const auto dir_vis_len = 36.f; //std::clamp(max_p2p_distance * (velocity / max_p2p_distance), 0.f, max_p2p_distance);
		draw->AddLine(game::pp_viewpos_info->pos, game::pp_viewpos_info->pos.forward(direction, dir_vis_len), 0xFFFFFFFF, 3.f); // visualize player direction
	}
}

auto features::aim_assist::on_osu_set_raw_coords(sdk::vec2 * raw_coords) -> void
{
	collect_sampling(*raw_coords);

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

auto features::aim_assist::check_aim_assist() -> void
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

	sdk::hitobject * target {};

	// find target
	for (const auto & ho : game::pp_phitobject)
	{
		if (ho->is_hit)
			continue;

		target = ho;
		break;
	}

	if (!target)
		return;

	auto player_field_pos = game::pp_viewpos_info->pos.view_to_field();

	// check fov
	if (player_field_pos.distance(target->position) > distance_fov)
	{
		// ~disable when we go out of fov~
		// just keep it despite going out of fov
		// if ()

		return;
	}

	// check if player direction

	// predict if player will reach hitobject in the given time based off the current average velocity

}

auto features::aim_assist::move_aim_assist() -> void
{

}

auto features::aim_assist::collect_sampling(const sdk::vec2 & cpoint) -> void
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

auto features::aim_assist::run_sampling() -> void
{
	if (point_records)
	{
		int count_vel {};
		//int count_dir {};
		float total_vel {};
		float total_dir {};
		point_record * last = nullptr;

		//sdk::vec2 last_direction {};

		// i think this should iterate in reverse
		for (auto & prt : *point_records)
		{
			if (prt.tick < GetTickCount() - max_tick_sample)
				continue;

			if (!last)
			{
				last = &prt;
				continue;
			}

			auto last_in_field = last->point.view_to_field();
			auto now_in_field = prt.point.view_to_field();

			// velocity
			total_vel += last_in_field.distance(now_in_field);

			#if 0
			// direction
			if (count_dir < count_direction_sampling)
			{
				if (last_direction == sdk::vec2())
				{
					last_direction = last_in_field.normalize_towards(now_in_field);
				}
				else
				{
					auto curr_direction = last_in_field.normalize_towards(now_in_field);
					total_dir += last_direction.vec2vec_angle(curr_direction) / 2;
					++count_dir;
				}
			}
			#endif

			++count_vel;
			last = &prt;
		}

		// direction = sdk::vec2::from_deg(total_dir / count_direction_sampling);

		if (const auto pr_cnt = point_records->size(); pr_cnt > count_direction_sampling)
		{
			direction = (*point_records)[pr_cnt - count_direction_sampling].point.normalize_towards(point_records->back().point);
		}

		velocity  = total_vel ? total_vel / count_vel : 0.f;
	}
}
