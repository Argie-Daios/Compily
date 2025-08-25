#pragma once

#include "Expression.h"

enum class EOperationCode
{
	Add, Substract, Multiply, Divide, Assign, Jump, JumpEq, JumpNEq, JumpLess, JumpLessEq, JumpGreater, JumpGreaterEq,
	Call, PushParameter, GetReturnValue
};

struct TQuad
{
	EOperationCode OperationCode;
	TExpression* Arg1;
	TExpression* Arg2;
	TExpression* Result;
	size_t Line;
};