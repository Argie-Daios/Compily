#pragma once

#include "RuleTypes/Expression.h"

enum EOperationCode
{
	OperationCode_Add, OperationCode_Substract, OperationCode_Multiply, OperationCode_Divide, OperationCode_Assign,
	OperationCode_Jump, OperationCode_JumpEq, OperationCode_JumpNEq, OperationCode_JumpLess, OperationCode_JumpLessEq,
	OperationCode_JumpGreater, OperationCode_JumpGreaterEq, OperationCode_Call, OperationCode_PushParameter,
	OperationCode_GetReturnValue, OperationCode_Cast
};

struct TQuad
{
	TExpression* Arg1;
	TExpression* Arg2;
	TExpression* Result;
	size_t Line;

	TQuad() = default;
	TQuad(TExpression* arg1, TExpression* arg2, TExpression* result, size_t line)
		: Arg1(arg1), Arg2(arg2), Result(result), Line(line)
	{

	}
};