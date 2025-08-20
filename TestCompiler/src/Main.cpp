#include "TestParser.h"

int main()
{
	std::ifstream stream("Source.src");
	TestParser parser(stream);
	return 0;
}