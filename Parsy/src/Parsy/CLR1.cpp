#include "CLR1.h"

#include "Parser.h"

#include <iostream>
#include <vector>

#include <Windows.h>

namespace Parsy
{
	static BottomUpAction s_ErrorAction(BottomUpActionType_Error);

	CLR1::CLR1(Parser* parserRef)
		: m_ParserRef(parserRef)
	{
		m_Symbols.insert({ CFGElementType::Dollar, -1 });
	}

	void CLR1::AddElement(const CFGElement& element)
	{
		switch (element.Type)
		{
		case CFGElementType::NonTerminal:
		{
			m_NonTerminals.insert(element);
			break;
		}
		case CFGElementType::Epsilon:
		{

			break;
		}
		default:
		{
			m_Symbols.insert(element);
			break;
		}
		}
	}

	BottomUpAction& CLR1::GetAction(int32_t state, const CFGElement& symbol)
	{
		int32_t symbolIndex = 0U;
		for (auto& setSymbol : m_Symbols)
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

	int32_t& CLR1::GetGotoState(int32_t state, const CFGElement& nonTerminal)
	{
		int32_t nonTerminalIndex = 0U;
		for (auto& setNonTerminal : m_NonTerminals)
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

	void CLR1::GenerateStateGraph()
	{
		CLR1State state;
		state.CFGSet.emplace_back(m_ParserRef->m_StartingRule, 0, 0, true);
		state.CFGSet.back().LookAheadSymbols.insert({CFGElementType::Dollar, -1});
		int32_t id = m_StateGraph.PushVertex(state);
		CLR1State& stateRef = m_StateGraph.GetVertex(id).Data;
		ExpandNonTerminals(stateRef);

		std::vector<int32_t> vertexStack = { id };
		std::unordered_map<CLR1StateCFG, int32_t> stateMemo;
		while (!vertexStack.empty())
		{
			int32_t top = vertexStack.back();
			vertexStack.pop_back();

			auto& state = m_StateGraph.GetVertex(top).Data;
			std::unordered_map<CFGElement, int32_t> elementMemo;
			for(auto& element : state.CFGSet)
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
					int32_t id = m_StateGraph.PushVertex(CLR1State());
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

	void CLR1::GenerateTable()
	{
		size_t totalVertices = m_StateGraph.GetTotalVertices();
		m_ActionTable.resize(m_Symbols.size() * totalVertices, BottomUpAction(BottomUpActionType_Error));
		m_GotoTable.resize(m_NonTerminals.size() * totalVertices);
		for (int32_t i = 0; i < totalVertices; i++)
		{
			CLR1State& vertexState = m_StateGraph.GetVertex(i).Data;
			if (vertexState.IsAccept)
			{
				BottomUpAction& action = GetAction(i, { CFGElementType::Dollar, -1 });
				action.Types = BottomUpActionType_Accept;
				action.TypeCount++;
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
					action.ID = edge.Destination;
					action.Types = BottomUpActionType_Shift;
					action.TypeCount++;
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
						action.ID = stateCFG.Rule;
						action.ReducedProduction = stateCFG.Production;
						action.Types = BottomUpActionType_Reduce;
						action.TypeCount++;
					}
				}
			}
		}
	}

	void Parsy::CLR1::PrintSymbols()
	{
		std::cout << "{ ";
		for (auto& symbol : m_Symbols)
		{
			symbol.Print();
			std::cout << " ";
		}
		std::cout << "}" << std::endl;
	}

	void Parsy::CLR1::PrintNonTerminals()
	{
		std::cout << "{ ";
		for (auto& nonTerminal : m_NonTerminals)
		{
			nonTerminal.Print();
			std::cout << " ";
		}
		std::cout << "}" << std::endl;
	}

	void CLR1::PrintStateGraph()
	{
		auto& vertices = m_StateGraph.GetVertices();
		for (auto& [id, vertex] : vertices)
		{
			std::cout << "[State " << id << "]" << std::endl;
			auto& edges = m_StateGraph.GetEdgesOfVertex(id);
			for (auto& edge : edges)
			{
				std::cout << "Edge to " << edge.Destination << " with ";
				edge.Data.Print();
				std::cout << std::endl;
			}
			for (auto& element : vertex.Data.CFGSet)
			{
				std::cout << "\tRule: " << element.Rule << ", Production: " << element.Production
					<< ", DotPosition: " << element.DotPosition << ", IsAccept: " <<
					(element.IsAccept ? "true" : "false");
				std::cout << " | [ ";
				for (auto& symbol : element.LookAheadSymbols)
				{
					symbol.Print();
					std::cout << " ";
				}
				std::cout << "]" << std::endl;
			}
		}
	}

	void CLR1::PrintTable()
	{
		std::cout << "Actions" << std::endl;
		std::cout << "===========================" << std::endl;
		for (int32_t i = 0; i < m_ActionTable.size(); i++)
		{
			if (i % m_Symbols.size() == 0) std::cout << std::endl;
			BottomUpAction& action = m_ActionTable.at(i);
			int32_t index = i % (int32_t)m_Symbols.size();
			auto& setIt = m_Symbols.begin();
			std::advance(setIt, index);
			std::cout << "Element { State: " << i % m_Symbols.size() << ", Symbol: ";
			setIt->Print();
			std::cout << " }" << ": [ ";
			std::cout << "Types: " << action.TypeCount << ", Type: ";
			switch (action.Types)
			{
			case BottomUpActionType_Accept:
			{
				std::cout << "Accept";
				break;
			}
			case BottomUpActionType_Shift:
			{
				std::cout << "Shift";
				break;
			}
			case BottomUpActionType_Reduce:
			{
				std::cout << "Reduce";
				break;
			}
			case BottomUpActionType_Error:
			{
				std::cout << "Error";
				break;
			}
			}
			std::cout << ", ID: " << action.ID;
			std::cout << ", Production: " << action.ReducedProduction;
			std::cout << " ]" << std::endl;
		}

		std::cout << std::endl;
		std::cout << "GOTO" << std::endl;
		std::cout << "===========================" << std::endl;
		for (int32_t i = 0; i < m_GotoTable.size(); i++)
		{
			int32_t stateID = m_GotoTable.at(i);
			std::cout << "Element " << i % m_NonTerminals.size() << ": " << stateID << std::endl;
		}
	}

	void CLR1::AdvanceIfEpsilon(const Production& production, CLR1StateCFG& stateCFG)
	{
		for (auto& element : production)
		{
			if (element.Type != CFGElementType::Epsilon) break;
			stateCFG.DotPosition++;
		}
	}

	const std::unordered_set<CFGElement> CLR1::CalculateFirstOfElement(const CFGElement& element)
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

	const std::unordered_set<CFGElement> CLR1::CalculateFirstOfProduction(const Production& production)
	{
		std::unordered_set<CFGElement> firstSet;
		for (int32_t i = 0; i < production.size(); i++)
		{
			auto& element = production.at(i);
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

	const std::unordered_set<CFGElement>& CLR1::CalculateFirstOfRule(int32_t ruleID)
	{
		auto& it = m_RulesSets.find(ruleID);
		if (it != m_RulesSets.end()) return it->second.FirstSet;

		m_RulesSets.try_emplace(ruleID);
		auto& set = m_RulesSets.at(ruleID).FirstSet;
		auto& cfg = m_ParserRef->m_CFGMap.at(ruleID);
		auto& productions = cfg.Grammar.GetProductions();
		for (auto& production : productions)
		{
			const std::unordered_set<CFGElement>& productionSet = CalculateFirstOfProduction(production);
			for (auto& element : productionSet)
			{
				set.insert(element);
			}
		}

		return set;
	}

	void CLR1::GenerateFirstSets()
	{
		for (auto& [ruleID, cfg] : m_ParserRef->m_CFGMap)
		{
			CalculateFirstOfRule(ruleID);
		}
	}

	void CLR1::GenerateFollowSets()
	{

	}

	const std::unordered_set<CFGElement>& CLR1::GetFirstSet(const CFGElement& element)
	{
		if (element.Type != CFGElementType::NonTerminal)
		{
			return {element};
		}

		return m_RulesSets.at(element.ID).FirstSet;
	}

	const std::unordered_set<CFGElement>& CLR1::GetFollowSet(const CFGElement& element)
	{
		if (element.Type != CFGElementType::NonTerminal)
		{
			return {};
		}

		return  m_RulesSets.at(element.ID).FollowSet;
	}

	void CLR1::GenerateLookAheadSymbols(std::unordered_set<CFGElement>& lookaheadSymbols,
		CLR1StateCFG& expandedRule)
	{
		auto& productions = m_ParserRef->m_CFGMap.at(expandedRule.Rule).Grammar.GetProductions();
		auto& production = productions.at(expandedRule.Production);

		for (int32_t i = expandedRule.DotPosition + 1; i < production.size(); i++)
		{
			auto& element = production.at(i);
			switch (element.Type)
			{
			case CFGElementType::NonTerminal:
			{
				auto& set = m_RulesSets.at(element.ID).FirstSet;
				for (auto& element : set)
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
			for (auto& element : expandedRule.LookAheadSymbols)
			{
				lookaheadSymbols.insert(element);
			}
		}
	}

	void CLR1::ExpandNonTerminals(CLR1State& state)
	{
		size_t index = 0;
		std::unordered_set<int32_t> calculatedProductions;
		while (index < state.CFGSet.size())
		{
			auto& element = state.CFGSet.at(index);
			auto& productions = m_ParserRef->m_CFGMap.at(element.Rule).Grammar.GetProductions();
			auto& production = productions.at(element.Production);
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