#pragma once

#include "Lexy.h"

class TestLexer : public Lexy::Lexer
{
public:
	TestLexer(const std::ifstream& inputStream);
	TestLexer(const std::string& sourceCodePath);
private:
	void UpdateLineCountFromString(const std::string& contentStr);
	void RemoveQuotes(std::string& string, char quotes = '\"');
	void UnescapeString(std::string& string);
};