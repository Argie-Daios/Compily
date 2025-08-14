#pragma once

#include <vector>
#include <functional>

namespace Parsy
{
	enum class PrecedenceAssociativity
	{
		None,
		Left,
		Right,
		NonAssociate
	};

	enum class CFGElementType
	{
		Epsilon,
		Dollar,
		Error,
		Symbol,
		NonTerminal
	};

	struct CFGElement
	{
		CFGElementType Type;
		int32_t ID = -1;

		CFGElement() = default;
		CFGElement(const CFGElementType& type, int32_t id) : Type(type), ID(id) {}
		CFGElement(const CFGElement&) = default;
		CFGElement(CFGElement&&) = default;

		void Print() const;

		CFGElement& operator=(const CFGElement& other)
		{
			if (this == &other) return *this;

			Type = other.Type;
			ID = other.ID;

			return *this;
		}

		bool operator==(const CFGElement& other) const
		{
			if (&other == this) return true;

			if (other.Type == this->Type && (other.Type == CFGElementType::Dollar || other.Type == CFGElementType::Error ||
				other.Type == CFGElementType::Epsilon)) return true;

			return other.Type == this->Type && other.ID == this->ID;
		}
	};

	using Production = std::vector<CFGElement>;
	struct ProductionData
	{
		Production Elements;
		int32_t Priority = 0;
		PrecedenceAssociativity Associativity = PrecedenceAssociativity::None;

		ProductionData() = default;
		ProductionData(const ProductionData&) = default;
		ProductionData(ProductionData&&) = default;
	};

	class CFG
	{
	public:
		CFG() = default;
		CFG(const CFG&) = default;
		CFG(CFG&&) = default;

		void AddElement(const CFGElement& element)
		{
			if (m_ProductionCount == 0U)
			{
				Union();
			}
			m_Elements.at(m_ProductionCount - 1).Elements.emplace_back(element);
		}

		void Union()
		{
			m_Elements.emplace_back();
			m_ProductionCount++;
		}

		size_t GetProductionCount() const { return m_ProductionCount; }
		const std::vector<ProductionData>& GetProductions() const { return m_Elements; }
	private:
		std::vector<ProductionData> m_Elements;
		size_t m_ProductionCount = 0U;

		friend class Parser;
	};
}

namespace std
{
	template<>
	struct hash<Parsy::CFGElement>
	{
		std::size_t operator()(const Parsy::CFGElement& elem) const
		{
			std::size_t h1 = std::hash<Parsy::CFGElementType>{}(elem.Type);
			std::size_t h2 = std::hash<int32_t>{}(elem.ID);
			return h1 ^ (h2 << 1);
		}
	};
}