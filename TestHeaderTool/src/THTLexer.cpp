#include "THTLexer.h"

#include "RuleAndTokenIDS.h"

static Lexy::Lexer::RuleBuffer s_Keywords = {
	{CLASS, R"(class)"}
};

static Lexy::Lexer::RuleBuffer s_Punctuation = {
	{LEFT_BRACE, R"(\{)"},
	{RIGHT_BRACE, R"(\})"},
	{LEFT_PARENTHESIS, R"(\()"},
	{RIGHT_PARENTHESIS, R"(\))"},
	{SEMICOLON, R"(\;)"},
};

THTLexer::THTLexer(const std::ifstream& inputStream)
	: Lexy::Lexer(inputStream)
{
	CreateRule(s_Keywords);

	CreateRule(s_Punctuation);

	CreateRule(R"([A-Za-z][A-Za-z0-9\_]*)", [this]() {
		auto& defaultValue = GetDefaultTokenValue();
		defaultValue = GetTokenContent();
		return IDENTIFIER;
	});

	CreateRule(R"([ \t]+)", [this]() { return TOKEN_IGNORE; });

	CreateRule(R"([\r\n]+)", [this]() {
		const std::string& content = GetTokenContent();
		UpdateLineCountFromString(content);
		return TOKEN_IGNORE;
	});

	CreateRule(R"(.*)", [this]() { return TOKEN_IGNORE; });

	GenerateDFA();
}

THTLexer::THTLexer(const std::string& sourceCodePath)
	: THTLexer(std::ifstream(sourceCodePath))
{
}

void THTLexer::UpdateLineCountFromString(const std::string& contentStr)
{
	for (size_t i = 0; i < contentStr.size(); ++i)
	{
		if (contentStr[i] == '\r')
		{
			if (i + 1 < contentStr.size() && contentStr[i + 1] == '\n')
				++i;

			AdvanceLineCount();
		}
		else if (contentStr[i] == '\n')
		{
			AdvanceLineCount();
		}
	}
}

void THTLexer::RemoveQuotes(std::string& string, char quotes)
{
	if (string.size() >= 2 && string.front() == quotes && string.back() == quotes)
	{
		string.erase(string.begin());
		string.pop_back();
	}
}

void THTLexer::UnescapeString(std::string& string)
{
	size_t w = 0;
	for (size_t r = 0; r < string.size(); ++r)
	{
		if (string[r] == '\\' && r + 1 < string.size())
		{
			char next = string[r + 1];
			switch (next) {
			case 'n': string[w++] = '\n'; break;
			case 't': string[w++] = '\t'; break;
			case 'r': string[w++] = '\r'; break;
			case '\\': string[w++] = '\\'; break;
			case '"': string[w++] = '"'; break;
			default:
			{
				ASSERT(false, "Unknown escaped character");
				break;
			}
			}
			r++;
		}
		else {
			string[w++] = string[r];
		}
	}
	string.resize(w);
}
