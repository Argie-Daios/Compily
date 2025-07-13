#include <iostream>

#include "MyLexer.h"

int main()
{
	MyLexer lexer(R"(
	if(x == 2.0)
	{
		x = x / 2.0;
	}
	else
	{
		x = 0.0;
	}
	)");
	return 0;
}