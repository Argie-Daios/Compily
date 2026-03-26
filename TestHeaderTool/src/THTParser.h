#pragma once

#include "Parsy.h"

class THTParser : public Parsy::CLRParser
{
public:
	THTParser(const std::string& sourceCodePath);
protected:
	virtual const std::string RuleToStr(Parsy::RuleID_t ruleID) const override;
	virtual const std::string TokenToStr(Lexy::TokenID_t tokenID) const override;
	virtual void SyntaxErrorHandler() override;
private:
	Utilities::LoggerInstance m_Logger;
};