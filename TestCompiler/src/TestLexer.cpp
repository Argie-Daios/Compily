#include "TestLexer.h"

#include "RuleAndTokenIDS.h"

static Lexy::Lexer::RuleBuffer s_Keywords = {
	{INTEGER_TYPE, R"(int)"},
	{BOOL_TYPE, R"(bool)"},
	{FLOAT_TYPE, R"(float)"},
	{DOUBLE_TYPE, R"(double)"},
	{CHARACTER_TYPE, R"(char)"},
	{SHORT_TYPE, R"(short)"},
	{LONG_TYPE, R"(long)"},
	{UNSIGNED_KEYWORD, R"(unsigned)"},
	{CONST_KEYWORD, R"(const)"},
	{IF, R"(if)"},
	{ELSE, R"(else)"},
	{FOR, R"(for)"},
	{WHILE, R"(while)"},
	{STRUCT, R"(struct)"}
};

static Lexy::Lexer::RuleBuffer s_Operators = {
	{PLUS, R"(\+)"},
	{DASH, R"(\-)"},
	{STAR, R"(\*)"},
	{SLASH, R"(\/)"},
	{PERCENT, R"(\%)"},
	{CARET, R"(\^)"}
};

static Lexy::Lexer::RuleBuffer s_Punctuation = {
	{EQUALS, R"(\=)"},
	{PLUS_PLUS, R"(\+\+)"},
	{DASH_DASH, R"(\-\-)"},
	{PLUS_EQUALS, R"(\+\=)"},
	{DASH_EQUALS, R"(\-\=)"},
	{STAR_EQUALS, R"(\*\=)"},
	{SLASH_EQUALS, R"(\/\=)"},
	{PERCENT_EQUALS, R"(\%\=)"},
	{CARET_EQUALS, R"(\^\=)"},
	{EQUALS_EQUALS, R"(\=\=)"},
	{EXCLAMATION_EQUALS, R"(\!\=)"},
	{LESS, R"(\<)"},
	{LESS_EQUALS, R"(\<\=)"},
	{GREATER, R"(\>)"},
	{GREATER_EQUALS, R"(\>\=)"},
	{PIPE_PIPE, R"(\|\|)"},
	{AMPERSAND_AMPERSAND, R"(\&\&)"},
	{DOT, R"(\.)"},
	{DOT_DOT, R"(\.\.)"},
	{COMMA, R"(\,)"},
	{QUESTIONMARK, R"(\?)"},
	{COLON, R"(\:)"},
	{COLON_COLON, R"(\:\:)"},
	{OPENING_PARENTHESIS, R"(\()"},
	{CLOSING_PARENTHESIS, R"(\))"},
	{OPENING_BRACKET, R"(\[)"},
	{CLOSING_BRACKET, R"(\])"},
	{OPENING_BRACE, R"(\{)"},
	{CLOSING_BRACE, R"(\})"},
	{SEMICOLON, R"(\;)"},
	{AMPERSAND, R"(\&)"},
	{ARROW, R"(\-\>)"}
};

