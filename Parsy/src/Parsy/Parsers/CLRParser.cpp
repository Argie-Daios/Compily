#include "CLRParser.h"

namespace Parsy
{
	CLRParser::CLRParser(const std::ifstream& inputStream)
		: Parser(inputStream)
	{
        m_LR1 = std::make_unique<CLR1>(this);
	}

	bool CLRParser::Parse()
	{
        if (m_Lexer == nullptr)
        {
            std::cout << "Null" << std::endl;
            return false;
        }
        m_LR1->GenerateFirstSets();
        m_LR1->GenerateStateGraph();
        m_LR1->GenerateTable();
        Print();

        m_InputStack.Stack.clear();
        m_InputStack.Stack.push_back({ 0, {CFGElementType::Epsilon, -1} });
        Lexy::Lexer::Token token;
        bool keepToken = false;
        while (true)
        {
            if (!keepToken)
                token = m_Lexer->NextToken();
            keepToken = false;
            ParseEntryData& topState = m_InputStack.Stack.back();
            CFGElement& tokenElement = GetTokenElement(token);

            BottomUpAction& action = m_LR1->GetAction(topState.State, tokenElement);
            switch (action.Type)
            {
            case BottomUpActionType::Shift:
            {
                Shift(action, token);
                break;
            }
            case BottomUpActionType::Reduce:
            {
                Reduce(action);
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
        }

        std::cout << "Not Accepted" << std::endl;
        return false;
	}

    std::any& CLRParser::Get(int32_t offset)
    {
        return m_InputStack.Stack.at(m_InputStack.Stack.size() - m_Elements + offset).Entry;
    }

	void CLRParser::Shift(const BottomUpAction& action,
		const Lexy::Lexer::Token& token)
	{
        auto& inputStack = m_InputStack.Stack;
        inputStack.push_back({ action.ActionData.at(BottomUpActionType::Shift).RuleID,
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
	}

	void CLRParser::Reduce(const BottomUpAction& action)
	{
        auto& inputStack = m_InputStack.Stack;
        auto& productions = m_CFGMap.at(action.ActionData.at(BottomUpActionType::Reduce).RuleID)
            .Grammar.GetProductions();
        const BottomUpActionData& bottomActionData = action.ActionData.at(BottomUpActionType::Reduce);
        const Production& production = productions
            .at(bottomActionData.ReducedProduction);
        m_Elements = production.size();

        ParseEntryData& entry = 
            ConstructEntryAndInvokeCallbacks(bottomActionData.RuleID, bottomActionData.ReducedProduction);

        for (int32_t i = 0; i < m_Elements; i++)
        {
            if (production.at(i).Type == CFGElementType::Epsilon) continue;
            inputStack.pop_back();
        }

        ParseEntryData& newParseEntry = inputStack.back();
        int32_t gotoID = m_LR1->GetGotoState(newParseEntry.State,
            { CFGElementType::NonTerminal, action.ActionData.at(BottomUpActionType::Reduce).RuleID });
        entry.State = gotoID;
        inputStack.push_back(entry);
	}
}