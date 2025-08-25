#include "DFA.h"

namespace Lexy
{
	struct CharacterDataSetHash 
	{
		std::size_t operator()(const std::unordered_set<CharacterData, CharacterDataHash>& s) const noexcept
		{
			std::size_t h = 0;
			for (const auto& c : s) 
			{
				h ^= CharacterDataHash{}(c)+0x9e3779b9 + (h << 6) + (h >> 2);
			}
			return h;
		}
	};


	struct IntSetHash
	{
		std::size_t operator()(const std::unordered_set<int32_t>& s) const noexcept 
		{
			std::size_t h = 0;
			for (int32_t c : s) 
			{
				h += std::hash<int32_t>{}(c);
			}
			return h;
		}
	};

	struct IntSetEqual 
	{
		bool operator()(const std::unordered_set<int32_t>& a,
			const std::unordered_set<int32_t>& b) const noexcept 
		{
			return a == b;
		}
	};

	MatchResult DFA::Match(const std::string& input, int start)
	{
		MatchResult result;
		result.Length = -1;

		int32_t currentState = m_Start;

		int32_t maxPriorityTokenIndex = -1;
		if (IsStateAccepting(currentState, maxPriorityTokenIndex))
		{
			result.Length = 0;
			result.TokenIndex = maxPriorityTokenIndex;
		}

		for (int i = start; i < input.length(); ++i)
		{
			char character = input[i];
			CharacterData& data = CharacterData::CharacterToCharacterData(input, i);

			auto& edges = m_Graph.GetEdgesOfVertex(currentState);
			for (const auto& edge : edges)
			{
				if (edge.Data == data)
				{
					currentState = edge.Destination;
					break;
				}
			}

			auto& nodeSet = m_Graph.GetVertex(currentState).Data;
			if (nodeSet.empty())
			{
				break;
			}

			int32_t maxPriorityTokenIndex = -1;
			if (IsStateAccepting(currentState, maxPriorityTokenIndex))
			{
				result.Length = i - start + 1;
				result.TokenIndex = maxPriorityTokenIndex;
			}
		}

		return result;
	}

	void DFA::Print()
	{
		std::ofstream stream("DFA.txt");

		const auto& vertices = m_Graph.GetVertices();
		std::vector<int32_t> stateGraphIDSSorted;
		stateGraphIDSSorted.reserve(vertices.size());
		for (const auto& [vertexID, vertexData] : vertices) stateGraphIDSSorted.push_back(vertexID);
		std::sort(stateGraphIDSSorted.begin(), stateGraphIDSSorted.end(), [](int32_t left, int32_t right) {
			return left < right;
			});
		for (int32_t vertexID : stateGraphIDSSorted)
		{
			const DFAGraph::Vertex& vertexData = vertices.at(vertexID);
			if (m_Accepting.find(vertexID) != m_Accepting.end())
				stream << "[State " << vertexID << "] (Accepting)\n";
			else
				stream << "[State " << vertexID << "]\n";
			stream << "Edges = { ";
			const auto& edges = m_Graph.GetEdgesOfVertex(vertexID);
			for (size_t i = 0; i < edges.size(); i++)
			{
				const DFAGraph::Edge& edge = edges.at(i);
				stream << "State" << edge.Destination << "[";
				stream << edge.Data.Character;
				stream << "]";
				if (i < edges.size() - 1)
					stream << ", ";
			}
			stream << " }\n";
			for (auto& vertexNodeData : vertexData.Data)
			{
				stream << "\tNFAState: " << vertexNodeData.StateId << ", TokenIndex: " << vertexNodeData.TokenIndex <<
					", Priority: " << vertexNodeData.Priority;
				stream << '\n';
			}
			stream << '\n';
		}

		stream.close();
	}

	bool DFA::IsSubsetOrEqual(const CharacterData& charData,
		const std::unordered_set<CharacterData, CharacterDataHash>& set)
	{
		return set.find(charData) != set.end();
	}

