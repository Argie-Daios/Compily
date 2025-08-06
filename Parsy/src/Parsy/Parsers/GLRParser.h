#pragma once

#include "Parser.h"

namespace Parsy
{
	struct GLRParseEntryData : public ParseEntryData
	{
		int32_t SymbolIndex = -1;
		int32_t ReducedFromState = -1;
		int32_t Elements = -1;

		GLRParseEntryData() = default;
		GLRParseEntryData(int32_t state, int32_t symbolIndex)
			: ParseEntryData(state), SymbolIndex(symbolIndex) {
		}
		GLRParseEntryData(int32_t state, int32_t symbolIndex, int32_t reducedFromState)
			: ParseEntryData(state), SymbolIndex(symbolIndex), ReducedFromState(reducedFromState) {}
		GLRParseEntryData(const GLRParseEntryData&) = default;
		GLRParseEntryData(GLRParseEntryData&&) = default;
		GLRParseEntryData& operator=(const GLRParseEntryData& other)
		{
			if (this == &other) return *this;

			ParseEntryData::operator=(other);
			ReducedFromState = other.ReducedFromState;
			SymbolIndex = other.SymbolIndex;
			Elements = other.Elements;

			return *this;
		}
	};

	struct GLRGSSEdgeData
	{
		uint32_t Distance = 0;
		BottomUpActionType ActionType;
		int32_t RuleID = -1;
		int32_t Production = -1;

		GLRGSSEdgeData() = default;
		GLRGSSEdgeData(uint32_t distance, BottomUpActionType actionType)
			: Distance(distance), ActionType(actionType) {}
		GLRGSSEdgeData(const GLRGSSEdgeData&) = default;
		GLRGSSEdgeData(GLRGSSEdgeData&&) = default;
		GLRGSSEdgeData& operator=(const GLRGSSEdgeData& other)
		{
			if (this == &other) return *this;

			Distance = other.Distance;
			ActionType = other.ActionType;
			RuleID = other.RuleID;
			Production = other.Production;

			return *this;
		}
	};

	using GSSGraph = Utilities::Graph<GLRParseEntryData, GLRGSSEdgeData>;
	struct GLRGSS
	{
		GSSGraph m_GSSGraph;
		int32_t m_Start;
		std::vector<int32_t> m_AcceptStates;
		std::vector<int32_t> m_CurrentStates;
	};

	struct ActionData
	{
		int32_t State;
		BottomUpActionType Type;
		int32_t RuleID = -1;
		int32_t Production = -1;
		int32_t ReducedElementCount = -1;

		ActionData() = default;
		ActionData(int32_t state, BottomUpActionType type)
			: State(state), Type(type)
		{}
		ActionData(int32_t state, BottomUpActionType type, int32_t ruleID, int32_t production,
			int32_t reducedElementCount)
			: State(state), Type(type), RuleID(ruleID), Production(production), ReducedElementCount(reducedElementCount)
		{ }
		ActionData(const ActionData&) = default;
		ActionData(ActionData&&) = default;
		ActionData& operator=(const ActionData& other)
		{
			if (this == &other) return *this;

			State = other.State;
			Type = other.Type;
			RuleID = other.RuleID;
			Production = other.Production;
			ReducedElementCount = other.ReducedElementCount;

			return *this;
		}
	};

	class GLRParser : public Parser
	{
	public:
		GLRParser(const std::ifstream& inputStream);

		virtual bool Parse() override;
		virtual std::any& Get(int32_t offset) override;
	private:
		void Shift(int32_t currentStateIndex, const BottomUpAction& action,
			int32_t tokenIndex);
		void Reduce(int32_t currentStateIndex, const BottomUpActionData& actionData,
			int32_t tokenIndex);
		void ExtractMinimumSpanningStack();
		bool ExecuteSemanticAnalysis();
		CFGElement GetLastPushedTerminal(int32_t currentIndex);

		/////GSS/////////////////////////////////////////////////////////////////////////////////////

		template<typename ... Args>
		int32_t PushStateOnGSS(Args&& ... args)
		{
			return PushStateOnGSSIndexed(0, std::forward<Args>(args)...);
		}
		
