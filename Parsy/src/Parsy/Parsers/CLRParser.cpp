#include "CLRParser.h"

#include "Parsy/BottomUp/CLR1.h"

namespace Parsy
{
	CLRParser::CLRParser(const std::ifstream& inputStream, int32_t flags)
		: LRParser(inputStream), m_Flags(flags)
	{
        m_LR1 = std::make_unique<CLR1>(this);
	}

	bool CLRParser::OnShift(int32_t state, const BottomUpAction& action, const Lexy::Lexer::Token& token)
	{
		int32_t rightOperatorPriority = m_TokenMap.at(token.TokenID).Priority;
		if (m_Flags & CLRParserFlags_ForcePrecedence && rightOperatorPriority > 0)
		{
			CFGElement* lastTerminal = GetLastTerminal();
			if (lastTerminal == nullptr) return LRParser::OnShift(state, action, token);
			int32_t leftOperatorPriority = m_TokenMap.at(lastTerminal->ID).Priority;
			if (leftOperatorPriority == 0 || leftOperatorPriority >= rightOperatorPriority)
			{
				BottomUpAction* dollarAction = nullptr;
				const auto& symbols = m_LR1->GetSymbols();
				for (const CFGElement& element : symbols)
				{
					BottomUpAction& action = m_LR1->GetAction(state, element);
					if (action.Type == BottomUpActionType::Reduce)
					{
						dollarAction = &action;
						break;
					}
				}
				if (dollarAction == nullptr) return LRParser::OnShift(state, action, token);
				OnReduce(state, *dollarAction);
				return true;
			}
		}

		return LRParser::OnShift(state, action, token);
	}

	bool CLRParser::OnEmpty(int32_t state, const BottomUpAction& action, const Lexy::Lexer::Token& token)
	{
		if (m_Flags & CLRParserFlags_ForceReduce)
		{
			CFGElement* lastTerminal = GetLastTerminal();
			if(lastTerminal == nullptr) return LRParser::OnEmpty(state, action, token);
			BottomUpAction* dollarAction = nullptr;
			const auto& symbols = m_LR1->GetSymbols();
			for (const CFGElement& element : symbols)
			{
				BottomUpAction& action = m_LR1->GetAction(state, element);
				if (action.Type == BottomUpActionType::Reduce)
				{
					dollarAction = &action;
					break;
				}
			}
			if (dollarAction == nullptr) return LRParser::OnEmpty(state, action, token);
			OnReduce(state, *dollarAction);
			return true;
		}

		return LRParser::OnEmpty(state, action, token);
	}
}