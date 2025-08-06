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
	using TypeCallback = std::function<void(std::any&)>;

	class Parser
	{
	public:
		Parser(const std::ifstream& inputStream);

		virtual bool Parse() = 0;
		void Print();

		template<typename Type>
		void DeclareTokenType(Lexy::TokenID_t token)
		{
			m_TokenMap.try_emplace(token);
			m_TokenMap.at(token).TokenTypeConstructCallback = [](std::any& any) { any = Type(); };
		}

		template<typename ... Args>
		void DeclarePrecedence(Args&& ... token)
		{
			static_assert((std::is_convertible_v<Args, int32_t> && ...),
				"All arguments must be convertible to int32_t");

			++m_HighestPriority;
			([&]()
				{
					m_TokenMap.try_emplace(token);
					m_TokenMap.at(token).Priority = m_HighestPriority;
				}(), ...);
		}

		void StartRule(RuleID_t rule);
		void BeginRule(RuleID_t rule);
		template<typename Type>
		void BeginRule(RuleID_t rule)
		{
			BeginRule(rule);
			if constexpr (std::is_pointer<Type>::value)
			{
				m_CFGMap.at(rule).RuleTypeConstructCallback = [](std::any& any) { any = Type(); };
			}
			else
			{
				m_CFGMap.at(rule).RuleTypeConstructCallback = [](std::any& any) { 
					using PointerType = std::remove_pointer_t<Type>;
					any = new PointerType();
					};
			}
			m_CFGMap.at(rule).RuleTypeConstructCallback = [](std::any& any)
				{
					if constexpr (std::is_pointer<Type>::value)
					{
						using Pointee = std::remove_pointer_t<Type>;
						any = new Pointee{};
					}
					else
					{
						any = Type{};
					}
				};
		}
		void Add(const CFGElementType& type, int32_t id, const TypeCallback& callback = [](std::any& any) {});
		void Union();
		virtual std::any& Get(int32_t offset) = 0;
		void EndRule();
	protected:
		virtual const std::string RuleToStr(RuleID_t ruleID);
		virtual const std::string TokenToStr(Lexy::TokenID_t tokenID);
	private:
		struct RuleProperties
		{
			CFG Grammar;
			TypeCallback RuleTypeConstructCallback = [](std::any& any) { any.reset(); };
			std::unordered_map<int32_t, std::vector<TypeCallback>> RuleProductionCallbacks;
		};

		struct TokenProperties
		{
			int32_t Priority = 0;
			TypeCallback TokenTypeConstructCallback = [](std::any& any) { any.reset(); };
		};
	private:
		CFGElement GetNextTokenElement();
		CFGElement GetTokenElement(const Lexy::Lexer::Token& token);
		ParseEntryData ConstructEntry(RuleID_t ruleID, int32_t production);
		ParseEntryData InvokeCallbacks(RuleID_t ruleID, int32_t production);
		ParseEntryData ConstructEntryAndInvokeCallbacks(RuleID_t ruleID, int32_t production);
	protected:
		Lexy::Lexer* m_Lexer = nullptr;

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