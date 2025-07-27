#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <Utilities.h>

#include "Parsy/CFG.h"

namespace Parsy
{
	enum class BottomUpActionType
	{
		Empty,
		Shift,
		Reduce,
		ShiftReduce,
		ReduceReduce,
		Accept,
		Error
	};

	struct BottomUpActionData
	{
		int32_t RuleID = -1;
		int32_t ReducedProduction = -1;

		BottomUpActionData() = default;
		BottomUpActionData(int32_t ruleID)
			: RuleID(ruleID)
		{

		}
		BottomUpActionData(int32_t ruleID, int32_t reducedProduction)
			: RuleID(ruleID), ReducedProduction(reducedProduction)
		{

		}
	};

	struct BottomUpAction
	{
		BottomUpActionType Type;
		std::unordered_map<BottomUpActionType, BottomUpActionData> ActionData;

		BottomUpAction() = default;
		BottomUpAction(BottomUpActionType type)
			: Type(type) {
		}
		BottomUpAction(const BottomUpAction&) = default;
		BottomUpAction(BottomUpAction&&) = default;
	};

	struct BottomUpStateProduction
	{
		int32_t Rule = -1;
		int32_t Production = -1;
		int32_t DotPosition = -1;
		std::unordered_set<CFGElement> LookAheadSymbols;
		bool IsAccept = false;

		BottomUpStateProduction() = default;
		BottomUpStateProduction(int32_t rule, int32_t production,
			int32_t dotPosition, bool isAccept)
			: Rule(rule), Production(production), DotPosition(dotPosition), IsAccept(isAccept)
		{
		}
		BottomUpStateProduction(const BottomUpStateProduction&) = default;
		BottomUpStateProduction(BottomUpStateProduction&&) = default;

		bool operator==(const BottomUpStateProduction& other) const
		{
			if (&other == this) return true;

			return Rule == other.Rule && Production == other.Production
				&& DotPosition == other.DotPosition && LookAheadSymbols == other.LookAheadSymbols;
		}

		BottomUpStateProduction& operator=(const BottomUpStateProduction& other)
		{
			if (this == &other) return *this;

			Rule = other.Rule;
			Production = other.Production;
			DotPosition = other.DotPosition;
			LookAheadSymbols = other.LookAheadSymbols;
			IsAccept = other.IsAccept;
			return *this;
		}

		BottomUpStateProduction& operator=(BottomUpStateProduction&& other)
		{
			if (this == &other) return *this;

			Rule = other.Rule;
			Production = other.Production;
			DotPosition = other.DotPosition;
			LookAheadSymbols = std::move(other.LookAheadSymbols);
			IsAccept = other.IsAccept;
			return *this;
		}
	};

	struct BottomUpState
	{
		std::vector<BottomUpStateProduction> CFGSet;
		bool IsAccept = false;

		BottomUpState() = default;
		BottomUpState(const BottomUpState&) = default;
		BottomUpState(BottomUpState&&) = default;

		BottomUpState& operator=(const BottomUpState& other)
		{
			if (this == &other) return *this;

			CFGSet = other.CFGSet;
			IsAccept = other.IsAccept;
			return *this;
		}

		BottomUpState& operator=(BottomUpState&& other)
		{
			if (this == &other) return *this;

			CFGSet = std::move(other.CFGSet);
			IsAccept = other.IsAccept;
			return *this;
		}
	};

	using StateGraph = Utilities::Graph<BottomUpState, CFGElement>;
	class LR1
	{
	public:
		LR1(class Parser* parserRef);

		void RegisterToken(const CFGElement& element);
		void RegisterNonTerminal(const CFGElement& element);

		BottomUpAction& GetAction(int32_t state, const CFGElement& symbol);
		int32_t& GetGotoState(int32_t state, const CFGElement& nonTerminal);

		const std::unordered_set<CFGElement>& GetFirstSet(const CFGElement& element);
		const std::unordered_set<CFGElement>& GetFollowSet(const CFGElement& element);

		void GenerateFirstSets();
		void GenerateFollowSets();
		virtual void GenerateStateGraph();
		virtual void GenerateTable();
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
		void AdvanceIfEpsilon(const Production& production, BottomUpStateProduction& stateCFG);
		const std::unordered_set<CFGElement> CalculateFirstOfElement(const CFGElement& element);
		const std::unordered_set<CFGElement> CalculateFirstOfProduction(const Production& production);
		const std::unordered_set<CFGElement>& CalculateFirstOfRule(int32_t ruleID);
		void GenerateLookAheadSymbols(std::unordered_set<CFGElement>& lookaheadSymbols,
			BottomUpStateProduction& expandedRule);
		void ExpandNonTerminals(BottomUpState& state);
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
	struct hash<Parsy::BottomUpStateProduction>
	{
		std::size_t operator()(const Parsy::BottomUpStateProduction& elem) const
		{
			std::size_t h1 = std::hash<int32_t>{}(elem.Rule);
			std::size_t h2 = std::hash<int32_t>{}(elem.Production);
			std::size_t h3 = std::hash<int32_t>{}(elem.DotPosition +
				(int32_t)elem.LookAheadSymbols.size());
			return h1 ^ (h2 << h3);
		}
	};
}