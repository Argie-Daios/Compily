#include "MyLexer.h"

#include "Lexy.h"

namespace MyLexer
{
	enum TokenType
	{
		COMMENT,
		KEYWORD,
		PUNCTUATION,
		STRING,
		IDENTIFIER,
		DOUBLE,
		INTEGER,
		WHITESPACE
	};

	void Init()
	{
		Lexy::Lexer::Init(R"(
			if(x==2.0)
			{
				y = z + 5;
			}
		)");

		Lexy::Lexer::CreateRule(R"(\/\/.*)", []() -> int32_t {
			return COMMENT;
			});

		Lexy::Lexer::CreateRule(R"(if|ifelse|while|for|class|struct)", []() {
			return KEYWORD;
			});

		Lexy::Lexer::CreateRule(R"(\(|\)|\=\=|\!\=|\>|\>\=|\<|\<\=|\{|\}|\+|\-|\*|\/|\=|\;)",
			[]() {
				return PUNCTUATION;
			});

		Lexy::Lexer::CreateRule(R"(\".*\")", []() {
			return STRING;
			});

		Lexy::Lexer::CreateRule(R"([A-Za-z][A-Za-z0-9\_]*)", []() {
			return IDENTIFIER;
			});

		Lexy::Lexer::CreateRule(R"([0-9]+\.[0-9]+)", []() {
			return DOUBLE;
			});

		Lexy::Lexer::CreateRule(R"([0-9]+)", []() {
			return INTEGER;
			});

		Lexy::Lexer::CreateRule(R"([ \t\r\n]+)");

		Lexy::Lexer::Token token;
		while ((token = Lexy::Lexer::NextToken()).State != Lexy::Lexer::TokenState::End)
		{
			switch (token.TokenType)
			{
			case COMMENT:
			{
				std::cout << "Comment" << std::endl;
				break;
			}
			case KEYWORD:
			{
				std::cout << "Keyword" << std::endl;
				break;
			}
			case PUNCTUATION:
			{
				std::cout << "Punctuation" << std::endl;
				break;
			}
			case STRING:
			{
				std::cout << "String" << std::endl;
				break;
			}
			case IDENTIFIER:
			{
				std::cout << "Identifier" << std::endl;
				break;
			}
			case DOUBLE:
			{
				std::cout << "Double" << std::endl;
				break;
			}
			case INTEGER:
			{
				std::cout << "Integer" << std::endl;
				break;
			}
			default:
			{
				std::cout << "Whitespace" << std::endl;
				break;
			}
			}
		}
	}
}