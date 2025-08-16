#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <functional>
#include <any>
#include <Utilities.h>

#include "ThompsonCalculator.h"

#define TOKEN_IGNORE -1

namespace Lexy
{
	using TokenID_t = int32_t;

	class Lexer
	{
	public:
		using RuleBufferCallback = std::function<void(TokenID_t)>;
		using RuleCallback = std::function<TokenID_t()>;
		using RuleBuffer = std::vector<std::pair<TokenID_t, std::string>>;

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
			size_t StringOffset = 0;
			size_t StringCount = 0;

			Token() = default;
			Token(const TokenState& state, TokenID_t id, size_t offset = 0, size_t count = 0)
				: State(state), TokenID(id), StringOffset(offset), StringCount(count)
			{

			}
		};
		
		struct OfflineToken
		{
			Token TokenData;
			std::any DefaultVaue;
		};

	public:
		Lexer(const std::ifstream& inputStream);
		std::vector<OfflineToken> Tokenize();
		Token NextToken();
		TokenID_t CreateRule(const std::string& regex, const RuleCallback& callback =
			[]() { return TOKEN_IGNORE; });
		void CreateRule(const RuleBuffer& ruleBuffer, const RuleBufferCallback& callback =
			[](TokenID_t tokenID) { });
		inline std::any& GetDefaultTokenValue() { return m_DefaultTokenValue; }
		inline const std::string& GetTokenContent() { return m_TokenContent; }
		inline void AdvanceLineCount() { m_LineCount++; }
		inline uint32_t GetLineCount() { return m_LineCount; }
		inline const std::string& GetInputString() const { return m_Input; }
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

		std::any m_DefaultTokenValue;
		std::string m_TokenContent;
		uint32_t m_LineCount = 1U;
	};
}