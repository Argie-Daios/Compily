#pragma once

#include <Utilities.h>
#include <unordered_set>

namespace Lexy
{
	struct MatchResult
	{
		int32_t Length = -1;
		int32_t TokenIndex = -1;
	};

	class FA
	{
	public:
		FA();
		FA(const FA& fa);
		FA(FA&& other) noexcept;

		virtual MatchResult Match(const std::string& input, int start) = 0;

		inline void SetStart(int32_t start) { m_Start = start; }
		inline int32_t GetStart() const { return m_Start; }
		inline std::unordered_set<int32_t>& GetAccepting() { return m_Accepting; }
	protected:
		int32_t m_Start;
		std::unordered_set<int32_t> m_Accepting;
	public:
		inline static uint32_t s_ConstructCount = 0U;
		inline static uint32_t s_CopyCount = 0U;
		inline static uint32_t s_MoveCount = 0U;
	};
}