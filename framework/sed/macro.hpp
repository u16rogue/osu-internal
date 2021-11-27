#pragma once

#define OC_PAD(n) char _pad_ __LINE__ _ __COUNTER__ [n]

#define OC_UNS_PAD(n, type, name) \
	struct \
	{ \
		OC_PAD(n); \
		type name; \
	};