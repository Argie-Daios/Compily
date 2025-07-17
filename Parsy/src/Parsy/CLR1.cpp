#include "CLR1.h"

#include "Parser.h"

#include <iostream>
#include <vector>

#include <Windows.h>

namespace Parsy
{
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
		int32_t index = state * m_Symbols.size() + symbolIndex;
		return m_ActionTable.at(index);
	}

	int32_t CLR1::GetGotoState(int32_t state, const CFGElement& nonTerminal)
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
		int32_t id = m_StateGraph.PushVertex(state);
		CLR1State& stateRef = m_StateGraph.GetVertex(id).Data;
		ExpandNonTerminals(stateRef);

		std::vector<int32_t> vertexQueue = { id };
		std::unordered_map<CLR1StateCFG, int32_t> stateMemo;
		while (!vertexQueue.empty())
		{
			int32_t top = vertexQueue.back();
			vertexQueue.pop_back();

			auto& state = m_StateGraph.GetVertex(top).Data;
			std::unordered_map<CFGElement, int32_t> elementMemo;
			std::unordered_set<int32_t> newVertices;
			for(auto& element : state.CFGSet)
			{
				auto& productions = m_ParserRef->m_CFGMap.at(element.Rule).Grammar.GetProductions();
				auto& production = productions.at(element.Production);
				if (element.DotPosition >= production.size()) continue;
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
					stateMemo.emplace(element, id);
					vertexQueue.push_back(id);
					newVertices.insert(id);
					m_StateGraph.PushEdge(top, id, dotElement);
					elementMemo.emplace(dotElement, id);
				}

				int32_t matchStateIndex = elementMemo.at(dotElement);
				auto& matchState = m_StateGraph.GetVertex(matchStateIndex).Data;
				matchState.CFGSet.emplace_back(element.Rule, element.Production, element.DotPosition + 1,
					element.IsAccept);
			}
			for (auto& vertexID : newVertices)
			{
				auto& vertex = m_StateGraph.GetVertex(vertexID).Data;
				ExpandNonTerminals(vertex);
			}
		}

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
					(element.IsAccept ? "true" : "false") << std::endl;
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
					state.CFGSet.emplace_back(dotElement.ID, i, 0, false);
				}
				calculatedProductions.insert(dotElement.ID);
			}
			index++;
		}
	}
}