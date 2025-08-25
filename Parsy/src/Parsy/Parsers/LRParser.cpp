#include "LRParser.h"

#include "Parsy/BottomUp/CLR1.h"
#include "Parsy/Macros.h"

namespace Parsy
{
    LRParser::LRParser(int32_t flags)
        : Parser(flags)
    {
        
    }

    bool LRParser::Parse()
    {
        ASSERT(m_Lexer != nullptr, "Lexer reference is nullptr!!");
        GenerateParsingData();
        GenerateOutputFiles();

        m_InputStack.Stack.clear();
        m_InputStack.Stack.push_back({ 0, {CFGElementType::Dollar, -1} });
        Lexy::Lexer::Token token;
        std::vector<std::string> labels = { "States", "Stack", "Token", "Action", "String Range"};
        std::vector<std::string> elements;

        Utilities::Time::TimerHandle timer;
        double actionTimer = 0.0f;
        double shiftTimer = 0.0f;
        double reduceTimer = 0.0f;
        double conflictTimer = 0.0f;
        double emptyTimer = 0.0f;

        while (true)
        {
            timer.Start();

            if (!m_KeepToken)
                token = m_Lexer->NextToken();
            timer.End();
            actionTimer += timer.GetTimeElapsed();
            m_KeepToken = false;
            ParseEntryData& topState = m_InputStack.Stack.back();
            CFGElement& tokenElement = GetTokenElement(token);
            SubmitDataToTable(labels, elements, tokenElement);


            BottomUpAction& action = m_LR1->GetAction(topState.State, tokenElement);
            switch (action.Type)
            {
            case BottomUpActionType::Shift:
            {
                timer.Start();
                OnShift(topState.State, action, CFGElementType::Symbol, token);
                timer.End();
                shiftTimer += timer.GetTimeElapsed();
                break;
            }
            case BottomUpActionType::Reduce:
            {
                timer.Start();
                OnReduce(topState.State, action);
                timer.End();
                reduceTimer += timer.GetTimeElapsed();
                break;
            }
            case BottomUpActionType::Accept:
            {
                ExportResult("Accepted", labels, elements);
                PARSY_LOG_INFO("Actions took {}ms", actionTimer);
                PARSY_LOG_INFO("Shifts took {}ms", shiftTimer);
                PARSY_LOG_INFO("Reduces took {}ms", reduceTimer);
                PARSY_LOG_INFO("Conflicts took {}ms", conflictTimer);
                PARSY_LOG_INFO("Empties took {}ms", emptyTimer);
                return true;
            }
            case BottomUpActionType::Conflict:
            {
                timer.Start();
                if (!OnConflict(topState.State, action, token))
                {
                    auto& errorAction = m_LR1->GetAction(topState.State, CFGElement(CFGElementType::Error, -1));
                    if (errorAction.Type == BottomUpActionType::Empty)
                    {
                        ExportResult("Error", labels, elements);
                        SyntaxErrorHandler();
                    }
                    Shift(topState.State, errorAction, CFGElementType::Error, token);
                    m_KeepToken = true;
                }
                timer.End();
                conflictTimer += timer.GetTimeElapsed();
                break;
            }
            case BottomUpActionType::Empty:
            {
                timer.Start();
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
                timer.End();
                emptyTimer += timer.GetTimeElapsed();
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

        size_t stringOffset = 0;
        size_t count = 0;
        for (int32_t i = 0; i < m_Elements; i++)
        {
            if (production.Elements.at(i).Type == CFGElementType::Epsilon) continue;

            ParseEntryData& topEntry = inputStack.back();
            if(i == m_Elements - 1)
                stringOffset = topEntry.Token.StringOffset;
            count += topEntry.Token.StringCount;

            inputStack.pop_back();
        }

        ParseEntryData& topParseEntry = inputStack.back();
        int32_t gotoID = m_LR1->GetGotoState(topParseEntry.State,
            { CFGElementType::NonTerminal, reduceData->RuleID });
        entry.State = gotoID;
        entry.Token = Lexy::Lexer::Token(Lexy::Lexer::TokenState::Success, -1, stringOffset, count);
        inputStack.emplace_back(std::move(entry));
        m_KeepToken = true;

#ifdef COMPILY_DEBUG
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
#endif
        
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

        const BottomUpActionData* reduceActionData = action.GetActionData(BottomUpActionType::Reduce);
        const BottomUpActionData* shiftActionData = action.GetActionData(BottomUpActionType::Shift);

        const TokenProperties& leftProductionData = m_TokenMap.at(lastTerminal->ID);
        PrecedenceData leftPrecedenceData = leftProductionData.Precedence;
        TryGetRulePrecedence(reduceActionData, leftPrecedenceData);

        const TokenProperties& rightProductionData = m_TokenMap.at(tokenElement.ID);
        PrecedenceData rightPrecedenceData = rightProductionData.Precedence;
        if (leftPrecedenceData.Priority < rightPrecedenceData.Priority)
        {
            ASSERT(shiftActionData != nullptr, "There is no shift action!!");
            Shift(state, action, CFGElementType::Symbol, token);
        }
        else if (leftPrecedenceData.Priority > rightPrecedenceData.Priority)
        {
            ASSERT(reduceActionData != nullptr, "There is no reduce action!!");
            Reduce(state, action);
        }
        else
        {
            TieBreakWithAssociativity(leftPrecedenceData, rightPrecedenceData, state, action, action, token);
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
#ifdef COMPILY_DEBUG
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
#endif
    }

    void LRParser::ExportResult(const std::string& message, std::vector<std::string>& labels,
        std::vector<std::string>& elements)
    {
        for (size_t i = 0; i < labels.size(); i++)
        {
            elements.push_back(message);
        }
        ExportParseStack(labels, elements);
    }

    void LRParser::SubmitDataToTable(std::vector<std::string>& labels, std::vector<std::string>& elements,
        const CFGElement& tokenElement)
    {
#ifdef COMPILY_DEBUG
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
#endif
    }

    void LRParser::TryGetRulePrecedence(const BottomUpActionData* reduceActionData, PrecedenceData& precedenceData)
    {
        if (reduceActionData != nullptr)
        {
            const ProductionData& productionData = m_CFGMap.at(reduceActionData->RuleID).Grammar.GetProductions()
                .at(reduceActionData->ReducedProduction);
            const PrecedenceData& preductionPrecedenceData = productionData.Precedence;
            if (preductionPrecedenceData.Associativity != PrecedenceAssociativity::None)
            {
                precedenceData.Priority = preductionPrecedenceData.Priority;
                precedenceData.Associativity = preductionPrecedenceData.Associativity;
            }
        }
    }

    void LRParser::TieBreakWithAssociativity(const PrecedenceData& leftPrecedenceData,
        const PrecedenceData& rightPrecedenceData, int32_t state, const BottomUpAction& reduceAction,
        const BottomUpAction& shiftAction, const Lexy::Lexer::Token& token)
    {
        ASSERT(leftPrecedenceData.Associativity == rightPrecedenceData.Associativity,
            "There is no tie on associativity stage!!");
        switch (leftPrecedenceData.Associativity)
        {
        case PrecedenceAssociativity::Left:
        {
            const BottomUpActionData* reduceActionData = reduceAction.GetActionData(BottomUpActionType::Reduce);
            ASSERT(reduceActionData != nullptr, "There is no reduce action!!");
            Reduce(state, reduceAction);
            break;
        }
        case PrecedenceAssociativity::Right:
        {
            const BottomUpActionData* shiftActionData = shiftAction.GetActionData(BottomUpActionType::Shift);
            ASSERT(shiftActionData != nullptr, "There is no shift action!!");
            Shift(state, shiftAction, CFGElementType::Symbol, token);
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