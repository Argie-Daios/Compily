#pragma once

#include "Logger.h"

#define BIT(x) 1 << x
#define BIND_CALLBACK(function) \
	[this](auto&&... args) -> decltype(auto)\
	{ return function(std::forward<decltype(args)>(args)...); }

#ifdef BIT
	#define ASSERT(condition, ...) {if(!(condition)) { \
		Utilities::Logger::Fatal("Assert",\
		"Assertion failed at {}:{} int {}: {}", __FILE__, __LINE__, __FUNCTION__, fmt::format(__VA_ARGS__));\
		exit(1); } \
	}
#else
	#define ASSERT(condition, ...) { if(!(condition)) {} }
#endif