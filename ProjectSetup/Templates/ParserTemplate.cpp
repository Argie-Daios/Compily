#include "#parserName#.h"

#include "#lexerName#.h"
#include "RuleAndTokenIDS.h"

#parserName#::#parserName#(const std::ifstream& inputStream)
	: SLRParser(inputStream)
{
	m_Lexer = new #lexerName#(inputStream);
}

const std::string #parserName#::RuleToStr(Parsy::RuleID_t ruleID) const
{
	return Parser::RuleToStr(ruleID);
}

const std::string #parserName#::TokenToStr(Lexy::TokenID_t tokenID) const
{
	return Parser::TokenToStr(tokenID);
}