#include <iostream>

#include "NFA.h"
#include "ThompsonCalculator.h"

static Lexy::NFA& CreateRule(const std::string& ruleRegex)
{
	Lexy::ThompsonCalculator calcuator(ruleRegex);
	Lexy::NFA& nfa = calcuator.CalculateNFA();
	return nfa;
}

int main()
{
	std::vector<std::pair<std::string, Lexy::NFA&>> rules = {
		{"Comment", CreateRule("\\/\\/.*")},
		{"Keyword", CreateRule("if|while|for|class|struct")},
		{"Identifier", CreateRule("([A-Z]|[a-z])([A-Z]|[a-z]|[0-9]|\\_)*")},
		{"Double", CreateRule("[0-9]+\\.[0-9]+")},
		{"Integer", CreateRule("[0-9]+")},
	};

	std::cout << "NFA total constructs: " << Lexy::NFA::s_ConstructCount << std::endl;
	std::cout << "NFA total copies: " << Lexy::NFA::s_CopyCount << std::endl;
	std::cout << "Graph total constructs: " << Lexy::FA::s_ConstructCount << std::endl;
	std::cout << "Graph total copies: " << Lexy::FA::s_CopyCount << std::endl;
	std::cout << "Vertex total constructs: " << Lexy::FA::s_VertexConstructCount << std::endl;
	std::cout << "Vertex total copies: " << Lexy::FA::s_VertexCopyCount << std::endl;
	std::cout << "Vertex total moves: " << Lexy::FA::s_VertexMoveCount << std::endl;
	std::cout << "Edge total constructs: " << Lexy::FA::s_EdgeConstructCount << std::endl;
	std::cout << "Edge total copies: " << Lexy::FA::s_EdgeCopyCount << std::endl;
	std::cout << "Edge total moves: " << Lexy::FA::s_EdgeMoveCount << std::endl;

	while (true)
	{
		std::string input;
		std::cout << "Input: ";
		std::getline(std::cin, input);

		std::string result = "None";
		for (auto& [name, rule] : rules)
		{
			if (rule.IsAccepting(input))
			{
				result = name;
				break;
			}
		}

		std::cout << "Matched rule: " << result << std::endl << std::endl;
	}

	return 0;
}