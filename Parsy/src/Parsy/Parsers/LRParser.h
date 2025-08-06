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

	class LRParser : public Parser
	{
	public:
		virtual bool Parse() override;
		virtual std::any& Get(int32_t offset) override;
	protected:
		LRParser(const std::ifstream& inputStream);

		virtual bool OnShift(int32_t state, const BottomUpAction& action,
			const Lexy::Lexer::Token& token);
		bool Shift(int32_t state, const BottomUpAction& action,
			const Lexy::Lexer::Token& token);
		virtual bool OnReduce(int32_t state, const BottomUpAction& action);
		bool Reduce(int32_t state, const BottomUpAction& action);
		virtual bool OnConflict(int32_t state, const BottomUpAction& action,
			const Lexy::Lexer::Token& token);
		bool Conflict(int32_t state, const BottomUpAction& action,
			const Lexy::Lexer::Token& token);
		virtual bool OnEmpty(int32_t state, const BottomUpAction& action,
			const Lexy::Lexer::Token& token);
		bool Empty(int32_t state, const BottomUpAction& action,
			const Lexy::Lexer::Token& token);
		CFGElement* GetLastTerminal();
	private:
		ParseTree m_InputStack;
		int32_t m_Elements = -1;
		bool m_KeepToken = false;
	};
}