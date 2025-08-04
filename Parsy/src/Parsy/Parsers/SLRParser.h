#pragma once

#include "CLRParser.h"

namespace Parsy
{
	class SLRParser : public CLRParser
	{
	public:
		SLRParser(const std::ifstream& inputStream);
	private:
	};
}