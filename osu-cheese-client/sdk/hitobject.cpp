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

auto sdk::ho_vector::begin() -> hitobject **
{
	return &this->container->hitobjects[0];
}

auto sdk::ho_vector::end() -> hitobject **
{
	return &this->container->hitobjects[this->count];
}

auto sdk::pp_phitobject_t::begin() -> hitobject **
{
	return (*this->ptr)->ho2->ho1->ho_vec->begin();
}

auto sdk::pp_phitobject_t::end() -> hitobject **
{
	return (*this->ptr)->ho2->ho1->ho_vec->end();
}

auto sdk::pp_phitobject_t::get_coming_hitobject(std::uint32_t time) -> std::pair<hitobject *, int>
{
	return (*this->ptr)->ho2->ho1->ho_vec->get_coming_hitobject(time);
}

auto sdk::pp_phitobject_t::count() -> std::uint32_t
{
	return (*this->ptr)->ho2->ho1->ho_vec->count;
}

auto sdk::pp_phitobject_t::operator[](int index) -> hitobject *
{
	return (*this->ptr)->ho2->ho1->ho_vec->container->hitobjects[index];
}

sdk::pp_phitobject_t::operator bool() const noexcept
{
	// not gonna macro this one
	if (!this->ptr)
		return false;

	auto p1 = *this->ptr;
	if (!p1)
		return false;

	auto p2 = p1->ho2;
	if (!p2)
		return false;

	auto p3 = p2->ho1;
	if (!p3)
		return false;

	auto p4 = p3->ho_vec;
	if (!p4)
		return false;

	auto p5 = p4->container;
	if (!p5)
		return false;

	return true;
}

auto sdk::beatmapbase::circle_size() -> float
{
	return 54.4f - 4.48f * DifficultyCircleSize;
}
