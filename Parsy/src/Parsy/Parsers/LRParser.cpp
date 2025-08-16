#include "LRParser.h"

#include "Parsy/BottomUp/CLR1.h"

namespace Parsy
{
    LRParser::LRParser(const std::ifstream& inputStream, int32_t flags)
        : Parser(inputStream, flags)
    {
        
    }

    bool LRParser::Parse()
    {
        ASSERT(m_Lexer != nullptr, "Lexer reference is nullptr!!");
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
        std::vector<std::string> labels = { "States", "Stack", "Token", "Action", "String Range"};
        std::vector<std::string> elements;

        while (true)
        {
            if (!m_KeepToken)
                token = m_Lexer->NextToken();
            m_KeepToken = false;
            ParseEntryData& topState = m_InputStack.Stack.back();
            CFGElement& tokenElement = GetTokenElement(token);

            std::string states;
            std::string stack;
            std::string tokenStr;
            std::string stringRange = "---";
            size_t count = 0;
            for (int32_t i = 0; i < m_InputStack.Stack.size(); i++)
            {
                const ParseEntryData& entry = m_InputStack.Stack.at(i);
                count += entry.Token.StringCount;

                if (i == 1)
                {
                    stringRange = std::to_string(entry.Token.StringOffset);
                }
                if (count > 0 && i == m_InputStack.Stack.size() - 1)
                {
                    stringRange += " - " + std::to_string(count);
                }

                states += '(' + std::to_string(entry.State) + ')';
                switch (entry.Symbol.Type)
                {
                case CFGElementType::NonTerminal:
                {
                    stack += '{' + CFGElementToStr(entry.Symbol) + '}';
                    break;
                }
                case CFGElementType::Symbol:
                {
                    stack += '{' + CFGElementToStr(entry.Symbol) + '}';
                    break;
                }
                default:
                {
                    stack += CFGElementToStr(entry.Symbol);
                    break;
                }
                }
            }
            if (tokenElement.Type == CFGElementType::Dollar)
                tokenStr += '$';
            else
                tokenStr += TokenToStr(tokenElement.ID);
                
            elements.push_back(states);
            elements.push_back(stack);
            elements.push_back(tokenStr);
            if (m_ActionString.empty())
                elements.push_back("---");
            else
                elements.push_back(m_ActionString);
            elements.push_back(stringRange);

            BottomUpAction& action = m_LR1->GetAction(topState.State, tokenElement);
            switch (action.Type)
            {
            case BottomUpActionType::Shift:
            {
                OnShift(topState.State, action, CFGElementType::Symbol, token);
                break;
            }
            case BottomUpActionType::Reduce:
            {
                OnReduce(topState.State, action);
                break;
            }
            case BottomUpActionType::Accept:
            {
                ExportResult("Accepted", labels, elements);
                return true;
            }
            case BottomUpActionType::Conflict:
            {
                if (!OnConflict(topState.State, action, token))
                {
                    ExportResult("Error", labels, elements);
                    auto& errorAction = m_LR1->GetAction(topState.State, CFGElement(CFGElementType::Error, -1));
                    if (errorAction.Type == BottomUpActionType::Empty)
                    {
                        SyntaxErrorHandler();
                    }
                    Shift(topState.State, errorAction, CFGElementType::Error, token);
                    m_KeepToken = true;
                }
                break;
            }
            case BottomUpActionType::Empty:
            {
                if (topState.Symbol.Type == CFGElementType::Error)
                {
                    SyntaxErrorHandler();
                }
                if (!OnEmpty(topState.State, action, token))
                {
                    auto& errorAction = m_LR1->GetAction(topState.State, CFGElement(CFGElementType::Error, -1));
                    if (errorAction.Type == BottomUpActionType::Empty)
                    {
                        ExportResult("Error", labels, elements);
                        SyntaxErrorHandler();
                    }
                    switch (errorAction.Type)
                    {
                    case BottomUpActionType::Empty:
                    {
                        ExportResult("Error", labels, elements);
                        SyntaxErrorHandler();
                        break;
                    }
                    case BottomUpActionType::Shift:
                    {
                        Shift(topState.State, errorAction, CFGElementType::Error, token);
                        break;
                    }
                    case BottomUpActionType::Reduce:
                    {
                        Reduce(topState.State, errorAction);
                        break;
                    }
                    }
                    m_KeepToken = true;
                }
                break;
            }
            case BottomUpActionType::Error:
            {
                ExportResult("Error", labels, elements);

                auto& errorAction = m_LR1->GetAction(topState.State, CFGElement(CFGElementType::Error, -1));
                if (errorAction.Type == BottomUpActionType::Empty)
                {
                    SyntaxErrorHandler();
                }
                Shift(topState.State, errorAction, CFGElementType::Error, token);
                m_KeepToken = true;
            }
            }
        }

        ExportResult("Not Accepted", labels, elements);
        return false;
    }

