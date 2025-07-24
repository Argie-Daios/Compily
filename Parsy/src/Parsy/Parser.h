#pragma once

#include "Lexy.h"

#include "CFG.h"
#include "BottomUp/CLR1.h"

#include <any>
#include <memory>

namespace Parsy
{
	using RuleID_t = int32_t;
	using TypeCallback = std::function<void(std::any&)>;

	struct StackState
	{
		int32_t state;
		CFGElement symbol;
		std::any entry;

		StackState() = default;
		StackState(const StackState&) = default;
		StackState& operator=(const StackState& other)
		{
			if (this == &other) return *this;

			state = other.state;
			symbol = other.symbol;
			entry = std::move(other.entry);

			return *this;
		}
	};

	struct ParseTree
	{
		std::vector<StackState> Stack;
		bool IsDead = false;

		ParseTree() = default;
	};

	class Parser
	{
	public:
		Parser(const std::ifstream& inputStream);

		bool Parse();
		void Print();

		template<typename Type>
		void DeclareTokenType(Lexy::TokenID_t token)
		{
			m_TokenMap.emplace(token, TokenProperties());
			m_TokenMap.at(token).TokenTypeConstructCallback = [](std::any& any) { any = Type(); };
		}

		void BeginRule(RuleID_t rule, bool startRule = false);
		template<typename Type>
		void BeginRule(RuleID_t rule, bool startRule = false)
		{
			BeginRule(rule, startRule);
			m_CFGMap.at(rule).RuleTypeConstructCallback = [](std::any& any) { any = Type(); };
		}
		void Add(const CFGElement& element);
		void Union(const TypeCallback& callback = [](std::any& any) {});
		std::any& Get(int32_t offset);
		void EndRule(const TypeCallback& callback = [](std::any& any) {});
	private:
		struct RuleProperties
		{
			CFG Grammar;
			TypeCallback RuleTypeConstructCallback = [](std::any& any) { any.reset(); };
			std::unordered_map<int32_t, TypeCallback> RuleProductionCallbacks;
		};

		struct TokenProperties
		{
			TypeCallback TokenTypeConstructCallback = [](std::any& any) { any.reset(); };
		};
	private:
		void Shift(ParseTree& parseTree, const BottomUpAction& action,
			const Lexy::Lexer::Token& token);
		void Reduce(ParseTree& parseTree, const BottomUpAction& action);
	protected:
		Lexy::Lexer* m_Lexer = nullptr;

	private:
		RuleID_t m_StartingRule = -100;
		std::unordered_map<RuleID_t, RuleProperties> m_CFGMap;
		std::unordered_map<Lexy::TokenID_t, TokenProperties> m_TokenMap;
		CLR1 m_CLR1;
		ParseTree inputStack;
		int32_t elements = -1;

		RuleID_t m_BoundRule = -1;
	
		friend class CLR1;
		friend class LR1;
	};
}