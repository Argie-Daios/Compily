#include "GLRParser.h"

#include "Parsy/BottomUp/SLR1.h"

namespace Parsy
{
	GLRParser::GLRParser(const std::ifstream& inputStream)
		: Parser(inputStream)
	{
		m_LR1 = std::make_unique<SLR1>(this);
	}

	bool GLRParser::Parse()
	{

		if (m_Lexer == nullptr)
		{
			std::cout << "Null" << std::endl;
			return false;
		}
		m_LR1->GenerateFirstSets();
		m_LR1->GenerateFollowSets();
		m_LR1->GenerateStateGraph();
		m_LR1->GenerateTable();
		m_LR1->PrintStateGraph();
		m_LR1->PrintTable();
		Print();

		PushStateOnGSS(0, 0);
		m_TokenStream.clear();
		m_TokenStream = m_Lexer->Tokenize();
		while (true)
		{
			size_t size = m_GSS.m_CurrentStates.size();
			std::vector<int32_t> deletionQueue;
			for (int32_t i = 0; i < size; i++)
			{
				int32_t vertexID = m_GSS.m_CurrentStates.at(i);
				GLRParseEntryData& entryData = m_GSS.m_GSSGraph.GetVertex(vertexID).Data;
				Lexy::Lexer::Token& token = m_TokenStream.at(entryData.SymbolIndex).TokenData;
				CFGElement& tokenElement = GetTokenElement(token);
				BottomUpAction& action = m_LR1->GetAction(entryData.State, tokenElement);
				switch (action.Type)
				{
				case BottomUpActionType::Shift:
				{
					std::cout << "Shift" << std::endl;
					Shift(i, action, entryData.SymbolIndex);
					break;
				}
				case BottomUpActionType::Reduce:
				{
					std::cout << "Reduce" << std::endl;
					const BottomUpActionData* reduceActionData = action.GetActionData(BottomUpActionType::Reduce);
					Reduce(i, *reduceActionData, entryData.SymbolIndex);
					break;
				}
				/*case BottomUpActionType::ShiftReduce:
				{
					std::cout << "ShiftReduce" << std::endl;
					int32_t forkedStateIndex = ForkStateOnGSSIndexed(i);
					Shift(i, action, entryData.SymbolIndex);
					Reduce(forkedStateIndex, action, entryData.SymbolIndex);
					break;
				}
				case BottomUpActionType::ReduceReduce:
				{
					std::cout << "ReduceReduce" << std::endl;
					int32_t forkedStateIndex = ForkStateOnGSSIndexed(i);
					Reduce(i, action, entryData.SymbolIndex);
					Reduce(forkedStateIndex, action, entryData.SymbolIndex);
					break;
				}*/
				case BottomUpActionType::Conflict:
				{
					std::cout << "Confict" << std::endl;

					// TODO: Problematic (Current problem leftout parenthesis results cause precedence issues)
					CFGElement& lastToken = GetLastPushedTerminal(i);

					int32_t leftElementPriority = m_TokenMap.at(lastToken.ID).Priority;
					int32_t rightElementPriority = m_TokenMap.at(tokenElement.ID).Priority;
					if (leftElementPriority < rightElementPriority)
					{
						const BottomUpActionData* shiftAction = action.GetActionData(BottomUpActionType::Shift);
						if (shiftAction == nullptr)
						{
							std::cout << "FATAL ERROR" << std::endl;
							return false;
						}
						Shift(i, action, entryData.SymbolIndex);
					}
					else
					{
						const BottomUpActionData* reduceAction = action.GetActionData(BottomUpActionType::Reduce);
						if (reduceAction == nullptr)
						{
							std::cout << "FATAL ERROR" << std::endl;
							return false;
						}
						Reduce(i, *reduceAction, entryData.SymbolIndex);
					}

					break;
				}
				case BottomUpActionType::Empty:
				{
					std::cout << "Empty" << std::endl;
					size_t totalSymbols = m_LR1->GetTotalSymbols();
					bool isError = true;
					for (int32_t j = 0; j < totalSymbols; j++)
					{
						int32_t index = entryData.State * totalSymbols + j;
						const BottomUpAction& stateAction = m_LR1->GetAction(index);
						std::vector<const BottomUpActionData*> reduceActionsData =
							stateAction.GetActionDataMultiple(BottomUpActionType::Reduce);
						for (const BottomUpActionData* data : reduceActionsData)
						{
							isError = false;
							Reduce(i, *data, entryData.SymbolIndex);
						}
					}
					if(isError)
						deletionQueue.push_back(i);
					break;
				}
				case BottomUpActionType::Accept:
				{
					std::cout << "Accept" << std::endl;
					m_GSS.m_AcceptStates.push_back(vertexID);
					deletionQueue.push_back(i);
					break;
				}
				case BottomUpActionType::Error:
				{
					std::cout << "ERROR" << std::endl;
					deletionQueue.push_back(i);
					break;
				}
				}
			}

			for (int32_t index : deletionQueue)
			{
				m_GSS.m_CurrentStates.erase(m_GSS.m_CurrentStates.begin() + index);
			}
			if (m_GSS.m_CurrentStates.empty())
				break;

			std::cout << "================================================================" << std::endl;

			std::cout << "Current state count: " << m_GSS.m_CurrentStates.size() << std::endl;
			auto& vertices = m_GSS.m_GSSGraph.GetVertices();
			for (const auto& [vertexID, vertex] : vertices)
			{
				std::cout << "Vertex " << vertexID << " | State: " << vertex.Data.State
					<< " | TokenIndex: " << vertex.Data.SymbolIndex << " | Elements: " << vertex.Data.Elements <<
					" | ReducedFrom: " << vertex.Data.ReducedFromState << std::endl;
				auto& edges = m_GSS.m_GSSGraph.GetEdgesOfVertex(vertexID);
				for (const auto& edge : edges)
				{
					std::cout << "Edge to " << edge.Destination << " with distance " << 
						edge.Data.Distance << std::endl;
				}
			}

			std::cout << "================================================================" << std::endl
				<< std::endl;

			//std::cin.get();
		}

		// Now we have the gss, need to get the minimum spanning stack on the gss and extract semantic tokens
		ExecuteSemanticAnalysis();
		std::cout << std::endl;
		std::cout << "Minimum Spanning Stack: { " << std::endl;
		for (ActionData& actionData : m_Stack)
		{
			std::cout << "State: " << actionData.State << ", Type: " << 
				(int32_t)actionData.Type << ", RuleID: " << actionData.RuleID <<
				", Production: " << actionData.Production << " ]" << std::endl;
		}
		std::cout << "}" << std::endl;

		return false;
	}

