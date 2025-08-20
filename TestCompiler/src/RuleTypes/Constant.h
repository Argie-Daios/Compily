#pragma once

#include <variant>
#include <string>

enum class ConstantType
{
	Int32,
	Float,
	Double,
	Char,
	String
};

struct TConstant
{
	ConstantType Type;
	std::variant<int32_t, float, double, char, std::string> Data;
};