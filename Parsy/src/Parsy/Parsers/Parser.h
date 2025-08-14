#pragma once

#include "Lexy.h"

#include "Parsy/CFG.h"
#include "Parsy/BottomUp/LR1.h"
#include "Parsy/ParseStructs.h"

#include <memory>
#include <type_traits>

namespace Parsy
{
	using RuleID_t = int32_t;
	using TypeCallback = std::function<void(EntryValue&)>;

	enum ParserFlags
	{
		CLRParserFlags_None = 0,
		CLRParserFlags_ForceReduce = BIT(0),
		// Forces precedence even on empty/non-conflict cells of the parsing table
		CLRParserFlags_ForcePrecedence = BIT(1) | CLRParserFlags_ForceReduce,
		LRParserFlags_IncludeDollarLookAhead = BIT(2)
	};

	class Parser
	{
	public:
		Parser(const std::ifstream& inputStream, int32_t flags = 0);

		virtual bool Parse() = 0;
		void Print();

		template<typename Type>
		void DeclareTokenType(Lexy::TokenID_t token)
		{
			m_TokenMap.try_emplace(token);
			m_TokenMap.at(token).TokenTypeConstructCallback = [](EntryValue& entry) { entry = Type(); };
		}

		template<typename ... Args>
		void DeclarePrecedence(const PrecedenceAssociativity& associativity, Args&& ... token)
		{
			static_assert((std::is_convertible_v<Args, int32_t> && ...),
				"All arguments must be convertible to int32_t");

			++m_HighestPriority;
			([&]()
				{
					m_TokenMap.try_emplace(token);
					m_TokenMap.at(token).Priority = m_HighestPriority;
					m_TokenMap.at(token).Associativity = associativity;
				}(), ...);
		}

		void DeclareRootRule(RuleID_t rule);
		void BeginRule(RuleID_t rule);
		template<typename Type>
		void BeginRule(RuleID_t rule)
		{
			BeginRule(rule);
			if constexpr (std::is_pointer<Type>::value)
			{
				m_CFGMap.at(rule).RuleTypeConstructCallback = [](EntryValue& entry) { entry = Type(); };
			}
			else
			{
				m_CFGMap.at(rule).RuleTypeConstructCallback = [](EntryValue& entry) {
					using PointerType = std::remove_pointer_t<Type>;
					entry = new PointerType();
					};
			}
			m_CFGMap.at(rule).RuleTypeConstructCallback = [](EntryValue& entry)
				{
					if constexpr (std::is_pointer<Type>::value)
					{
						using Pointee = std::remove_pointer_t<Type>;
						entry = new Pointee{};
					}
					else
					{
						entry = Type{};
					}
				};
		}
		void Add(const CFGElementType& type, int32_t id, const TypeCallback& callback = [](EntryValue& entry) {});
		void Add(const CFGElementType& type, const TypeCallback& callback = [](EntryValue& entry) {});
		void Prec(Lexy::TokenID_t tokenID);
		void Union();
		void EndRule();
		template<typename Type>
		Type& Get(int32_t index)
		{
			return std::any_cast<Type&>(GetValue(index));
		}
		template<typename Type>
		Type& Get(EntryValue& entryValue)
		{
			return std::any_cast<Type&>(entryValue);
		}
	protected:
		virtual EntryValue& GetValue(int32_t offset) = 0;
		virtual const std::string RuleToStr(RuleID_t ruleID) const;
		virtual const std::string TokenToStr(Lexy::TokenID_t tokenID) const;
		const std::string CFGElementToStr(const CFGElement& element) const;

		virtual void SyntaxErrorHandler();
	private:
		struct RuleProperties
		{
			CFG Grammar;
			TypeCallback RuleTypeConstructCallback = [](EntryValue& entry) { entry.reset(); };
			std::unordered_map<int32_t, std::vector<TypeCallback>> RuleProductionCallbacks;
		};

		struct TokenProperties
		{
			int32_t Priority = 0;
			PrecedenceAssociativity Associativity = PrecedenceAssociativity::None;
			TypeCallback TokenTypeConstructCallback = [](EntryValue& entry) { entry.reset(); };
		};
	private:
		CFGElement GetNextTokenElement();
		CFGElement GetTokenElement(const Lexy::Lexer::Token& token);
		ParseEntryData ConstructEntry(RuleID_t ruleID, int32_t production);
		ParseEntryData InvokeCallbacks(RuleID_t ruleID, int32_t production);
		ParseEntryData ConstructEntryAndInvokeCallbacks(RuleID_t ruleID, int32_t production);
	protected:
		Lexy::Lexer* m_Lexer = nullptr;
		int32_t m_Flags = 0;

	private:
		RuleID_t m_StartingRule = INT_MIN;
		std::unordered_map<RuleID_t, RuleProperties> m_CFGMap;
		std::unordered_map<Lexy::TokenID_t, TokenProperties> m_TokenMap;
		std::unique_ptr<LR1> m_LR1;

		RuleID_t m_BoundRule = -1;
		int32_t m_HighestPriority = 0;
	
		friend class CLR1;
		friend class LR1;
		friend class LRParser;
		friend class CLRParser;
		friend class SLRParser;
		friend class GLRParser;
	};
}