#pragma once

#include "LRParser.h"

namespace Parsy
{
	class SLRParser : public LRParser
	{
	public:
		SLRParser(int32_t flags = 0);
	private:
	};
}