		template<typename ... Args>
		int32_t PushStateOnGSSIndexed(int32_t currentStateIndex, Args&& ... args)
		{
			int32_t id = m_GSS.m_GSSGraph.PushVertex(std::forward<Args>(args)...);
			if (m_GSS.m_CurrentStates.empty())
				m_GSS.m_CurrentStates.push_back(id);
			else
			{
				int32_t& currentStateID = m_GSS.m_CurrentStates.at(currentStateIndex);
				m_GSS.m_GSSGraph.PushEdge(id, currentStateID, GLRGSSEdgeData());
				currentStateID = id;
			}

			return id;
		}

		int32_t GetMinimumAcceptState()
		{
			uint32_t minDistance = UINT_MAX;
			int32_t minAcceptState = -1;
			for (int32_t acceptState : m_GSS.m_AcceptStates)
			{
				int32_t minimumDistance = GetMinimumDistanceToStateWithID(acceptState);
				if (minimumDistance < minDistance)
				{
					minDistance = minimumDistance;
					minAcceptState = acceptState;
				}
			}
			return minAcceptState;
		}

		GLRParseEntryData& GetStateData(int32_t currentStateIndex)
		{
			int32_t id = m_GSS.m_CurrentStates.at(currentStateIndex);
			return m_GSS.m_GSSGraph.GetVertex(id).Data;
		}

		uint32_t GetMinimumDistanceToState(int32_t currentStateIndex)
		{
			int32_t id = m_GSS.m_CurrentStates.at(currentStateIndex);
			return GetMinimumDistanceToStateWithID(id);
		}

		uint32_t GetMinimumDistanceToStateWithID(int32_t stateID)
		{
			auto& edges = m_GSS.m_GSSGraph.GetEdgesOfVertex(stateID);
			if (edges.empty()) return 0;
			int32_t minDistance = UINT_MAX;
			for (const GSSGraph::Edge& edge : edges)
			{
				const GLRGSSEdgeData& edgeData = edge.Data;
				minDistance = (edgeData.Distance < minDistance ? edgeData.Distance : minDistance);
			}
			return minDistance;
		}

		const GSSGraph::Edge* GetMinimumDistanceEdgeToState(int32_t stateID)
		{
			auto& edges = m_GSS.m_GSSGraph.GetEdgesOfVertex(stateID);
			if (edges.empty()) return 0;
			int32_t minDistance = UINT_MAX;
			GSSGraph::Edge* minEdge = nullptr;
			for (GSSGraph::Edge& edge : edges)
			{
				const GLRGSSEdgeData& edgeData = edge.Data;
				if (edgeData.Distance < minDistance)
				{
					minDistance = edgeData.Distance;
					minEdge = &edge;
				}
			}
			return minEdge;
		}

		int32_t ForkStateOnGSS(int32_t vertexID)
		{
			m_GSS.m_CurrentStates.push_back(vertexID);
			return m_GSS.m_CurrentStates.size() - 1;
		}

		int32_t ForkStateOnGSSIndexed(int32_t currentStateIndex)
		{
			int32_t vertexID = m_GSS.m_CurrentStates.at(currentStateIndex);
			m_GSS.m_CurrentStates.push_back(vertexID);
			return m_GSS.m_CurrentStates.size() - 1;
		}

		std::vector<int32_t> GetAncestors(int32_t currentStateIndex, size_t steps)
		{
			int32_t vertexID = m_GSS.m_CurrentStates.at(currentStateIndex);
			std::vector<int32_t> currentLayer = { vertexID };
			std::vector<int32_t> nextLayer;

			for (size_t i = 0; i < steps; ++i)
			{
				if (currentLayer.empty()) break;
				for (int32_t node : currentLayer)
				{
					const auto& edges = m_GSS.m_GSSGraph.GetEdgesOfVertex(node);
					for (const auto& edge : edges)
					{
						nextLayer.push_back(edge.Destination);
					}
				}
				currentLayer = nextLayer;
				nextLayer.clear();
			}

			return currentLayer;
		}

		int32_t GetStateBefore(int32_t state);

		/////////////////////////////////////////////////////////////////////////////////////////////
	private:
		GLRGSS m_GSS;
		std::vector<Lexy::Lexer::OfflineToken> m_TokenStream;
		std::vector<ActionData> m_Stack;
		int32_t m_Elements = -1;
		int32_t m_CurrentAction = -1;
	};
}