	void DFA::GenerateFromNFA(NFA& nfa)
	{
		for (int32_t accepting : nfa.m_Accepting)
		{
			nfa.m_Graph.GetVertex(accepting).Data.IsAccepting = true;
		}

		// Store all unique edges
		std::unordered_set<CharacterData, CharacterDataHash> edgesSet;
		auto& vertices = nfa.m_Graph.GetVertices();
		for (auto& [vertexID, vertex] : vertices)
		{
			vertex.Data.StateId = vertexID;
			auto& edges = nfa.m_Graph.GetEdgesOfVertex(vertexID);
			for (auto& edge : edges)
			{
				for (const CharacterData& charData : edge.Data)
				{
					if (edgesSet.find(charData) != edgesSet.end()) continue;
					if (edge.Data.empty()) continue;
					edgesSet.insert(charData);
				}
			}
		}

		// Create start state
		std::vector<int32_t> states = { nfa.GetStart() };
		nfa.ExpandEpsilonClosure(states);

		int32_t id = m_Graph.PushVertex();
		m_Start = id;
		auto& startVertex = m_Graph.GetVertex(id);
		for (int32_t state : states)
		{
			auto& nfaState = nfa.m_Graph.GetVertex(state);
			startVertex.Data.insert(nfaState.Data);
		}

		std::vector<int32_t> stateStack = { id };
		std::unordered_map<std::unordered_set<int32_t>, int32_t, IntSetHash, IntSetEqual> stateMemo;
		while (!stateStack.empty())
		{
			int32_t top = stateStack.back();
			stateStack.pop_back();
			auto& topNodeDataSet = m_Graph.GetVertex(top).Data;
			// Generate edges
			std::unordered_set<NodeData, NodeDataHash>& nodeDataSet = m_Graph.GetVertex(top).Data;
			for (auto& edge : edgesSet)
			{
				// If set is empty(void state) we make edges to itself
				if (topNodeDataSet.empty())
				{
					m_Graph.PushEdge(top, top, edge);
					continue;
				}

				// Generate state set from edges
				std::unordered_set<int32_t> newStateData;
				for (auto& nodeData : nodeDataSet)
				{
					auto& edges = nfa.m_Graph.GetEdgesOfVertex(nodeData.StateId);
					for (auto& nodeEdge : edges)
					{
						if (IsSubsetOrEqual(edge, nodeEdge.Data))
						{
							newStateData.insert(nodeEdge.Destination);
						}
					}
				}
				
				// Expand epsilon closure
				if(!newStateData.empty())
					nfa.ExpandEpsilonClosure(newStateData);

				// Check if there is already a state with those states
				auto& it = stateMemo.find(newStateData);
				if (it != stateMemo.end())
				{
					m_Graph.PushEdge(top, it->second, edge);
					continue;
				}

				// Create new state of dfa, make edge, insert it on memo and push it on state stack for further expansion
				int32_t id = m_Graph.PushVertex();
				auto& startVertex = m_Graph.GetVertex(id);
				bool isAccepted = false;
				for (int32_t newState : newStateData)
				{
					NodeData& nodeData = nfa.m_Graph.GetVertex(newState).Data;
					startVertex.Data.insert(nodeData);
					if (nodeData.IsAccepting)
						isAccepted = true;
				}
				if (isAccepted)
					m_Accepting.insert(id);
				m_Graph.PushEdge(top, id, edge);
				stateMemo.emplace(newStateData, id);
				stateStack.push_back(id);
			}
		}
	}

	bool DFA::IsStateAccepting(int32_t state, int32_t& maxPriorityTokenIndex)
	{
		int32_t maxPriority = INT_MIN;
		bool isAccepting = false;

		auto& it = m_Accepting.find(state);
		if (it == m_Accepting.end()) return false;
		auto& nodeDataSet = m_Graph.GetVertex(state).Data;
		for (const NodeData& nodeData : nodeDataSet)
		{
			if (nodeData.IsAccepting && nodeData.Priority > maxPriority)
			{
				maxPriority = nodeData.Priority;
				maxPriorityTokenIndex = nodeData.TokenIndex;
				isAccepting = true;
			}
		}	
		return isAccepting;
	}
}