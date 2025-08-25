#pragma once

#include "NFA.h"

namespace Lexy
{
	using DFAGraph = Utilities::Graph<std::unordered_set<NodeData, NodeDataHash>, CharacterData>;
	class DFA : public FA
	{
	public:
		DFA() = default;
		DFA(const DFA&) = default;
		DFA(DFA&&) noexcept = default;

		virtual MatchResult Match(const std::string& input, int start) override;

		inline DFAGraph& GetFiniteAutomate() { return m_Graph; }
		inline size_t GetTotalStates() const { return m_Graph.GetTotalVertices(); }
		void Print();

		bool IsSubsetOrEqual(const CharacterData& charData,
			const std::unordered_set<CharacterData, CharacterDataHash>& set);
		void GenerateFromNFA(NFA& nfa);
	private:
		bool IsStateAccepting(int32_t state, int32_t& maxPriorityTokenIndex);
	private:
		DFAGraph m_Graph;
	};
}