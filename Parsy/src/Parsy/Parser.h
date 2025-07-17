#pragma once

#include "Lexy.h"

#include "CFG.h"
#include "CLR1.h"

#include <any>
#include <memory>

namespace Parsy
{
	using RuleID_t = int32_t;
	using TypeCallback = std::function<void(std::any&)>;

	class Parser
	{
	public:
		Parser(const std::ifstream& inputStream);

		void Parse();

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
		void Union();
		void EndRule();
	private:
		struct RuleProperties
		{
			CFG Grammar;
			TypeCallback RuleTypeConstructCallback = [](std::any& any) { any.reset(); };
		};

		struct TokenProperties
		{
			TypeCallback TokenTypeConstructCallback = [](std::any& any) { any.reset(); };
		};
	private:
		Lexy::Lexer m_Lexer;

		RuleID_t m_StartingRule = -100;
		std::unordered_map<RuleID_t, RuleProperties> m_CFGMap;
		std::unordered_map<Lexy::TokenID_t, TokenProperties> m_TokenMap;
		CLR1 m_CLR1;

		RuleID_t m_BoundRule = -1;
	
		friend class CLR1;
	};
}