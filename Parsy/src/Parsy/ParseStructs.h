#pragma once

#include "CFG.h"

#include <any>

namespace Parsy
{
	using EntryValue = std::any;
	struct ParseEntryData
	{
		int32_t State;
		CFGElement Symbol;
		Lexy::Lexer::Token Token;
		EntryValue Entry;
		bool IsAccept = false;

		ParseEntryData() = default;
		ParseEntryData(int32_t state)
			: State(state) {}
		ParseEntryData(int32_t state, CFGElement symbol)
			: State(state), Symbol(symbol) {}
		ParseEntryData(const ParseEntryData&) = default;
		ParseEntryData(ParseEntryData&&) = default;
		ParseEntryData& operator=(const ParseEntryData& other)
		{
			if (this == &other) return *this;

			State = other.State;
			Symbol = other.Symbol;
			Entry = other.Entry;
			IsAccept = other.IsAccept;

			return *this;
		}
	};
}