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
		if (m_Flags & CLRParserFlags_ForcePrecedence && rightOperatorPriority > 0)
		{
			CFGElement* lastTerminal = GetLastTerminal();
			if (lastTerminal == nullptr) return LRParser::OnShift(state, action, type, token);
			int32_t leftOperatorPriority = m_TokenMap.at(lastTerminal->ID).Priority;
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
				if (dollarAction == nullptr) return false;
				OnReduce(state, *dollarAction);
				return true;
			}
			else if (leftOperatorPriority < rightOperatorPriority)
			{
				return LRParser::OnShift(state, action, type, token);
			}
			else
			{
				const PrecedenceAssociativity& leftElementAssociativity = m_TokenMap.at(lastTerminal->ID).Associativity;
				const PrecedenceAssociativity& rightElementAssociativity = m_TokenMap.at(token.TokenID).Associativity;
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