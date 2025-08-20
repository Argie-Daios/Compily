#pragma once

#include "Parsy.h"
#include "SymbolTable.h"

class TestParser : public Parsy::CLRParser
{
public:
	TestParser(const std::ifstream& inputStream);
protected:
	virtual const std::string RuleToStr(Parsy::RuleID_t ruleID) const override;
	virtual const std::string TokenToStr(Lexy::TokenID_t tokenID) const override;
private:
	SymbolTable m_SymbolTable;
};