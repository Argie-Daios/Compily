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

			auto edges = m_Graph.GetEdgesOfVertex(stateID);
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
		auto vertices = m_Graph.GetVertices();
		if (vertices.empty()) return false;
		std::vector<int32_t> currentStates = { m_Start };
		ExpandEpsilonClosure(currentStates);

		for (char character : string)
		{
			std::vector<int32_t> nextStates;

			for (int32_t stateID : currentStates)
			{
				auto edges = m_Graph.GetEdgesOfVertex(stateID);
				for (const auto& edge : edges)
				{
					if (edge.Data == character)
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

	NFA operator|(NFA& nfaLeft, NFA& nfaRight)
	{
		NFA newNFA;

		newNFA.m_Graph |= nfaLeft.m_Graph;

		int32_t rightStart = (int32_t)newNFA.GetTotalStates() + nfaRight.m_Start;
		int32_t rightAccepting = (int32_t)newNFA.GetTotalStates() + nfaRight.m_Accepting;

		newNFA.m_Graph |= nfaRight.m_Graph;

		newNFA.m_Start = newNFA.m_Graph.PushVertex(false);
		newNFA.m_Graph.PushEdge(newNFA.m_Start, nfaLeft.m_Start, EPSILON);
		newNFA.m_Graph.PushEdge(newNFA.m_Start, rightStart, EPSILON);

		newNFA.m_Accepting = newNFA.m_Graph.PushVertex(false);
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

	NFA NFA::Kleene()
	{
		NFA newNFA;

		newNFA.m_Graph = m_Graph;

		newNFA.m_Start = newNFA.m_Graph.PushVertex(false);
		newNFA.m_Accepting = newNFA.m_Graph.PushVertex(false);

		newNFA.m_Graph.PushEdge(newNFA.m_Start, m_Start, EPSILON);
		newNFA.m_Graph.PushEdge(newNFA.m_Start, newNFA.m_Accepting, EPSILON);
		newNFA.m_Graph.PushEdge(m_Accepting, newNFA.m_Accepting, EPSILON);
		newNFA.m_Graph.PushEdge(m_Accepting, m_Start, EPSILON);

		return newNFA;
	}
}