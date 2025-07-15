#pragma once

#include "Lexy.h"

#include "CFG.h"
#include "LALR1.h"

namespace Parsy
{
	using RuleID_t = int32_t;

	class Parser
	{
	public:
		Parser(const std::ifstream& inputStream);

		void Parse();

		void BeginRule(RuleID_t rule);
		void Add(const CFGElement& element);
		void Union();
		void EndRule();
	private:
		Lexy::Lexer m_Lexer;

		RuleID_t m_StartingRule = -1;
		std::unordered_map<RuleID_t, CFG> m_CFGMap;
		LALR1 m_LALR1;

		RuleID_t m_BoundRule = -1;
	
		friend class LALR1;
	};
}