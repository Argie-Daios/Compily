#include <iostream>

#include "MyLexer.h"

#include <Parsy.h>

#include "TokenType.h"

enum RuleType
{
	EXPRESSION
};

int main()
{
	std::ifstream stream("SourceCode.src");
	//MyLexer lexer(stream);

	Parsy::Parser parser(stream);

	parser.BeginRule(EXPRESSION, true);

	parser.Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION });
	parser.Add({ Parsy::CFGElementType::Symbol, PLUS });
	parser.Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION });

	parser.Union();

	parser.Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION });
	parser.Add({ Parsy::CFGElementType::Symbol, MINUS });
	parser.Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION });

	parser.Union();

	parser.Add({ Parsy::CFGElementType::Symbol, DOUBLE });

	parser.Union();

	parser.Add({ Parsy::CFGElementType::Symbol, INTEGER });

	parser.EndRule();

	parser.Parse();

	return 0;
}