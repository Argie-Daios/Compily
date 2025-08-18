#pragma once

#include "Parsy.h"

class #parserName# : public Parsy::SLRParser
{
public:
	#parserName#(const std::ifstream& inputStream);
protected:
	virtual const std::string RuleToStr(Parsy::RuleID_t ruleID) const override;
	virtual const std::string TokenToStr(Lexy::TokenID_t tokenID) const override;
};