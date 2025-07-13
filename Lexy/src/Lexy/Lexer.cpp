#include "Lexer.h"

#define INITIAL_RULE_SLOTS 50

namespace Lexy
{
	Lexer::Lexer(const std::string& input)
	{
        m_Input = input;
		m_Rules.reserve(INITIAL_RULE_SLOTS);
	}

	void Lexer::Tokenize()
	{
        Lexy::Lexer::Token token;
        while ((token = Lexy::Lexer::NextToken()).State == Lexy::Lexer::TokenState::Success)
        {

        }
	}

    Lexer::Token Lexer::NextToken()
	{
        if (m_InputOffset >= m_Input.length()) return Token(TokenState::End, -1);

        int32_t maxLength = -1;
        int32_t maxRule = -1;

        for (int32_t i = 0; i < m_Rules.size(); i++)
        {
            auto& rule = m_Rules.at(i);
            int len = rule.NFAGraph.Match(m_Input, m_InputOffset);

            if (len > maxLength) 
            {
                maxLength = len;
                maxRule = i;
            }
        }

        if (maxLength <= 0) 
        {
            std::cout << "Error: Invalid token" << std::endl;
            return Token(TokenState::Failure, -1);
        }
        
        m_TokenContent = m_Input.substr(m_InputOffset, maxLength);
        m_InputOffset += maxLength;

        auto& rule = m_Rules.at(maxRule);
        int32_t tokenType = rule.Callback();

        if (tokenType == -1)
        {
            return NextToken();
        }

        return Token(TokenState::Success, tokenType);
	}

	int32_t Lexer::CreateRule(const std::string& regex, const RuleCallback& callback)
	{
		m_ThompsonCalculator.ChangeRegularExpression(regex);
		m_Rules.emplace_back(std::move(m_ThompsonCalculator.CalculateNFA()), callback);
		return (int32_t)m_Rules.size() - 1;
	}

    void Lexer::CreateRule(const RuleBuffer& ruleBuffer, const RuleBufferCallback& callback)
    {
        for (auto& [tokenType, regex] : ruleBuffer)
        {
            m_ThompsonCalculator.ChangeRegularExpression(regex);
            m_Rules.emplace_back(std::move(m_ThompsonCalculator.CalculateNFA()), [=]() {
                callback(tokenType);
                return tokenType;
            });
        }
    }
}