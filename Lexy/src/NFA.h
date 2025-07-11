#pragma once

#include "Graph.h"

#include <string>

#define EPSILON '\0'
#define WILDCARD '\xFF'

namespace Lexy
{
	using FA = Graph<std::string, char>;

	class NFA
	{
	public:
		NFA()
		{
			s_ConstructCount++;
		}

		NFA(const NFA& nfa)
			: m_Graph(nfa.m_Graph), m_Start(nfa.m_Start), m_Accepting(nfa.m_Accepting)
		{
			s_CopyCount++;
		}

		bool IsAccepting(const std::string& string);

		inline FA& GetFiniteAutomate() { return m_Graph; }
		inline size_t GetTotalStates() const { return m_Graph.GetTotalVertices(); }

		inline void SetStart(int32_t start) { m_Start = start; }
		inline void SetAccepting(int32_t accepting) { m_Accepting = accepting; }
		inline int32_t GetStart() const { return m_Start; }
		inline int32_t GetAccepting() const { return m_Accepting; }

		friend NFA operator|(NFA& nfaLeft, NFA& nfaRight);
		friend NFA operator&(NFA& nfaLeft, NFA& nfaRight);
		NFA& operator|=(NFA& nfa);
		NFA& operator&=(NFA& nfa);
		NFA& Kleene();
		NFA& Plus();
		NFA& QuestionMark();
	private:
		void ExpandEpsilonClosure(std::vector<int32_t>& states);
	private:
		FA m_Graph;
		int32_t m_Start;
		int32_t m_Accepting;
	public:
		inline static uint32_t s_ConstructCount = 0U;
		inline static uint32_t s_CopyCount = 0U;
	};
}