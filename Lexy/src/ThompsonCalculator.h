#pragma once

#include "NFA.h"

#include <string>
#include <stack>

namespace Lexy
{
	enum class RegexElementType
	{
		UNDEFINED,
		CHARACTER,
		UNION,
		CONCATENATION,
		KLEENE,
		PLUS,
		QUESTIONMARK,
		OPENING_PARENTHESIS,
		CLOSING_PARENTHESIS,
		BACKSLASH,
		DOT
	};

	struct RegexElement
	{
		char Character;
		RegexElementType Type = RegexElementType::UNDEFINED;
		int32_t Priority = 0;
	};

	class ThompsonCalculator
	{
	public:
		ThompsonCalculator(const std::string& regexExpression)
			: m_RegexExpression(regexExpression)
		{

		}

		NFA& CalculateNFA();
		void ChangeRegularExpression(const std::string& regexExpression);
	private:
		void SymbolToNFA(char symbol);
		void ConcatenateNFA();
		void UnionNFA();
		void KleeneNFA();
		void PlusNFA();
		void QuestionMarkNFA();

		std::vector<RegexElement> PreProcessRegexExpression();
		void ExecuteOperator(RegexElementType op);
	private:
		std::string m_RegexExpression;
		NFA* m_NFA = nullptr;
		std::vector<NFA> m_NFAStack;
		std::vector<RegexElementType> m_OperatorStack;
	};
}