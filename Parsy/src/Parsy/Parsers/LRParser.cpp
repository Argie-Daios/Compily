#include "LRParser.h"

#include "Parsy/BottomUp/CLR1.h"

namespace Parsy
{
    LRParser::LRParser(const std::ifstream& inputStream)
        : Parser(inputStream)
    {
        
    }

    bool LRParser::Parse()
    {
        if (m_Lexer == nullptr)
        {
            std::cout << "Null" << std::endl;
            return false;
        }
        m_LR1->GenerateFirstSets();
        m_LR1->GenerateFollowSets();
        m_LR1->GenerateStateGraph();
        m_LR1->GenerateTable();
        Print();
        m_LR1->PrintStateGraph();
        m_LR1->PrintTable();

        m_InputStack.Stack.clear();
        m_InputStack.Stack.push_back({ 0, {CFGElementType::Dollar, -1} });
        Lexy::Lexer::Token token;
        while (true)
        {
            if (!m_KeepToken)
                token = m_Lexer->NextToken();
            m_KeepToken = false;
            ParseEntryData& topState = m_InputStack.Stack.back();
            CFGElement& tokenElement = GetTokenElement(token);

            BottomUpAction& action = m_LR1->GetAction(topState.State, tokenElement);
            switch (action.Type)
            {
            case BottomUpActionType::Shift:
            {
                OnShift(topState.State, action, token);
                std::cout << "Shift pushed " << m_InputStack.Stack.back().State << std::endl;
                break;
            }
            case BottomUpActionType::Reduce:
            {
                OnReduce(topState.State, action);
                std::cout << "Reduce pushed " << m_InputStack.Stack.back().State << std::endl;
                break;
            }
            case BottomUpActionType::Accept:
            {
                std::cout << "Accepted" << std::endl;
                return true;
            }
            case BottomUpActionType::Conflict:
            {
                std::cout << "Conflict" << std::endl;
                if (!OnConflict(topState.State, action, token)) return false;
                break;
            }
            case BottomUpActionType::Empty:
            {
                std::cout << "State: " << topState.State << std::endl;
                if (!OnEmpty(topState.State, action, token)) return false;
                std::cout << "Empty pushed " << m_InputStack.Stack.back().State << std::endl;
                break;
            }
            case BottomUpActionType::Error:
            {
                std::cout << "ERROR" << std::endl;
                return false;
            }
            }
        }

        std::cout << "Not Accepted" << std::endl;
        return false;
    }

    std::any& LRParser::Get(int32_t offset)
    {
        return m_InputStack.Stack.at(m_InputStack.Stack.size() - m_Elements + offset).Entry;
    }

    bool LRParser::OnShift(int32_t state, const BottomUpAction& action,
        const Lexy::Lexer::Token& token)
    {
        return Shift(state, action, token);
    }

    bool LRParser::Shift(int32_t state, const BottomUpAction& action, const Lexy::Lexer::Token& token)
    {
        auto& inputStack = m_InputStack.Stack;
        inputStack.push_back({ action.GetActionData(BottomUpActionType::Shift)->RuleID,
            {CFGElementType::Symbol, token.TokenID} });

        auto& defaultTokenValue = m_Lexer->GetDefaultTokenValue();
        if (defaultTokenValue.has_value())
        {
            inputStack.back().Entry = std::move(defaultTokenValue);
        }
        else
        {
            m_TokenMap.at(token.TokenID).TokenTypeConstructCallback(inputStack.back().Entry);
        }

        return true;
    }

    bool LRParser::OnReduce(int32_t state, const BottomUpAction& action)
    {
        return Reduce(state, action);
    }

    bool LRParser::Reduce(int32_t state, const BottomUpAction& action)
    {
        const BottomUpActionData* reduceData = action.GetActionData(BottomUpActionType::Reduce);
        auto& inputStack = m_InputStack.Stack;
        auto& productions = m_CFGMap.at(reduceData->RuleID)
            .Grammar.GetProductions();
        const Production& production = productions
            .at(reduceData->ReducedProduction);
        m_Elements = production.size();

        ParseEntryData& entry =
            ConstructEntryAndInvokeCallbacks(reduceData->RuleID, reduceData->ReducedProduction);

        for (int32_t i = 0; i < m_Elements; i++)
        {
            if (production.at(i).Type == CFGElementType::Epsilon) continue;
            inputStack.pop_back();
        }

        ParseEntryData& newParseEntry = inputStack.back();
        int32_t gotoID = m_LR1->GetGotoState(newParseEntry.State,
            { CFGElementType::NonTerminal, reduceData->RuleID });
        entry.State = gotoID;
        inputStack.push_back(entry);
        m_KeepToken = true;
        return true;
    }

    bool LRParser::OnConflict(int32_t state, const BottomUpAction& action, const Lexy::Lexer::Token& token)
    {
        return Conflict(state, action, token);
    }

    bool LRParser::Conflict(int32_t state, const BottomUpAction& action, const Lexy::Lexer::Token& token)
    {
        CFGElement& tokenElement = GetTokenElement(token);
        CFGElement* lastTerminal = GetLastTerminal();
        if (lastTerminal == nullptr)
        {
            std::cout << "FATAL ERROR" << std::endl;
            return false;
        }

        int32_t leftElementPriority = m_TokenMap.at(lastTerminal->ID).Priority;
        int32_t rightElementPriority = m_TokenMap.at(tokenElement.ID).Priority;
        if (leftElementPriority < rightElementPriority)
        {
            const BottomUpActionData* shiftAction = action.GetActionData(BottomUpActionType::Shift);
            if (shiftAction == nullptr)
            {
                std::cout << "FATAL ERROR" << std::endl;
                return false;
            }
            Shift(state, action, token);
        }
        else
        {
            const BottomUpActionData* reduceAction = action.GetActionData(BottomUpActionType::Reduce);
            if (reduceAction == nullptr)
            {
                std::cout << "FATAL ERROR" << std::endl;
                return false;
            }
            Reduce(state, action);
        }
    }

    bool LRParser::OnEmpty(int32_t state, const BottomUpAction& action, const Lexy::Lexer::Token& token)
    {
        return Empty(state, action, token);
    }

    bool LRParser::Empty(int32_t state, const BottomUpAction& action, const Lexy::Lexer::Token& token)
    {
        std::cout << "ERROR" << std::endl;
        return false;
    }

    CFGElement* LRParser::GetLastTerminal()
    {
        CFGElement* lastTerminal = nullptr;
        for (auto it = m_InputStack.Stack.rbegin(); it != m_InputStack.Stack.rend(); it++)
        {
            if (it->Symbol.Type == CFGElementType::Symbol || it->Symbol.Type == CFGElementType::Dollar)
            {
                lastTerminal = &it->Symbol;
                break;
            }
        }

        return lastTerminal;
    }
}