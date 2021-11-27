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