#pragma once

#include "FA.h"

#include <string>

#define EPSILON '\0'

namespace Lexy
{
	class NFA : public FA
	{
	public:
		NFA() = default;
		NFA(const NFA& nfa) = default;
		NFA(NFA&& other) noexcept = default;

		virtual bool IsAccepting(const std::string& string) override;
		virtual int Match(const std::string& input, int start) override;

		friend NFA operator|(NFA& nfaLeft, NFA& nfaRight);
		friend NFA operator&(NFA& nfaLeft, NFA& nfaRight);
		NFA& operator|=(NFA& nfa);
		NFA& operator&=(NFA& nfa);
		NFA& Kleene();
		NFA& Plus();
		NFA& QuestionMark();
	private:
		void ExpandEpsilonClosure(std::vector<int32_t>& states);

	};
}