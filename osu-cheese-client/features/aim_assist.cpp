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
	ImGui::Checkbox("do_prediction", &do_prediction);
	ImGui::InputInt("[DEBUG] max_tick_sample", &max_tick_sample);
	ImGui::InputInt("[DEBUG] max_reach_time_offset (late and over)", &max_reach_time_offset);
	ImGui::SliderFloat("opx Distance", &distance_fov, 0.f, 300.f);
	ImGui::SliderFloat("directional_fov", &directional_fov, 0.f, 180.f);
	ImGui::SliderFloat("t_val", &t_val, 0.f, 1.f);

	ImGui::Combo("Pathing mode", reinterpret_cast<int *>(&path_mode), "Linear\0Curve thing (bezier)\0");

	ImGui::NewLine();

	ImGui::Text("[DEBUG] Use for directional sampling:");
	ImGui::SameLine();
	if (ImGui::Button(ds_use_count ? "count_direction_sampling" : "dst_direction_sampling"))
		ds_use_count = !ds_use_count;

	ImGui::InputInt("[DEBUG] count_direction_sampling", &count_direction_sampling);
	ImGui::InputFloat("[DEBUG] dst_direction_sampling", &dst_direction_sampling);

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

	const auto stext = [&](const ImVec2 & pos, const char * str, ImU32 color = 0xFFFFFFFF) -> void
	{
		draw->AddText(ImVec2(pos.x + 1.f, pos.y + 1.f), 0xFF000000, str);
		draw->AddText(pos, color, str);
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

			draw->AddLine(last->point, prt.point, 0xFF00FF00, 3.f);
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
	const auto & curpos = game::pp_viewpos_info->pos;

	stext(ImVec2(20.f, 88.f), "AIM ASSIST LOCK", locking ? 0xFF00FF00 : 0xFF0000FF);

	stext(ImVec2(20.f, 100.f), ("Avg. Velocity: " + std::to_string(velocity)).c_str());

	auto opx_dist = curpos.view_to_field().distance(curpnt);
	stext(ImVec2(20.f, 112.f), ("p cl2sv dd: " + std::to_string(opx_dist) + " opx").c_str());

	const auto max_p2p_distance = sdk::vec2(0.f, 0.f).distance(sdk::vec2(512.f, 384.f)); // srfgwsvergvserg
	stext(ImVec2(20.f, 124.f), ("p cl2sv d%: " + std::to_string(opx_dist / max_p2p_distance * 100.f) + "%").c_str());

	stext(ImVec2(20.f, 136.f), ("dst per 1 tick: " + std::to_string((1.f / static_cast<float>(max_tick_sample)) * velocity)).c_str());

	// Draw player position
	if (enable)
	{
		draw->AddCircleFilled(curpnt.field_to_view(), 4.f, 0xFF00FF00);
		//draw->AddCircleFilled(aa_end_point.field_to_view(), 4.f, 0xFF00FFFF);
		draw->AddCircle(curpos, distance_fov * manager::game_field::field_ratio, 0xFFFFFFFF); // opx distance visualization

		const auto dir_vis_len = 60.f; //std::clamp(max_p2p_distance * (velocity / max_p2p_distance), 0.f, max_p2p_distance);
		draw->AddLine(game::pp_viewpos_info->pos, game::pp_viewpos_info->pos.forward(direction, dir_vis_len), 0xFFFFFFFF, 3.f); // visualize player direction

		// visualize directional fov
		const auto dir_ang = direction.from_norm_to_deg();
		
		const auto a1 = curpos.forward(sdk::vec2::from_deg(dir_ang - directional_fov), dir_vis_len);
		draw->AddLine(curpos, a1, 0xFFFFFFFF, 3.f);

		const auto a2 = curpos.forward(sdk::vec2::from_deg(dir_ang + directional_fov), dir_vis_len);
		draw->AddLine(curpos, a2, 0xFFFFFFFF, 3.f);
	}
}

auto features::aim_assist::on_osu_set_raw_coords(sdk::vec2 * raw_coords) -> void
{
	collect_sampling(*raw_coords);

	if (enable && !silent && use_set)
		*raw_coords = set_point.field_to_view();

	return;
}

