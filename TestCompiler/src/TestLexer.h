#pragma once

#include "Lexy.h"

class TestLexer : public Lexy::Lexer
{
public:
	TestLexer(const std::ifstream& inputStream);
};