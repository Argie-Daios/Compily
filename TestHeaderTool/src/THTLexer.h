#pragma once

#include "Lexy.h"

class THTLexer : public Lexy::Lexer
{
public:
	THTLexer(const std::ifstream& inputStream);
	THTLexer(const std::string& sourceCodePath);
private:
	void UpdateLineCountFromString(const std::string& contentStr);
	void RemoveQuotes(std::string& string, char quotes = '\"');
	void UnescapeString(std::string& string);
};