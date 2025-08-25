#include "SLRParser.h"

#include "Parsy/BottomUp/SLR1.h"

namespace Parsy
{
	SLRParser::SLRParser(int32_t flags)
		: LRParser(flags)
	{
		m_LR1 = std::make_unique<SLR1>(this);
	}
}