	std::any& GLRParser::Get(int32_t offset)
	{
		int32_t id = m_Stack.at(m_CurrentAction - m_Elements + offset).State;
		return m_GSS.m_GSSGraph.GetVertex(id).Data.Entry;
	}

	void GLRParser::Shift(int32_t currentStateIndex, const BottomUpAction& action,
		int32_t tokenIndex)
	{
		uint32_t minDistance = GetMinimumDistanceToState(currentStateIndex);
		int32_t id = PushStateOnGSSIndexed(currentStateIndex, action.GetActionData(BottomUpActionType::Shift)->RuleID,
			tokenIndex + 1);
		auto& edges = m_GSS.m_GSSGraph.GetEdgesOfVertex(id);
		for (GSSGraph::Edge& edge : edges)
		{
			edge.Data.Distance = minDistance + 1;
		}
	}

	void GLRParser::Reduce(int32_t currentStateIndex, const BottomUpActionData& actionData,
		int32_t tokenIndex)
	{
		uint32_t minDistance = GetMinimumDistanceToState(currentStateIndex);
		int32_t reducedOriginState = m_GSS.m_CurrentStates.at(currentStateIndex);
		auto& productions = m_CFGMap.at(actionData.RuleID)
			.Grammar.GetProductions();
		const Production& production = productions
			.at(actionData.ReducedProduction);
		size_t totalElements = 0U;
		for (const auto& element : production)
		{
			if (element.Type == CFGElementType::Epsilon) continue;
			totalElements++;
		}

		auto& ancestors = GetAncestors(currentStateIndex, totalElements);
		m_GSS.m_CurrentStates.erase(m_GSS.m_CurrentStates.begin() + currentStateIndex);
		for (int32_t ancestorID : ancestors)
		{
			std::cout << "Ancestor" << std::endl;
			int32_t stateIndex = ForkStateOnGSS(ancestorID);
			GLRParseEntryData& stateEntry = m_GSS.m_GSSGraph.GetVertex(ancestorID).Data;
			int32_t gotoID = m_LR1->GetGotoState(stateEntry.State,
				{ CFGElementType::NonTerminal, actionData.RuleID });
			int32_t id = PushStateOnGSSIndexed(stateIndex, gotoID, tokenIndex);
			GLRParseEntryData& entryData = m_GSS.m_GSSGraph.GetVertex(id).Data;
			entryData.ReducedFromState = reducedOriginState;
			entryData.Elements = totalElements;
			auto& edges = m_GSS.m_GSSGraph.GetEdgesOfVertex(id);
			for (GSSGraph::Edge& edge : edges)
			{
				edge.Data.Distance = minDistance + 1;
				edge.Data.RuleID = actionData.RuleID;
				edge.Data.Production = actionData.ReducedProduction;
			}
		}
	}

