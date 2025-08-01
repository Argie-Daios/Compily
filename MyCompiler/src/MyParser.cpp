#include "MyParser.h"

#include "MyLexer.h"
#include "TokenType.h"

#include <Utilities.h>

enum RuleType
{
	STATEMENTS,
	STATEMENT,
	IF_STATEMENT,
	ASSIGNMENT,
	EXPRESSION,
};

struct Operation
{
	int32_t OpCode = -1;
	int32_t RightValue;
};

std::unordered_map<std::string, int32_t> s_SymbolTable;

MyParser::MyParser(const std::ifstream& inputStream)
	: CLRParser(inputStream)
{
	m_Lexer = new MyLexer(inputStream);

	BeginRule<int32_t>(STATEMENT, true);

	Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION });
	Add({ Parsy::CFGElementType::Symbol, SEMICOLON });

	EndRule();

	BeginRule<int32_t>(EXPRESSION);

	Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION });
	Add({ Parsy::CFGElementType::Symbol, PLUS });
	Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION });

	Union();


	Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION });
	Add({ Parsy::CFGElementType::Symbol, MULTIPLY });
	Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION });

	Union();

	Add({ Parsy::CFGElementType::Symbol, INTEGER });

	EndRule();

	//DeclareTokenType<std::string>(IDENTIFIER);
	//DeclareTokenType<int32_t>(INTEGER);

	///////////////////////////////////////////////////////////////////////////////////////////////////

	//BeginRule(STATEMENTS, true);

	//Add({ Parsy::CFGElementType::NonTerminal, STATEMENTS });
	//Add({ Parsy::CFGElementType::NonTerminal, STATEMENT });

	//Union();

	//Add({ Parsy::CFGElementType::NonTerminal, STATEMENT });

	//EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	//BeginRule(STATEMENT);

	//Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION });
	//Add({ Parsy::CFGElementType::Symbol, SEMICOLON });

	///*Union();

	//Add({ Parsy::CFGElementType::NonTerminal, ASSIGNMENT });
	//Add({ Parsy::CFGElementType::Symbol, SEMICOLON });

	//Union();

	//Add({ Parsy::CFGElementType::NonTerminal, IF_STATEMENT });*/

	//EndRule();

	//BeginRule<int32_t>(EXPRESSION);

	//Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION });
	//Add({ Parsy::CFGElementType::Symbol, PLUS });
	//Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION }, [this](std::any& any)
	//	{
	//		int32_t& leftExpressionValue = std::any_cast<int32_t&>(Get(0));
	//		int32_t& rightExpressionValue = std::any_cast<int32_t&>(Get(2));
	//		std::cout << leftExpressionValue << "+" << rightExpressionValue << std::endl;
	//	});

	//Union();

	//Add({ Parsy::CFGElementType::Symbol, INTEGER }, [this](std::any& any)
	//	{
	//		int32_t& expressionValue = std::any_cast<int32_t&>(any);
	//		int32_t& integerValue = std::any_cast<int32_t&>(Get(0));
	//		expressionValue = integerValue;
	//		std::cout << "Expression value: " << expressionValue << std::endl;
	//	});

	//EndRule();
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////

	Parse();

}

int32_t MyParser::ExecuteOperation(int32_t opCode, int32_t leftValue, int32_t rightValue)
{
	int32_t result;
	switch (opCode)
	{
	case 1:
	{
		result = leftValue + rightValue;
		break;
	}
	case 2:
	{
		result = leftValue - rightValue;
		break;
	}
	case 3:
	{
		result = (leftValue == rightValue ? 1 : 0);
		break;
	}
	default:
	{

		result = leftValue;
		break;
	}
	}

	return result;
}