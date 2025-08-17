#pragma once

#include "LR1.h"

namespace Parsy
{
	class SLR1 : public LR1
	{
	public:
		SLR1(class Parser* parserRef);
	protected:
		virtual void HandleReduceTable(int32_t state, const BottomUpStateProduction& production) override;
		virtual void GenerateLookAheadSymbols(std::unordered_set<CFGElement>& lookaheadSymbols,
			BottomUpStateProduction& expandedRule) override;
	};
}