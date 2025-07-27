#include "MyParser.h"

#include "MyLexer.h"
#include "TokenType.h"

#include <Utilities.h>

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

	DeclareTokenType<std::string>(IDENTIFIER);
	
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

	Add({ Parsy::CFGElementType::NonTerminal, ASSIGNMENT });
	Add({ Parsy::CFGElementType::Symbol, SEMICOLON }, [this](std::any& any) {
		int32_t value = std::any_cast<int32_t>(Get(0));
		std::cout << "Assignment value: " << value << std::endl;
		});

	Union();

	Add({ Parsy::CFGElementType::NonTerminal, IF_STATEMENT });

	EndRule();

	/////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<int32_t>(ASSIGNMENT);

	Add({ Parsy::CFGElementType::Symbol, IDENTIFIER });
	Add({ Parsy::CFGElementType::Symbol, EQUAL });
	Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION },
		[this](std::any& any) {
			int32_t& value = std::any_cast<int32_t&>(any);
			std::string& id = std::any_cast<std::string&>(Get(0));

			if (s_SymbolTable.find(id) == s_SymbolTable.end())
				s_SymbolTable.emplace(id, 0);
			int32_t& idValue = s_SymbolTable.at(id);

			int32_t* expressionValue = std::any_cast<int32_t*>(Get(2));
			value = *expressionValue;
			idValue = *expressionValue;
			std::cout << "Assignment" << std::endl;
		});

	EndRule();

	/////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<int32_t*>(EXPRESSION);

	Add({ Parsy::CFGElementType::Symbol, INTEGER });
	Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION_AUGMENTED },
		[this](std::any& any) {
			int32_t* value = std::any_cast<int32_t*>(any);
			int32_t& integer = std::any_cast<int32_t&>(Get(0));
			Operation& operation = std::any_cast<Operation&>(Get(1));

			*value = ExecuteOperation(operation.OpCode, integer, operation.RightValue);
		});

	Union();

	Add({ Parsy::CFGElementType::Symbol, IDENTIFIER });
	Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION_AUGMENTED },
		[this](std::any& any) {
			int32_t* value = std::any_cast<int32_t*>(any);
			std::string& id = std::any_cast<std::string&>(Get(0));
			int32_t& idValue = s_SymbolTable.at(id);
			Operation& operation = std::any_cast<Operation&>(Get(1));

			*value = ExecuteOperation(operation.OpCode, idValue, operation.RightValue);
		});

	EndRule();

	/////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<Operation>(EXPRESSION_AUGMENTED);

	Add({ Parsy::CFGElementType::Symbol, PLUS });
	Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION },
		[this](std::any& any) {
			Operation& value = std::any_cast<Operation&>(any);
			int32_t* integer = std::any_cast<int32_t*>(Get(1));

			value.OpCode = 1;
			value.RightValue = *integer;
		});

	Union();

	Add({ Parsy::CFGElementType::Symbol, MINUS });
	Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION },
		[this](std::any& any) {
			Operation& value = std::any_cast<Operation&>(any);
			int32_t* integer = std::any_cast<int32_t*>(Get(1));

			value.OpCode = 2;
			value.RightValue = *integer;
		});

	Union();

	Add({ Parsy::CFGElementType::Symbol, EQUAL_EQUAL });
	Add({ Parsy::CFGElementType::NonTerminal, EXPRESSION },
		[this](std::any& any) {
			Operation& value = std::any_cast<Operation&>(any);
			int32_t* integer = std::any_cast<int32_t*>(Get(1));

			value.OpCode = 3;
			value.RightValue = *integer;
		});

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
	//Add({ Parsy::CFGElementType::NonTerminal, STATEMENT });
	Add({ Parsy::CFGElementType::Symbol, RIGHT_BRACE },
		[this](std::any& any) {
			int32_t* expressionValue = std::any_cast<int32_t*>(Get(2));
			if (*expressionValue)
			{
				std::cout << "If in" << std::endl;
			}
		});

	EndRule();

	/////////////////////////////////////////////////////////////////////////////////////////////////

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
