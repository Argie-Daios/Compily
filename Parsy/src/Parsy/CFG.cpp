#include "CFG.h"

#include <iostream>

namespace Parsy
{
	void CFGElement::Print() const
	{
        switch (Type)
        {
        case CFGElementType::Symbol:
        {
            std::cout << "Token(" << ID << ")";
            break;
        }
        case CFGElementType::NonTerminal:
        {
            std::cout << "Rule(" << ID << ")";
            break;
        }
        case CFGElementType::Epsilon:
        {
            std::cout << "Empty";
            break;
        }
        case CFGElementType::Dollar:
        {
            std::cout << "Dollar";
        }
        }
	}
}