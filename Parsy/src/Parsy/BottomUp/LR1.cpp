#include "LR1.h"

#include "Parsy/Parsers/Parser.h"

namespace Parsy
{
	static BottomUpAction s_ErrorAction(BottomUpActionType::Error);
	static const std::unordered_set<CFGElement> s_EmptySet;

	LR1::LR1(Parser* parserRef)
		: m_ParserRef(parserRef)
	{
		m_Symbols.insert({ CFGElementType::Dollar, -1 });
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

	}

	void LR1::GenerateStateGraph()
	{
		BottomUpState state;
		state.CFGSet.emplace_back(m_ParserRef->m_StartingRule, 0, 0, true);
		state.CFGSet.back().LookAheadSymbols.insert({ CFGElementType::Dollar, -1 });
		int32_t id = m_StateGraph.PushVertex(state);
		auto& stateRef = m_StateGraph.GetVertex(id).Data;
		ExpandNonTerminals(stateRef);

		std::vector<int32_t> vertexStack = { id };
		std::unordered_map<BottomUpStateProduction, int32_t> stateMemo;
		while (!vertexStack.empty())
		{
			int32_t top = vertexStack.back();
			vertexStack.pop_back();

			auto& state = m_StateGraph.GetVertex(top).Data;
			std::unordered_map<CFGElement, int32_t> elementMemo;
			for (auto& element : state.CFGSet)
			{
				auto& productions = m_ParserRef->m_CFGMap.at(element.Rule).Grammar.GetProductions();
				auto& production = productions.at(element.Production);

				if (element.DotPosition >= production.size())
				{
					if (element.IsAccept)
					{
						auto& state = m_StateGraph.GetVertex(top).Data;
						state.IsAccept = true;
					}
					continue;
				}

				auto& dotElement = production.at(element.DotPosition);

				auto& it = stateMemo.find(element);
				if (it != stateMemo.end())
				{
					m_StateGraph.PushEdge(top, it->second, dotElement);
					continue;
				}

				if (elementMemo.find(dotElement) == elementMemo.end())
				{
					int32_t id = m_StateGraph.PushVertex(BottomUpState());
					m_StateGraph.PushEdge(top, id, dotElement);
					vertexStack.push_back(id);
					stateMemo.emplace(element, id);
					elementMemo.emplace(dotElement, id);
				}

				int32_t matchStateIndex = elementMemo.at(dotElement);
				auto& matchState = m_StateGraph.GetVertex(matchStateIndex).Data;
				matchState.CFGSet.emplace_back(element.Rule, element.Production, element.DotPosition + 1,
					element.IsAccept);
				for (auto& element : element.LookAheadSymbols)
				{
					matchState.CFGSet.back().LookAheadSymbols.insert(element);
				}
				AdvanceIfEpsilon(production, matchState.CFGSet.back());
			}

			for (auto& [element, vertexID] : elementMemo)
			{
				auto& vertex = m_StateGraph.GetVertex(vertexID).Data;
				ExpandNonTerminals(vertex);
			}
		}
	}

	void LR1::GenerateTable()
	{
		size_t totalVertices = m_StateGraph.GetTotalVertices();
		m_ActionTable.resize(m_Symbols.size() * totalVertices, BottomUpAction(BottomUpActionType::Empty));
		m_GotoTable.resize(m_NonTerminals.size() * totalVertices);
		for (int32_t i = 0; i < totalVertices; i++)
		{
			auto& vertexState = m_StateGraph.GetVertex(i).Data;
			if (vertexState.IsAccept)
			{
				BottomUpAction& action = GetAction(i, { CFGElementType::Dollar, -1 });
				action.Type = BottomUpActionType::Accept;
				continue;
			}
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
					action.ActionData.emplace(BottomUpActionType::Shift,
						BottomUpActionData(edge.Destination));
					if (action.Type == BottomUpActionType::Reduce)
					{
						action.Type = BottomUpActionType::ShiftReduce;
					}
					else
					{
						action.Type = BottomUpActionType::Shift;
					}
					break;
				}
				}
			}
			for (auto& stateCFG : vertexState.CFGSet)
			{
				auto& productions = m_ParserRef->m_CFGMap.at(stateCFG.Rule).Grammar.GetProductions();
				auto& production = productions.at(stateCFG.Production);
				if (stateCFG.DotPosition >= production.size())
				{
					for (auto& lookAheadSymbol : stateCFG.LookAheadSymbols)
					{
						BottomUpAction& action = GetAction(i, lookAheadSymbol);
						action.ActionData.emplace(BottomUpActionType::Reduce,
							BottomUpActionData(stateCFG.Rule, stateCFG.Production));
						if (action.Type == BottomUpActionType::Shift)
						{
							action.Type = BottomUpActionType::ShiftReduce;
						}
						else if (action.Type == BottomUpActionType::Reduce)
						{
							action.Type = BottomUpActionType::ReduceReduce;
						}
						else
						{
							action.Type = BottomUpActionType::Reduce;
						}
					}
				}
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
		for (const Production& production : productions)
		{
			const std::unordered_set<CFGElement>& productionSet = CalculateFirstOfProduction(production);
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
		const Production& production = productions.at(expandedRule.Production);

		for (int32_t i = expandedRule.DotPosition + 1; i < production.size(); i++)
		{
			const CFGElement& element = production.at(i);
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
				if (i < production.size() - 1)
				{
					auto& epsilonIT = lookaheadSymbols.find({ CFGElementType::Epsilon, -1 });
					lookaheadSymbols.erase(epsilonIT);
				}
				break;
			}
			case CFGElementType::Epsilon:
			{
				if (i == production.size() - 1)
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

	void LR1::ExpandNonTerminals(BottomUpState& state)
	{
		size_t index = 0;
		std::unordered_set<int32_t> calculatedProductions;
		while (index < state.CFGSet.size())
		{
			auto& element = state.CFGSet.at(index);
			auto& productions = m_ParserRef->m_CFGMap.at(element.Rule).Grammar.GetProductions();
			const Production& production = productions.at(element.Production);
			if (element.DotPosition >= production.size())
			{
				index++;
				continue;
			}
			auto& dotElement = production.at(element.DotPosition);

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
					AdvanceIfEpsilon(nonTerminalProductions.at(i), state.CFGSet.back());
				}
				calculatedProductions.insert(dotElement.ID);
			}
			index++;
		}
	}
}