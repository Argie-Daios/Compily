#include <iostream>

#include "TokenType.h"
#include "MyParser.h"

#include <Utilities.h>

#include <assert.h>

int main()
{
	std::ifstream stream("SourceCode.src");
	std::ifstream stream2("SourceCode2.src");

	std::cout << "Pasring 1st file" << std::endl;
	std::cout << "===============================" << std::endl;

	MyParser parser(stream, "SourceCode.src");

	std::cout << "===============================" << std::endl << std::endl;

	std::cout << "Pasring 2nd file" << std::endl;
	std::cout << "===============================" << std::endl;

	MyParser parser2(stream2, "SourceCode2.src");

	std::cout << "===============================" << std::endl << std::endl;

	return 0;
}