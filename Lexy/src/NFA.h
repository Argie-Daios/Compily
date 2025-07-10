#pragma once

#include "Graph.h"

#include <string>

#define EPSILON '\0'

namespace Lexy
{
	using FA = Graph<bool, char>;

	class NFA
	{
	public:
		NFA() = default;

		bool IsAccepting(const std::string& string);

		inline FA& GetFiniteAutomate() { return m_Graph; }
		inline size_t GetTotalStates() const { return m_Graph.GetTotalVertices(); }

		inline void SetStart(int32_t start) { m_Start = start; }
		inline void SetAccepting(int32_t accepting) { m_Accepting = accepting; }
		inline int32_t GetStart() const { return m_Start; }
		inline int32_t GetAccepting() const { return m_Accepting; }

		friend NFA operator|(NFA& nfaLeft, NFA& nfaRight);
		friend NFA operator&(NFA& nfaLeft, NFA& nfaRight);
		NFA Kleene();
	private:
		void ExpandEpsilonClosure(std::vector<int32_t>& states);
	private:
		FA m_Graph;
		int32_t m_Start;
		int32_t m_Accepting;
	};
}