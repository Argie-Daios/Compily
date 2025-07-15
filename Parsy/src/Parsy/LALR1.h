#pragma once

namespace Parsy
{
	// TODO: Make state graph generation algorithm
	// TODO: Make LALR(1) table generation algorithm
	// TODO: Make input parsing algorithm

	class LALR1
	{
	public:
		LALR1(class Parser* parserRef);
	private:
		class Parser* m_ParserRef;
	};
}