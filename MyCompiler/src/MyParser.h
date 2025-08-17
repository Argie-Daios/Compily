#pragma once

#include "Parsy.h"

enum class ConstantValueType
{
	Int,
	String
};

struct Constant
{
	ConstantValueType Type;
	int32_t IntVal = 0;
	std::string StrVal;

	Constant() = default;
	Constant(const Constant&) = default;
};

class MyParser : public Parsy::CLRParser
{
public:
	MyParser(const std::ifstream& inputStream, const std::string& inputPath);
protected:
	virtual const std::string RuleToStr(Parsy::RuleID_t ruleID) const override;
	virtual const std::string TokenToStr(Lexy::TokenID_t tokenID) const override;
private:
	ConstantValueType ResultValue(const Constant& leftValue, const Constant& rightValue);
	Constant ExecuteOperation(int32_t opCode, const Constant& leftValue, const Constant& rightValue);
private:
	std::string m_InputPath;
};