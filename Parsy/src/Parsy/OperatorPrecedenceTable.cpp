#include "OperatorPrecedenceTable.h"

#include "Parsers/Parser.h"

namespace Parsy
{
	OperatorPrecedenceTable::OperatorPrecedenceTable(Parser* parserRef)
		: m_ParserRef(parserRef)
	{

	}

	void OperatorPrecedenceTable::AddOperator(int32_t token, const OperatorData& data)
	{
		m_OperatorsIndexMap.emplace(token, (int32_t)m_OperatorsData.size());
		m_OperatorsData.push_back(data);
	}

	const OperatorTableCellData& OperatorPrecedenceTable::GetTableCellData(int32_t tokenX, int32_t tokenY)
	{
		int32_t rowIndex = m_OperatorsIndexMap.at(tokenX);
		int32_t colIndex = m_OperatorsIndexMap.at(tokenY);
		int32_t index = rowIndex * m_OperatorsData.size() + colIndex;
		return m_OperatorTable.at(index);
	}

	void OperatorPrecedenceTable::CalculateTable()
	{

	}

	const std::unordered_set<CFGElement> OperatorPrecedenceTable::CalculateLeadingOfElement(const CFGElement& element)
	{
		return std::unordered_set<CFGElement>();
	}

	const std::unordered_set<CFGElement> OperatorPrecedenceTable::CalculateLeadingOfProduction(
		const Production& production)
	{
		return std::unordered_set<CFGElement>();
	}

	const std::unordered_set<CFGElement>& OperatorPrecedenceTable::CalculateLeadingOfRule(int32_t ruleID)
	{
		// TODO: insert return statement here
		return std::unordered_set<CFGElement>();
	}

	void OperatorPrecedenceTable::GenerateLeadingSets()
	{
		for (auto& [ruleID, cfg] : m_ParserRef->m_CFGMap)
		{
			CalculateLeadingOfRule(ruleID);
		}
	}

	const std::unordered_set<CFGElement> OperatorPrecedenceTable::CalculateTrailingOfElement(const CFGElement& element)
	{
		return std::unordered_set<CFGElement>();
	}

	const std::unordered_set<CFGElement> OperatorPrecedenceTable::CalculateTrailingOfProduction(
		const Production& production)
	{
		return std::unordered_set<CFGElement>();
	}

	const std::unordered_set<CFGElement>& OperatorPrecedenceTable::CalculateTrailingOfRule(int32_t ruleID) 
	{
		// TODO: insert return statement here
		return std::unordered_set<CFGElement>();
	}

	void OperatorPrecedenceTable::GenerateTrailingSets()
	{
		for (auto& [ruleID, cfg] : m_ParserRef->m_CFGMap)
		{
			CalculateTrailingOfRule(ruleID);
		}
	}
}