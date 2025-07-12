#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <functional>

#include "ThompsonCalculator.h"

namespace Lexy
{
	class Lexer
	{
		using RuleCallback = std::function<int32_t()>;
	public:
		enum class TokenState
		{
			Success,
			Failure,
			End
		};

		struct Token
		{
			TokenState State;
			int32_t TokenType;

			Token() = default;
			Token(const TokenState& state, int32_t rule)
				: State(state), TokenType(rule)
			{

			}
		};
	public:
		static void Init(const std::string& inputStream);
		static void Tokenize();
		static Token NextToken();
		static int32_t CreateRule(const std::string& regex, const RuleCallback& callback = []() { return -1; });
		inline static const std::string& GetTokenContent() { return m_TokenContent; }
	private:
		struct Rule
		{
			NFA NFAGraph;
			RuleCallback Callback;

			Rule() = default;
			Rule(NFA&& nfa, const RuleCallback& callback)
				: NFAGraph(std::move(nfa)), Callback(callback)
			{
				
			}
		};
	private:
		inline static std::string m_Input;
		inline static uint32_t m_InputOffset = 0U;
		inline static ThompsonCalculator m_ThompsonCalculator;
		inline static std::vector<Rule> m_Rules;

		inline static std::string m_TokenContent;
	};
}