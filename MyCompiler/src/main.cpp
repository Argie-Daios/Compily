#include <iostream>

#include "TokenType.h"
#include "MyParser.h"

int main()
{
	std::ifstream stream("SourceCode.src");
	//MyLexer lexer(stream);

	MyParser parser(stream);

	return 0;
}