	void GLRParser::ExtractMinimumSpanningStack()
	{
		m_Stack.clear();
		int32_t minimumAcceptStateID = GetMinimumAcceptState();
		if (minimumAcceptStateID == -1) return;
		m_Stack = { { minimumAcceptStateID, BottomUpActionType::Accept } };

		while (true)
		{
			ActionData& state = m_Stack.front();
			GLRParseEntryData& entryData = m_GSS.m_GSSGraph.GetVertex(state.State).Data;
			const auto& closestEdge = GetMinimumDistanceEdgeToState(state.State);
			if (entryData.ReducedFromState != -1)
			{
				auto& productions = m_CFGMap.at(closestEdge->Data.RuleID)
					.Grammar.GetProductions();
				const Production& production = productions
					.at(closestEdge->Data.Production);
				size_t totalElements = 0U;
				for (const auto& element : production)
				{
					if (element.Type == CFGElementType::Epsilon) continue;
					totalElements++;
				}

				m_Stack.insert(m_Stack.begin(),
					ActionData(entryData.ReducedFromState, BottomUpActionType::Reduce,
					closestEdge->Data.RuleID, closestEdge->Data.Production, (int32_t)totalElements));
				continue;
			}
			if (closestEdge == nullptr) break;
			m_Stack.insert(m_Stack.begin(), ActionData(closestEdge->Destination, BottomUpActionType::Shift));
		}
	}

	bool GLRParser::ExecuteSemanticAnalysis()
	{
		ExtractMinimumSpanningStack();
		std::cout << "Total vertices: " << m_GSS.m_GSSGraph.GetTotalVertices() << std::endl;
		std::cout << "Stack" << std::endl;
		std::cout << "====================================" << std::endl << std::endl;
		for (auto& elem : m_Stack)
		{
			std::cout << "State: " << elem.State << " | Type: " << (int32_t)elem.Type <<
				" | RuleID: " << elem.RuleID << " | Production: " << elem.Production <<
				"| Elements: " << elem.ReducedElementCount << std::endl << std::endl;
		}
		std::cout << "====================================" << std::endl;

		for (m_CurrentAction = 0; m_CurrentAction < m_Stack.size();)
		{
			ActionData& actionData = m_Stack.at(m_CurrentAction);
			GLRParseEntryData& entryData = m_GSS.m_GSSGraph.GetVertex(actionData.State).Data;
			m_Elements = actionData.ReducedElementCount;
			switch (actionData.Type)
			{
			case BottomUpActionType::Shift:
			{
				Lexy::Lexer::OfflineToken& offlineTokenRef = m_TokenStream.at(entryData.SymbolIndex);
				auto& defaultTokenValue = offlineTokenRef.DefaultVaue;
				if (defaultTokenValue.has_value())
				{
					entryData.Entry = std::move(defaultTokenValue);
				}
				else
				{
					m_TokenMap.at(offlineTokenRef.TokenData.TokenID)
						.TokenTypeConstructCallback(entryData.Entry);
				}
				break;
			}
			case BottomUpActionType::Reduce:
			{
				ParseEntryData& parseEntryData =
					ConstructEntryAndInvokeCallbacks(actionData.RuleID, actionData.Production);
				entryData.Entry = std::move(parseEntryData.Entry);
				int32_t firstElementIndex = m_CurrentAction - m_Elements;
				int32_t lastElementIndex = firstElementIndex + m_Elements;
				if (firstElementIndex < lastElementIndex)
				{
					m_Stack.erase(m_Stack.begin() + firstElementIndex, m_Stack.begin() + lastElementIndex);
					m_CurrentAction = firstElementIndex;
				}
				break;
			}
			case BottomUpActionType::Accept:
			{
				return true;
			}
			default:
			{
				std::cout << "WTFFFFF" << std::endl;
				break;
			}
			}
			m_CurrentAction++;
		}

		return false;
	}

	CFGElement GLRParser::GetLastPushedTerminal(int32_t currentIndex)
	{
		int32_t id = m_GSS.m_CurrentStates.at(currentIndex);
		int32_t symbolIndex = m_GSS.m_GSSGraph.GetVertex(id).Data.SymbolIndex;
		id = GetStateBefore(id);
		int32_t totalReducedTokens = 1;
		while (id != -1)
		{
			GLRParseEntryData& entryData = m_GSS.m_GSSGraph.GetVertex(id).Data;
			if (entryData.ReducedFromState == -1)
			{
				Lexy::Lexer::OfflineToken& token = m_TokenStream.at(entryData.SymbolIndex);
				return { CFGElementType::Symbol, m_TokenStream.at(symbolIndex - totalReducedTokens).TokenData.TokenID };
			}
			totalReducedTokens += entryData.Elements;
			id = GetStateBefore(id);

		}
		return { CFGElementType::Dollar, -1 };
	}

	int32_t GLRParser::GetStateBefore(int32_t state)
	{
		GLRParseEntryData& entryData = m_GSS.m_GSSGraph.GetVertex(state).Data;
		if (entryData.ReducedFromState != -1) return entryData.ReducedFromState;

		auto& edges = m_GSS.m_GSSGraph.GetEdgesOfVertex(state);
		if (edges.empty()) return -1;

		int32_t minDistance = INT_MAX;
		int32_t minState = -1;
		for (auto& edge : edges)
		{
			if (edge.Data.Distance < minDistance)
			{
				minDistance = edge.Data.Distance;
				minState = edge.Destination;
			}
		}
		return minState;
	}
}