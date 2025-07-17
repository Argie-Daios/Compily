#include "Parser.h"

#include <iostream>

namespace Parsy
{
    Parser::Parser(const std::ifstream& inputStream)
        : m_Lexer(inputStream), m_CLR1(this)
    {
        m_CFGMap.emplace(m_StartingRule, RuleProperties());
        m_CFGMap.at(m_StartingRule).Grammar.AddElement({});
    }

    void Parser::Parse()
    {
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

        std::cout << "CLR1 Symbols = " << std::endl;
        m_CLR1.PrintSymbols();

        std::cout << std::endl;

        std::cout << "CLR1 Non Terminals = " << std::endl;
        m_CLR1.PrintNonTerminals();

        std::cout << std::endl;

        m_CLR1.GenerateStateGraph();
    }

    void Parser::BeginRule(RuleID_t rule, bool startRule)
    {
        m_CFGMap.emplace(rule, RuleProperties());
        m_CLR1.AddElement(CFGElement(CFGElementType::NonTerminal, rule));
        if (startRule)
        {
            auto& grammar = m_CFGMap.at(m_StartingRule).Grammar;
            grammar.m_Elements.at(grammar.m_ProductionCount - 1).at(0).Type = CFGElementType::NonTerminal;
            grammar.m_Elements.at(grammar.m_ProductionCount - 1).at(0).ID = rule;
        }
        m_BoundRule = rule;
    }

    void Parser::Add(const CFGElement& element)
    {
        if (element.Type == CFGElementType::Symbol && m_TokenMap.find(element.ID) == m_TokenMap.end())
        {
            m_TokenMap.emplace(element.ID, TokenProperties());
        }
        m_CFGMap.at(m_BoundRule).Grammar.AddElement(element);
        m_CLR1.AddElement(element);
    }

    void Parser::Union()
    {
        m_CFGMap.at(m_BoundRule).Grammar.Union();
    }

    void Parser::EndRule()
    {
        m_BoundRule = -1;
    }
}