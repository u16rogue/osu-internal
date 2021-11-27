cmake_minimum_required(VERSION 3.18)

add_library(imgui STATIC
	"imgui.h"
	"imgui.cpp"
	"imconfig.h"
	"imgui_impl_opengl3.cpp"
	"imgui_impl_opengl3.h"
	"imgui_impl_opengl3_loader.h"
	"imgui_impl_win32.cpp"
	"imgui_impl_win32.h"
	"imgui_internal.h"
	"imgui_tables.cpp"
	"imgui_widgets.cpp"
	"imstb_rectpack.h"
	"imstb_textedit.h"
	"imstb_truetype.h"
)

set(imgui-includes "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)