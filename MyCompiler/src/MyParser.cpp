#include "MyParser.h"

#include "MyLexer.h"
#include "TokenType.h"

#include <Utilities.h>

enum RuleType
{
	STATEMENTS,
	STATEMENT,
	EXPRESSION,
	CONSTANT
};

struct Operation
{
	int32_t OpCode = -1;
	int32_t RightValue;
};

std::unordered_map<std::string, int32_t> s_SymbolTable;

MyParser::MyParser(const std::ifstream& inputStream)
	: CLRParser(inputStream, Parsy::CLRParserFlags_ForcePrecedence | Parsy::LRParserFlags_IncludeDollarLookAhead)
{
	m_Lexer = new MyLexer(inputStream);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	DeclareTokenType<int32_t>(INTEGER);
	DeclareTokenType<std::string>(STRING);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	DeclarePrecedence(Parsy::PrecedenceAssociativity::Left, PLUS, MINUS);
	DeclarePrecedence(Parsy::PrecedenceAssociativity::Left, MULTIPLY, DIVIDE);
	DeclarePrecedence(Parsy::PrecedenceAssociativity::Right, POWER);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	DeclareRootRule(STATEMENTS);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(STATEMENTS);

	Add(Parsy::CFGElementType::NonTerminal, STATEMENT);
	Add(Parsy::CFGElementType::NonTerminal, STATEMENTS);

	Union();

	Add(Parsy::CFGElementType::NonTerminal, STATEMENT);

	EndRule();

	BeginRule<int32_t>(STATEMENT);

	Add(Parsy::CFGElementType::NonTerminal, EXPRESSION );
	Add(Parsy::CFGElementType::Symbol, SEMICOLON, [this](Parsy::EntryValue& any) {
		Constant& expressionValue = Get<Constant>(0);

		switch (expressionValue.Type)
		{
		case ConstantValueType::Int:
		{
			std::cout << "Expression value: " << expressionValue.IntVal << std::endl;
			break;
		}
		case ConstantValueType::String:
		{
			std::cout << "Expression value: " << expressionValue.StrVal << std::endl;
			break;
		}
		}
		});

	Union();

	Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
	Add(Parsy::CFGElementType::Error, [](Parsy::EntryValue& any) { std::cout << "Missing ';'" << std::endl; });

	EndRule();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<Constant>(EXPRESSION);

	Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
	Add(Parsy::CFGElementType::Symbol, PLUS);
	Add(Parsy::CFGElementType::NonTerminal, EXPRESSION, [this](Parsy::EntryValue& any) {
		Constant& expressionValue = Get<Constant>(any);
		Constant& leftExpressionValue = Get<Constant>(0);
		Constant& rightExpressionValue = Get<Constant>(2);
		expressionValue = ExecuteOperation(0, leftExpressionValue, rightExpressionValue);
		std::cout << leftExpressionValue.IntVal << " + " << rightExpressionValue.IntVal << std::endl;
		});

	Union();

	Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
	Add(Parsy::CFGElementType::Symbol, MULTIPLY);
	Add(Parsy::CFGElementType::NonTerminal, EXPRESSION, [this](Parsy::EntryValue& any) {
		Constant& expressionValue = Get<Constant>(any);
		Constant& leftExpressionValue = Get<Constant>(0);
		Constant& rightExpressionValue = Get<Constant>(2);
		expressionValue = ExecuteOperation(1, leftExpressionValue, rightExpressionValue);
		std::cout << leftExpressionValue.IntVal << " * " << rightExpressionValue.IntVal << std::endl;
		});

	Union();

	Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
	Add(Parsy::CFGElementType::Symbol, POWER);
	Add(Parsy::CFGElementType::NonTerminal, EXPRESSION, [this](Parsy::EntryValue& any) {
		Constant& expressionValue = Get<Constant>(any);
		Constant& leftExpressionValue = Get<Constant>(0);
		Constant& rightExpressionValue = Get<Constant>(2);
		expressionValue = ExecuteOperation(2, leftExpressionValue, rightExpressionValue);
		std::cout << leftExpressionValue.IntVal << " * " << rightExpressionValue.IntVal << std::endl;
		});

	Union();

	Add(Parsy::CFGElementType::Symbol, LEFT_PARENTHESIS);
	Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
	Add(Parsy::CFGElementType::Symbol, RIGHT_PARENTHESIS, [this](Parsy::EntryValue& any) {
		Constant& expressionValue = Get<Constant>(any);
		Constant& innerExpressionValue = Get<Constant>(1);
		expressionValue = innerExpressionValue;
		});

	Union();

	Add(Parsy::CFGElementType::Symbol, LEFT_PARENTHESIS);
	Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
	Add(Parsy::CFGElementType::Error, [](Parsy::EntryValue& any) { std::cout << "Missing ')'" << std::endl; });

	Union();

	Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
	Add(Parsy::CFGElementType::Error, [](Parsy::EntryValue& any) { std::cout << "Missing '('" << std::endl; });
	Add(Parsy::CFGElementType::Symbol, RIGHT_PARENTHESIS);

	Union();

	Add(Parsy::CFGElementType::Symbol, LEFT_PARENTHESIS);
	Add(Parsy::CFGElementType::Error, [](Parsy::EntryValue& any) { std::cout << "Empty Parenthesis" << std::endl; });
	Add(Parsy::CFGElementType::Symbol, RIGHT_PARENTHESIS);

	Union();

	Add(Parsy::CFGElementType::NonTerminal, CONSTANT, [this](Parsy::EntryValue& any) {
		Constant& expressionValue = Get<Constant>(any);
		Constant& innerExpressionValue = Get<Constant>(0);
		expressionValue = innerExpressionValue;
		});

	EndRule();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<Constant>(CONSTANT);

	Add(Parsy::CFGElementType::Symbol, INTEGER, [this](Parsy::EntryValue& any) {
		Constant& constantValue = Get<Constant>(any);
		int32_t& integerValue = Get<int32_t>(0);
		constantValue.Type = ConstantValueType::Int;
		constantValue.IntVal = integerValue;
		});

	Union();

	Add(Parsy::CFGElementType::Symbol, STRING, [this](Parsy::EntryValue& any) {
		Constant& constantValue = Get<Constant>(any);
		std::string& stringValue = Get<std::string>(0);
		constantValue.Type = ConstantValueType::String;
		constantValue.StrVal = stringValue;
		});

	EndRule();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Parse();

}

