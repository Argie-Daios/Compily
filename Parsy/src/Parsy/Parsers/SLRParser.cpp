#include "SLRParser.h"

#include "Parsy/BottomUp/SLR1.h"

namespace Parsy
{
	SLRParser::SLRParser(const std::ifstream& inputStream)
		: CLRParser(inputStream)
	{
		m_LR1 = std::make_unique<SLR1>(this);
	}
}