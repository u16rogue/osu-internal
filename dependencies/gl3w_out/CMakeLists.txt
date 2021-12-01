# no idea how to use gl3w's cmake file... TODO: figure it out
# Generate this dep by running: py gl3w_gen.py --root ../gl3w_out/
# Note: rename the generated c file to a cpp
# TODO: not ghetto this and implement properly to no longer require editing the source and header files + includes

cmake_minimum_required(VERSION 3.18)

add_library(gl3w
	"src/gl3w.cpp"
)

set(gl3w-includes "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)
set_target_properties(gl3w PROPERTIES LINKER_LANGUAGE CXX)