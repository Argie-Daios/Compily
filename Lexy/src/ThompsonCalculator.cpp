#include "ThompsonCalculator.h"

#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <regex>

namespace Lexy
{
	static RegexElementType TryParseOperator(char character)
	{
		switch (character)
		{
		case '|': return RegexElementType::UNION;
		case '&': return RegexElementType::CONCATENATION;
		case '*': return RegexElementType::KLEENE;
		case '+': return RegexElementType::PLUS;
		case '?': return RegexElementType::QUESTIONMARK;
		case '(': return RegexElementType::OPENING_PARENTHESIS;
		case ')': return RegexElementType::CLOSING_PARENTHESIS;
		}

		return RegexElementType::UNDEFINED;
	}

	static bool IsOperator(const RegexElementType& type)
	{
		switch (type)
		{
		case RegexElementType::UNION:
		case RegexElementType::CONCATENATION:
		case RegexElementType::KLEENE:
		case RegexElementType::PLUS:
		case RegexElementType::QUESTIONMARK:
		case RegexElementType::OPENING_PARENTHESIS:
		case RegexElementType::CLOSING_PARENTHESIS: return true;
		}

		return false;
	}

	static int32_t GetOperatorPriority(const RegexElementType& type)
	{
		switch (type)
		{
		case RegexElementType::UNION: return 1;
		case RegexElementType::CONCATENATION: return 2;

		case RegexElementType::KLEENE:
		case RegexElementType::PLUS:
		case RegexElementType::QUESTIONMARK: return 3;
		}

		return 0;
	}

	static bool IsOperatorUnary(const RegexElementType& type)
	{
		switch (type)
		{
		case RegexElementType::KLEENE:
		case RegexElementType::PLUS:
		case RegexElementType::QUESTIONMARK: return true;
		}

		return false;
	}

	static RegexElementType TryParseSymbol(char character)
	{
		switch (character)
		{
		case '\\': return RegexElementType::BACKSLASH;
		case '.': return RegexElementType::DOT;
		}

		return RegexElementType::CHARACTER;
	}

	static bool CanConcatenante(const RegexElement& elementLeft, const RegexElement& elementRight)
	{
		if (elementLeft.Type == RegexElementType::UNDEFINED ||
			elementRight.Type == RegexElementType::UNDEFINED) return false;
		if (TryParseOperator(elementRight.Character) != RegexElementType::UNDEFINED &&
			elementRight.Type != RegexElementType::OPENING_PARENTHESIS) return false;

		bool isLeftElementOperator = TryParseOperator(elementLeft.Character)
			!= RegexElementType::UNDEFINED;
		if (isLeftElementOperator && IsOperatorUnary(elementLeft.Type)) return true;

		return !isLeftElementOperator || elementLeft.Type == RegexElementType::CLOSING_PARENTHESIS;
	}


	std::string ExpandBracketContent(const std::string& content)
	{
		std::string result;
		bool isNegated = !content.empty() && content[0] == '^';
		std::unordered_set<char> negatedChars;

		size_t i = isNegated ? 1 : 0;
		auto parseEscapedChar = [](const std::string& str, size_t& i) -> char {
			if (i >= str.size()) return '\0';
			if (str[i] != '\\') return str[i];

			i++; // skip '\'
			if (i >= str.size()) return '\\';

			switch (str[i]) {
			case 'n': return '\n';
			case 't': return '\t';
			case 'r': return '\r';
			case '\\': return '\\';
			case '"': return '\"';
			case '\'': return '\'';
			default: return str[i]; // unknown escape, return as is
			}
			};

		while (i < content.size())
		{
			char start = parseEscapedChar(content, i);
			i++;
			if (i + 1 < content.size() && content[i] == '-')
			{
				i++;
				char end = parseEscapedChar(content, i);
				i++;

				if (start <= end)
				{
					for (char c = start; c <= end; ++c)
					{
						if (isNegated) negatedChars.insert(c);
						else
						{
							result += c;
							result += '|';
						}
					}
				}
				else
				{
					if (isNegated)
					{
						negatedChars.insert(start);
						negatedChars.insert(end);
					}
					else
					{
						result += start;
						result += '|';
						result += end;
						result += '|';
					}
				}
			}
			else
			{
				if (isNegated)
				{
					negatedChars.insert(start);
				}
				else
				{
					result += start;
					result += '|';
				}
			}
		}

		if (isNegated)
		{
			for (char c = 32; c <= 126; ++c)
			{
				if (negatedChars.find(c) == negatedChars.end())
				{
					if (IsOperator(TryParseOperator(c)))
					{
						result += '\\';
					}
					result += c;
					result += '|';
				}
			}
		}

		if (!result.empty() && result.back() == '|')
			result.pop_back();

		return result;
	}

