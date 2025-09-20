#pragma once

#include "Parsy.h"
#include "SymbolTable.h"

#include "RuleTypes/Constant.h"
#include "RuleTypes/Lvalue.h"
#include "RuleTypes/Struct.h"
#include "RuleTypes/Expression.h"
#include "QuadGenerator.h"
#include "TypeConvertionManager.h"

enum class EOperationType
{
	ADD, SUB, MUL, DIV
};

class TestParser : public Parsy::CLRParser
{
public:
	TestParser(const std::string& sourceCodePath);
protected:
	virtual const std::string RuleToStr(Parsy::RuleID_t ruleID) const override;
	virtual const std::string TokenToStr(Lexy::TokenID_t tokenID) const override;
	virtual void SyntaxErrorHandler() override;
private:
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool VerifyOperation(const EOperationType& operation, const TExpression& leftExpression,
		const TExpression& rightExpression);
	bool VerifyTypeParity(const TExpression& leftExpression, const TExpression& rightExpression);
	bool VerifyADD(const TExpression& leftExpression, const TExpression& rightExpression);
	bool VerifySUB(const TExpression& leftExpression, const TExpression& rightExpression);
	bool VerifyMUL(const TExpression& leftExpression, const TExpression& rightExpression);
	bool VerifyDIV(const TExpression& leftExpression, const TExpression& rightExpression);
	bool VerifyASSIGN(const TLvalue& lvalue, const TExpression& expression);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const TDataTypeProperties* GetDataTypeProperties(const TExpression& expression);
	template<typename ... Args>
	const std::string CreateTempVariable(Args&& ... args)
	{
		std::string id = "t" + std::to_string(m_TempVariables++);
		m_SymbolTable.Emplace(id, std::forward<Args>(args)...);
		return id;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	Utilities::LoggerInstance m_Logger;
	SymbolTable m_SymbolTable;
	QuadGenerator m_QuadGenerator;
	TypeConvertionManager m_TypeConvertionManager;
	size_t m_Scope = 0U;
	size_t m_TempVariables = 0U;
	friend class TypeConvertionManager;
};