const std::string MyParser::RuleToStr(Parsy::RuleID_t ruleID) const
{
	switch (ruleID)
	{
	case STATEMENTS: return "Statements";
	case STATEMENT: return "Statement";
	case EXPRESSION: return "Expression";
	case CONSTANT: return "Constant";
	}

	return Parser::RuleToStr(ruleID);
}

const std::string MyParser::TokenToStr(Lexy::TokenID_t tokenID) const
{
	switch (tokenID)
	{
	case COMMENT: return "Comment";
	case STRING: return "String";
	case IDENTIFIER: return "Identifier";
	case DOUBLE: return "Double";
	case INTEGER: return "Integer";
	case IF: return "If";
	case ELSE: return "Else";
	case WHILE: return "While";
	case FOR: return "For";
	case PLUS: return "Plus";
	case MINUS: return "Minus";
	case MULTIPLY: return "Multiply";
	case DIVIDE: return "Divide";
	case MOD: return "Mod";
	case POWER: return "Power";
	case EQUAL: return "Equal";
	case EQUAL_EQUAL: return "EqualEqual";
	case NOT_EQUAL: return "NotEqual";
	case LESS: return "Less";
	case LESS_EQUAL: return "LessEqual";
	case GREATER: return "Greater";
	case GREATER_EQUAL: return "GreaterEqual";
	case LEFT_BRACE: return "LeftBrace";
	case RIGHT_BRACE: return "RightBrace";
	case LEFT_BRACKET: return "LeftBracket";
	case RIGHT_BRACKET: return "RightBracket";
	case LEFT_PARENTHESIS: return "LeftParenthesis";
	case RIGHT_PARENTHESIS: return "RightParenthesis";
	case SEMICOLON: return "Semicolon";
	}

	return Parser::TokenToStr(tokenID);
}

ConstantValueType MyParser::ResultValue(const Constant& leftValue, const Constant& rightValue)
{
	if (leftValue.Type == rightValue.Type) return leftValue.Type;

	return (int32_t)leftValue.Type < (int32_t)rightValue.Type ? rightValue.Type : leftValue.Type;
}

Constant MyParser::ExecuteOperation(int32_t opCode, const Constant& leftValue, const Constant& rightValue)
{
	ConstantValueType resultType = ResultValue(leftValue, rightValue);
	Constant constant;
	switch (opCode)
	{
	case 0:
	{
		switch (resultType)
		{
		case ConstantValueType::Int:
		{
			int32_t leftValueInt = (leftValue.Type == ConstantValueType::Int ? leftValue.IntVal :
				std::stoi(leftValue.StrVal));
			int32_t rightValueInt = (rightValue.Type == ConstantValueType::Int ? rightValue.IntVal :
				std::stoi(rightValue.StrVal));
			constant.Type = ConstantValueType::Int;
			constant.IntVal = leftValueInt + rightValueInt;
			break;
		}
		case ConstantValueType::String:
		{
			std::string leftValueStr = (leftValue.Type == ConstantValueType::String ? leftValue.StrVal :
				std::to_string(leftValue.IntVal));
			std::string rightValueStr = (rightValue.Type == ConstantValueType::String ? rightValue.StrVal :
				std::to_string(rightValue.IntVal));
			constant.Type = ConstantValueType::String;
			constant.StrVal = leftValueStr + rightValueStr;
			break;
		}
		}
		break;
	}
	case 1:
	{
		switch (resultType)
		{
		case ConstantValueType::Int:
		{
			int32_t leftValueInt = (leftValue.Type == ConstantValueType::Int ? leftValue.IntVal :
				std::stoi(leftValue.StrVal));
			int32_t rightValueInt = (rightValue.Type == ConstantValueType::Int ? rightValue.IntVal :
				std::stoi(rightValue.StrVal));
			constant.Type = ConstantValueType::Int;
			constant.IntVal = leftValueInt * rightValueInt;
			break;
		}
		case ConstantValueType::String:
		{
			std::cout << "Cannot multiply with string" << std::endl;
			exit(1);
		}
		}
		break;
	}
	case 2:
	{
		switch (resultType)
		{
		case ConstantValueType::Int:
		{
			int32_t leftValueInt = (leftValue.Type == ConstantValueType::Int ? leftValue.IntVal :
				std::stoi(leftValue.StrVal));
			int32_t rightValueInt = (rightValue.Type == ConstantValueType::Int ? rightValue.IntVal :
				std::stoi(rightValue.StrVal));
			constant.Type = ConstantValueType::Int;
			constant.IntVal = pow(leftValueInt, rightValueInt);
			break;
		}
		case ConstantValueType::String:
		{
			std::cout << "Cannot perform power with string" << std::endl;
			exit(1);
		}
		}
		break;
	}
	}

	return constant;
}
