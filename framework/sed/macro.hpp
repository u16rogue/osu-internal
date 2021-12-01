#pragma once

#define OC_GLUE(x, y) x ## y
#define OC_MGLUE(x, y) OC_GLUE(x, y)

#define OC_PAD(n) char OC_MGLUE(_pad_, __LINE__) [n]

#define OC_UNS_PAD(n, type, name) \
	struct \
	{ \
		OC_PAD(n); \
		type name; \
	};

#if defined(OSU_CHEESE_DEBUG_BUILD) && OSU_CHEESE_DEBUG_BUILD
	#include <cstdio>
	#include <iostream>
	#define DEBUG_PRINTF(fmt, ...) printf(fmt, __VA_ARGS__)
	#define DEBUG_WPRINTF(fmt, ...) wprintf(fmt, __VA_ARGS__)
#else
	#define DEBUG_PRINTF(fmt, ...)
	#define DEBUG_WPRINTF(fmt, ...)
#endif