	std::string ExtractRegexBracketPatterns(const std::string& input) {
		std::regex bracket_pattern(R"(\[([^\]]+)\])");
		std::string output;
		std::sregex_iterator begin(input.begin(), input.end(), bracket_pattern);
		std::sregex_iterator end;

		size_t last_pos = 0;

		for (auto it = begin; it != end; ++it) {
			std::smatch match = *it;
			output += input.substr(last_pos, match.position() - last_pos);

			std::string group = match[1];
			std::string replacement = "(" + ExpandBracketContent(group) + ")";

			output += replacement;
			last_pos = match.position() + match.length();
		}

		output += input.substr(last_pos);

		return output;
	}

	NFA& ThompsonCalculator::CalculateNFA()
	{
		if(m_NFA != nullptr) return *m_NFA;

		auto regexSequence = PreProcessRegexExpression();
		for (auto& regexElement : regexSequence)
		{
			if (IsOperator(regexElement.Type))
			{ 
				if (regexElement.Type == RegexElementType::CLOSING_PARENTHESIS)
				{
					while (!m_OperatorStack.empty())
					{
						RegexElementType prevOperator = m_OperatorStack.back();
						if (prevOperator == RegexElementType::OPENING_PARENTHESIS)
							break;
						m_OperatorStack.pop_back();
						ExecuteOperator(prevOperator);
					}
					m_OperatorStack.pop_back();
				}
				else
				{
					while (!m_OperatorStack.empty())
					{
						RegexElementType prevOperator = m_OperatorStack.back();
						int32_t prevOperatorPriority = GetOperatorPriority(prevOperator);
						int32_t currOperatorPriority = GetOperatorPriority(regexElement.Type);

						if (regexElement.Type == RegexElementType::OPENING_PARENTHESIS)
							break;
						if (prevOperatorPriority < currOperatorPriority)
							break;

						m_OperatorStack.pop_back();
						ExecuteOperator(prevOperator);
					}

					m_OperatorStack.push_back(regexElement.Type);
				}
			}
			else
			{
				SymbolToNFA(regexElement.Character);
			}
		}

		while (!m_OperatorStack.empty())
		{
			RegexElementType op = m_OperatorStack.back();
			m_OperatorStack.pop_back();
			ExecuteOperator(op);
		}

		if(!m_NFAStack.empty())
			m_NFA = &(m_NFAStack.back());

		return *m_NFA;
	}

	void ThompsonCalculator::ChangeRegularExpression(const std::string& regexExpression)
	{
		m_RegexExpression = regexExpression;
		m_NFA = nullptr;
		while (!m_NFAStack.empty()) m_NFAStack.pop_back();
		while (!m_OperatorStack.empty()) m_OperatorStack.pop_back();
	}

	static const char* TypeToStr(const RegexElementType& type)
	{
		switch (type)
		{
		case RegexElementType::UNDEFINED: return "Undefined";
		case RegexElementType::CHARACTER: return "Character";
		case RegexElementType::UNION: return "Union";
		case RegexElementType::CONCATENATION: return "Concatenation";
		case RegexElementType::KLEENE: return "Kleene";
		case RegexElementType::PLUS: return "Plus";
		case RegexElementType::QUESTIONMARK: return "Questionmark";
		case RegexElementType::OPENING_PARENTHESIS: return "Opening Parenthesis";
		case RegexElementType::CLOSING_PARENTHESIS: return "Closing Parenthesis";
		case RegexElementType::BACKSLASH: return "Backslash";
		case RegexElementType::DOT: return "Dot";
		}

		return "NULL";
	}

