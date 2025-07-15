#include "Parser.h"

#include <iostream>

namespace Parsy
{
    Parser::Parser(const std::ifstream& inputStream)
        : m_Lexer(inputStream), m_LALR1(this)
    {

    }

    void Parser::Parse()
    {
        for (auto& [ruleID, cfg] : m_CFGMap)
        {
            std::cout << "Rule(" << ruleID << ") ===> ";
            for (auto& elementList : cfg.GetProductions())
            {
                for (auto& element : elementList)
                {
                    switch (element.Type)
                    {
                    case CFGElementType::Symbol:
                    {
                        std::cout << "Token(" << element.ID << ")";
                        break;
                    }
                    case CFGElementType::NonTerminal:
                    {
                        std::cout << "Rule(" << element.ID << ")";
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
                    std::cout << " ";
                }
                std::cout << " | ";
            }
            std::cout << std::endl;
        }
    }

    void Parser::BeginRule(RuleID_t rule)
    {
        m_CFGMap.emplace(rule, CFG());
        m_BoundRule = rule;
    }

    void Parser::Add(const CFGElement& element)
    {
        m_CFGMap.at(m_BoundRule).AddElement(element);
    }

    void Parser::Union()
    {
        m_CFGMap.at(m_BoundRule).Union();
    }

    void Parser::EndRule()
    {
        m_BoundRule = -1;
    }
}