#pragma once

#include "Parser.h"

namespace Parsy
{
	struct ParseTree
	{
		std::vector<ParseEntryData> Stack;
		bool IsDead = false;

		ParseTree() = default;
	};

	class CLRParser : public Parser
	{
	public:
		CLRParser(const std::ifstream& inputStream);

		virtual bool Parse() override;
		virtual std::any& Get(int32_t offset) override;
	private:
		virtual void Shift(const BottomUpAction& action,
			const Lexy::Lexer::Token& token) override;
		virtual void Reduce(const BottomUpAction& action) override;
	private:
		ParseTree m_InputStack;
		int32_t m_Elements = -1;
	};
}