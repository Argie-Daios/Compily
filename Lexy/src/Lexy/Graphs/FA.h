#pragma once

#include "Graph.h"

#define WILDCARD '\xFF'

namespace Lexy
{
	using FAGraph = Graph<bool, char>;

	class FA
	{
	public:
		FA();
		FA(const FA& fa);
		FA(FA&& other) noexcept;

		virtual bool IsAccepting(const std::string& string) = 0;
		virtual int Match(const std::string& input, int start) = 0;

		inline FAGraph& GetFiniteAutomate() { return m_Graph; }
		inline size_t GetTotalStates() const { return m_Graph.GetTotalVertices(); }

		inline void SetStart(int32_t start) { m_Start = start; }
		inline void SetAccepting(int32_t accepting) { m_Accepting = accepting; }
		inline int32_t GetStart() const { return m_Start; }
		inline int32_t GetAccepting() const { return m_Accepting; }
	protected:
		FAGraph m_Graph;
		int32_t m_Start;
		int32_t m_Accepting;
	public:
		inline static uint32_t s_ConstructCount = 0U;
		inline static uint32_t s_CopyCount = 0U;
		inline static uint32_t s_MoveCount = 0U;
	};
}