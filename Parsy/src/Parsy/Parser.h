#pragma once

#include "Lexy.h"

#include "CFG.h"
#include "CLR1.h"

namespace Parsy
{
	using RuleID_t = int32_t;

	class Parser
	{
	public:
		Parser(const std::ifstream& inputStream);

		void Parse();

		void BeginRule(RuleID_t rule, bool startRule = false);
		void Add(const CFGElement& element);
		void Union();
		void EndRule();
	private:
		Lexy::Lexer m_Lexer;

		CFGElement m_StartingRule;
		std::unordered_map<RuleID_t, CFG> m_CFGMap;
		CLR1 m_CLR1;

		RuleID_t m_BoundRule = -1;
	
		friend class LALR1;
	};
}