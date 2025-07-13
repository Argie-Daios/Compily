#include "NFA.h"

#include <vector>
#include <unordered_set>
#include <algorithm>

namespace Lexy
{
	void NFA::ExpandEpsilonClosure(std::vector<int32_t>& states)
	{
		std::unordered_set<int32_t> visited(states.begin(), states.end());
		std::vector<int32_t> stack(states.begin(), states.end());

		while (!stack.empty())
		{
			int32_t stateID = stack.back();
			stack.pop_back();

			auto& edges = m_Graph.GetEdgesOfVertex(stateID);
			for (const auto& edge : edges)
			{
				if (edge.Data == EPSILON && visited.find(edge.Destination) == visited.end())
				{
					visited.insert(edge.Destination);
					stack.push_back(edge.Destination);
					states.push_back(edge.Destination);
				}
			}
		}
	}

	bool NFA::IsAccepting(const std::string& string)
	{
		auto& vertices = m_Graph.GetVertices();
		if (vertices.empty()) return false;
		std::vector<int32_t> currentStates = { m_Start };
		ExpandEpsilonClosure(currentStates);

		for (char character : string)
		{
			std::vector<int32_t> nextStates;

			for (int32_t stateID : currentStates)
			{
				auto& edges = m_Graph.GetEdgesOfVertex(stateID);
				for (const auto& edge : edges)
				{
					if (edge.Data == WILDCARD || edge.Data == character)
					{
						nextStates.push_back(edge.Destination);
					}
				}
			}

			if (nextStates.empty())
				return false;

			ExpandEpsilonClosure(nextStates);

			currentStates = std::move(nextStates);
		}

		
		return std::find(currentStates.begin(), currentStates.end(), m_Accepting)
			!= currentStates.end();
	}

	int NFA::Match(const std::string& input, int start)
	{
		int maxLength = -1;

		std::vector<int32_t> currentStates = { m_Start };
		ExpandEpsilonClosure(currentStates);

		if (std::find(currentStates.begin(), currentStates.end(), m_Accepting) != currentStates.end()) 
		{
			maxLength = 0;
		}

		for (int i = start; i < input.length(); ++i) 
		{
			char character = input[i];
			std::vector<int32_t> nextStates;

			for (int32_t stateID : currentStates) 
			{
				auto& edges = m_Graph.GetEdgesOfVertex(stateID);
				for (const auto& edge : edges) 
				{
					if (edge.Data == WILDCARD || edge.Data == character) 
					{
						nextStates.push_back(edge.Destination);
					}
				}
			}

			if (nextStates.empty()) 
			{
				break;
			}

			ExpandEpsilonClosure(nextStates);
			currentStates = std::move(nextStates);

			if (std::find(currentStates.begin(), currentStates.end(), m_Accepting) != currentStates.end()) 
			{
				maxLength = i - start + 1;
			}
		}

		return maxLength;
	}

	NFA operator|(NFA& nfaLeft, NFA& nfaRight)
	{
		NFA newNFA;

		newNFA.m_Graph |= nfaLeft.m_Graph;

		int32_t rightStart = (int32_t)newNFA.GetTotalStates() + nfaRight.m_Start;
		int32_t rightAccepting = (int32_t)newNFA.GetTotalStates() + nfaRight.m_Accepting;

		newNFA.m_Graph |= nfaRight.m_Graph;

		newNFA.m_Start = newNFA.m_Graph.PushVertex(" ");
		newNFA.m_Graph.PushEdge(newNFA.m_Start, nfaLeft.m_Start, EPSILON);
		newNFA.m_Graph.PushEdge(newNFA.m_Start, rightStart, EPSILON);

		newNFA.m_Accepting = newNFA.m_Graph.PushVertex(" ");
		newNFA.m_Graph.PushEdge(nfaLeft.m_Accepting, newNFA.m_Accepting, EPSILON);
		newNFA.m_Graph.PushEdge(rightAccepting, newNFA.m_Accepting, EPSILON);

		return newNFA;
	}

	NFA operator&(NFA& nfaLeft, NFA& nfaRight)
	{
		NFA newNFA;

		newNFA.m_Graph |= nfaLeft.m_Graph;

		int32_t rightStart = (int32_t)newNFA.GetTotalStates() + nfaRight.m_Start;
		int32_t rightAccepting = (int32_t)newNFA.GetTotalStates() + nfaRight.m_Accepting;

		newNFA.m_Graph |= nfaRight.m_Graph;

		newNFA.m_Start = nfaLeft.m_Start;
		newNFA.m_Accepting = rightAccepting;
		newNFA.m_Graph.PushEdge(nfaLeft.m_Accepting, rightStart, EPSILON);

		return newNFA;
	}

	NFA& NFA::operator|=(NFA& nfa)
	{
		if (this == &nfa) return *this;

		int32_t rightStart = (int32_t)GetTotalStates() + nfa.m_Start;
		int32_t rightAccepting = (int32_t)GetTotalStates() + nfa.m_Accepting;

		m_Graph |= nfa.m_Graph;

		int32_t newStart = m_Graph.PushVertex(" ");
		m_Graph.PushEdge(newStart, m_Start, EPSILON);
		m_Graph.PushEdge(newStart, rightStart, EPSILON);

		int32_t newAccepting = m_Graph.PushVertex(" ");
		m_Graph.PushEdge(m_Accepting, newAccepting, EPSILON);
		m_Graph.PushEdge(rightAccepting, newAccepting, EPSILON);

		m_Start = newStart;
		m_Accepting = newAccepting;

		return *this;
	}

	NFA& NFA::operator&=(NFA& nfa)
	{
		int32_t rightStart = (int32_t)GetTotalStates() + nfa.m_Start;
		int32_t rightAccepting = (int32_t)GetTotalStates() + nfa.m_Accepting;

		m_Graph |= nfa.m_Graph;

		m_Graph.PushEdge(m_Accepting, rightStart, EPSILON);
		m_Accepting = rightAccepting;

		return *this;
	}

	NFA& NFA::Kleene()
	{
		int32_t newStart = m_Graph.PushVertex(" ");
		int32_t newAccepting = m_Graph.PushVertex(" ");

		m_Graph.PushEdge(newStart, m_Start, EPSILON);
		m_Graph.PushEdge(newStart, newAccepting, EPSILON);
		m_Graph.PushEdge(m_Accepting, newAccepting, EPSILON);
		m_Graph.PushEdge(m_Accepting, m_Start, EPSILON);

		m_Start = newStart;
		m_Accepting = newAccepting;

		return *this;
	}

	NFA& NFA::Plus()
	{
		int32_t newStart = m_Graph.PushVertex(" ");
		int32_t newAccepting = m_Graph.PushVertex(" ");

		m_Graph.PushEdge(newStart, m_Start, EPSILON);
		m_Graph.PushEdge(m_Accepting, newAccepting, EPSILON);
		m_Graph.PushEdge(m_Accepting, m_Start, EPSILON);

		m_Start = newStart;
		m_Accepting = newAccepting;

		return *this;
	}

	NFA& NFA::QuestionMark()
	{
		int32_t newStart = m_Graph.PushVertex(" ");
		int32_t newAccepting = m_Graph.PushVertex(" ");

		m_Graph.PushEdge(newStart, m_Start, EPSILON);
		m_Graph.PushEdge(newStart, newAccepting, EPSILON);
		m_Graph.PushEdge(m_Accepting, newAccepting, EPSILON);

		m_Start = newStart;
		m_Accepting = newAccepting;

		return *this;
	}
}