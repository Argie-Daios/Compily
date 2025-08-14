#include "LR1.h"

#include "Parsy/Parsers/Parser.h"

#include <Utilities.h>

namespace Parsy
{
	static BottomUpAction s_ErrorAction(BottomUpActionType::Error);
	static const std::unordered_set<CFGElement> s_EmptySet;

	/////BottomUpActionData/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const BottomUpActionData* BottomUpAction::GetActionData(const BottomUpActionType& type) const
	{
		for (const BottomUpActionData& data : ActionData)
		{
			if (data.Type == type)
				return &data;
		}

		return nullptr;
	}

	std::vector<const BottomUpActionData*> BottomUpAction::GetActionDataMultiple(const BottomUpActionType& type) const
	{
		std::vector<const BottomUpActionData*> actionData;
		actionData.reserve(ActionData.size());
		for (const BottomUpActionData& data : ActionData)
		{
			if (data.Type == type)
				actionData.push_back(&data);
		}
		return actionData;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LR1::LR1(Parser* parserRef)
		: m_ParserRef(parserRef)
	{
		m_Symbols.insert({ CFGElementType::Dollar, -1 });
		m_Symbols.insert({ CFGElementType::Error, -1 });
	}

	void LR1::RegisterToken(const CFGElement& element)
	{
		if (element.Type != CFGElementType::Symbol) return;
		m_Symbols.insert(element);
	}

	void LR1::RegisterNonTerminal(const CFGElement& element)
	{
		if (element.Type != CFGElementType::NonTerminal) return;
		m_NonTerminals.insert(element);
	}

	BottomUpAction& LR1::GetAction(int32_t state, const CFGElement& symbol)
	{
		int32_t symbolIndex = 0U;
		for (const CFGElement& setSymbol : m_Symbols)
		{
			if (setSymbol == symbol)
			{
				break;
			}
			symbolIndex++;
		}
		if (symbolIndex == m_Symbols.size()) return s_ErrorAction;
		int32_t index = state * m_Symbols.size() + symbolIndex;
		return m_ActionTable.at(index);
	}

	int32_t& LR1::GetGotoState(int32_t state, const CFGElement& nonTerminal)
	{
		int32_t nonTerminalIndex = 0U;
		for (const CFGElement& setNonTerminal : m_NonTerminals)
		{
			if (setNonTerminal == nonTerminal)
			{
				break;
			}
			nonTerminalIndex++;
		}
		int32_t index = state * m_NonTerminals.size() + nonTerminalIndex;
		return m_GotoTable.at(index);
	}

	void LR1::PrintStateGraph()
	{
		std::ofstream stream("StateGraph.txt");

		auto& vertices = m_StateGraph.GetVertices();
		for (const auto& [vertexID, vertexData] : vertices)
		{
			stream << "[I" << vertexID << "]\n";
			stream << "Edges = { ";
			const auto& edges = m_StateGraph.GetEdgesOfVertex(vertexID);
			for (size_t i = 0; i < edges.size(); i++)
			{
				const StateGraph::Edge& edge = edges.at(i);
				stream << "I" << edge.Destination << "[";
				switch (edge.Data.Type)
				{
				case CFGElementType::Epsilon:
				{
					stream << "Epsilon";
					break;
				}
				case CFGElementType::Dollar:
				{
					stream << "$";
					break;
				}
				case CFGElementType::Error:
				{
					stream << "Error";
					break;
				}
				case CFGElementType::NonTerminal:
				{
					stream << m_ParserRef->RuleToStr(edge.Data.ID);
					break;
				}
				case CFGElementType::Symbol:
				{
					stream << m_ParserRef->TokenToStr(edge.Data.ID);
					break;
				}
				}
				stream << "]";
				if (i < edges.size() - 1)
					stream << ", ";
			}
			stream << " }\n";
			for (const BottomUpStateProduction& bottomUpProduction : vertexData.Data.CFGSet)
			{
				stream << '\t' << (bottomUpProduction.Rule == m_ParserRef->m_StartingRule ?
					"StartRule" : m_ParserRef->RuleToStr(bottomUpProduction.Rule)) << " --> ";
				auto& productions = m_ParserRef->m_CFGMap.at(bottomUpProduction.Rule).Grammar.GetProductions();
				auto& production = productions.at(bottomUpProduction.Production);
				bool dotAdded = false;
				for (size_t i = 0; i < production.Elements.size(); i++)
				{
					const CFGElement& element = production.Elements.at(i);
					if (bottomUpProduction.DotPosition == i)
					{
						stream << '.';
						dotAdded = true;
					}
					stream << '[' << m_ParserRef->CFGElementToStr(element) << ']';
				}
				if (!dotAdded)
					stream << '.';
				stream << ", { ";

				for (size_t i = 0; i < bottomUpProduction.LookAheadSymbols.size(); i++)
				{
					auto& it = bottomUpProduction.LookAheadSymbols.begin();
					std::advance(it, i);
					const CFGElement& element = *it;
					stream << m_ParserRef->CFGElementToStr(element);
					if (i < bottomUpProduction.LookAheadSymbols.size() - 1)
						stream << ", ";
				}
				stream << " }";
				stream << '\n';
			}
			stream << '\n';
		}

		stream.close();
	}

	static const char* ActionTypeToString(const BottomUpActionType& type)
	{
		switch (type)
		{
		case BottomUpActionType::Accept: return "Accept";
		case BottomUpActionType::Shift: return "Shift";
		case BottomUpActionType::Reduce: return "Reduce";
		case BottomUpActionType::Conflict: return "Conflict";
		case BottomUpActionType::Error: return "Error";
		}

		return "InvalidType";
	}

	void LR1::PrintTable()
	{
		Utilities::TableStream tableStream("Table.txt", std::ios::out, Utilities::TableStreamFlags_ColumnsLabel |
			Utilities::TableStreamFlags_ColumnsSameWidth | Utilities::TableStreamFlags_RowsLabel
			| Utilities::TableStreamFlags_RowSeperator);

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		tableStream.BindGetTotalRowsCallback([&]() -> size_t { 
			return m_ActionTable.size() / m_Symbols.size(); 
		});

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		tableStream.BindGetTotalColumnsCallback([&]() -> size_t { 
			return m_Symbols.size() + m_NonTerminals.size();
		});

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		tableStream.BindGetColumnLabelCallback([&](size_t col) -> const std::string& {
			static std::string helperString;
			helperString.clear();
			if (col >= m_Symbols.size())
			{
				size_t index = (col - m_Symbols.size());
				auto& it = m_NonTerminals.begin();
				std::advance(it, index);
				helperString = m_ParserRef->RuleToStr(it->ID);
				return helperString;
			}

			auto& it = m_Symbols.begin();
			std::advance(it, col);
			helperString = m_ParserRef->CFGElementToStr(*it);
			return helperString;
		});

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		tableStream.BindGetRowLabelCallback([&](size_t row) -> const std::string& {
			static std::string label;
			label.clear();
			label = std::to_string(row);
			return label;
			});

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		tableStream.BindGetElementStringCallback([&](size_t row, size_t col) -> const std::string& {
			static std::string helperString;
			helperString.clear();
			if (col >= m_Symbols.size())
			{
				size_t index = m_NonTerminals.size() * row + (col - m_Symbols.size());
				int32_t data = m_GotoTable.at(index);
				if (data == -1)
					helperString = "-";
				else
					helperString = std::to_string(data);
				return helperString;
			}

			size_t index = m_Symbols.size() * row + col;
			BottomUpAction& data = m_ActionTable.at(index);
			switch (data.Type)
			{
			case BottomUpActionType::Shift:
			{
				const BottomUpActionData* actionData = data.GetActionData(BottomUpActionType::Shift);
				helperString = "S[" + std::to_string(actionData->RuleID) + ","
					+ std::to_string(actionData->ReducedProduction) + "]";
				break;
			}
			case BottomUpActionType::Reduce:
			{
				const BottomUpActionData* actionData = data.GetActionData(BottomUpActionType::Reduce);
				helperString = "R[" + std::to_string(actionData->RuleID) + "]";
				break;
			}
			case BottomUpActionType::Conflict:
			{
				for (auto& actionData : data.ActionData)
				{
					if (actionData.Type == BottomUpActionType::Shift)
					{
						helperString += "S[" + std::to_string(actionData.RuleID) + ","
							+ std::to_string(actionData.ReducedProduction) + "]";
					}
					else
					{
						helperString += "R[" + std::to_string(actionData.RuleID) + "]";
					}
				}
				break;
			}
			case BottomUpActionType::Empty:
			{
				helperString = "Empty";
				break;
			}
			case BottomUpActionType::Error:
			{
				helperString = "Error";
				break;
			}
			case BottomUpActionType::Accept:
			{
				helperString = "Accept";
				break;
			}
			}

			return helperString;
		});

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		tableStream.SetLabel("LR Parsing Table");
		tableStream.SetLabelHorizontalAlignment(Utilities::HorizontalAlignment::Center);
		tableStream.SetHorizontalAlignment(Utilities::HorizontalAlignment::Center);
		tableStream.SetRowHorizontalSpacing(4);
		tableStream.SetColumnHorizontalSpacing(4);
		tableStream.SetColumnHorizontalSpacing(4);

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		tableStream.Export();
	}

	const std::unordered_set<CFGElement>& LR1::GetFirstSet(const CFGElement& element)
	{
		if (element.Type != CFGElementType::NonTerminal) return s_EmptySet;
		return m_RulesSets.at(element.ID).FirstSet;
	}

	const std::unordered_set<CFGElement>& LR1::GetFollowSet(const CFGElement& element)
	{
		if (element.Type != CFGElementType::NonTerminal) return s_EmptySet;
		return m_RulesSets.at(element.ID).FollowSet;
	}

	void LR1::GenerateFirstSets()
	{
		for (auto& [ruleID, cfg] : m_ParserRef->m_CFGMap)
		{
			CalculateFirstOfRule(ruleID);
		}
	}

	void LR1::GenerateFollowSets()
	{
		for (auto& [ruleID, cfg] : m_ParserRef->m_CFGMap)
		{
			CalculateFollowOfRule(ruleID);
		}
	}

	struct VectorHash 
	{
		size_t operator()(const std::vector<BottomUpStateProduction>& vector) const 
		{
			size_t h = 0;
			for (const BottomUpStateProduction& bottomUpProduction : vector)
			{
				h ^= BottomUpStateProductionHash{}(bottomUpProduction);
			}
			return h;
		}
	};

	void LR1::GenerateStateGraph()
	{
		BottomUpState state;
		state.CFGSet.emplace_back(m_ParserRef->m_StartingRule, 0, 0, true);
		state.CFGSet.back().LookAheadSymbols.insert({ CFGElementType::Dollar, -1 });
		int32_t id = m_StateGraph.PushVertex(state);
		auto& stateRef = m_StateGraph.GetVertex(id).Data;
		ExpandNonTerminals(stateRef);

		std::vector<int32_t> vertexStack = { id };
		std::unordered_map<std::vector<BottomUpStateProduction>, int32_t, VectorHash> stateMemo;
		while (!vertexStack.empty())
		{
			int32_t top = vertexStack.back();
			vertexStack.pop_back();

			auto& state = m_StateGraph.GetVertex(top).Data;
			std::unordered_map<CFGElement, std::vector<BottomUpStateProduction>> edgeCollection;
			for (auto& element : state.CFGSet)
			{
				auto& productions = m_ParserRef->m_CFGMap.at(element.Rule).Grammar.GetProductions();
				auto& production = productions.at(element.Production);

				if (element.DotPosition >= production.Elements.size())
				{
					if (element.IsAccept)
					{
						BottomUpState& state = m_StateGraph.GetVertex(top).Data;
						state.IsAccept = true;
					}
					continue;
				}

				const CFGElement& dotElement = production.Elements.at(element.DotPosition);
				if (edgeCollection.find(dotElement) == edgeCollection.end())
				{
					edgeCollection.emplace(dotElement, std::vector<BottomUpStateProduction>());
				}
				edgeCollection[dotElement].push_back(element);
			}

			for (auto& [element, bottomUpProductions] : edgeCollection)
			{
				auto& it = stateMemo.find(bottomUpProductions);
				if (it != stateMemo.end())
				{
					m_StateGraph.PushEdge(top, it->second, element);
					continue;
				}
				int32_t id = m_StateGraph.PushVertex(BottomUpState());
				vertexStack.push_back(id);
				StateGraph::Vertex& vertex = m_StateGraph.GetVertex(id);
				m_StateGraph.PushEdge(top, id, element);
				stateMemo.emplace(bottomUpProductions, id);
				std::unordered_set<CFGElement> elementMemo;
				for (BottomUpStateProduction& bottomUpProduction : bottomUpProductions)
				{
					auto& productions = m_ParserRef->m_CFGMap.at(bottomUpProduction.Rule).Grammar.GetProductions();
					auto& production = productions.at(bottomUpProduction.Production);
					const CFGElement& dotElement = production.Elements.at(bottomUpProduction.DotPosition);
					vertex.Data.CFGSet.emplace_back(bottomUpProduction.Rule, bottomUpProduction.Production,
						bottomUpProduction.DotPosition + 1, bottomUpProduction.IsAccept);
					for (auto& element : bottomUpProduction.LookAheadSymbols)
					{
						vertex.Data.CFGSet.back().LookAheadSymbols.insert(element);
					}
					AdvanceIfEpsilon(production.Elements, vertex.Data.CFGSet.back());
					if (elementMemo.find(dotElement) == elementMemo.end())
					{
						ExpandNonTerminals(vertex.Data);
						elementMemo.insert(dotElement);
					}
				}
			}
		}
	}

	void LR1::GenerateTable()
	{
		size_t totalVertices = m_StateGraph.GetTotalVertices();
		m_ActionTable.resize(m_Symbols.size() * totalVertices, BottomUpAction(BottomUpActionType::Empty));
		m_GotoTable.resize(m_NonTerminals.size() * totalVertices, -1);
		for (int32_t i = 0; i < totalVertices; i++)
		{
			auto& vertexState = m_StateGraph.GetVertex(i).Data;
			auto& edges = m_StateGraph.GetEdgesOfVertex(i);
			for (auto& edge : edges)
			{
				CFGElement& element = edge.Data;
				switch (element.Type)
				{
				case CFGElementType::NonTerminal:
				{
					int32_t& gotoState = GetGotoState(i, element);
					gotoState = edge.Destination;
					break;
				}
				case CFGElementType::Epsilon:
				{
					std::cout << "Error" << std::endl;
					break;
				}
				default:
				{
					BottomUpAction& action = GetAction(i, element);
					action.Type = BottomUpActionType::Shift;
					action.ActionData.insert(BottomUpActionData(BottomUpActionType::Shift, edge.Destination));
					if (action.ActionData.size() > 1)
						action.Type = BottomUpActionType::Conflict;
					break;
				}
				}
			}
			for (auto& stateCFG : vertexState.CFGSet)
			{
				auto& productions = m_ParserRef->m_CFGMap.at(stateCFG.Rule).Grammar.GetProductions();
				auto& production = productions.at(stateCFG.Production);
				if (stateCFG.DotPosition >= production.Elements.size())
				{
					HandleReduceTable(i, stateCFG);
				}
			}

			if (vertexState.IsAccept)
			{
				BottomUpAction& action = GetAction(i, { CFGElementType::Dollar, -1 });
				action.Type = BottomUpActionType::Accept;
			}
		}
	}

	void LR1::AdvanceIfEpsilon(const Production& production, BottomUpStateProduction& stateCFG)
	{
		for (const CFGElement& element : production)
		{
			if (element.Type != CFGElementType::Epsilon) break;
			stateCFG.DotPosition++;
		}
	}

	const std::unordered_set<CFGElement> LR1::CalculateFirstOfElement(const CFGElement& element)
	{
		std::unordered_set<CFGElement> firstSet;
		switch (element.Type)
		{
		case CFGElementType::NonTerminal:
		{
			return CalculateFirstOfRule(element.ID);
		}
		default:
		{
			firstSet.insert(element);
			break;
		}
		}

		return firstSet;
	}

	const std::unordered_set<CFGElement> LR1::CalculateFirstOfProduction(const Production& production)
	{
		std::unordered_set<CFGElement> firstSet;
		for (int32_t i = 0; i < production.size(); i++)
		{
			const CFGElement& element = production.at(i);
			auto& elementSet = CalculateFirstOfElement(element);
			for (auto& element : elementSet)
			{
				if (i < production.size() - 1 && element.Type == CFGElementType::Epsilon) continue;
				firstSet.insert(element);
			}
			if (elementSet.find({ CFGElementType::Epsilon, -1 }) == elementSet.end()) break;
		}

		return firstSet;
	}

	const std::unordered_set<CFGElement>& LR1::CalculateFirstOfRule(int32_t ruleID)
	{
		auto& it = m_RulesSets.find(ruleID);
		if (it != m_RulesSets.end()) return it->second.FirstSet;

		m_RulesSets.try_emplace(ruleID);
		auto& set = m_RulesSets.at(ruleID).FirstSet;
		Parser::RuleProperties& cfg = m_ParserRef->m_CFGMap.at(ruleID);
		auto& productions = cfg.Grammar.GetProductions();
		for (const ProductionData& production : productions)
		{
			const std::unordered_set<CFGElement>& productionSet = CalculateFirstOfProduction(production.Elements);
			for (const CFGElement& element : productionSet)
			{
				set.insert(element);
			}
		}

		return set;
	}

	void LR1::GenerateLookAheadSymbols(std::unordered_set<CFGElement>& lookaheadSymbols,
		BottomUpStateProduction& expandedRule)
	{
		auto& productions = m_ParserRef->m_CFGMap.at(expandedRule.Rule).Grammar.GetProductions();
		const ProductionData& production = productions.at(expandedRule.Production);

		if (m_ParserRef->m_Flags & LRParserFlags_IncludeDollarLookAhead)
			lookaheadSymbols.insert({ CFGElementType::Dollar, -1 });

		for (int32_t i = expandedRule.DotPosition + 1; i < production.Elements.size(); i++)
		{
			const CFGElement& element = production.Elements.at(i);
			switch (element.Type)
			{
			case CFGElementType::NonTerminal:
			{
				auto& set = m_RulesSets.at(element.ID).FirstSet;
				for (const CFGElement& element : set)
				{
					lookaheadSymbols.insert(element);
				}
				auto& it = set.find({ CFGElementType::Epsilon, -1 });
				if (it == set.end())
				{
					return;
				}
				if (i < production.Elements.size() - 1)
				{
					auto& epsilonIT = lookaheadSymbols.find({ CFGElementType::Epsilon, -1 });
					lookaheadSymbols.erase(epsilonIT);
				}
				break;
			}
			case CFGElementType::Epsilon:
			{
				if (i == production.Elements.size() - 1)
				{
					lookaheadSymbols.insert(element);
				}
				break;
			}
			default:
			{
				lookaheadSymbols.insert(element);
				return;
			}
			}
		}
		auto& epsilonIT = lookaheadSymbols.find({ CFGElementType::Epsilon, -1 });
		bool hasEpsilon = epsilonIT != lookaheadSymbols.end();
		if (hasEpsilon)
		{
			lookaheadSymbols.erase(epsilonIT);
		}
		if (lookaheadSymbols.empty() || hasEpsilon)
		{
			for (const CFGElement& element : expandedRule.LookAheadSymbols)
			{
				lookaheadSymbols.insert(element);
			}
		}
	}

	const std::unordered_set<CFGElement> LR1::CalculateFollowOfElement(const CFGElement& element)
	{
		return std::unordered_set<CFGElement>();
	}

	const std::unordered_set<CFGElement> LR1::CalculateFollowOfProduction(const Production& production)
	{
		return std::unordered_set<CFGElement>();
	}

	const std::unordered_set<CFGElement>& LR1::CalculateFollowOfRule(int32_t ruleID)
	{
		auto& cfg = m_ParserRef->m_CFGMap.at(ruleID);
		auto& ruleSet = m_RulesSets[ruleID];
		auto& followSet = ruleSet.FollowSet;

		if (ruleID == m_ParserRef->m_StartingRule)
			followSet.insert({ CFGElementType::Dollar, -1 });

		bool updated = true;
		while (updated)
		{
			updated = false;

			for (auto& [rid, ruleProps] : m_ParserRef->m_CFGMap)
			{
				for (const auto& production : ruleProps.Grammar.GetProductions())
				{
					for (size_t i = 0; i < production.Elements.size(); ++i)
					{
						const CFGElement& B = production.Elements[i];
						if (B.Type != CFGElementType::NonTerminal) continue;

						std::unordered_set<CFGElement> trailer;
						bool addFollowOfLHS = false;

						if (i + 1 < production.Elements.size())
						{
							Production beta(production.Elements.begin() + i + 1, production.Elements.end());
							auto betaFirst = CalculateFirstOfProduction(beta);
							for (const auto& sym : betaFirst)
							{
								if (sym.Type != CFGElementType::Epsilon)
									trailer.insert(sym);
							}
							if (betaFirst.find({ CFGElementType::Epsilon, -1 }) != betaFirst.end())
								addFollowOfLHS = true;
						}
						else
						{
							addFollowOfLHS = true;
						}

						auto& targetSet = m_RulesSets[B.ID].FollowSet;
						for (const auto& sym : trailer)
						{
							if (targetSet.insert(sym).second)
								updated = true;
						}

						if (addFollowOfLHS)
						{
							auto& lhsFollow = m_RulesSets[rid].FollowSet;
							for (const auto& sym : lhsFollow)
							{
								if (m_RulesSets[B.ID].FollowSet.insert(sym).second)
									updated = true;
							}
						}
					}
				}
			}
		}

		return m_RulesSets[ruleID].FollowSet;
	}

	void LR1::ExpandNonTerminals(BottomUpState& state)
	{
		size_t index = 0;
		std::unordered_set<int32_t> calculatedProductions;
		while (index < state.CFGSet.size())
		{
			auto& element = state.CFGSet.at(index);
			auto& productions = m_ParserRef->m_CFGMap.at(element.Rule).Grammar.GetProductions();
			const ProductionData& production = productions.at(element.Production);
			if (element.DotPosition >= production.Elements.size())
			{
				index++;
				continue;
			}
			auto& dotElement = production.Elements.at(element.DotPosition);

			if (dotElement.Type == CFGElementType::NonTerminal &&
				calculatedProductions.find(dotElement.ID) == calculatedProductions.end())
			{
				auto& nonTerminalProductions = m_ParserRef->m_CFGMap.at(dotElement.ID).Grammar
					.GetProductions();
				for (size_t i = 0U; i < nonTerminalProductions.size(); i++)
				{
					auto& element = state.CFGSet.at(index);
					std::unordered_set<CFGElement> lookAheadSymbols;
					GenerateLookAheadSymbols(lookAheadSymbols, element);
					state.CFGSet.emplace_back(dotElement.ID, i, 0, false);
					state.CFGSet.back().LookAheadSymbols = std::move(lookAheadSymbols);
					AdvanceIfEpsilon(nonTerminalProductions.at(i).Elements, state.CFGSet.back());
				}
				calculatedProductions.insert(dotElement.ID);
			}
			index++;
		}
	}

	void LR1::HandleReduceTable(int32_t state, const BottomUpStateProduction& production)
	{
		for (auto& lookAheadSymbol : production.LookAheadSymbols)
		{
			BottomUpAction& action = GetAction(state, lookAheadSymbol);
			action.Type = BottomUpActionType::Reduce;
			action.ActionData.insert(BottomUpActionData(BottomUpActionType::Reduce,
				production.Rule, production.Production));
			if (action.ActionData.size() > 1)
				action.Type = BottomUpActionType::Conflict;
		}
	}
}