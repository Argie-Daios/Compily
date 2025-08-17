#include "CLRParser.h"

#include "Parsy/BottomUp/CLR1.h"

namespace Parsy
{
	CLRParser::CLRParser(const std::ifstream& inputStream, int32_t flags)
		: LRParser(inputStream, flags)
	{
        m_LR1 = std::make_unique<CLR1>(this);
	}

	bool CLRParser::OnShift(int32_t state, const BottomUpAction& action, const CFGElementType& type,
		const Lexy::Lexer::Token& token)
	{
		const TokenProperties& rightProductionData = m_TokenMap.at(token.TokenID);
		PrecedenceData rightPrecedenceData = rightProductionData.Precedence;

		if (m_Flags & CLRParserFlags_ForcePrecedence && rightPrecedenceData.Priority > 0)
		{
			CFGElement* lastTerminal = GetLastTerminal();
			if (lastTerminal == nullptr) return LRParser::OnShift(state, action, type, token);

			const BottomUpActionData* shiftActionData = action.GetActionData(BottomUpActionType::Shift);

			const TokenProperties& leftProductionData = m_TokenMap.at(lastTerminal->ID);
			PrecedenceData leftPrecedenceData = leftProductionData.Precedence;
			const BottomUpAction* reduceAction = GetReduceAction(state);

			if (reduceAction != nullptr)
			{
				const BottomUpActionData* reduceActionData = reduceAction->GetActionData(BottomUpActionType::Reduce);
				TryGetRulePrecedence(reduceActionData, leftPrecedenceData);
			}

			if (leftPrecedenceData.Priority == 0)
			{
				/*if (reduceAction == nullptr) return LRParser::OnShift(state, action, type, token);
				OnReduce(state, *reduceAction);
				return true;*/
				return LRParser::OnShift(state, action, type, token);
			}
			else if (leftPrecedenceData.Priority > rightPrecedenceData.Priority)
			{
				if (reduceAction == nullptr) return LRParser::OnShift(state, action, type, token);
				OnReduce(state, *reduceAction);
				return true;
			}
			else if (leftPrecedenceData.Priority < rightPrecedenceData.Priority)
			{
				return LRParser::OnShift(state, action, type, token);
			}
			else
			{
				TieBreakWithAssociativity(leftPrecedenceData, rightPrecedenceData, state, *reduceAction, action, token);
				return true;
			}
		}

		return LRParser::OnShift(state, action, type, token);
	}

	bool CLRParser::OnEmpty(int32_t state, const BottomUpAction& action, const Lexy::Lexer::Token& token)
	{
		if (m_Flags & CLRParserFlags_ForceReduce)
		{
			CFGElement* lastTerminal = GetLastTerminal();
			if(lastTerminal == nullptr) return LRParser::OnEmpty(state, action, token);
			const BottomUpAction* reduceAction = GetReduceAction(state);
			if (reduceAction == nullptr) return LRParser::OnEmpty(state, action, token);
			OnReduce(state, *reduceAction);
			return true;
		}

		return LRParser::OnEmpty(state, action, token);
	}

	const BottomUpAction* CLRParser::GetReduceAction(int32_t state)
	{
		BottomUpAction* reduceAction = nullptr;
		const auto& symbols = m_LR1->GetSymbols();
		for (const CFGElement& element : symbols)
		{
			BottomUpAction& action = m_LR1->GetAction(state, element);
			if (action.Type == BottomUpActionType::Reduce)
			{
				reduceAction = &action;
				break;
			}
		}

		return reduceAction;
	}
}