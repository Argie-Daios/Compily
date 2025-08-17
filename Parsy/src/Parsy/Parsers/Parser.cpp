#include "Parser.h"

#include <iostream>

namespace Parsy
{
    Parser::Parser(const std::ifstream& inputStream, int32_t flags)
        : m_LR1(nullptr), m_Flags(flags)
    {
        m_CFGMap.emplace(m_StartingRule, RuleProperties());
        m_CFGMap.at(m_StartingRule).Grammar.AddElement({});
        m_TokenMap.try_emplace(-1);
    }

    void Parser::Print()
    { 
        /*auto& set = m_LR1->GetFirstSet({ CFGElementType::NonTerminal, 0 });
        std::cout << "First Set of Rule(0) { ";
        for (auto& element : set)
        {
            element.Print();
            std::cout << " ";
        }
        std::cout << "}" << std::endl << std::endl;*/

        std::ofstream stream("CFGInfo.txt");
        stream << "[Terminals]" << std::endl;
        for (const CFGElement& element : m_LR1->GetSymbols())
        {
            if (element.Type != CFGElementType::Symbol) continue;
            stream << '\t' << TokenToStr(element.ID) << std::endl;
        }
        stream << std::endl;

        stream << "[Non-Terminals]" << std::endl;
        for (const CFGElement& element : m_LR1->GetNonTerminals())
        {
            if (element.Type != CFGElementType::NonTerminal || element.ID == m_StartingRule) continue;
            stream << '\t' << RuleToStr(element.ID) << std::endl;
        }
        stream << std::endl;

        stream << "[CFG Grammar]" << std::endl;
        for (auto& [ruleID, ruleProps] : m_CFGMap)
        {
            stream << '\t';
            if (ruleID == m_StartingRule)
                stream << "Start";
            else
                stream << RuleToStr(ruleID);
            stream << " --> ";
            auto& productions = ruleProps.Grammar.GetProductions();
            for (int32_t i = 0; i < productions.size(); i++)
            {
                auto& elementList = productions.at(i);
                for (auto& element : elementList.Elements)
                {
                    EntryValue entry;
                    switch (element.Type)
                    {
                    case CFGElementType::Symbol:
                    {
                        stream << TokenToStr(element.ID);
                        m_TokenMap.at(element.ID).TokenTypeConstructCallback(entry);
                        break;
                    }
                    case CFGElementType::NonTerminal:
                    {
                        stream << RuleToStr(element.ID);
                        m_CFGMap.at(element.ID).RuleTypeConstructCallback(entry);
                        break;
                    }
                    case CFGElementType::Epsilon:
                    {
                        stream << "Empty";
                        break;
                    }
                    case CFGElementType::Dollar:
                    {
                        stream << "$";
                        break;
                    }
                    case CFGElementType::Error:
                    {
                        stream << "Error";
                        break;
                    }
                    }
                    stream << " ";
                }
                if(i < productions.size() - 1)
                    stream << " | " << std::endl << "\t\t\t";
            }
            stream << std::endl << std::endl;
        }

        stream << "[Conflicts]" << std::endl;
        for (size_t i = 0; i < m_LR1->m_ActionTable.size(); i++)
        {
            const BottomUpAction& action = m_LR1->m_ActionTable.at(i);
            size_t state = i / m_LR1->GetTotalSymbols();
            if (action.Type != BottomUpActionType::Conflict) continue;
            size_t shiftCount = 0U;
            size_t reduceCount = 0U;
            for (const BottomUpActionData& actionData : action.ActionData)
            {
                switch (actionData.Type)
                {
                case BottomUpActionType::Shift:
                {
                    shiftCount++;
                    break;
                }
                case BottomUpActionType::Reduce:
                {
                    reduceCount++;
                    break;
                }
                }
            }
            stream << '\t' << shiftCount << " Shifts / " << reduceCount << " Reduces" << " on state " << state << std::endl;
        }
    }

    void Parser::DeclareRootRule(RuleID_t rule)
    {
        auto& grammar = m_CFGMap.at(m_StartingRule).Grammar;
        grammar.m_Elements.at(grammar.m_ProductionCount - 1).Elements.at(0).Type = CFGElementType::NonTerminal;
        grammar.m_Elements.at(grammar.m_ProductionCount - 1).Elements.at(0).ID = rule;
    }

    void Parser::BeginRule(RuleID_t rule)
    {
        m_CFGMap.emplace(rule, RuleProperties());
        m_LR1->RegisterNonTerminal(CFGElement(CFGElementType::NonTerminal, rule));
        m_BoundRule = rule;
    }

    void Parser::Add(const CFGElementType& type, int32_t id, const TypeCallback& callback)
    {
        CFGElement element(type, id);
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

    void Parser::Add(const CFGElementType& type, const TypeCallback& callback)
    {
        Add(type, -1, callback);
    }

    void Parser::Prec(Lexy::TokenID_t tokenID)
    {
        RuleProperties& ruleProps = m_CFGMap.at(m_BoundRule);
        ProductionData& productionData = ruleProps.Grammar.m_Elements.back();
        const TokenProperties& tokenProperties = m_TokenMap.at(tokenID);
        productionData.Precedence.Priority = tokenProperties.Precedence.Priority;
        productionData.Precedence.Associativity = tokenProperties.Precedence.Associativity;
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

    const std::string Parser::RuleToStr(RuleID_t ruleID) const
    {
        return std::to_string(ruleID);
    }

    const std::string Parser::TokenToStr(Lexy::TokenID_t tokenID) const
    {
        return std::to_string(tokenID);
    }

    const std::string Parser::CFGElementToStr(const CFGElement& element) const
    {
        switch (element.Type)
        {
        case CFGElementType::Epsilon: return "Epsilon";
        case CFGElementType::Dollar: return "$";
        case CFGElementType::Symbol: return TokenToStr(element.ID);
        case CFGElementType::NonTerminal: return RuleToStr(element.ID);
        case CFGElementType::Error: return "Error";
        }

        return "INVALID_CFG_ELEMENT";
    }

    void Parser::SyntaxErrorHandler()
    {
        std::cout << "Syntax Error" << std::endl;
        exit(1);
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