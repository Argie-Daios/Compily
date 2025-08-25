#pragma once

#include "Constant.h"

enum class ELvalueType
{
	Modifiable,
	Const
};

struct TLvalue
{
	ELvalueType Type = ELvalueType::Const;
	TDataTypeProperties DataType;
	std::string SymbolTableEntry;
};