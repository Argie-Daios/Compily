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
	{MOD, R"(\%)"},
	{POWER, R"(\*\*)"}
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
	{SEMICOLON, R"(\;)"},
	{COMMA, R"(\,)"}
};

static void UnescapeInPlace(std::string& s)
{
	size_t w = 0;
	for (size_t r = 0; r < s.size(); ++r) 
	{
		if (s[r] == '\\' && r + 1 < s.size()) 
		{
			char next = s[r + 1];
			switch (next) {
			case 'n': s[w++] = '\n'; break;
			case 't': s[w++] = '\t'; break;
			case 'r': s[w++] = '\r'; break;
			case '\\': s[w++] = '\\'; break;
			case '"': s[w++] = '"'; break;
			default:
			{
				ASSERT(false, "Unknown escaped character");
				break;
			}
			}
			r++;
		}
		else {
			s[w++] = s[r];
		}
	}
	s.resize(w);
}

static void StripQuotesInPlaceString(std::string& s)
{
	if (s.size() >= 2 && s.front() == '"' && s.back() == '"') 
	{
		s.erase(s.begin());
		s.pop_back();
	}
}

static void StripQuotesInPlaceCharacter(std::string& s)
{
	if (s.size() >= 2 && s.front() == '\'' && s.back() == '\'')
	{
		s.erase(s.begin());
		s.pop_back();
	}
}

MyLexer::MyLexer(const std::ifstream& inputStream, const std::string& inputPath)
	: Lexy::Lexer(inputStream), m_InputPath(inputPath)
{
	/////RULES///////////////////////////////////////////////////////////////////////////////////////////

	CreateRule(R"(\/\/.*)", []() -> int32_t { return COMMENT; });

	CreateRule(s_Keywords);

	CreateRule(s_Operators);

	CreateRule(s_Punctuation);

	CreateRule(R"(print)", [this]() { return PRINT_FUNCTION; });

	CreateRule(R"(\"[^"\n]*\")", [this]() {  // Needs fix it takes hole thing
		auto& defaultValue = GetDefaultTokenValue();
		defaultValue = GetTokenContent();
		std::string& string = std::any_cast<std::string&>(defaultValue);
		StripQuotesInPlaceString(string);
		UnescapeInPlace(string);
		return STRING; 
		});

	CreateRule(R"(\'[^'\n]\')", [this]() {
		auto& defaultValue = GetDefaultTokenValue();
		defaultValue = GetTokenContent();
		std::string& string = std::any_cast<std::string&>(defaultValue);
		StripQuotesInPlaceCharacter(string);
		UnescapeInPlace(string);
		return CHARACTER;
	});

	CreateRule(R"([A-Za-z][A-Za-z0-9\_]*)", [this]() { 
		auto& defaultValue = GetDefaultTokenValue();
		defaultValue = GetTokenContent();
		return IDENTIFIER; 
	});

	CreateRule(R"([0-9]+\.[0-9]+)", []() { return DOUBLE; });

	CreateRule(R"([0-9]+)", [this]() { 
		auto& defaultValue = GetDefaultTokenValue();
		defaultValue = std::stoi(GetTokenContent());
		return INTEGER; 
		});

	CreateRule(R"([ \t]+)");

	CreateRule(R"([\r\n]+)", [this]() { AdvanceLineCount(); return TOKEN_IGNORE; });

	/////////////////////////////////////////////////////////////////////////////////////////////////////
}