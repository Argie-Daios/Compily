#pragma once

#include "LR1.h"

namespace Parsy
{
	class CLR1 : public LR1
	{
	public:
		CLR1(class Parser* parserRef);
	};
}