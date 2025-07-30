#include "Parser.h"

#include <iostream>

namespace Parsy
{
    Parser::Parser(const std::ifstream& inputStream)
        : m_LR1(nullptr)
    {
        m_CFGMap.emplace(m_StartingRule, RuleProperties());
        m_CFGMap.at(m_StartingRule).Grammar.AddElement({});
    }

    void Parser::Print()
    { 
        auto& set = m_LR1->GetFirstSet({ CFGElementType::NonTerminal, 0 });
        std::cout << "First Set of Rule(0) { ";
        for (auto& element : set)
        {
            element.Print();
            std::cout << " ";
        }
        std::cout << "}" << std::endl << std::endl;

        for (auto& [ruleID, ruleProps] : m_CFGMap)
        {
            std::cout << "Rule(" << ruleID << ") ===> ";
            for (auto& elementList : ruleProps.Grammar.GetProductions())
            {
                for (auto& element : elementList)
                {
                    std::any any;
                    switch (element.Type)
                    {
                    case CFGElementType::Symbol:
                    {
                        std::cout << "Token(" << element.ID << ")";
                        m_TokenMap.at(element.ID).TokenTypeConstructCallback(any);
                        break;
                    }
                    case CFGElementType::NonTerminal:
                    {
                        std::cout << "Rule(" << element.ID << ")";
                        m_CFGMap.at(element.ID).RuleTypeConstructCallback(any);
                        break;
                    }
                    case CFGElementType::Epsilon:
                    {
                        std::cout << "Empty";
                        break;
                    }
                    case CFGElementType::Dollar:
                    {
                        std::cout << "Dollar";
                    }
                    }
                    if (any.has_value())
                    {
                        std::cout << "[Type: " << any.type().name() << "]";
                    }
                    else
                    {
                        std::cout << "[Type: NaN]";
                    }
                    std::cout << " ";
                }
                std::cout << " | ";
            }
            std::cout << std::endl;
        }

        std::cout << std::endl;
    }

    void Parser::BeginRule(RuleID_t rule, bool startRule)
    {
        m_CFGMap.emplace(rule, RuleProperties());
        m_LR1->RegisterNonTerminal(CFGElement(CFGElementType::NonTerminal, rule));
        if (startRule)
        {
            auto& grammar = m_CFGMap.at(m_StartingRule).Grammar;
            grammar.m_Elements.at(grammar.m_ProductionCount - 1).at(0).Type = CFGElementType::NonTerminal;
            grammar.m_Elements.at(grammar.m_ProductionCount - 1).at(0).ID = rule;
        }
        m_BoundRule = rule;
    }

    void Parser::Add(const CFGElement& element, const TypeCallback& callback)
    {
        if (element.Type == CFGElementType::Symbol && m_TokenMap.find(element.ID) == m_TokenMap.end())
        {
            m_TokenMap.emplace(element.ID, TokenProperties());
        }
        RuleProperties& ruleProps = m_CFGMap.at(m_BoundRule);
        ruleProps.Grammar.AddElement(element);
        size_t totalProductions = ruleProps.Grammar.GetProductionCount();

        switch (element.Type)
        {
        case CFGElementType::NonTerminal:
        {
            m_LR1->RegisterNonTerminal(element);
            break;
        }
        case CFGElementType::Symbol:
        {
            m_LR1->RegisterToken(element);
            break;
        }
        }

        ruleProps.RuleProductionCallbacks.try_emplace(totalProductions - 1);
        ruleProps.RuleProductionCallbacks.at(totalProductions - 1).push_back(std::move(callback));
    }

    void Parser::Union()
    {
        RuleProperties& ruleProps = m_CFGMap.at(m_BoundRule);
        ruleProps.Grammar.Union();
    }

    void Parser::EndRule()
    {
        m_BoundRule = -1;
    }

    CFGElement Parser::GetNextTokenElement()
    {
        Lexy::Lexer::Token token = m_Lexer->NextToken();
        CFGElement tokenElement;
        switch (token.State)
        {
        case Lexy::Lexer::TokenState::Success:
        {
            tokenElement.Type = CFGElementType::Symbol;
            tokenElement.ID = token.TokenID;
            break;
        }
        case Lexy::Lexer::TokenState::End:
        {
            tokenElement.Type = CFGElementType::Dollar;
            tokenElement.ID = -1;
            break;
        }
        case Lexy::Lexer::TokenState::Failure:
        {

            break;
        }
        }
        return tokenElement;
    }

    CFGElement Parser::GetTokenElement(const Lexy::Lexer::Token& token)
    {
        CFGElement tokenElement;
        switch (token.State)
        {
        case Lexy::Lexer::TokenState::Success:
        {
            tokenElement.Type = CFGElementType::Symbol;
            tokenElement.ID = token.TokenID;
            break;
        }
        case Lexy::Lexer::TokenState::End:
        {
            tokenElement.Type = CFGElementType::Dollar;
            tokenElement.ID = -1;
            break;
        }
        case Lexy::Lexer::TokenState::Failure:
        {

            break;
        }
        }
        return tokenElement;
    }

    ParseEntryData Parser::ConstructEntry(RuleID_t ruleID, int32_t production)
    {
        ParseEntryData parseEntry;
        parseEntry.State = -1;
        parseEntry.Symbol = { CFGElementType::NonTerminal,
            ruleID };
        m_CFGMap.at(ruleID).
            RuleTypeConstructCallback(parseEntry.Entry);
        return parseEntry;
    }

    ParseEntryData Parser::InvokeCallbacks(RuleID_t ruleID, int32_t production)
    {
        ParseEntryData parseEntry;
        parseEntry.State = -1;
        parseEntry.Symbol = { CFGElementType::NonTerminal,
            ruleID };
        
        auto& productionCallbacks = m_CFGMap.at(ruleID).RuleProductionCallbacks.
            at(production);
        for (auto& function : productionCallbacks)
        {
            function(parseEntry.Entry);
        }

        return parseEntry;
    }

    ParseEntryData Parser::ConstructEntryAndInvokeCallbacks(RuleID_t ruleID, int32_t production)
    {
        ParseEntryData parseEntry;
        parseEntry.State = -1;
        parseEntry.Symbol = { CFGElementType::NonTerminal,
            ruleID };
        m_CFGMap.at(ruleID).
            RuleTypeConstructCallback(parseEntry.Entry);

        auto& productionCallbacks = m_CFGMap.at(ruleID).RuleProductionCallbacks.
            at(production);
        for (auto& function : productionCallbacks)
        {
            function(parseEntry.Entry);
        }

        return parseEntry;
    }
}