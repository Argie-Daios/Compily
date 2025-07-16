#include "CLR1.h"

#include "Parser.h"

#include <iostream>

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
}