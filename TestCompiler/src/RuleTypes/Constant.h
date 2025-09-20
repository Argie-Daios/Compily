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

	const std::string ToString() const
	{
		std::string result;
		switch (Type)
		{
		case EDataType::Int16:
		{
			result = "Int16";
			break;
		}
		case EDataType::Int32:
		{
			result = "Int32";
			break;
		}
		case EDataType::Int64:
		{
			result = "Int64";
			break;
		}
		case EDataType::Bool:
		{
			result = "Bool";
			break;
		}
		case EDataType::Float:
		{
			result = "Float";
			break;
		}
		case EDataType::Double:
		{
			result = "Double";
			break;
		}
		case EDataType::Char:
		{
			result = "Char";
			break;
		}
		case EDataType::String:
		{
			result = "String";
			break;
		}
		case EDataType::Struct:
		{
			result = "Struct";
			break;
		}
		}

		if (result.empty()) return "---";

		for (size_t i = 0; i < PointerDepth; i++)
		{
			result += '*';
		}
		return result;
	}

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

struct TDataTypePropertiesHash
{
	size_t operator()(const TDataTypeProperties& data) const
	{
		size_t h1 = std::hash<int32_t>{}(static_cast<int32_t>(data.Type));
		size_t h2 = std::hash<size_t>{}(data.PointerDepth);
		return h1 ^ (h2 << 1);
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

	template<typename Visitor>
	decltype(auto) Visit(Visitor&& visitor)
	{
		return std::visit(std::forward<Visitor>(visitor), Data);
	}

	template<typename Visitor>
	decltype(auto) Visit(Visitor&& visitor) const
	{
		return std::visit(std::forward<Visitor>(visitor), Data);
	}

	template<typename Type, typename ConvertToType>
	void Convert()
	{
		Type& value = std::get<Type>(Data);
		Data = static_cast<ConvertToType>(value);
	}
};