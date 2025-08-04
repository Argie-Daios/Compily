#include "SLR1.h"

namespace Parsy
{
	SLR1::SLR1(Parser* parserRef)
		: LR1(parserRef)
	{

	}

	void SLR1::HandleReduceTable(int32_t state, const BottomUpStateProduction& production)
	{
		auto& followSet = GetFollowSet({ CFGElementType::NonTerminal, production.Rule });
		for (const CFGElement& element : followSet)
		{
			BottomUpAction& action = GetAction(state, element);
			action.Type = BottomUpActionType::Reduce;
			action.ActionData.insert(BottomUpActionData(BottomUpActionType::Reduce,
				production.Rule, production.Production));
			if (action.ActionData.size() > 1)
				action.Type = BottomUpActionType::Conflict;
		}
	}
}