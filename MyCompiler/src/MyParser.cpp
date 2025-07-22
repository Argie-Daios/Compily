#include "MyParser.h"

#include "MyLexer.h"
#include "TokenType.h"

enum RuleType
{
	STATEMENTS,
	STATEMENTS_AUGMENTED,
	STATEMENT,
	IF_STATEMENT,
	ASSIGNMENT,
	EXPRESSION,
	EXPRESSION_AUGMENTED
};

MyParser::MyParser(const std::ifstream& inputStream)
	: Parser(inputStream)
{
	m_Lexer = new MyLexer(inputStream);
	
	/////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(STATEMENTS, true);

	Add({ Parsy::CFGElementType::NonTerminal, STATEMENT });
	Add({ Parsy::CFGElementType::NonTerminal, STATEMENTS_AUGMENTED });

	EndRule();

	/////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(STATEMENTS_AUGMENTED);

	Add({ Parsy::CFGElementType::NonTerminal, STATEMENT });
	Add({ Parsy::CFGElementType::NonTerminal, STATEMENTS_AUGMENTED });

	Union();

	Add({ Parsy::CFGElementType::Epsilon, -1 });

	EndRule();

	/////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(STATEMENT);

	Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION });
	Add({ Parsy::CFGElementType::Symbol, SEMICOLON });

	Union();

	Add({ Parsy::CFGElementType::NonTerminal, ASSIGNMENT });
	Add({ Parsy::CFGElementType::Symbol, SEMICOLON });

	Union();

	Add({ Parsy::CFGElementType::NonTerminal, IF_STATEMENT });

	EndRule();

	/////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(ASSIGNMENT);

	Add({ Parsy::CFGElementType::Symbol, IDENTIFIER });
	Add({ Parsy::CFGElementType::Symbol, EQUAL });
	Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION });

	EndRule();

	/////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(EXPRESSION);

	Add({ Parsy::CFGElementType::Symbol, INTEGER });
	Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION_AUGMENTED });

	EndRule();

	/////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(EXPRESSION_AUGMENTED);

	Add({ Parsy::CFGElementType::Symbol, PLUS });
	Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION });

	Union();

	Add({ Parsy::CFGElementType::Symbol, MINUS });
	Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION });

	Union();

	Add({ Parsy::CFGElementType::Epsilon, -1 });

	EndRule();

	/////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(IF_STATEMENT);

	Add({ Parsy::CFGElementType::Symbol, IF });
	Add({ Parsy::CFGElementType::Symbol, LEFT_PARENTHESIS });
	Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION });
	Add({ Parsy::CFGElementType::Symbol, RIGHT_PARENTHESIS });
	Add({ Parsy::CFGElementType::Symbol, LEFT_BRACE });
	Add({ Parsy::CFGElementType::NonTerminal, STATEMENT });
	Add({ Parsy::CFGElementType::Symbol, RIGHT_BRACE });

	EndRule();

	/////////////////////////////////////////////////////////////////////////////////////////////////

	Parse();

}
