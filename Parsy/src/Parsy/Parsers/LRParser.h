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
	protected:
		LRParser(const std::ifstream& inputStream, int32_t flags = 0);

		virtual std::any& GetValue(int32_t offset) override;
		virtual bool OnShift(int32_t state, const BottomUpAction& action, const CFGElementType& type,
			const Lexy::Lexer::Token& token);
		bool Shift(int32_t state, const BottomUpAction& action, const CFGElementType& type,
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
		void ExportParseStack(std::vector<std::string>& labels, std::vector<std::string>& elements);
		void ExportResult(const std::string& message, std::vector<std::string>& labels, std::vector<std::string>& elements);
	private:
		ParseTree m_InputStack;
		std::string m_ActionString;
		int32_t m_Elements = -1;
		bool m_KeepToken = false;
	};
}