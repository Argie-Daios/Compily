#include "MyLexer.h"

#include "TokenType.h"

static Lexy::Lexer::RuleBuffer s_Keywords = {
	{IF, R"(if)"},
	{ELSE, R"(else)"},
	{WHILE, R"(while)"},
	{FOR, R"(for)"}
};

static Lexy::Lexer::RuleBuffer s_Operators = {
	{PLUS, R"(\+)"},
	{MINUS, R"(\-)"},
	{MULTIPLY, R"(\*)"},
	{DIVIDE, R"(\/)"},
	{MOD, R"(\%)"}
};

static Lexy::Lexer::RuleBuffer s_Punctuation = {
	{EQUAL, R"(\=)"},
	{EQUAL_EQUAL, R"(\=\=)"},
	{NOT_EQUAL, R"(\!\=)"},
	{LESS, R"(\<)"},
	{LESS_EQUAL, R"(\<\=)"},
	{GREATER, R"(\>)"},
	{GREATER_EQUAL, R"(\>\=)"},
	{LEFT_BRACE, R"(\{)"},
	{RIGHT_BRACE, R"(\})"},
	{LEFT_BRACKET, R"(\[)"},
	{RIGHT_BRACKET, R"(\])"},
	{LEFT_PARENTHESIS, R"(\()"},
	{RIGHT_PARENTHESIS, R"(\))"},
	{SEMICOLON, R"(\;)"}
};

MyLexer::MyLexer(const std::ifstream& inputStream)
	: Lexy::Lexer(inputStream)
{
	/////RULES///////////////////////////////////////////////////////////////////////////////////////////

	CreateRule(R"(\/\/.*)", []() -> int32_t { return COMMENT; });

	CreateRule(s_Keywords, [](int32_t type) { std::cout << "Type: " << type << std::endl; });

	CreateRule(s_Operators, [](int32_t type) { std::cout << "Type: " << type << std::endl; });

	CreateRule(s_Punctuation, [](int32_t type) { std::cout << "Type: " << type << std::endl; });

	CreateRule(R"(\".*\")", []() { return STRING; });

	CreateRule(R"([A-Za-z][A-Za-z0-9\_]*)", []() { return IDENTIFIER; });

	CreateRule(R"([0-9]+\.[0-9]+)", []() { return DOUBLE; });

	CreateRule(R"([0-9]+)", []() { return INTEGER; });

	CreateRule(R"([ \t]+)");

	CreateRule(R"([\r\n]+)", [this]() { AdvanceLineCount(); return IGNORE; });

	/////////////////////////////////////////////////////////////////////////////////////////////////////

	Tokenize();
}