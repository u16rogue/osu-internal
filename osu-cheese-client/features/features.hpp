#include "aim_assist.hpp"
#include "relax.hpp"
#include "esp.hpp"

namespace features
{
	template <class... fts>
	struct _features_pack
	{
		static auto on_tab_render() -> void
		{
			(fts::on_tab_render(), ...);
		}

		static auto on_wndproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, void * reserved) -> bool
		{
			return (fts::on_wndproc(hWnd, Msg, wParam, lParam, reserved) || ...);
		}

		static auto on_render() -> void
		{
			(fts::on_render(), ...);
		}
	};

	using feature = _features_pack<
		aim_assist,
		relax,
		esp
	>;
}