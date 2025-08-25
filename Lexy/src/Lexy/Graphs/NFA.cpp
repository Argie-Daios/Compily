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
				if (edge.Data.empty() && visited.find(edge.Destination) == visited.end())
				{
					visited.insert(edge.Destination);
					stack.push_back(edge.Destination);
					states.push_back(edge.Destination);
				}
			}
		}
	}

	void NFA::ExpandEpsilonClosure(std::unordered_set<int32_t>& states)
	{
		std::vector<int32_t> stack(states.begin(), states.end());

		while (!stack.empty())
		{
			int32_t stateID = stack.back();
			stack.pop_back();

			auto& edges = m_Graph.GetEdgesOfVertex(stateID);
			for (const auto& edge : edges)
			{
				if (edge.Data.empty() && states.find(edge.Destination) == states.end())
				{
					states.insert(edge.Destination);
					stack.push_back(edge.Destination);
				}
			}
		}
	}

	CharacterData CharacterData::CharacterToCharacterData(const std::string& string, int32_t& index)
	{
		if (string.at(index) != '\\') return { string.at(index), false };

		index++;
		switch (string.at(index))
		{
		case 'n': return { '\n', true };
		case 't': return { '\t', true };
		case 'r': return { '\r', true };
		case '\\': return { '\\', true };
		case '"': return { '\"', true };
		case '\'': return { '\'', true };
		}

		std::cout << "Unsupported escaped character" << std::endl;
		return { EOF, false };
	}

	bool NFA::IsStateAccepting(const std::vector<int32_t>& states, int32_t& maxPriorityTokenIndex)
	{
		int32_t maxPriority = INT_MIN;
		bool isAccepting = false;
		for (int32_t id : states)
		{
			auto& it = m_Accepting.find(id);
			if (it == m_Accepting.end()) continue;
			NodeData& nodeData = m_Graph.GetVertex(id).Data;
			if (nodeData.Priority > maxPriority)
			{
				maxPriority = nodeData.Priority;
				maxPriorityTokenIndex = nodeData.TokenIndex;
				isAccepting = true;
			}
		}
		return isAccepting;
	}

	NFA::NFA(const NFA& nfa)
		: FA(nfa), m_Graph(nfa.m_Graph)
	{

	}

	NFA::NFA(NFA&& nfa)
		: FA(nfa), m_Graph(std::move(nfa.m_Graph))
	{

	}

	MatchResult NFA::Match(const std::string& input, int start)
	{
		MatchResult result;
		result.Length = -1;

		std::vector<int32_t> currentStates = { m_Start };
		ExpandEpsilonClosure(currentStates);

		int32_t maxPriorityTokenIndex = -1;
		if (IsStateAccepting(currentStates, maxPriorityTokenIndex))
		{
			result.Length = 0;
			result.TokenIndex = maxPriorityTokenIndex;
		}

		for (int i = start; i < input.length(); ++i)
		{
			char character = input[i];
			CharacterData& data = CharacterData::CharacterToCharacterData(input, i);
			std::vector<int32_t> nextStates;

			for (int32_t stateID : currentStates)
			{
				auto& edges = m_Graph.GetEdgesOfVertex(stateID);
				for (const auto& edge : edges)
				{
					if (edge.Data.find(data) != edge.Data.end())
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

			int32_t maxPriorityTokenIndex = -1;
			if (IsStateAccepting(currentStates, maxPriorityTokenIndex))
			{
				result.Length = i - start + 1;
				result.TokenIndex = maxPriorityTokenIndex;
			}
		}

		return result;
	}

	void NFA::Print()
	{
		std::ofstream stream("NFA.txt");

		const auto& vertices = m_Graph.GetVertices();
		std::vector<int32_t> stateGraphIDSSorted;
		stateGraphIDSSorted.reserve(vertices.size());
		for (const auto& [vertexID, vertexData] : vertices) stateGraphIDSSorted.push_back(vertexID);
		std::sort(stateGraphIDSSorted.begin(), stateGraphIDSSorted.end(), [](int32_t left, int32_t right) {
			return left < right;
			});
		for (int32_t vertexID : stateGraphIDSSorted)
		{
			const NFAGraph::Vertex& vertexData = vertices.at(vertexID);
			if(m_Accepting.find(vertexID) != m_Accepting.end())
				stream << "[State " << vertexID << "] (Accepting)\n";
			else
				stream << "[State " << vertexID << "]\n";
			stream << "Edges = { ";
			const auto& edges = m_Graph.GetEdgesOfVertex(vertexID);
			for (size_t i = 0; i < edges.size(); i++)
			{
				const NFAGraph::Edge& edge = edges.at(i);
				stream << "State" << edge.Destination << "[";
				if (edge.Data.size() == 1)
					stream << edge.Data.begin()->Character;
				else if (edge.Data.size() == 0)
					stream << "Epsilon";
				else
					stream << edge.Data.size();
				stream << "]";
				if (i < edges.size() - 1)
					stream << ", ";
			}
			stream << " }\n";
			stream << "\tTokenIndex: " << vertexData.Data.TokenIndex << ", Priority: " << vertexData.Data.Priority;
			stream << '\n' << '\n';
		}

		stream.close();
	}

	NFA& NFA::operator|=(NFA& nfa)
	{
		if (this == &nfa) return *this;

		int32_t rightStart = (int32_t)GetTotalStates() + nfa.m_Start;
		int32_t rightAccepting = (int32_t)GetTotalStates() + *nfa.m_Accepting.begin();

		m_Graph |= nfa.m_Graph;

		int32_t newStart = m_Graph.PushVertex();
		m_Graph.PushEdge(newStart, m_Start, EPSILON);
		m_Graph.PushEdge(newStart, rightStart, EPSILON);

		int32_t newAccepting = m_Graph.PushVertex();
		m_Graph.PushEdge(*m_Accepting.begin(), newAccepting, EPSILON);
		m_Graph.PushEdge(rightAccepting, newAccepting, EPSILON);

		m_Start = newStart;
		m_Accepting.clear();
		m_Accepting.insert(newAccepting);

		return *this;
	}

	NFA& NFA::operator&=(NFA& nfa)
	{
		int32_t rightStart = (int32_t)GetTotalStates() + nfa.m_Start;
		int32_t rightAccepting = (int32_t)GetTotalStates() + *nfa.m_Accepting.begin();

		m_Graph |= nfa.m_Graph;

		m_Graph.PushEdge(*m_Accepting.begin(), rightStart, EPSILON);
		m_Accepting.clear();
		m_Accepting.insert(rightAccepting);

		return *this;
	}

	NFA& NFA::Kleene()
	{
		int32_t newStart = m_Graph.PushVertex();
		int32_t newAccepting = m_Graph.PushVertex();

		m_Graph.PushEdge(newStart, m_Start, EPSILON);
		m_Graph.PushEdge(newStart, newAccepting, EPSILON);
		m_Graph.PushEdge(*m_Accepting.begin(), newAccepting, EPSILON);
		m_Graph.PushEdge(*m_Accepting.begin(), m_Start, EPSILON);

		m_Start = newStart;
		m_Accepting.clear();
		m_Accepting.insert(newAccepting);

		return *this;
	}

	NFA& NFA::Plus()
	{
		int32_t newStart = m_Graph.PushVertex();
		int32_t newAccepting = m_Graph.PushVertex();

		m_Graph.PushEdge(newStart, m_Start, EPSILON);
		m_Graph.PushEdge(*m_Accepting.begin(), newAccepting, EPSILON);
		m_Graph.PushEdge(*m_Accepting.begin(), m_Start, EPSILON);

		m_Start = newStart;
		m_Accepting.clear();
		m_Accepting.insert(newAccepting);

		return *this;
	}

	NFA& NFA::QuestionMark()
	{
		int32_t newStart = m_Graph.PushVertex();
		int32_t newAccepting = m_Graph.PushVertex();

		m_Graph.PushEdge(newStart, m_Start, EPSILON);
		m_Graph.PushEdge(newStart, newAccepting, EPSILON);
		m_Graph.PushEdge(*m_Accepting.begin(), newAccepting, EPSILON);

		m_Start = newStart;
		m_Accepting.clear();
		m_Accepting.insert(newAccepting);

		return *this;
	}
}