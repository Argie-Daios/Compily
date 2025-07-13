#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <functional>

#include "ThompsonCalculator.h"

#define IGNORE -1

namespace Lexy
{
	class Lexer
	{
	public:
		using RuleBufferCallback = std::function<void(int32_t)>;
		using RuleCallback = std::function<int32_t()>;
		using RuleBuffer = std::unordered_map<int32_t, std::string>;

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
		Lexer(const std::string& inputStream);
		void Tokenize();
		Token NextToken();
		int32_t CreateRule(const std::string& regex, const RuleCallback& callback =
			[]() { return IGNORE; });
		void CreateRule(const RuleBuffer& ruleBuffer, const RuleBufferCallback& callback =
			[](int32_t type) { });
		inline const std::string& GetTokenContent() { return m_TokenContent; }
		inline void AdvanceLineCount() { m_LineCount++; }
		inline uint32_t GetLineCount() { return m_LineCount; }
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
		std::string m_Input;
		uint32_t m_InputOffset = 0U;
		ThompsonCalculator m_ThompsonCalculator;
		std::vector<Rule> m_Rules;

		std::string m_TokenContent;
		uint32_t m_LineCount = 1U;
	};
}