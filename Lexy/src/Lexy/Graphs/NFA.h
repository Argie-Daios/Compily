#pragma once

#include "FA.h"

#include <string>

#define EPSILON {}

namespace Lexy
{
	struct CharacterData
	{
		char Character;
		bool IsEscaped = false;

		CharacterData() = default;
		CharacterData(const CharacterData&) = default;
		CharacterData(CharacterData&&) = default;
		bool operator==(const CharacterData& other) const
		{
			if (this == &other) return true;
			return Character == other.Character && IsEscaped == other.IsEscaped;
		}

		static CharacterData CharacterToCharacterData(const std::string& string, int32_t& index);
	};


	struct CharacterDataHash
	{
		size_t operator()(const CharacterData& data) const
		{
			size_t h1 = std::hash<int32_t>{}(static_cast<int32_t>(data.Character));
			size_t h2 = std::hash<bool>{}(data.IsEscaped);
			return h1 ^ (h2 << 1);
		}
	};

	struct NodeData
	{
		int32_t StateId = -1;
		int32_t TokenIndex = -1;
		int32_t Priority = 0;
		bool IsAccepting = false;

		bool operator==(const NodeData& other) const
		{
			if (this == &other) return true;
			return StateId == other.StateId && TokenIndex == other.TokenIndex && Priority == other.Priority
				&& IsAccepting == other.IsAccepting;
		}

		NodeData& operator=(const NodeData& other)
		{
			if (this == &other) return *this;
			StateId = other.StateId;
			TokenIndex = other.TokenIndex;
			Priority = other.Priority;
			IsAccepting = other.IsAccepting;
			return *this;
		}
	};

	struct NodeDataHash
	{
		size_t operator()(const NodeData& data) const
		{
			size_t h1 = std::hash<int32_t>{}(data.StateId);
			size_t h2 = std::hash<int32_t>{}(data.TokenIndex);
			size_t h3 = std::hash<int32_t>{}(data.Priority);
			size_t h4 = std::hash<bool>{}(data.IsAccepting);
			return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
		}
	};

	using NFAGraph = Utilities::Graph<NodeData, std::unordered_set<CharacterData, CharacterDataHash>>;
	class NFA : public FA
	{
	public:
		NFA() = default;
		NFA(const NFA& nfa);
		NFA(NFA&& other);

		virtual MatchResult Match(const std::string& input, int start) override;

		inline NFAGraph& GetFiniteAutomate() { return m_Graph; }
		inline size_t GetTotalStates() const { return m_Graph.GetTotalVertices(); }
		void Print();

		NFA& operator|=(NFA& nfa);
		NFA& operator&=(NFA& nfa);
		NFA& Kleene();
		NFA& Plus();
		NFA& QuestionMark();
	private:
		void ExpandEpsilonClosure(std::vector<int32_t>& states);
		void ExpandEpsilonClosure(std::unordered_set<int32_t>& states);
		bool IsStateAccepting(const std::vector<int32_t>& states, int32_t& maxPriorityTokenIndex);
	private:
		NFAGraph m_Graph;
		friend class DFA;
		friend class TestLexer;
	};
}