auto features::aim_assist::osu_set_field_coords_rebuilt(sdk::vec2 * out_coords) -> void
{
	if (enable && silent && use_set)
		*out_coords = set_point;

	return;
}

auto features::aim_assist::predict_time_to_point(const sdk::vec2 start, const sdk::vec2 end) -> float
{
	const auto ms_per_opx = (1.f / velocity) * max_tick_sample;
	const auto time_to_target = start.distance(end) * ms_per_opx;

	return time_to_target;
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

	struct _reset_values
	{
		~_reset_values()
		{
			if (should)
			{
				locking = false;
				last_lock = nullptr;
			}
		}

		bool should { true };
	} reset_values;

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

	if (do_prediction)
	{
		// doing an early prediction cause i wanna see if it actually works
		auto _pred = predict_time_to_point(player_field_pos, target->position);
		const auto predicted_time = game::p_game_info->beat_time + _pred;
		const auto prelim = target->time.start - max_reach_time_offset;
		const auto postlim = target->time.start + max_reach_time_offset;
		const bool in_time = predicted_time >= prelim && predicted_time <= postlim;

		const bool in_time_no_lower = predicted_time <= postlim;
		ImGui::GetBackgroundDrawList()->AddText(target->position.field_to_view(), in_time_no_lower ? 0xFF00FF00 : 0xFF0000FF, ("prediction: " + std::to_string(_pred) + "ms").c_str());

		// predict if player will reach hitobject in the given time based off the current average velocity
		// const auto ms_per_opx = (1.f / velocity) * max_tick_sample;
		// const auto time_to_target = dst_to_target * ms_per_opx;

		if (!in_time)
			return;
	}

	auto dst_to_target = player_field_pos.distance(target->position);
	const bool in_distance = dst_to_target <= distance_fov;
	if (!in_distance)
		return;

	// check if player direction
	const bool in_direction = player_field_pos.normalize_towards(target->position).vec2vec_angle(direction) <= directional_fov;
	if (!in_direction)
		return;

	reset_values.should = false;

	if (target == last_lock)
		return;

	// lock to target point
	aa_start_point = player_field_pos;
	aa_end_point   = target->position;
	aa_start_time  = game::p_game_info->beat_time;
	aa_end_time    = target->time.start;
	last_lock      = target;
	locking = true;
}

auto features::aim_assist::extrap_to_point(const sdk::vec2 & start, const sdk::vec2 & end, const float & t, const float & rate) -> sdk::vec2
{
	const auto distance = start.distance(end);
	const auto cur = distance * std::sinf((rate * std::numbers::pi_v<float>) / 2);

	return start.forward_towards(end, cur);
}

auto features::aim_assist::move_aim_assist() -> void
{
	const auto & cur_time = game::p_game_info->beat_time;
	if (locking && cur_time <= aa_end_time)
	{
		use_set = true;
		const auto norm_time = cur_time - aa_start_time;
		const auto end_time = aa_end_time - aa_start_time;
		set_point = extrap_to_point(aa_start_point, aa_end_point, t_val, (float)norm_time / (float)end_time);
	}
	else
	{
		use_set = false;
	}
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
		float total_vel {};
		point_record * last = nullptr;

		bool dir_set = false;

		const auto npnt = point_records->size();
		for (int i = npnt - 1; i != -1; --i)
		{
			auto & prt = (*point_records)[i];

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

			// TODO: do better sampling shit here
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

			if (!ds_use_count)
			{
				if (const auto & latest = point_records->back(); !dir_set && latest.point.distance(prt.point) >= dst_direction_sampling)
				{
					direction = prt.point.normalize_towards(latest.point);
					dir_set = true;
				}
			}

			++count_vel;
			last = &prt;
		}

		// direction = sdk::vec2::from_deg(total_dir / count_direction_sampling);
		if (ds_use_count)
		{
			if (const auto pr_cnt = point_records->size(); pr_cnt > count_direction_sampling)
			{
				direction = (*point_records)[pr_cnt - count_direction_sampling].point.normalize_towards(point_records->back().point);
			}
		}

		velocity  = total_vel ? total_vel / count_vel : 0.f;
	}
}
