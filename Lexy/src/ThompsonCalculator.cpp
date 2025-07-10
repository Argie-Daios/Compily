#include "ThompsonCalculator.h"

#include <unordered_map>
#include <functional>
#include <regex>

namespace Lexy
{
	using priority_value_t = int32_t;
	static std::unordered_map<char, priority_value_t> s_Operators = {
		{'|', 0}, 
		{'(', 0},
		{'&', 1},
		{'*', 2},
		{')', 0}
	};

	static std::string expandRange(char start, char end)
	{
		if (start > end)
			std::swap(start, end);

		std::string result = "(";
		for (char c = start; c <= end; ++c)
		{
			if (c != start) result += "|";
			result += c;
		}
		result += ")";
		return result;
	}

	static std::string Replacer(const std::smatch& match)
	{
		std::string matchStr = match.str();
		char start = matchStr[1];
		char end = matchStr[3];

		return expandRange(start, end);
	}

	static std::string ExtractRegexPatterns(const std::string& input, const std::regex& pattern)
	{
		std::string output;
		std::sregex_iterator iter(input.begin(), input.end(), pattern);
		std::sregex_iterator end;

		size_t lastPos = 0;
		while (iter != end)
		{
			std::smatch match = *iter;
			output += input.substr(lastPos, match.position() - lastPos);
			output += Replacer(match);
			lastPos = match.position() + match.length();
			++iter;
		}
		output += input.substr(lastPos);

		return output;
	}

	NFA& ThompsonCalculator::CalculateNFA()
	{
		auto regexSequence = PreProcessRegexExpression();
		for (auto& regexElement : regexSequence)
		{
			if (regexElement.IsOperator)
			{ 
				if (regexElement.Character == ')')
				{
					while (!m_OperatorStack.empty())
					{
						char prevOperator = m_OperatorStack.top();
						if (prevOperator == '(') break;
						m_OperatorStack.pop();
						ExecuteOperator(prevOperator);
					}
					m_OperatorStack.pop();
				}
				else
				{
					while (!m_OperatorStack.empty())
					{
						char prevOperator = m_OperatorStack.top();
						priority_value_t prevOperatorPriority = s_Operators.at(prevOperator);
						priority_value_t currOperatorPriority = s_Operators.at(regexElement.Character);
						if (regexElement.Character == '(' ||
							prevOperatorPriority <= currOperatorPriority)
							break;

						m_OperatorStack.pop();
						ExecuteOperator(prevOperator);
					}
					m_OperatorStack.push(regexElement.Character);
				}
			}
			else
			{
				m_NFAStack.push(SymbolToNFA(regexElement.Character));
			}
		}

		while (!m_OperatorStack.empty())
		{
			char op = m_OperatorStack.top();
			m_OperatorStack.pop();
			ExecuteOperator(op);
		}

		if(!m_NFAStack.empty())
			m_NFA = m_NFAStack.top();
		return m_NFA;
	}

	std::vector<RegexElement> ThompsonCalculator::PreProcessRegexExpression()
	{
		m_RegexExpression = ExtractRegexPatterns(m_RegexExpression, std::regex(R"(\[.\-.\])"));

		std::vector<RegexElement> regexSequence;
		size_t length = m_RegexExpression.length();
		for (size_t i = 0; i < length;)
		{
			const char character = m_RegexExpression[i];
			if (std::isspace(character)) continue;

			auto operatorIterator = s_Operators.find(character);
			if (operatorIterator != s_Operators.end())
			{
				if (character == '(')
				{
					if (!regexSequence.empty() && (!regexSequence.back().IsOperator ||
						regexSequence.back().Character == ')' || regexSequence.back().Character == '*'))
					{
						regexSequence.push_back({ '&', true });
					}
				}
				regexSequence.push_back({character, true});
			}
			else
			{
				if (!regexSequence.empty() && (!regexSequence.back().IsOperator ||
					regexSequence.back().Character == ')' || regexSequence.back().Character == '*'))
				{
					regexSequence.push_back({ '&', true });
				}

				if (character == '\\' && i + 1 < length)
				{
					regexSequence.push_back({ m_RegexExpression[i+1], false});
					i++;
				}
				else
				{
					regexSequence.push_back({ character, false });
				}
			}

			i++;
		}

		for (auto& elem : regexSequence)
		{
			std::cout << "Element: [ character: " << elem.Character << " | IsOperator: " <<
				(elem.IsOperator ? "true" : "false") << " ]" << std::endl;
		}

		return regexSequence;
	}

	void ThompsonCalculator::ExecuteOperator(char op)
	{

		switch (op)
		{
		case '|':
		{
			m_NFAStack.push(UnionNFA());
			break;
		}
		case '&':
		{
			m_NFAStack.push(ConcatenateNFA());
			break;
		}
		case '*':
		{
			m_NFAStack.push(KleeneNFA());
			break;
		}
		}
	}

	NFA ThompsonCalculator::SymbolToNFA(char symbol)
	{
		NFA nfa;

		FA& fa = nfa.GetFiniteAutomate();

		int32_t startState = fa.PushVertex(false);
		nfa.SetStart(startState);

		int32_t acceptingState = fa.PushVertex(false);
		nfa.SetAccepting(acceptingState);

		fa.PushEdge(startState, acceptingState, symbol);

		return nfa;
	}

	NFA ThompsonCalculator::ConcatenateNFA()
	{
		NFA nfaRight = m_NFAStack.top();
		m_NFAStack.pop();
		NFA nfaLeft = m_NFAStack.top();
		m_NFAStack.pop();

		NFA newNFA;
		newNFA = nfaLeft & nfaRight;

		return newNFA;
	}

	NFA ThompsonCalculator::UnionNFA()
	{
		NFA nfaRight = m_NFAStack.top();
		m_NFAStack.pop();
		NFA nfaLeft = m_NFAStack.top();
		m_NFAStack.pop();
		
		NFA newNFA;
		newNFA = nfaLeft | nfaRight;

		return newNFA;
	}

	NFA ThompsonCalculator::KleeneNFA()
	{
		NFA nfa= m_NFAStack.top();
		m_NFAStack.pop();
	
		NFA newNFA;
		newNFA = nfa.Kleene();

		return newNFA;
	}
}