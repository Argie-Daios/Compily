#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "CFG.h"

namespace Parsy
{
	enum class PrecedenceType : uint8_t
	{
		Left,
		Right,
		NonAssociate
	};

	enum class OperatorTableActionType : uint8_t
	{
		Shift,
		Reduce,
		Error
	};

	struct OperatorData
	{
		int32_t PriorityValue = 0;
		PrecedenceType PType;

		OperatorData() = default;
		OperatorData(const OperatorData&) = default;
		OperatorData(OperatorData&&) = default;
	};

	struct OperatorTableCellData
	{
		OperatorTableActionType ActionType;
		int32_t RuleID = - 1;
		int32_t Production = -1;

		OperatorTableCellData() = default;
		OperatorTableCellData(const OperatorTableCellData&) = default;
		OperatorTableCellData(OperatorTableCellData&&) = default;
	};

	class OperatorPrecedenceTable
	{
	public:
		OperatorPrecedenceTable(class Parser* parserRef);

		void AddOperator(int32_t token, const OperatorData& data);
		const OperatorTableCellData& GetTableCellData(int32_t tokenX, int32_t tokenY);
		void CalculateTable();
	private:
		struct RuleSets
		{
			std::unordered_set<CFGElement> LeadingSet;
			std::unordered_set<CFGElement> TrailingSet;

			RuleSets() = default;
			RuleSets(const RuleSets&) = default;
			RuleSets(RuleSets&&) = default;
		};
	private:
		const std::unordered_set<CFGElement> CalculateLeadingOfElement(const CFGElement& element);
		const std::unordered_set<CFGElement> CalculateLeadingOfProduction(const Production& production);
		const std::unordered_set<CFGElement>& CalculateLeadingOfRule(int32_t ruleID);
		void GenerateLeadingSets();

		const std::unordered_set<CFGElement> CalculateTrailingOfElement(const CFGElement& element);
		const std::unordered_set<CFGElement> CalculateTrailingOfProduction(const Production& production);
		const std::unordered_set<CFGElement>& CalculateTrailingOfRule(int32_t ruleID);
		void GenerateTrailingSets();
	private:
		class Parser* m_ParserRef = nullptr;

		std::unordered_map <int32_t, RuleSets> m_RulesSets;
		std::vector<OperatorTableCellData> m_OperatorTable;
		std::vector<OperatorData> m_OperatorsData;
		std::unordered_map<int32_t, int32_t> m_OperatorsIndexMap;
	};
}