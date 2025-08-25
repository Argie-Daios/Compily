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

	bool operator==(const TDataTypeProperties& dataTypeProps) const
	{
		if (this == &dataTypeProps) return true;
		return Type == dataTypeProps.Type && PointerDepth == dataTypeProps.PointerDepth;
	}

	bool operator!=(const TDataTypeProperties& dataTypeProps) const
	{
		return !(*this == dataTypeProps);
	}
};

struct TConstant
{
	TDataTypeProperties DataTypeProps;
	std::variant<int16_t, int32_t, int64_t, bool, float, double, char, std::string> Data;
	
	template<typename Type>
	Type& Get()
	{
		return std::get<Type>(Data);
	}

	template<typename Type>
	const Type& Get() const
	{
		return std::get<Type>(Data);
	}
};