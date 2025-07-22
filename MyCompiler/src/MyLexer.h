#pragma once

#include "Lexy.h"

class MyLexer : public Lexy::Lexer
{
public:
	MyLexer(const std::ifstream& inputStream);
};