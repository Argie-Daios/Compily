#include "Parser.h"

#include <iostream>

namespace Parsy
{
    struct StackState
    {
        int32_t state;
        CFGElement symbol;
    };

    Parser::Parser(const std::ifstream& inputStream)
        : m_CLR1(this)
    {
        m_CFGMap.emplace(m_StartingRule, RuleProperties());
        m_CFGMap.at(m_StartingRule).Grammar.AddElement({});
    }

    bool Parser::Parse()
    {
        if (m_Lexer == nullptr)
        {
            std::cout << "Null" << std::endl;
            return false;
        }
        m_CLR1.GenerateFirstSets();
        m_CLR1.GenerateStateGraph();
        m_CLR1.GenerateTable();
        Print();

        std::vector<StackState> inputStack = { {0, {CFGElementType::Epsilon, -1}} };
        Lexy::Lexer::Token token;
        bool keepToken = false;
        while (true)
        {
            if(!keepToken)
                token = m_Lexer->NextToken();
            keepToken = false;
            StackState& topState = inputStack.back();
            CFGElement element;
            switch (token.State)
            {
            case Lexy::Lexer::TokenState::Success:
            {
                element.Type = CFGElementType::Symbol;
                element.ID = token.TokenID;
                break;
            }
            case Lexy::Lexer::TokenState::End:
            {
                element.Type = CFGElementType::Dollar;
                element.ID = -1;
                break;
            }
            case Lexy::Lexer::TokenState::Failure:
            {

                break;
            }
            }
            
            BottomUpAction& action = m_CLR1.GetAction(topState.state, element);
            switch (action.Types)
            {
            case BottomUpActionType_Shift:
            {
                //std::cout << "Shift [ TokenID: " << token.TokenID << " ]" << std::endl;
                inputStack.push_back({ action.ID, {CFGElementType::Symbol, token.TokenID} });
                break;
            }
            case BottomUpActionType_Reduce:
            {
                auto& productions = m_CFGMap.at(action.ID).Grammar.GetProductions();
                const Production& production = productions.at(action.ReducedProduction);
                int32_t popTimes = production.size();

                for (int32_t i = 0; i < popTimes; i++)
                {
                    if (production.at(i).Type == CFGElementType::Epsilon) continue;
                    inputStack.pop_back();
                }

                StackState& newTopState = inputStack.back();
                int32_t gotoID = m_CLR1.GetGotoState(newTopState.state, { CFGElementType::NonTerminal, action.ID });
                inputStack.push_back({ gotoID, { CFGElementType::NonTerminal, action.ID } });

               /* std::cout << "Reduce [ RuleID: " << action.ID << ", ProductionID: " <<
                    action.ReducedProduction << " ]" << std::endl;*/
                keepToken = true;

                break;
            }
            case BottomUpActionType_Accept:
            {
                std::cout << "Accepted" << std::endl;
                return true;
            }
            case BottomUpActionType_Error:
            {
                std::cout << "ERROR" << std::endl;
                return false;
            }
            }

            /*std::cout << "Stack" << std::endl;
            std::cout << "====================" << std::endl;
            for (auto& it = inputStack.rbegin(); it != inputStack.rend(); it++)
            {
                std::cout << "StackElement: [ State: " << it->state << ", Symbol: ";
                it->symbol.Print();
                std::cout << " ]" << std::endl;
            }
            std::cout << "====================" << std::endl;*/
        }

        std::cout << "Not Accepted" << std::endl;
        return false;
    }

    void Parser::Print()
    { 
        auto& set = m_CLR1.GetFirstSet({ CFGElementType::NonTerminal, 0 });
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

        std::cout << "CLR1 Symbols = " << std::endl;
        m_CLR1.PrintSymbols();

        std::cout << std::endl;

        std::cout << "CLR1 Non Terminals = " << std::endl;
        m_CLR1.PrintNonTerminals();

        std::cout << std::endl;
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