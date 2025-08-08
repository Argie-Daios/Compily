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
	NUMBER,
	CONSTANT
};

struct Operation
{
	int32_t OpCode = -1;
	int32_t RightValue;
};

std::unordered_map<std::string, int32_t> s_SymbolTable;

MyParser::MyParser(const std::ifstream& inputStream)
	: CLRParser(inputStream, Parsy::CLRParserFlags_ForcePrecedence)
{
	m_Lexer = new MyLexer(inputStream);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	DeclareTokenType<int32_t>(NUMBER);
	DeclareTokenType<std::string>(STRING);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	DeclarePrecedence(PLUS, MINUS);
	DeclarePrecedence(MULTIPLY, DIVIDE);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	StartRule(STATEMENT);

	BeginRule<int32_t>(STATEMENT);

	Add(Parsy::CFGElementType::NonTerminal, EXPRESSION );
	Add(Parsy::CFGElementType::Symbol, SEMICOLON, [this](std::any& any) {
		Constant& expressionValue = std::any_cast<Constant&>(Get(0));

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

	EndRule();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<Constant>(EXPRESSION);

	Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
	Add(Parsy::CFGElementType::Symbol, PLUS);
	Add(Parsy::CFGElementType::NonTerminal, EXPRESSION, [this](std::any& any) {
		Constant& expressionValue = std::any_cast<Constant&>(any);
		Constant& leftExpressionValue = std::any_cast<Constant&>(Get(0));
		Constant& rightExpressionValue = std::any_cast<Constant&>(Get(2));
		expressionValue = ExecuteOperation(0, leftExpressionValue, rightExpressionValue);
		std::cout << leftExpressionValue.IntVal << " + " << rightExpressionValue.IntVal << std::endl;
		});

	Union();

	Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
	Add(Parsy::CFGElementType::Symbol, MULTIPLY);
	Add(Parsy::CFGElementType::NonTerminal, EXPRESSION, [this](std::any& any) {
		Constant& expressionValue = std::any_cast<Constant&>(any);
		Constant& leftExpressionValue = std::any_cast<Constant&>(Get(0));
		Constant& rightExpressionValue = std::any_cast<Constant&>(Get(2));
		expressionValue = ExecuteOperation(1, leftExpressionValue, rightExpressionValue);
		std::cout << leftExpressionValue.IntVal << " * " << rightExpressionValue.IntVal << std::endl;
		});

	Union();

	Add(Parsy::CFGElementType::Symbol, LEFT_PARENTHESIS);
	Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
	Add(Parsy::CFGElementType::Symbol, RIGHT_PARENTHESIS, [this](std::any& any) {
		Constant& expressionValue = std::any_cast<Constant&>(any);
		Constant& innerExpressionValue = std::any_cast<Constant&>(Get(1));
		expressionValue = innerExpressionValue;
		});

	Union();

	Add(Parsy::CFGElementType::NonTerminal, CONSTANT, [this](std::any& any) {
		Constant& expressionValue = std::any_cast<Constant&>(any);
		Constant& innerExpressionValue = std::any_cast<Constant&>(Get(0));
		expressionValue = innerExpressionValue;
		});

	EndRule();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<Constant>(CONSTANT);

	Add(Parsy::CFGElementType::Symbol, INTEGER, [this](std::any& any) {
		Constant& constantValue = std::any_cast<Constant&>(any);
		int32_t& integerValue = std::any_cast<int32_t&>(Get(0));
		constantValue.Type = ConstantValueType::Int;
		constantValue.IntVal = integerValue;
		});

	Union();

	Add(Parsy::CFGElementType::Symbol, STRING, [this](std::any& any) {
		Constant& constantValue = std::any_cast<Constant&>(any);
		std::string& stringValue = std::any_cast<std::string&>(Get(0));
		constantValue.Type = ConstantValueType::String;
		constantValue.StrVal = stringValue;
		});

	EndRule();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Parse();

}

const std::string MyParser::RuleToStr(Parsy::RuleID_t ruleID)
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

const std::string MyParser::TokenToStr(Lexy::TokenID_t tokenID)
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

	return TokenToStr(tokenID);
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
	}

	return constant;
}