    std::any& LRParser::GetValue(int32_t offset)
    {
        return m_InputStack.Stack.at(m_InputStack.Stack.size() - m_Elements + offset).Entry;
    }

    bool LRParser::OnShift(int32_t state, const BottomUpAction& action, const CFGElementType& type,
        const Lexy::Lexer::Token& token)
    {
        return Shift(state, action, type, token);
    }

    bool LRParser::Shift(int32_t state, const BottomUpAction& action, const CFGElementType& type,
        const Lexy::Lexer::Token& token)
    {
        auto& inputStack = m_InputStack.Stack;
        int32_t id = action.GetActionData(BottomUpActionType::Shift)->RuleID;
        inputStack.push_back({ id,
            {type, token.TokenID} });
        inputStack.back().Token = token;

        auto& defaultTokenValue = m_Lexer->GetDefaultTokenValue();
        if (defaultTokenValue.has_value())
        {
            inputStack.back().Entry = std::move(defaultTokenValue);
        }
        else
        {
            m_TokenMap.at(token.TokenID).TokenTypeConstructCallback(inputStack.back().Entry);
        }

        m_ActionString = "Shift(" + std::to_string(id) + ')';
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
        const auto& production = productions
            .at(reduceData->ReducedProduction);
        m_Elements = production.Elements.size();

        ParseEntryData& entry =
            ConstructEntryAndInvokeCallbacks(reduceData->RuleID, reduceData->ReducedProduction);

        Lexy::Lexer::Token startToken;
        size_t count = 0;
        for (int32_t i = 0; i < m_Elements; i++)
        {
            if (production.Elements.at(i).Type == CFGElementType::Epsilon) continue;

            if(i == m_Elements - 1)
                startToken = inputStack.back().Token;
            count += inputStack.back().Token.StringCount;

            inputStack.pop_back();
        }

        ParseEntryData& newParseEntry = inputStack.back();
        int32_t gotoID = m_LR1->GetGotoState(newParseEntry.State,
            { CFGElementType::NonTerminal, reduceData->RuleID });
        entry.State = gotoID;
        entry.Token = Lexy::Lexer::Token(Lexy::Lexer::TokenState::Success, -1, startToken.StringOffset, count);
        inputStack.push_back(entry);
        m_KeepToken = true;

        m_ActionString = "Reduce(" + RuleToStr(reduceData->RuleID) + " --> ";
        for (size_t i = 0; i < production.Elements.size(); i++)
        {
            const CFGElement& element = production.Elements.at(i);
            m_ActionString += CFGElementToStr(element);
            if (i < production.Elements.size() - 1)
                m_ActionString += ' ';
            else
                m_ActionString += ')';
        }
        
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
        ASSERT(lastTerminal != nullptr, "Fatal Error");

        int32_t leftElementPriority = m_TokenMap.at(lastTerminal->ID).Priority;
        PrecedenceAssociativity leftElementAssociativity = m_TokenMap.at(lastTerminal->ID).Associativity;
        const BottomUpActionData* reduceActionData = action.GetActionData(BottomUpActionType::Reduce);
        if (reduceActionData != nullptr)
        {
            const ProductionData& productionData = m_CFGMap.at(reduceActionData->RuleID).Grammar.GetProductions()
                .at(reduceActionData->ReducedProduction);
            if (productionData.Associativity != PrecedenceAssociativity::None)
            {
                leftElementAssociativity = productionData.Associativity;
                leftElementPriority = productionData.Priority;
            }
        }

        int32_t rightElementPriority = m_TokenMap.at(tokenElement.ID).Priority;
        PrecedenceAssociativity rightElementAssociativity = m_TokenMap.at(tokenElement.ID).Associativity;
        if (leftElementPriority < rightElementPriority)
        {
            const BottomUpActionData* shiftAction = action.GetActionData(BottomUpActionType::Shift);
            ASSERT(shiftAction != nullptr, "There is no shift action!!");
            Shift(state, action, CFGElementType::Symbol, token);
        }
        else if (leftElementPriority > rightElementPriority)
        {
            ASSERT(reduceActionData != nullptr, "There is no reduce action!!");
            Reduce(state, action);
        }
        else
        {
            ASSERT(leftElementAssociativity == rightElementAssociativity, "There is no tie on associativity stage!!");
            switch (leftElementAssociativity)
            {
            case PrecedenceAssociativity::Left:
            {
                const BottomUpActionData* reduceAction = action.GetActionData(BottomUpActionType::Reduce);
                ASSERT(reduceAction != nullptr, "There is no reduce action!!");
                Reduce(state, action);
                break;
            }
            case PrecedenceAssociativity::Right:
            {
                const BottomUpActionData* shiftAction = action.GetActionData(BottomUpActionType::Shift);
                ASSERT(shiftAction != nullptr, "There is no shift action!!");
                Shift(state, action, CFGElementType::Symbol, token);
                break;
            }
            case PrecedenceAssociativity::NonAssociate:
            {
                ASSERT(false, "Tie breaker with non-associate associativity!!");
                break;
            }
            }
        }
    }

