#pragma once

#include "Logger.h"

inline static Utilities::Logger s_Logger("Assert");

#define BIT(x) 1 << x
#define BIND_CALLBACK(function) \
	[this](auto&&... args) -> decltype(auto)\
	{ return this->function(std::forward<decltype(args)>(args)...); }

#ifdef COMPILY_DEBUG
	#define ASSERT(condition, ...) {if(!(condition)) { \
		s_Logger.Fatal("Assertion failed at {}:{} int {}: {}", __FILE__, __LINE__, __FUNCTION__, fmt::format(__VA_ARGS__));\
		exit(1); } \
	}
#else
	#define ASSERT(condition, ...) { if(!(condition)) {} }
#endif