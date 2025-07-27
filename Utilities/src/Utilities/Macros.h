#pragma once

#define BIT(x) 1 << x
#define BIND_CALLBACK(function) \
	[this](auto&&... args) -> decltype(auto)\
	{ return this->function(std::forward<decltype(args)>(args)...); }