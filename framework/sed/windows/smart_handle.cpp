#include "smart_handle.hpp"

sed::smart_handle::smart_handle()
	: hnd(nullptr) {}

sed::smart_handle::smart_handle(HANDLE hnd_)
	: hnd(hnd_) {}

sed::smart_handle::~smart_handle()
{
	if (this->hnd)
		CloseHandle(this->hnd);
}

sed::smart_handle::smart_handle(smart_handle && other)
{
	if (this->hnd)
		CloseHandle(this->hnd);

	this->hnd = other.hnd;
	other.hnd = nullptr;
}

sed::smart_handle::operator HANDLE() const noexcept
{
	return this->hnd;
}

sed::smart_handle::operator bool() const noexcept
{
	return this->hnd != nullptr;
}
