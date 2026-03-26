#pragma once

#include "Lexy.h"

#include "Parsy/CFG.h"
#include "Parsy/BottomUp/LR1.h"
#include "Parsy/ParseStructs.h"

#include <memory>
#include <array>
#include <type_traits>

#include <Utilities.h>

namespace Parsy
{
	using RuleID_t = int32_t;
	using TypeCallback = std::function<void(EntryValue&)>;
	using TypeIDCheckCallback = std::function<bool(int32_t id)>;

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
		Parser(int32_t flags = 0);

		template<typename LexerSubclass, typename ... Args>
		void AttachLexer(Args&& ... args)
		{
			static_assert(
				std::is_same<LexerSubclass, Lexy::Lexer>::value ||
				std::is_base_of<Lexy::Lexer, LexerSubclass>::value,
				"AttachLexer requires a type that is Lexy::Lexer or derived from it"
				);
			m_Lexer = new LexerSubclass(std::forward<Args>(args)...);
		}
		inline const Lexy::Lexer* GetLexer() const { return m_Lexer; }
		virtual bool Parse() = 0;
		void GenerateOutputFiles();

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
					m_TokenMap.at(token).Precedence.Priority = m_HighestPriority;
					m_TokenMap.at(token).Precedence.Associativity = associativity;
				}(), ...);
		}

		inline void DeclareTokenIDValidationCheck(const TypeIDCheckCallback& validationCallback)
		{
			m_TokenIDCheckCallback = validationCallback;
		}
		inline void DeclareRuleIDValidationCheck(const TypeIDCheckCallback& validationCallback)
		{
			m_RuleIDCheckCallback = validationCallback;
		}
		void DeclareRootRule(RuleID_t rule);
		inline void EnableDebugTools() { m_IsDebugToolsEnabled = true; }
		void BeginRule(RuleID_t rule);
		template<typename Type, typename ... Args>
		void BeginRule(RuleID_t rule, Args&& ... args)
		{
			BeginRule(rule);

			m_CFGMap.at(rule).RuleTypeConstructCallback =
				[capturedArgs = std::make_tuple(std::forward<Args>(args)...)](EntryValue& entry) mutable {
					std::apply([&](auto&&... unpackedArgs) {
							if constexpr (std::is_pointer<Type>::value)
							{
								using Pointee = std::remove_pointer_t<Type>;
								entry = new Pointee(std::forward<decltype(unpackedArgs)>(unpackedArgs)...);
							}
							else
							{
								entry = Type(std::forward<decltype(unpackedArgs)>(unpackedArgs)...);
							}
						}, capturedArgs);
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
			PrecedenceData Precedence;
			TypeCallback TokenTypeConstructCallback = [](EntryValue& entry) { entry.reset(); };
		};
	private:
		CFGElement GetNextTokenElement();
		CFGElement GetTokenElement(const Lexy::Lexer::Token& token);
		ParseEntryData ConstructEntry(RuleID_t ruleID, int32_t production);
		ParseEntryData InvokeCallbacks(RuleID_t ruleID, int32_t production);
		ParseEntryData ConstructEntryAndInvokeCallbacks(RuleID_t ruleID, int32_t production);
		void GenerateParsingData();
		void GenerateCFGInfo();
		void CheckIDValidation();
		std::string ProductionToStr(const ProductionData& production, int32_t invalidElementIndex = -1) const;
	protected:
		Lexy::Lexer* m_Lexer = nullptr;
	private:
		int32_t m_Flags = 0;
		RuleID_t m_StartingRule = INT_MIN;
		std::unordered_map<RuleID_t, RuleProperties> m_CFGMap;
		std::unordered_map<Lexy::TokenID_t, TokenProperties> m_TokenMap;
		std::unique_ptr<LR1> m_LR1;
		TypeIDCheckCallback m_RuleIDCheckCallback;
		TypeIDCheckCallback m_TokenIDCheckCallback;
		bool m_IsDebugToolsEnabled = false;

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