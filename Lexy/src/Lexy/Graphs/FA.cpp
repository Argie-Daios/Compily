#include "FA.h"

namespace Lexy
{
	FA::FA()
	{
		s_ConstructCount++;
	}

	FA::FA(const FA& fa)
		: m_Start(fa.m_Start), m_Accepting(fa.m_Accepting)
	{
		s_CopyCount++;
	}

	FA::FA(FA&& other) noexcept
		: m_Start(other.m_Start), m_Accepting(other.m_Accepting)
	{
		s_MoveCount++;
	}
}