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
		int32_t rightOperatorPriority = m_TokenMap.at(token.TokenID).Priority;
		PrecedenceAssociativity rightElementAssociativity = m_TokenMap.at(token.TokenID).Associativity;

		if (m_Flags & CLRParserFlags_ForcePrecedence && rightOperatorPriority > 0)
		{
			CFGElement* lastTerminal = GetLastTerminal();
			if (lastTerminal == nullptr) return LRParser::OnShift(state, action, type, token);

			int32_t leftOperatorPriority = m_TokenMap.at(lastTerminal->ID).Priority;
			PrecedenceAssociativity leftElementAssociativity = m_TokenMap.at(lastTerminal->ID).Associativity;

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

			if (reduceAction != nullptr)
			{
				const BottomUpActionData* reduceActionData = reduceAction->GetActionData(BottomUpActionType::Reduce);
				const ProductionData& productionData = m_CFGMap.at(reduceActionData->RuleID).Grammar.GetProductions()
					.at(reduceActionData->ReducedProduction);
				if (productionData.Associativity != PrecedenceAssociativity::None)
				{
					leftElementAssociativity = productionData.Associativity;
					leftOperatorPriority = productionData.Priority;
				}
			}

			if (leftOperatorPriority == 0)
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
				if (dollarAction == nullptr) return LRParser::OnShift(state, action, type, token);
				OnReduce(state, *dollarAction);
				return true;
			}
			else if (leftOperatorPriority > rightOperatorPriority)
			{
				if (reduceAction == nullptr) return LRParser::OnShift(state, action, type, token);
				OnReduce(state, *reduceAction);
				return true;
			}
			else if (leftOperatorPriority < rightOperatorPriority)
			{
				return LRParser::OnShift(state, action, type, token);
			}
			else
			{
				if (leftElementAssociativity != rightElementAssociativity)
				{
					std::cout << "FATAL ERROR" << std::endl;
					return false;
				}

				switch (leftElementAssociativity)
				{
				case PrecedenceAssociativity::Left:
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
					if (dollarAction == nullptr)
					{
						return false;
					}
					OnReduce(state, *dollarAction);
					return true;
				}
				case PrecedenceAssociativity::Right:
				{
					std::cout << "Reudejfmcis" << std::endl;
					return LRParser::OnShift(state, action, type, token);
				}
				case PrecedenceAssociativity::NonAssociate:
				{
					std::cout << "Non-Associate Error" << std::endl;
					return false;
				}
				}
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