#pragma once

#include <variant>
#include <string>

enum class EDataType
{
	None,
	Int16,
	Int32,
	Int64,
	Bool,
	Float,
	Double,
	Char,
	String,
	Struct
};

struct TDataTypeProperties
{
	EDataType Type = EDataType::None;
	size_t PointerDepth = 0;
};

struct TConstant
{
	TDataTypeProperties DataTypeProps;
	std::variant<int32_t, bool, float, double, char, std::string> Data;
};