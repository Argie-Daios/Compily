#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <algorithm>

#include "RuleTypes/Lvalue.h"

struct SymbolTableEntry
{
	ELvalueType Type = ELvalueType::Modifiable;
	TDataTypeProperties DataType;
	size_t Scope = 0;
	size_t Offset = 0;
	size_t Line = 0;
	bool Hidden = false;

	SymbolTableEntry() = default;
	SymbolTableEntry(size_t scope, size_t line)
		: Scope(scope), Line(line)
	{

	}
	SymbolTableEntry(ELvalueType type, TDataTypeProperties dataType, size_t scope, size_t line)
		: Type(type), DataType(dataType), Scope(scope), Line(line)
	{

	}
	SymbolTableEntry(const SymbolTableEntry&) = default;
	SymbolTableEntry(SymbolTableEntry&&) noexcept = default;
};

class SymbolTable
{
public:
	SymbolTable() = default;

	template<typename ... Args>
	SymbolTableEntry& Emplace(const std::string& id, Args&& ... args)
	{
		m_Entries.try_emplace(id);
		auto& entries = m_Entries.at(id);
		entries.emplace_back(std::forward<Args>(args)...);
		return entries.back();
	}

	std::vector<SymbolTableEntry>& LookUp(const std::string& id)
	{
		auto& entries = m_Entries.at(id);
		return entries;
	}

	SymbolTableEntry* LookUp(const std::string& id, size_t scope)
	{
		auto& entries = LookUp(id);
		auto& it = std::find_if(entries.begin(), entries.end(), [scope](const SymbolTableEntry& entry) {
			return entry.Scope == scope;
		});
		if (it == entries.end()) return nullptr;
		return &(*it);
	}

	std::vector<SymbolTableEntry*> LookUp(size_t scope)
	{
		std::vector<SymbolTableEntry*> sameScopeEntries;
		for (auto& [id, entries] : m_Entries)
		{
			for (SymbolTableEntry& entry : entries)
			{
				if (entry.Scope == scope)
				{
					sameScopeEntries.push_back(&entry);
				}
			}
		}
		return sameScopeEntries;
	}
private:
	std::unordered_map<std::string, std::vector<SymbolTableEntry>> m_Entries;
};