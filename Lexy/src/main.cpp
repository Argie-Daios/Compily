#include <iostream>

#include "NFA.h"
#include "ThompsonCalculator.h"

int main()
{
	Lexy::ThompsonCalculator calcuator("[a-c]*[1-3]*([d-f]|\\&)");
	Lexy::NFA nfa = calcuator.CalculateNFA();

	/*auto fa = nfa.GetFiniteAutomate();
	auto vertices = fa.GetVertices();
	std::cout << "NFA Starting: [" << nfa.GetStart() << "]" << std::endl;
	std::cout << "NFA Accepting: [" << nfa.GetAccepting() << "]" << std::endl;
	for (auto& [id, vertex] : vertices)
	{
		std::cout << "[" << id << "]" << std::endl;
		auto edges = fa.GetEdgesOfVertex(id);
		for (auto& edge : edges)
		{
			std::string data;
			data += edge.Data;
			std::cout << "\t====> [" << edge.Destination << "] with " <<
				(edge.Data == EPSILON ? "<epsilon>" : data) << std::endl;
		}
	}*/

	std::cout << "abc: " << nfa.IsAccepting("aabbbbccc2132321def") << std::endl;
	std::cout << "bc: " << nfa.IsAccepting("acb123&") << std::endl;

	return 0;
}