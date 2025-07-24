#include "Parser.h"

#include <iostream>

namespace Parsy
{
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

        inputStack.Stack.clear();
        inputStack.Stack.push_back({ 0, {CFGElementType::Epsilon, -1} });
        Lexy::Lexer::Token token;
        bool keepToken = false;
        while (true)
        {
            if(!keepToken)
                token = m_Lexer->NextToken();
            keepToken = false;
            StackState& topState = inputStack.Stack.back();
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
            switch (action.Type)
            {
            case BottomUpActionType::Shift:
            {
                //std::cout << "Shift [ TokenID: " << token.TokenID << " ]" << std::endl;
                /*inputStack.Stack.push_back({ action.ActionData.at(BottomUpActionType::Shift).RuleID,
                    {CFGElementType::Symbol, token.TokenID}});*/
                Shift(inputStack, action, token);
                break;
            }
            case BottomUpActionType::Reduce:
            {
                Reduce(inputStack, action);
                /*auto& productions = m_CFGMap.at(action.ActionData.at(BottomUpActionType::Reduce).RuleID)
                    .Grammar.GetProductions();
                const Production& production = productions
                    .at(action.ActionData.at(BottomUpActionType::Reduce).ReducedProduction);
                int32_t popTimes = production.size();

                for (int32_t i = 0; i < popTimes; i++)
                {
                    if (production.at(i).Type == CFGElementType::Epsilon) continue;
                    inputStack.Stack.pop_back();
                }

                StackState& newTopState = inputStack.Stack.back();
                int32_t gotoID = m_CLR1.GetGotoState(newTopState.state, { CFGElementType::NonTerminal,
                    action.ActionData.at(BottomUpActionType::Reduce).RuleID });
                inputStack.Stack.push_back({ gotoID, { CFGElementType::NonTerminal,
                    action.ActionData.at(BottomUpActionType::Reduce).RuleID } });*/

               /* std::cout << "Reduce [ RuleID: " << action.ID << ", ProductionID: " <<
                    action.ReducedProduction << " ]" << std::endl;*/
                keepToken = true;

                break;
            }
            case BottomUpActionType::Accept:
            {
                std::cout << "Accepted" << std::endl;
                return true;
            }
            case BottomUpActionType::Empty:
            case BottomUpActionType::Error:
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
        //m_CLR1.PrintSymbols();

        std::cout << std::endl;

        std::cout << "CLR1 Non Terminals = " << std::endl;
       // m_CLR1.PrintNonTerminals();

        std::cout << std::endl;
    }

    void Parser::BeginRule(RuleID_t rule, bool startRule)
    {
        m_CFGMap.emplace(rule, RuleProperties());
        m_CLR1.RegisterNonTerminal(CFGElement(CFGElementType::NonTerminal, rule));
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

        switch (element.Type)
        {
        case CFGElementType::NonTerminal:
        {
            m_CLR1.RegisterNonTerminal(element);
            break;
        }
        case CFGElementType::Symbol:
        {
            m_CLR1.RegisterToken(element);
            break;
        }
        }
    }

    void Parser::Union(const TypeCallback& callback)
    {
        RuleProperties& ruleProps = m_CFGMap.at(m_BoundRule);
        size_t totalProductions = ruleProps.Grammar.GetProductionCount();
        ruleProps.RuleProductionCallbacks.emplace(totalProductions - 1, callback);
        ruleProps.Grammar.Union();
    }

    std::any& Parser::Get(int32_t offset)
    {
        return inputStack.Stack.at(inputStack.Stack.size() - elements + offset).entry;
    }

    void Parser::EndRule(const TypeCallback& callback)
    {
        RuleProperties& ruleProps = m_CFGMap.at(m_BoundRule);
        size_t totalProductions = ruleProps.Grammar.GetProductionCount();
        m_CFGMap.at(m_BoundRule).RuleProductionCallbacks.emplace(totalProductions - 1, callback);
        m_BoundRule = -1;
    }

    void Parser::Shift(ParseTree& parseTree, const BottomUpAction& action,
        const Lexy::Lexer::Token& token)
    {
        auto& inputStack = parseTree.Stack;
        inputStack.push_back({ action.ActionData.at(BottomUpActionType::Shift).RuleID,
            {CFGElementType::Symbol, token.TokenID} });

        auto& defaultTokenValue = m_Lexer->GetDefaultTokenValue();
        if (defaultTokenValue.has_value())
        {
            inputStack.back().entry = std::move(defaultTokenValue);
        }
        else
        {
            m_TokenMap.at(token.TokenID).TokenTypeConstructCallback(inputStack.back().entry);
        }
    }

    void Parser::Reduce(ParseTree& parseTree, const BottomUpAction& action)
    {
        auto& inputStack = parseTree.Stack;
        auto& productions = m_CFGMap.at(action.ActionData.at(BottomUpActionType::Reduce).RuleID)
            .Grammar.GetProductions();
        const Production& production = productions
            .at(action.ActionData.at(BottomUpActionType::Reduce).ReducedProduction);
        elements = production.size();

        StackState ruleState;
        ruleState.state = -1;
        ruleState.symbol = { CFGElementType::NonTerminal,
            action.ActionData.at(BottomUpActionType::Reduce).RuleID };
        m_CFGMap.at(action.ActionData.at(BottomUpActionType::Reduce).RuleID).
            RuleTypeConstructCallback(ruleState.entry);
        m_CFGMap.at(action.ActionData.at(BottomUpActionType::Reduce).RuleID).
            RuleProductionCallbacks.
            at(action.ActionData.at(BottomUpActionType::Reduce).ReducedProduction)(ruleState.entry);

        for (int32_t i = 0; i < elements; i++)
        {
            if (production.at(i).Type == CFGElementType::Epsilon) continue;
            inputStack.pop_back();
        }

        StackState& newTopState = inputStack.back();
        int32_t gotoID = m_CLR1.GetGotoState(newTopState.state, 
            { CFGElementType::NonTerminal, action.ActionData.at(BottomUpActionType::Reduce).RuleID });
        ruleState.state = gotoID;
        inputStack.push_back(ruleState);
    }
}