TestLexer::TestLexer(const std::ifstream& inputStream)
	: Lexy::Lexer(inputStream)
{
	CreateRule(R"(\/\*([^*]|\*[^/])*\*\/)", [this]() { 
		const std::string& content = GetTokenContent();
		uint32_t lineCount = GetLineCount();
		UpdateLineCountFromString(content);
		return TOKEN_IGNORE;
	});

	CreateRule(R"(\/\/[^\n\r]*)", []() { return TOKEN_IGNORE; });

	CreateRule(s_Keywords);

	CreateRule(s_Operators);

	CreateRule(s_Punctuation);

	CreateRule(R"(\"[^"\n\r]*\")", [this]() { 
		auto& defaultValue = GetDefaultTokenValue();
		std::string tokenContent = GetTokenContent();
		RemoveQuotes(tokenContent);
		UnescapeString(tokenContent);
		defaultValue = std::move(tokenContent);
		return _STRING; 
	});

	CreateRule(R"(\'[^'\n\r]\')", [this]() { 
		auto& defaultValue = GetDefaultTokenValue();
		std::string tokenContent = GetTokenContent();
		RemoveQuotes(tokenContent, '\'');
		UnescapeString(tokenContent);
		defaultValue = tokenContent.front();
		return _CHARACTER; 
	});

	CreateRule(R"([A-Za-z][A-Za-z0-9\_]*)", [this]() {
		auto& defaultValue = GetDefaultTokenValue();
		defaultValue = GetTokenContent();
		return IDENTIFIER; 
	});

	CreateRule(R"([0-9]+\.[0-9]+f)", [this]() {
		auto& defaultValue = GetDefaultTokenValue();
		defaultValue = std::stof(GetTokenContent());
		return _FLOAT;
	});

	CreateRule(R"([0-9]+\.[0-9]+)", [this]() {
		auto& defaultValue = GetDefaultTokenValue();
		defaultValue = std::stod(GetTokenContent());
		return _DOUBLE; 
	});

	CreateRule(R"(true|false)", [this]() {
		auto& defaultValue = GetDefaultTokenValue();
		const std::string tokenContent = GetTokenContent();
		if (tokenContent == "true")
			defaultValue = true;
		else
			defaultValue = false;
		return _BOOL;
		});

	CreateRule(R"([0-9]+)", [this]() { 
		auto& defaultValue = GetDefaultTokenValue();
		defaultValue = std::stoi(GetTokenContent());
		return _INTEGER; 
	});

	CreateRule(R"([ \t]+)", [this]() { return TOKEN_IGNORE; });

	CreateRule(R"([\r\n]+)", [this]() { 
		const std::string& content = GetTokenContent();
		UpdateLineCountFromString(content);
		return TOKEN_IGNORE; 
	});

	GenerateDFA();

	/*Lexy::NFA nfa;

	int32_t state0 = nfa.GetFiniteAutomate().PushVertex();
	nfa.GetFiniteAutomate().GetVertex(state0).Data.StateId = state0;

	int32_t state1 = nfa.GetFiniteAutomate().PushVertex();
	nfa.GetFiniteAutomate().GetVertex(state1).Data.StateId = state1;

	int32_t state2 = nfa.GetFiniteAutomate().PushVertex();
	nfa.GetFiniteAutomate().GetVertex(state2).Data.StateId = state2;

	int32_t state3 = nfa.GetFiniteAutomate().PushVertex();
	nfa.GetFiniteAutomate().GetVertex(state3).Data.StateId = state3;

	nfa.GetFiniteAutomate().PushEdge(state0, state1, {});
	nfa.GetFiniteAutomate().PushEdge(state1, state1, { {'a'} });
	nfa.GetFiniteAutomate().PushEdge(state1, state2, { {'a'}, {'b'} });
	nfa.GetFiniteAutomate().PushEdge(state2, state2, { {'a'} });
	nfa.GetFiniteAutomate().PushEdge(state2, state0, { {'a'} });
	nfa.GetFiniteAutomate().PushEdge(state2, state3, { {'b'} });
	nfa.GetFiniteAutomate().PushEdge(state3, state1, { {'b'} });

	nfa.SetStart(state0);
	nfa.GetAccepting().insert(state0);
	nfa.Print();
	Lexy::DFA dfa;
	dfa.GenerateFromNFA(nfa);
	dfa.Print();
	exit(1);*/
}

TestLexer::TestLexer(const std::string& sourceCodePath)
	: TestLexer(std::ifstream(sourceCodePath))
{
}

void TestLexer::UpdateLineCountFromString(const std::string& contentStr)
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

void TestLexer::RemoveQuotes(std::string& string, char quotes)
{
	if (string.size() >= 2 && string.front() == quotes && string.back() == quotes)
	{
		string.erase(string.begin());
		string.pop_back();
	}
}

void TestLexer::UnescapeString(std::string& string)
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