	std::vector<RegexElement> ThompsonCalculator::PreProcessRegexExpression()
	{
		m_RegexExpression = ExtractRegexBracketPatterns(m_RegexExpression);

		std::vector<RegexElement> regexSequence;
		size_t length = m_RegexExpression.length();
		size_t totalSymbols = 0U;
		for (size_t i = 0; i < length;)
		{
			const char character = m_RegexExpression[i];

			RegexElement element;
			element.Character = character;
			element.Type = TryParseOperator(character);
			if (element.Type != RegexElementType::UNDEFINED)
			{
				element.Priority = GetOperatorPriority(element.Type);
			}
			else
			{
				element.Type = TryParseSymbol(character);
				switch (element.Type)
				{
				case RegexElementType::BACKSLASH:
				{
					if (i + 1 < length)
					{
						element.Character = m_RegexExpression[i + 1];
						i++;
					}
					break;
				}
				case RegexElementType::DOT:
				{
					element.Character = WILDCARD;
					break;
				}
				}

				totalSymbols++;
			}

			if (!regexSequence.empty() && CanConcatenante(regexSequence.back(), element))
			{
				RegexElementType type = RegexElementType::CONCATENATION;
				regexSequence.push_back({ '&', type, GetOperatorPriority(type) });
			}
			regexSequence.push_back(element);
			i++;
		}

		/*for (auto& elem : regexSequence)
		{
			std::cout << "Element: [ character: " << elem.Character << " | Type: " <<
				TypeToStr(elem.Type) << " | Priority: " << elem.Priority << " ]" << std::endl;
		}*/

		m_NFAStack.reserve(totalSymbols);
		return regexSequence;
	}

	void ThompsonCalculator::ExecuteOperator(RegexElementType op)
	{
		switch (op)
		{
		case RegexElementType::UNION:
		{
			UnionNFA();
			break;
		}
		case RegexElementType::CONCATENATION:
		{
			ConcatenateNFA();
			break;
		}
		case RegexElementType::KLEENE:
		{
			KleeneNFA();
			break;
		}
		case RegexElementType::PLUS:
		{
			PlusNFA();
			break;
		}
		case RegexElementType::QUESTIONMARK:
		{
			QuestionMarkNFA();
			break;
		}
		}
	}

	void ThompsonCalculator::SymbolToNFA(char symbol)
	{
		m_NFAStack.emplace_back();
		NFA& nfa = m_NFAStack.back();
		FAGraph& fa = nfa.GetFiniteAutomate();

		int32_t startState = fa.PushVertex(" ");
		nfa.SetStart(startState);

		int32_t acceptingState = fa.PushVertex(" ");
		nfa.SetAccepting(acceptingState);

		fa.PushEdge(startState, acceptingState, symbol);
	}

	void ThompsonCalculator::ConcatenateNFA()
	{
		size_t size = m_NFAStack.size();
		NFA& nfaRight = m_NFAStack.at(size - 1);
		NFA& nfaLeft = m_NFAStack.at(size - 2);

		nfaLeft &= nfaRight;
		m_NFAStack.pop_back();
	}

	void ThompsonCalculator::UnionNFA()
	{
		size_t size = m_NFAStack.size();
		NFA& nfaRight = m_NFAStack.at(size - 1);
		NFA& nfaLeft = m_NFAStack.at(size - 2);
		
		nfaLeft |= nfaRight;
		m_NFAStack.pop_back();
	}

	void ThompsonCalculator::KleeneNFA()
	{
		NFA& nfa = m_NFAStack.back();
		nfa.Kleene();
	}

	void ThompsonCalculator::PlusNFA()
	{
		NFA& nfa = m_NFAStack.back();
		nfa.Plus();
	}

	void ThompsonCalculator::QuestionMarkNFA()
	{
		NFA& nfa = m_NFAStack.back();
		nfa.QuestionMark();
	}
}