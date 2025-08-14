#include "SLRParser.h"

#include "Parsy/BottomUp/SLR1.h"

namespace Parsy
{
	SLRParser::SLRParser(const std::ifstream& inputStream, int32_t flags)
		: LRParser(inputStream, flags)
	{
		m_LR1 = std::make_unique<SLR1>(this);
	}
}