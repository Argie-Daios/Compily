#pragma once

#include "Parsy.h"
#include "SymbolTable.h"

#include "RuleTypes/Constant.h"
#include "RuleTypes/Lvalue.h"
#include "RuleTypes/Struct.h"
#include "RuleTypes/Expression.h"

enum class EOperationType
{
	Plus
};

class TestParser : public Parsy::CLRParser
{
public:
	TestParser(const std::string& sourceCodePath);
protected:
	virtual const std::string RuleToStr(Parsy::RuleID_t ruleID) const override;
	virtual const std::string TokenToStr(Lexy::TokenID_t tokenID) const override;
	virtual void SyntaxErrorHandler() override;
private:
	TConstant GetConstantValueFromExpression(const TExpression& expression);
	TConstant ExecuteOperation(const EOperationType& operationType, const TConstant& leftConstant, const TConstant& rightConstant);
private:
	Utilities::LoggerInstance m_Logger;
	SymbolTable m_SymbolTable;
};