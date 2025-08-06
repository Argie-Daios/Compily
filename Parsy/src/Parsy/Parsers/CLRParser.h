#pragma once

#include "LRParser.h"
#include <Utilities.h>

namespace Parsy
{ 
	enum CLRParserFlags
	{
		CLRParserFlags_None = 0,
		CLRParserFlags_ForceReduce = BIT(0),
		// Forces precedence even on empty/non-conflict cells of the parsing table
		CLRParserFlags_ForcePrecedence = BIT(1) | CLRParserFlags_ForceReduce
	};

	class CLRParser : public LRParser
	{
	public:
		CLRParser(const std::ifstream& inputStream, int32_t flags = CLRParserFlags_None);
	protected:
		virtual bool OnShift(int32_t state, const BottomUpAction& action,
			const Lexy::Lexer::Token& token) override;
		virtual bool OnEmpty(int32_t state, const BottomUpAction& action,
			const Lexy::Lexer::Token& token) override;
	private:
		int32_t m_Flags;
	};
}