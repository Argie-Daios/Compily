#pragma once

#include "LRParser.h"
#include <Utilities.h>

namespace Parsy
{ 
	class CLRParser : public LRParser
	{
	public:
		CLRParser(const std::ifstream& inputStream, int32_t flags = CLRParserFlags_None);
	protected:
		virtual bool OnShift(int32_t state, const BottomUpAction& action, const CFGElementType& type,
			const Lexy::Lexer::Token& token) override;
		virtual bool OnEmpty(int32_t state, const BottomUpAction& action,
			const Lexy::Lexer::Token& token) override;
	};
}