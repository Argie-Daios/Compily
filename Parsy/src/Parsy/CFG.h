#pragma once

#include <vector>
#include <functional>

namespace Parsy
{
	enum class CFGElementType
	{
		Epsilon,
		Dollar,
		Symbol,
		NonTerminal
	};

#ifdef PARSY_CFG_OPTIMIZED
	struct ListBatchEntry
	{
		ListBatchEntry() = default;
		ListBatchEntry(size_t batchIndex, size_t stride) : BatchIndex(batchIndex), Stride(stride) { }
		ListBatchEntry(const ListBatchEntry&) = default;
		ListBatchEntry(ListBatchEntry&&) = default;
	protected:
		size_t BatchIndex;
		size_t Stride;
		friend class CFG;
	};

	struct CFGElement : public ListBatchEntry
	{
		CFGElementType Type;
		int32_t ID;

		CFGElement() = default;
		CFGElement(size_t batchIndex, const CFGElementType& type, int32_t id)
			: ListBatchEntry(batchIndex, 0), Type(type), ID(id) {}
		CFGElement(const CFGElementType& type, int32_t id) : Type(type), ID(id) {}
		CFGElement(const CFGElement&) = default;
		CFGElement(CFGElement&&) = default;
	};
	
	class VectorMask
	{
	public:
		VectorMask(std::vector<CFGElement>* vector, size_t start, size_t count)
			: m_Vector(vector), m_Start(start), m_Count(count)
		{
			
		}

		VectorMask(const VectorMask& other)
			: m_Vector(other.m_Vector), m_Start(other.m_Start), m_Count(other.m_Count)
		{

		}

		VectorMask(VectorMask&& other)
			: m_Vector(std::move(other.m_Vector)),
			m_Start(std::move(other.m_Start)), m_Count(std::move(other.m_Count))
		{
			other.m_Vector = nullptr;
		}

		std::vector<CFGElement>::iterator begin() { return m_Vector->begin() + m_Start; }
		std::vector<CFGElement>::iterator end() { return m_Vector->begin() + m_Start + m_Count; }
	private:
		std::vector<CFGElement>* m_Vector;
		size_t m_Start = -1;
		size_t m_Count = -1;
	};

	class CFG
	{
	public:
		CFG() = default;
		CFG(const CFG&) = default;
		CFG(CFG&&) = default;
		
		void AddElement(size_t batchIndex, const CFGElementType& type, int32_t id)
		{
			for (auto it = m_Elements.rbegin(); it != m_Elements.rend(); it++)
			{
				CFGElement& element = *it;
			}

			m_Elements.emplace_back(batchIndex, type, id);
		}

	private:
		std::vector<CFGElement> m_Elements;
	};
#else
	struct CFGElement
	{
		CFGElementType Type;
		int32_t ID = -1;

		CFGElement() = default;
		CFGElement(const CFGElementType& type, int32_t id) : Type(type), ID(id) {}
		CFGElement(const CFGElement&) = default;
		CFGElement(CFGElement&&) = default;

		void Print() const;

		bool operator==(const CFGElement& other) const
		{
			if (&other == this) return true;

			return other.Type == this->Type && other.ID == this->ID;
		}
	};

	using Production = std::vector<CFGElement>;
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
			m_Elements.at(m_ProductionCount - 1).emplace_back(element);
		}

		void Union()
		{
			m_Elements.emplace_back();
			m_ProductionCount++;
		}

		size_t GetProductionCount() const { return m_ProductionCount; }
		const std::vector<Production>& GetProductions() const { return m_Elements; }
	private:
		std::vector<Production> m_Elements;
		size_t m_ProductionCount = 0U;

		friend class Parser;
	};
#endif
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