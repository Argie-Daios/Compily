#pragma once

#include "Constant.h"

enum class ELvalueType
{
	Modifiable,
	Const
};

struct TLvalue
{
	std::string SymbolTableEntry;
	bool HasFinalType = true;
};