#pragma once

#include <Utilities.h>
#include <unordered_set>

namespace Lexy
{
	struct CharacterData
	{
		char Character;
		bool IsEscaped = false;

		CharacterData() = default;
		CharacterData(const CharacterData&) = default;
		CharacterData(CharacterData&&) = default;
		bool operator==(const CharacterData& other) const
		{
			if (this == &other) return true;
			return Character == other.Character && IsEscaped == other.IsEscaped;
		}
	};


	struct CharacterDataHash
	{
		size_t operator()(const CharacterData& data) const
		{
			size_t h1 = std::hash<int32_t>{}(static_cast<int32_t>(data.Character));
			size_t h2 = std::hash<bool>{}(data.IsEscaped);
			return h1 ^ (h2 << 1);
		}
	};

	using FAGraph = Utilities::Graph<bool, std::unordered_set<CharacterData, CharacterDataHash>>;
	class FA
	{
	public:
		FA();
		FA(const FA& fa);
		FA(FA&& other) noexcept;

		virtual bool IsAccepting(const std::string& string) = 0;
		virtual int Match(const std::string& input, int start) = 0;

		inline FAGraph& GetFiniteAutomate() { return m_Graph; }
		inline size_t GetTotalStates() const { return m_Graph.GetTotalVertices(); }

		inline void SetStart(int32_t start) { m_Start = start; }
		inline void SetAccepting(int32_t accepting) { m_Accepting = accepting; }
		inline int32_t GetStart() const { return m_Start; }
		inline int32_t GetAccepting() const { return m_Accepting; }
	protected:
		FAGraph m_Graph;
		int32_t m_Start;
		int32_t m_Accepting;
	public:
		inline static uint32_t s_ConstructCount = 0U;
		inline static uint32_t s_CopyCount = 0U;
		inline static uint32_t s_MoveCount = 0U;
	};
}