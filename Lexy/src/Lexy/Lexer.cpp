#include "Lexer.h"

#include <sstream>
#include "Macros.h"
#include "Graphs/DFA.h"

#define INITIAL_RULE_SLOTS 50

namespace Lexy
{
	Lexer::Lexer(const std::ifstream& inputStream)
	{
        if (!inputStream.good())
        {
            // Error
        }
        std::stringstream ss;
        ss << inputStream.rdbuf();
        m_Input = ss.str();
		m_Rules.reserve(INITIAL_RULE_SLOTS);

        int32_t id = m_NFA.GetFiniteAutomate().PushVertex();
        m_NFA.GetFiniteAutomate().GetVertex(id).Data.StateId = 0;
        m_NFA.GetFiniteAutomate().GetVertex(id).Data.TokenIndex = 0;
        m_NFA.SetStart(id);
	}

    Lexer::Lexer(const std::string& sourceCodePath)
        : Lexer(std::ifstream(sourceCodePath))
    {

    }

    void Lexer::GenerateDFA()
    {
        m_NFA.Print();
        m_DFA.GenerateFromNFA(m_NFA);
        m_DFA.Print();
    }

    std::vector<Lexer::OfflineToken> Lexer::Tokenize()
	{
        Lexy::Lexer::Token token;
        std::vector<OfflineToken> tokenStream;
        while ((token = Lexy::Lexer::NextToken()).State == Lexy::Lexer::TokenState::Success)
        {
            tokenStream.push_back({ token, std::move(m_DefaultTokenValue) });
        }
        if (token.State == TokenState::Failure)
        {
            tokenStream.push_back({ Lexy::Lexer::Token(Lexy::Lexer::TokenState::Failure, -1) });
        }
        else
        {
            tokenStream.push_back({ Lexy::Lexer::Token(Lexy::Lexer::TokenState::End, -1) });
        }
        return tokenStream;
	}

    Lexer::Token Lexer::NextToken()
	{
        m_DefaultTokenValue.reset();
        if (m_InputOffset >= m_Input.length())
        {
            return Token(TokenState::End, -1);
        }

        int32_t maxLength = -1;
        int32_t maxRule = -1;

        /*for (int32_t i = 0; i < m_Rules.size(); i++)
        {
            auto& rule = m_Rules.at(i);
            MatchResult len = rule.NFAGraph.Match(m_Input, m_InputOffset);

            if (len.Length > maxLength)
            {
                maxLength = len.Length;
                maxRule = len.TokenIndex;
            }
        }*/

        MatchResult len = m_DFA.Match(m_Input, m_InputOffset);
        maxLength = len.Length;
        maxRule = len.TokenIndex;

        if (maxLength <= 0) 
        {
            std::cout << "Error: Invalid token on line " << m_LineCount << std::endl;
            return Token(TokenState::Failure, -1);
        }
        
        int32_t offset = m_InputOffset;
        m_TokenContent = m_Input.substr(m_InputOffset, maxLength);
        m_InputOffset += maxLength;

        auto& rule = m_Rules.at(maxRule);
        TokenID_t tokenType = rule.Callback();

        if (tokenType == -1)
        {
            return NextToken();
        }

        return Token(TokenState::Success, tokenType, offset, maxLength);
	}

    TokenID_t Lexer::CreateRule(const std::string& regex, const RuleCallback& callback)
	{
		m_ThompsonCalculator.ChangeRegularExpression(regex);
		m_Rules.emplace_back(std::move(m_ThompsonCalculator.CalculateNFA()), callback);

        int32_t tokenIndex = m_Rules.size() - 1;
        NFAGraph& fa = m_Rules.back().NFAGraph.GetFiniteAutomate();
        auto& vertices = fa.GetVertices();
        auto& accepting = m_Rules.back().NFAGraph.GetAccepting();
        for (int32_t acceptState : accepting)
        {
            auto& vertex = m_Rules.back().NFAGraph.GetFiniteAutomate().GetVertex(acceptState);
            vertex.Data.TokenIndex = tokenIndex;
            vertex.Data.Priority = m_HighestPriority;
        }
        m_HighestPriority--;

        auto& mfa = m_NFA.GetFiniteAutomate();
        size_t totalVertices = mfa.GetTotalVertices();
        mfa |= fa;
        mfa.PushEdge(m_NFA.GetStart(), m_Rules.back().NFAGraph.GetStart() + totalVertices, EPSILON);
        m_NFA.GetAccepting().insert(*m_Rules.back().NFAGraph.GetAccepting().begin() + totalVertices);

		return tokenIndex;
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

            int32_t tokenIndex = m_Rules.size() - 1;
            auto& fa = m_Rules.back().NFAGraph.GetFiniteAutomate();
            auto& vertices = fa.GetVertices();
            auto& accepting = m_Rules.back().NFAGraph.GetAccepting();
            for (int32_t acceptState : accepting)
            {
                auto& vertex = m_Rules.back().NFAGraph.GetFiniteAutomate().GetVertex(acceptState);
                vertex.Data.TokenIndex = tokenIndex;
                vertex.Data.Priority = m_HighestPriority;
            }
            m_HighestPriority--;

            auto& mfa = m_NFA.GetFiniteAutomate();
            size_t totalVertices = mfa.GetTotalVertices();
            mfa |= fa;
            mfa.PushEdge(m_NFA.GetStart(), m_Rules.back().NFAGraph.GetStart() + totalVertices, EPSILON);
            m_NFA.GetAccepting().insert(*m_Rules.back().NFAGraph.GetAccepting().begin() + totalVertices);
        }
    }
}