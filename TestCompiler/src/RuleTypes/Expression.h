#pragma once

#include "Constant.h"

enum class EExpressionType
{
	None,
	Constant,
	Lvalue
};

struct TExpression
{
	EExpressionType Type = EExpressionType::None;
	std::string SymbolTableEntry;
	TConstant ConstantValue;
};