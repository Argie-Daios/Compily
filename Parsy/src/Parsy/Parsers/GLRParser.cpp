#include "GLRParser.h"

namespace Parsy
{
	GLRParser::GLRParser(const std::ifstream& inputStream)
		: Parser(inputStream)
	{
		m_LR1 = std::make_unique<CLR1>(this);
	}

	bool GLRParser::Parse()
	{

		if (m_Lexer == nullptr)
		{
			std::cout << "Null" << std::endl;
			return false;
		}
		m_LR1->GenerateFirstSets();
		m_LR1->GenerateStateGraph();
		m_LR1->GenerateTable();
		Print();

		PushStateOnGSS(0, CFGElement(CFGElementType::Epsilon, -1));
		Lexy::Lexer::Token token;
		while (true)
		{
			CFGElement& tokenElement = GetNextTokenElement();
			for (int32_t i = 0; i < m_GSS.m_CurrentStates.size(); i++)
			{
				int32_t vertexID = m_GSS.m_CurrentStates.at(i);
				ParseEntryData& entryData = m_GSS.m_GSSGraph.GetVertex(vertexID).Data;
				BottomUpAction& action = m_LR1->GetAction(entryData.State, tokenElement);
				switch (action.Type)
				{
				case BottomUpActionType::Shift:
				{
					
					break;
				}
				case BottomUpActionType::Reduce:
				{
					
					break;
				}
				case BottomUpActionType::ShiftReduce:
				{

					break;
				}
				case BottomUpActionType::ReduceReduce:
				{

					break;
				}
				case BottomUpActionType::Empty:
				{

					break;
				}
				case BottomUpActionType::Accept:
				{
					std::cout << "Accepted" << std::endl;
					return true;
				}
				case BottomUpActionType::Error:
				{
					std::cout << "ERROR" << std::endl;
					return false;
				}
				}
			}
		}

		return false;
	}

	std::any& GLRParser::Get(int32_t offset)
	{
		std::any any;
		return any;
	}

	void GLRParser::Shift(const BottomUpAction& action, const Lexy::Lexer::Token& token)
	{
	}

	void GLRParser::Reduce(const BottomUpAction& action)
	{
	}
}