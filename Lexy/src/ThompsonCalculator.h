#pragma once

#include "NFA.h"

#include <string>
#include <stack>

namespace Lexy
{
	struct RegexElement
	{
		char Character;
		bool IsOperator = false;
	};

	class ThompsonCalculator
	{
	public:
		ThompsonCalculator(const std::string& regexExpression)
			: m_RegexExpression(regexExpression)
		{

		}

		NFA& CalculateNFA();
		
		NFA SymbolToNFA(char symbol);
		NFA ConcatenateNFA();
		NFA UnionNFA();
		NFA KleeneNFA();
	private:
		std::vector<RegexElement> PreProcessRegexExpression();
		void ExecuteOperator(char op);
	private:
		std::string m_RegexExpression;
		NFA m_NFA;
		std::stack<NFA> m_NFAStack;
		std::stack<char> m_OperatorStack;
	};
}