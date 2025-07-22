#pragma once

#include <string>
#include <vector>
#include <unordered_set>

#include <Utilities.h>

#include "CFG.h"

#define BIT(x) 1 << x

namespace Parsy
{
	// TODO: Make state graph generation algorithm
	// TODO: Make CLR(1) table generation algorithm
	// TODO: Make input parsing algorithm

	enum BottomUpActionType
	{
		BottomUpActionType_Shift = BIT(0),
		BottomUpActionType_Reduce = BIT(1),
		BottomUpActionType_Accept = BIT(2),
		BottomUpActionType_Error = BIT(3)
	};

	struct BottomUpAction
	{
		int32_t Types;
		uint32_t TypeCount = 0U;
		int32_t ID = -1;
		int32_t ReducedProduction = -1;

		BottomUpAction() = default;
		BottomUpAction(int32_t types)
			: Types(types) {}
		BottomUpAction(const BottomUpAction&) = default;
		BottomUpAction(BottomUpAction&&) = default;
	};

	struct CLR1StateCFG
	{
		int32_t Rule = -1;
		int32_t Production = -1;
		int32_t DotPosition = -1;
		std::unordered_set<CFGElement> LookAheadSymbols;
		bool IsAccept = false;

		CLR1StateCFG() = default;
		CLR1StateCFG(int32_t rule, int32_t production, int32_t dotPosition, bool isAccept)
			: Rule(rule), Production(production), DotPosition(dotPosition), IsAccept(isAccept)
		{}
		CLR1StateCFG(const CLR1StateCFG&) = default;
		CLR1StateCFG(CLR1StateCFG&&) = default;

		bool operator==(const CLR1StateCFG& other) const
		{
			if (&other == this) return true;

			return Rule == other.Rule && Production == other.Production
				&& DotPosition == other.DotPosition && LookAheadSymbols == other.LookAheadSymbols;
		}
	};

	struct CLR1State
	{
		std::vector<CLR1StateCFG> CFGSet;
		bool IsAccept = false;

		CLR1State() = default;
		CLR1State(const CLR1State&) = default;
		CLR1State(CLR1State&&) = default;
	};

	using StateGraph = Utilities::Graph<CLR1State, CFGElement>;
	class CLR1
	{
	public:
		CLR1(class Parser* parserRef);

		void AddElement(const CFGElement& element);

		BottomUpAction& GetAction(int32_t state, const CFGElement& symbol);
		int32_t& GetGotoState(int32_t state, const CFGElement& nonTerminal);

		const std::unordered_set<CFGElement>& GetFirstSet(const CFGElement& element);
		const std::unordered_set<CFGElement>& GetFollowSet(const CFGElement& element);

		void GenerateFirstSets();
		void GenerateFollowSets();
		void GenerateStateGraph();
		void GenerateTable();

		void PrintSymbols();
		void PrintNonTerminals();
		void PrintStateGraph();
		void PrintTable();
	private:
		void AdvanceIfEpsilon(const Production& production, CLR1StateCFG& stateCFG);
		const std::unordered_set<CFGElement> CalculateFirstOfElement(const CFGElement& element);
		const std::unordered_set<CFGElement> CalculateFirstOfProduction(const Production& production);
		const std::unordered_set<CFGElement>& CalculateFirstOfRule(int32_t ruleID);
		void GenerateLookAheadSymbols(std::unordered_set<CFGElement>& lookaheadSymbols,
			CLR1StateCFG& expandedRule);
		void ExpandNonTerminals(CLR1State& state);
	private:
		struct RuleSets
		{
			std::unordered_set<CFGElement> FirstSet;
			std::unordered_set<CFGElement> FollowSet;

			RuleSets() = default;
			RuleSets(const RuleSets&) = default;
			RuleSets(RuleSets&&) = default;
		};
	private:
		class Parser* m_ParserRef;

		StateGraph m_StateGraph;
		
		std::unordered_map <int32_t, RuleSets> m_RulesSets;

		std::vector<BottomUpAction> m_ActionTable;
		std::vector<int32_t> m_GotoTable;
		std::unordered_set<CFGElement> m_Symbols;
		std::unordered_set<CFGElement> m_NonTerminals;
	};
}

namespace std
{
	template<>
	struct hash<Parsy::CLR1StateCFG>
	{
		std::size_t operator()(const Parsy::CLR1StateCFG& elem) const
		{
			std::size_t h1 = std::hash<int32_t>{}(elem.Rule);
			std::size_t h2 = std::hash<int32_t>{}(elem.Production);
			std::size_t h3 = std::hash<int32_t>{}(elem.DotPosition + 
				(int32_t)elem.LookAheadSymbols.size());
			return h1 ^ (h2 << h3);
		}
	};
}