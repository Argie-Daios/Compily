#include "TestLexer.h"

#include "RuleAndTokenIDS.h"

TestLexer::TestLexer(const std::ifstream& inputStream)
	: Lexy::Lexer(inputStream)
{

}