#pragma once

#include "Parsy.h"

class MyParser : public Parsy::Parser
{
public:
	MyParser(const std::ifstream& inputStream);
};