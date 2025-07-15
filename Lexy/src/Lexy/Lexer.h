#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <functional>

#include "ThompsonCalculator.h"

#define IGNORE -1

namespace Lexy
{
	using TokenID_t = int32_t;

	class Lexer
	{
	public:
		using RuleBufferCallback = std::function<void(TokenID_t)>;
		using RuleCallback = std::function<TokenID_t()>;
		using RuleBuffer = std::unordered_map<TokenID_t, std::string>;

		enum class TokenState
		{
			Success,
			Failure,
			End
		};

		struct Token
		{
			TokenState State;
			TokenID_t TokenID;

			Token() = default;
			Token(const TokenState& state, TokenID_t id)
				: State(state), TokenID(id)
			{

			}
		};
	public:
		Lexer(const std::ifstream& inputStream);
		void Tokenize();
		Token NextToken();
		TokenID_t CreateRule(const std::string& regex, const RuleCallback& callback =
			[]() { return IGNORE; });
		void CreateRule(const RuleBuffer& ruleBuffer, const RuleBufferCallback& callback =
			[](TokenID_t tokenID) { });
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