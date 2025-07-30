#pragma once

#include "Parsy.h"

class MyParser : public Parsy::GLRParser
{
public:
	MyParser(const std::ifstream& inputStream);
private:
	int32_t ExecuteOperation(int32_t opCode, int32_t leftValue, int32_t rightValue);
};