    bool LRParser::OnEmpty(int32_t state, const BottomUpAction& action, const Lexy::Lexer::Token& token)
    {
        return Empty(state, action, token);
    }

    bool LRParser::Empty(int32_t state, const BottomUpAction& action, const Lexy::Lexer::Token& token)
    {
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

    void LRParser::ExportParseStack(std::vector<std::string>& labels, std::vector<std::string>& elements)
    {
        Utilities::TableStream stackTable("ParseStack.txt", std::ios::out, Utilities::TableStreamFlags_ColumnsLabel
            | Utilities::TableStreamFlags_RowsLabel | Utilities::TableStreamFlags_RowSeperator);

        stackTable.BindGetColumnLabelCallback([&](size_t col) -> const std::string& { return labels.at(col); });
        stackTable.BindGetRowLabelCallback([&](size_t row) -> const std::string& {
            static std::string helper;
            helper.clear();
            helper = std::to_string(row);
            return helper;
            });
        stackTable.BindGetTotalColumnsCallback([&]() { return labels.size(); });
        stackTable.BindGetTotalRowsCallback([&]() { return elements.size() / labels.size(); });
        stackTable.BindGetElementStringCallback([&](size_t row, size_t col) -> const std::string& {
            size_t index = labels.size() * row + col;
            return elements.at(index);
            });

        stackTable.SetLabel("Parsing Stack");
        stackTable.SetLabelHorizontalAlignment(Utilities::HorizontalAlignment::Center);
        stackTable.SetHorizontalAlignment(Utilities::HorizontalAlignment::Center);
        stackTable.SetRowHorizontalSpacing(4);
        stackTable.SetColumnHorizontalSpacing(4);

        stackTable.Export();
    }

    void LRParser::ExportResult(const std::string& message, std::vector<std::string>& labels, std::vector<std::string>& elements)
    {
        elements.push_back(message);
        elements.push_back(message);
        elements.push_back(message);
        elements.push_back(message);
        elements.push_back(message);
        ExportParseStack(labels, elements);
    }
}