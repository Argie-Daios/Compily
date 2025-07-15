#pragma once

#include "Lexy.h"

class MyLexer : Lexy::Lexer
{
public:
	MyLexer(const std::ifstream& inputStream);
};