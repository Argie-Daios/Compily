#include "TestParser.h"

#include "TestLexer.h"
#include "RuleAndTokenIDS.h"

TestParser::TestParser(const std::ifstream& inputStream)
	: SLRParser(inputStream)
{
	m_Lexer = new TestLexer(inputStream);
}

const std::string TestParser::RuleToStr(Parsy::RuleID_t ruleID) const
{
	return Parser::RuleToStr(ruleID);
}

const std::string TestParser::TokenToStr(Lexy::TokenID_t tokenID) const
{
	return Parser::TokenToStr(tokenID);
}