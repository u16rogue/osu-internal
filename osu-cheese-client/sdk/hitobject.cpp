#include "hitobject.hpp"

auto sdk::ho_vector::get_coming_hitobject(std::uint32_t time) -> std::pair<hitobject *, int>
{
	for (int i = 0; i < count; ++i)
	{
		if (time <= container->hitobjects[i]->time.start && (i == 0 || time > container->hitobjects[i - 1]->time.start))
			return std::make_pair(container->hitobjects[i], i);
	}

	return std::make_pair(nullptr, -1);
}