#pragma once

#include "Parser.h"

namespace Parsy
{
	using GSSGraph = Utilities::Graph<ParseEntryData, bool>;
	struct GLRGSS
	{
		GSSGraph m_GSSGraph;
		int32_t m_Start;
		std::vector<int32_t> m_AcceptStates;
		std::vector<int32_t> m_CurrentStates;
	};

	class GLRParser : public Parser
	{
	public:
		GLRParser(const std::ifstream& inputStream);

		virtual bool Parse() override;
		virtual std::any& Get(int32_t offset) override;
	private:
		virtual void Shift(const BottomUpAction& action,
			const Lexy::Lexer::Token& token) override;
		virtual void Reduce(const BottomUpAction& action) override;

		/////GSS/////////////////////////////////////////////////////////////////////////////////////

		template<typename ... Args>
		int32_t PushStateOnGSS(Args&& ... args)
		{
			int32_t id = m_GSS.m_GSSGraph.PushVertex(std::forward<Args>(args)...);
			if (m_GSS.m_CurrentStates.empty())
				m_GSS.m_CurrentStates.push_back(id);
			else
			{
				int32_t& currentStateID = m_GSS.m_CurrentStates.back();
				m_GSS.m_GSSGraph.PushEdge(id, currentStateID, false);
				currentStateID = id;
			}
			
			return id;
		}



		/////////////////////////////////////////////////////////////////////////////////////////////
	private:
		GLRGSS m_GSS;
	};
}