#include "TestParser.h"

#include "TestLexer.h"
#include "RuleAndTokenIDS.h"

TestParser::TestParser(const std::string& sourceCodePath)
	: CLRParser(Parsy::CLRParserFlags_ForcePrecedence)
{
	AttachLexer<TestLexer>(sourceCodePath);
	m_Logger = Utilities::Logger::Register("TestParser", sourceCodePath);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	EnableDebugTools();
	DeclareTokenIDValidationCheck([](int32_t id) { return id >= 0 && id < TOKENID_MAX; });
	DeclareRuleIDValidationCheck([](int32_t id) { return id >= TOKENID_MAX && id < RULEID_MAX; });

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	DeclarePrecedence(Parsy::PrecedenceAssociativity::Left, PLUS, DASH);
	DeclarePrecedence(Parsy::PrecedenceAssociativity::Left, STAR, SLASH);
	DeclarePrecedence(Parsy::PrecedenceAssociativity::Right, UMINUS);
	DeclareRootRule(PROGRAM);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(PROGRAM);

		Add(Parsy::CFGElementType::NonTerminal, STATEMENTS);

		Union();

		Add(Parsy::CFGElementType::Epsilon);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(STATEMENTS);

		Add(Parsy::CFGElementType::NonTerminal, STATEMENT);
		Add(Parsy::CFGElementType::NonTerminal, STATEMENTS);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, STATEMENT);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(STATEMENT);

		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, SEMICOLON);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, STRUCT_DEFINITION);
		Add(Parsy::CFGElementType::Symbol, SEMICOLON);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, IF_STATEMENT);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, WHILE_STATEMENT);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, FOR_STATEMENT);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, FUNCTION_DEFINITION);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, ASSIGNMENT);
		Add(Parsy::CFGElementType::Symbol, SEMICOLON);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, LVALUE_DECLARATION_LIST);
		Add(Parsy::CFGElementType::Symbol, SEMICOLON, [this](Parsy::EntryValue& entry) {
			std::vector<TLvalue>& lvalueDeclarationList = Get<std::vector<TLvalue>>(0);
			for (TLvalue& lvalue : lvalueDeclarationList)
			{
				SymbolTableEntry* symEntry = m_SymbolTable.LookUp(lvalue.SymbolTableEntry, 0);
				std::cout << "Lvalue[" << lvalue.SymbolTableEntry << "] type: ";
				switch (lvalue.DataType.Type)
				{
				case EDataType::None:
				{
					std::cout << "None";
					break;
				}
				case EDataType::Int16:
				{
					std::cout << "Int16";
					for (int32_t i = 0; i < lvalue.DataType.PointerDepth; i++)
					{
						std::cout << '*';
					}
					if (std::holds_alternative<int16_t>(symEntry->ConstantValue.Data))
					{
						int16_t& value = std::get<int16_t>(symEntry->ConstantValue.Data);
						std::cout << " with value " << value;
					}
					break;
				}
				case EDataType::Int32:
				{
					std::cout << "Int32";
					for (int32_t i = 0; i < lvalue.DataType.PointerDepth; i++)
					{
						std::cout << '*';
					}
					if (std::holds_alternative<int32_t>(symEntry->ConstantValue.Data))
					{
						int32_t& value = std::get<int32_t>(symEntry->ConstantValue.Data);
						std::cout << " with value " << value;
					}
					break;
				}
				case EDataType::Int64:
				{
					std::cout << "Int64";
					for (int32_t i = 0; i < lvalue.DataType.PointerDepth; i++)
					{
						std::cout << '*';
					}
					if (std::holds_alternative<int64_t>(symEntry->ConstantValue.Data))
					{
						int64_t& value = std::get<int64_t>(symEntry->ConstantValue.Data);
						std::cout << " with value " << value;
					}
					break;
				}
				case EDataType::Bool:
				{
					std::cout << "Bool";
					for (int32_t i = 0; i < lvalue.DataType.PointerDepth; i++)
					{
						std::cout << '*';
					}
					if (std::holds_alternative<bool>(symEntry->ConstantValue.Data))
					{
						bool& value = std::get<bool>(symEntry->ConstantValue.Data);
						std::cout << " with value " << (value ? "true" : "false");
					}
					break;
				}
				case EDataType::Float:
				{
					std::cout << "Float";
					for (int32_t i = 0; i < lvalue.DataType.PointerDepth; i++)
					{
						std::cout << '*';
					}
					if (std::holds_alternative<float>(symEntry->ConstantValue.Data))
					{
						float& value = std::get<float>(symEntry->ConstantValue.Data);
						std::cout << " with value " << value;
					}
					break;
				}
				case EDataType::Double:
				{
					std::cout << "Double";
					for (int32_t i = 0; i < lvalue.DataType.PointerDepth; i++)
					{
						std::cout << '*';
					}
					if (std::holds_alternative<double>(symEntry->ConstantValue.Data))
					{
						double& value = std::get<double>(symEntry->ConstantValue.Data);
						std::cout << " with value " << value;
					}
					break;
				}
				case EDataType::Char:
				{
					std::cout << "Character";
					for (int32_t i = 0; i < lvalue.DataType.PointerDepth; i++)
					{
						std::cout << '*';
					}

					if (lvalue.DataType.PointerDepth == 1)
					{
						if (std::holds_alternative<std::string>(symEntry->ConstantValue.Data))
						{
							std::string& value = std::get<std::string>(symEntry->ConstantValue.Data);
							std::cout << " with value " << value;
						}
					}
					else if (lvalue.DataType.PointerDepth == 0)
					{
						if (std::holds_alternative<char>(symEntry->ConstantValue.Data))
						{
							char& value = std::get<char>(symEntry->ConstantValue.Data);
							std::cout << " with value " << value;
						}
					}
					break;
				}
				case EDataType::Struct:
				{
					std::cout << "Struct";
					for (int32_t i = 0; i < lvalue.DataType.PointerDepth; i++)
					{
						std::cout << '*';
					}
					/*if (std::holds_alternative<int16_t>(symEntry->ConstantValue.Data))
					{
						int16_t& value = std::get<int16_t>(symEntry->ConstantValue.Data);
						std::cout << " with value " << value;
					}*/
					break;
				}
				}
				std::cout << std::endl;
			}
		});

		Union();

		Add(Parsy::CFGElementType::Symbol, SEMICOLON);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(OPTIONAL_STATEMENT);

		Add(Parsy::CFGElementType::NonTerminal, STATEMENT);

		Union();

		Add(Parsy::CFGElementType::Epsilon);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(STATEMENT_LIST);

		Add(Parsy::CFGElementType::NonTerminal, STATEMENTS);

		Union();

		Add(Parsy::CFGElementType::Epsilon);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TExpression>(EXPRESSION);

		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, PLUS);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION, [this](Parsy::EntryValue& entry) {
			TExpression& entryValue = Get<TExpression>(entry);
			TExpression& leftExpressionValue = Get<TExpression>(0);
			TExpression& rightExpressionValue = Get<TExpression>(2);

			TConstant& leftConstant = GetConstantValueFromExpression(leftExpressionValue);
			TConstant& rightConstant = GetConstantValueFromExpression(rightExpressionValue);

			if (leftConstant.DataTypeProps != rightConstant.DataTypeProps)
			{
				m_Logger.Error("Cannot add different type values, on line {}!!",
					GetLexer()->GetLineCount());
				return;
			}

			entryValue.Type = EExpressionType::Constant;
			entryValue.ConstantValue = ExecuteOperation(EOperationType::Plus, leftConstant, rightConstant);
		});

		Union();

		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, DASH);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, STAR);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, SLASH);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, CARET);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, EQUALS_EQUALS);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, EXCLAMATION_EQUALS);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, LESS);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, LESS_EQUALS);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, GREATER);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, GREATER_EQUALS);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, TERMINAL, [this](Parsy::EntryValue& entry) {
			TExpression& entryValue = Get<TExpression>(entry);
			TExpression& termValue = Get<TExpression>(0);
			entryValue = termValue;
		});

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(OPTIONAL_EXPRESSION);

		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);

		Union();

		Add(Parsy::CFGElementType::Epsilon);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(EXPRESSION_LIST);

		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION_LIST_NEXT);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(EXPRESSION_LIST_NEXT);

		Add(Parsy::CFGElementType::Symbol, COMMA);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION_LIST_NEXT);

		Union();

		Add(Parsy::CFGElementType::Epsilon);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(ASSIGNMENT);

		Add(Parsy::CFGElementType::NonTerminal, LVALUE);
		Add(Parsy::CFGElementType::Symbol, EQUALS);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TExpression>(TERMINAL);

		Add(Parsy::CFGElementType::Symbol, OPENING_PARENTHESIS);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, CLOSING_PARENTHESIS);

		Union();

		Add(Parsy::CFGElementType::Symbol, DASH);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Prec(UMINUS);

		Union();

		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);
		Add(Parsy::CFGElementType::Symbol, PLUS_PLUS);

		Union();

		Add(Parsy::CFGElementType::Symbol, PLUS_PLUS);
		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);

		Union();

		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);
		Add(Parsy::CFGElementType::Symbol, DASH_DASH);

		Union();

		Add(Parsy::CFGElementType::Symbol, DASH_DASH);
		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, LVALUE, [this](Parsy::EntryValue& entry) {
			TExpression& entryValue = Get<TExpression>(entry);
			TLvalue& lvalueValue = Get<TLvalue>(0);
			entryValue.Type = EExpressionType::Lvalue;
			entryValue.SymbolTableEntry = lvalueValue.SymbolTableEntry;
		});

		Union();
	
		Add(Parsy::CFGElementType::NonTerminal, CONSTANT, [this](Parsy::EntryValue& entry) {
			TExpression& entryValue = Get<TExpression>(entry);
			TConstant& constantValue = Get<TConstant>(0);
			entryValue.Type = EExpressionType::Constant;
			entryValue.ConstantValue = constantValue;
		});

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TDataTypeProperties>(TYPE_DEFINITION);

		Add(Parsy::CFGElementType::Symbol, INTEGER_TYPE, [this](Parsy::EntryValue& entry) {
			TDataTypeProperties& entryValue = Get<TDataTypeProperties>(entry);
			entryValue.Type = EDataType::Int32;
		});

		Union();

		Add(Parsy::CFGElementType::Symbol, BOOL_TYPE, [this](Parsy::EntryValue& entry) {
			TDataTypeProperties& entryValue = Get<TDataTypeProperties>(entry);
			entryValue.Type = EDataType::Bool;
		});

		Union();

		Add(Parsy::CFGElementType::Symbol, FLOAT_TYPE, [this](Parsy::EntryValue& entry) {
			TDataTypeProperties& entryValue = Get<TDataTypeProperties>(entry);
			entryValue.Type = EDataType::Float;
			});

		Union();

		Add(Parsy::CFGElementType::Symbol, DOUBLE_TYPE, [this](Parsy::EntryValue& entry) {
			TDataTypeProperties& entryValue = Get<TDataTypeProperties>(entry);
			entryValue.Type = EDataType::Double;
			});

		Union();

		Add(Parsy::CFGElementType::Symbol, CHARACTER_TYPE, [this](Parsy::EntryValue& entry) {
			TDataTypeProperties& entryValue = Get<TDataTypeProperties>(entry);
			entryValue.Type = EDataType::Char;
			});

		Union();

		Add(Parsy::CFGElementType::Symbol, SHORT_TYPE, [this](Parsy::EntryValue& entry) {
			TDataTypeProperties& entryValue = Get<TDataTypeProperties>(entry);
			entryValue.Type = EDataType::Int16;
			});

		Union();

		Add(Parsy::CFGElementType::Symbol, LONG_TYPE, [this](Parsy::EntryValue& entry) {
			TDataTypeProperties& entryValue = Get<TDataTypeProperties>(entry);
			entryValue.Type = EDataType::Int64;
			});

		Union();

		Add(Parsy::CFGElementType::Symbol, IDENTIFIER, [this](Parsy::EntryValue& entry) {
			TDataTypeProperties& entryValue = Get<TDataTypeProperties>(entry);
			entryValue.Type = EDataType::Struct;
		});

		Union();

		Add(Parsy::CFGElementType::NonTerminal, TYPE_DEFINITION);
		Add(Parsy::CFGElementType::Symbol, STAR, [this](Parsy::EntryValue& entry) {
			TDataTypeProperties& entryValue = Get<TDataTypeProperties>(entry);
			TDataTypeProperties& dataTypeValue = Get<TDataTypeProperties>(0);
			dataTypeValue.PointerDepth++;
			entryValue = dataTypeValue;
		});

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TLvalue>(LVALUE_DECLARATION);

		Add(Parsy::CFGElementType::NonTerminal, TYPE_DEFINITION);
		Add(Parsy::CFGElementType::NonTerminal, LVALUE_DECLARATION_FORMAT, [this](Parsy::EntryValue& entry) {
			TLvalue& entryValue = Get<TLvalue>(entry);
			TDataTypeProperties& dataProperties = Get<TDataTypeProperties>(0);
			TLvalue& lvalueDeclaration = Get<TLvalue>(1);

			SymbolTableEntry* symEntry = m_SymbolTable.LookUp(lvalueDeclaration.SymbolTableEntry, 0);
			if (symEntry->ConstantValue.DataTypeProps.Type != EDataType::None && 
				(symEntry->ConstantValue.DataTypeProps.Type != dataProperties.Type ||
				symEntry->ConstantValue.DataTypeProps.PointerDepth != dataProperties.PointerDepth))
			{
				m_Logger.Error("Type mismatch between assigned value and type declaration on line {}!!",
					GetLexer()->GetLineCount());
			}
			else
			{
				lvalueDeclaration.DataType = dataProperties;
			}
			entryValue = lvalueDeclaration;
		});

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TLvalue>(LVALUE_DECLARATION_FORMAT);

		Add(Parsy::CFGElementType::Symbol, IDENTIFIER, [this](Parsy::EntryValue& entry) {
			TLvalue& entryValue = Get<TLvalue>(entry);
			entryValue.SymbolTableEntry = Get<std::string>(0);
			entryValue.Type = ELvalueType::Const;
			m_SymbolTable.Emplace(entryValue.SymbolTableEntry);
		});

		Union();

		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);
		Add(Parsy::CFGElementType::Symbol, EQUALS);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION, [this](Parsy::EntryValue& entry) {
			TLvalue& entryValue = Get<TLvalue>(entry);
			entryValue.SymbolTableEntry = Get<std::string>(0);
			entryValue.Type = ELvalueType::Const;
			m_SymbolTable.Emplace(entryValue.SymbolTableEntry, 0, 0);
			SymbolTableEntry* symEntry = m_SymbolTable.LookUp(entryValue.SymbolTableEntry, 0);

			TExpression& expressionValue = Get<TExpression>(2);
			switch (expressionValue.Type)
			{
			case EExpressionType::Constant:
			{
				symEntry->ConstantValue = expressionValue.ConstantValue;
				break;
			}
			case EExpressionType::Lvalue:
			{
				SymbolTableEntry* expressionSymEntry = m_SymbolTable.LookUp(expressionValue.SymbolTableEntry, 0);
				symEntry->ConstantValue = expressionSymEntry->ConstantValue;
				break;
			}
			}
		});

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TLvalue>(LVALUE);

		Add(Parsy::CFGElementType::Symbol, IDENTIFIER, [this](Parsy::EntryValue& entry) {
			TLvalue& entryValue = Get<TLvalue>(entry);
			entryValue.SymbolTableEntry = Get<std::string>(0);
			entryValue.Type = ELvalueType::Const;
			//m_SymbolTable.LookUp(entryValue.SymbolTableEntry, 0);
		});

		Union();

		Add(Parsy::CFGElementType::Symbol, STAR);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);

		Union();

		Add(Parsy::CFGElementType::Symbol, AMPERSAND);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, STRUCT_MEMBER_REFERENCE);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, FUNCTION_CALL);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<std::vector<TLvalue>>(LVALUE_DECLARATION_LIST);

		Add(Parsy::CFGElementType::NonTerminal, LVALUE_DECLARATION);
		Add(Parsy::CFGElementType::NonTerminal, LVALUE_LIST_NEXT, [this](Parsy::EntryValue& entry) {
			std::vector<TLvalue>& entryValue = Get<std::vector<TLvalue>>(entry);
			TLvalue& lvalueDeclarationValue = Get<TLvalue>(0);
			std::vector<TLvalue>& lvalueNextValue = Get<std::vector<TLvalue>>(1);

			entryValue.push_back(lvalueDeclarationValue);
			TDataTypeProperties currentType = lvalueDeclarationValue.DataType;
			for (TLvalue& lvalue : lvalueNextValue)
			{
				SymbolTableEntry* symEntry = m_SymbolTable.LookUp(lvalue.SymbolTableEntry, 0);
				if (lvalue.DataType.Type != EDataType::None && (lvalue.DataType.Type != currentType.Type ||
					lvalue.DataType.PointerDepth != currentType.PointerDepth))
				{
					currentType = lvalue.DataType;
				}

				if (symEntry->ConstantValue.DataTypeProps.Type != EDataType::None &&
					(symEntry->ConstantValue.DataTypeProps.Type != currentType.Type ||
						symEntry->ConstantValue.DataTypeProps.PointerDepth != currentType.PointerDepth))
				{
					m_Logger.Error("Type mismatch between assigned value and type declaration on line {}!!",
						GetLexer()->GetLineCount());
				}
				else if (lvalue.DataType.Type == EDataType::None)
				{
					lvalue.DataType = currentType;
				}
				entryValue.push_back(lvalue);
			}
		});

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<std::vector<TLvalue>>(LVALUE_LIST_NEXT);

		Add(Parsy::CFGElementType::Symbol, COMMA);
		Add(Parsy::CFGElementType::NonTerminal, LVALUE_DECLARATION);
		Add(Parsy::CFGElementType::NonTerminal, LVALUE_LIST_NEXT, [this](Parsy::EntryValue& entry) {
			std::vector<TLvalue>& entryValue = Get<std::vector<TLvalue>>(entry);
			TLvalue& lvalueDeclarationValue = Get<TLvalue>(1);
			std::vector<TLvalue>& lvalueNextValue = Get<std::vector<TLvalue>>(2);

			entryValue.push_back(lvalueDeclarationValue);
			TDataTypeProperties currentType = lvalueDeclarationValue.DataType;
			for (TLvalue& lvalue : lvalueNextValue)
			{
				SymbolTableEntry* symEntry = m_SymbolTable.LookUp(lvalue.SymbolTableEntry, 0);
				if (lvalue.DataType.Type != EDataType::None && (lvalue.DataType.Type != currentType.Type ||
					lvalue.DataType.PointerDepth != currentType.PointerDepth))
				{
					currentType = lvalue.DataType;
				}

				if (symEntry->ConstantValue.DataTypeProps.Type != EDataType::None &&
					(symEntry->ConstantValue.DataTypeProps.Type != currentType.Type ||
						symEntry->ConstantValue.DataTypeProps.PointerDepth != currentType.PointerDepth))
				{
					m_Logger.Error("Type mismatch between assigned value and type declaration on line {}!!",
						GetLexer()->GetLineCount());
				}
				else if (lvalue.DataType.Type == EDataType::None)
				{
					lvalue.DataType = currentType;
				}
				entryValue.push_back(lvalue);
			}
		});

		Union();

		Add(Parsy::CFGElementType::Symbol, COMMA);
		Add(Parsy::CFGElementType::NonTerminal, LVALUE_DECLARATION_FORMAT);
		Add(Parsy::CFGElementType::NonTerminal, LVALUE_LIST_NEXT, [this](Parsy::EntryValue& entry) {
			std::vector<TLvalue>& entryValue = Get<std::vector<TLvalue>>(entry);
			TLvalue& lvalueDeclarationValue = Get<TLvalue>(1);
			std::vector<TLvalue>& lvalueNextValue = Get<std::vector<TLvalue>>(2);

			entryValue.push_back(lvalueDeclarationValue);
			for (TLvalue& lvalue : lvalueNextValue)
			{
				entryValue.push_back(lvalue);
			}
		});

		Union();

		Add(Parsy::CFGElementType::Epsilon);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(IDENTIFIER_LIST);

		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);
		Add(Parsy::CFGElementType::NonTerminal, IDENTIFIER_LIST_NEXT);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(IDENTIFIER_LIST_NEXT);

		Add(Parsy::CFGElementType::Symbol, COMMA);
		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);
		Add(Parsy::CFGElementType::NonTerminal, IDENTIFIER_LIST_NEXT);

		Union();

		Add(Parsy::CFGElementType::Epsilon);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TConstant>(CONSTANT);

		Add(Parsy::CFGElementType::Symbol, _INTEGER, [this](Parsy::EntryValue& entry) {
			TConstant& constantValue = Get<TConstant>(entry);
			int32_t integerValue = Get<int32_t>(0);
			constantValue.DataTypeProps.Type = EDataType::Int32;
			constantValue.Data = integerValue;
		});

		Union();

		Add(Parsy::CFGElementType::Symbol, _BOOL, [this](Parsy::EntryValue& entry) {
			TConstant& constantValue = Get<TConstant>(entry);
			bool boolValue = Get<bool>(0);
			constantValue.DataTypeProps.Type = EDataType::Bool;
			constantValue.Data = boolValue;
			});

		Union();

		Add(Parsy::CFGElementType::Symbol, _FLOAT, [this](Parsy::EntryValue& entry) {
			TConstant& constantValue = Get<TConstant>(entry);
			float floatValue = Get<float>(0);
			constantValue.DataTypeProps.Type = EDataType::Float;
			constantValue.Data = floatValue;
		});

		Union();

		Add(Parsy::CFGElementType::Symbol, _DOUBLE, [this](Parsy::EntryValue& entry) {
			TConstant& constantValue = Get<TConstant>(entry);
			double doubleValue = Get<double>(0);
			constantValue.DataTypeProps.Type = EDataType::Double;
			constantValue.Data = doubleValue;
		});

		Union();

		Add(Parsy::CFGElementType::Symbol, _CHARACTER, [this](Parsy::EntryValue& entry) {
			TConstant& constantValue = Get<TConstant>(entry);
			char characterValue = Get<char>(0);
			constantValue.DataTypeProps.Type = EDataType::Char;
			constantValue.Data = characterValue;
		});

		Union();

		Add(Parsy::CFGElementType::Symbol, _STRING, [this](Parsy::EntryValue& entry) {
			TConstant& constantValue = Get<TConstant>(entry);
			std::string& stringValue = Get<std::string>(0);
			constantValue.DataTypeProps.Type = EDataType::Char;
			constantValue.DataTypeProps.PointerDepth = 1;
			constantValue.Data = stringValue;
		});

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TStruct>(STRUCT_DEFINITION);

		Add(Parsy::CFGElementType::Symbol, STRUCT);
		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);
		Add(Parsy::CFGElementType::Symbol, OPENING_BRACE);
		Add(Parsy::CFGElementType::NonTerminal, STRUCT_MEMBER_DECLARATION_LIST);
		Add(Parsy::CFGElementType::Symbol, CLOSING_BRACE);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<std::vector<TStructMemberDeclaration>>(STRUCT_MEMBER_DECLARATION_LIST);

		Add(Parsy::CFGElementType::NonTerminal, STRUCT_MEMBER_DECLARATIONS);
		
		Union();

		Add(Parsy::CFGElementType::Epsilon);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<std::vector<TStructMemberDeclaration>>(STRUCT_MEMBER_DECLARATIONS);

		Add(Parsy::CFGElementType::NonTerminal, STRUCT_MEMBER_DECLARATION);
		Add(Parsy::CFGElementType::NonTerminal, STRUCT_MEMBER_DECLARATIONS);
		
		Union();
		
		Add(Parsy::CFGElementType::NonTerminal, STRUCT_MEMBER_DECLARATION);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TStructMemberDeclaration>(STRUCT_MEMBER_DECLARATION);

		Add(Parsy::CFGElementType::NonTerminal, LVALUE_DECLARATION_LIST);
		Add(Parsy::CFGElementType::Symbol, SEMICOLON);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TStructMemberDeclaration>(STRUCT_MEMBER_REFERENCE);

		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, DOT);
		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, ARROW);
		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TStructMemberDeclaration>(IF_STATEMENT);

		Add(Parsy::CFGElementType::Symbol, IF);
		Add(Parsy::CFGElementType::Symbol, OPENING_PARENTHESIS);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, CLOSING_PARENTHESIS);
		Add(Parsy::CFGElementType::Symbol, OPENING_BRACE);
		Add(Parsy::CFGElementType::NonTerminal, OPTIONAL_STATEMENT);
		Add(Parsy::CFGElementType::Symbol, CLOSING_BRACE);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TStructMemberDeclaration>(WHILE_STATEMENT);

		Add(Parsy::CFGElementType::Symbol, WHILE);
		Add(Parsy::CFGElementType::Symbol, OPENING_PARENTHESIS);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, CLOSING_PARENTHESIS);
		Add(Parsy::CFGElementType::Symbol, OPENING_BRACE);
		Add(Parsy::CFGElementType::NonTerminal, OPTIONAL_STATEMENT);
		Add(Parsy::CFGElementType::Symbol, CLOSING_BRACE);

	EndRule();

	BeginRule<TStructMemberDeclaration>(FOR_DECLARATION_SLOT);

		Add(Parsy::CFGElementType::NonTerminal, LVALUE_DECLARATION_LIST);

		Union();

		Add(Parsy::CFGElementType::Epsilon);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TStructMemberDeclaration>(FOR_STATEMENT);

		Add(Parsy::CFGElementType::Symbol, FOR);
		Add(Parsy::CFGElementType::Symbol, OPENING_PARENTHESIS);
		Add(Parsy::CFGElementType::NonTerminal, FOR_DECLARATION_SLOT);
		Add(Parsy::CFGElementType::Symbol, SEMICOLON);
		Add(Parsy::CFGElementType::NonTerminal, OPTIONAL_EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, SEMICOLON);
		Add(Parsy::CFGElementType::NonTerminal, OPTIONAL_EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, CLOSING_PARENTHESIS);
		Add(Parsy::CFGElementType::Symbol, OPENING_BRACE);
		Add(Parsy::CFGElementType::NonTerminal, OPTIONAL_STATEMENT);
		Add(Parsy::CFGElementType::Symbol, CLOSING_BRACE);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TStructMemberDeclaration>(FUNCTION_PARAMETER_DECLARATION_LIST);

		Add(Parsy::CFGElementType::NonTerminal, LVALUE_DECLARATION);
		Add(Parsy::CFGElementType::NonTerminal, FUNCTION_PARAMETER_DECLARATION_LIST_NEXT);

		Union();

		Add(Parsy::CFGElementType::Epsilon);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TStructMemberDeclaration>(FUNCTION_PARAMETER_DECLARATION_LIST_NEXT);

		Add(Parsy::CFGElementType::Symbol, COMMA);
		Add(Parsy::CFGElementType::NonTerminal, LVALUE_DECLARATION);
		Add(Parsy::CFGElementType::NonTerminal, FUNCTION_PARAMETER_DECLARATION_LIST_NEXT);

		Union();

		Add(Parsy::CFGElementType::Epsilon);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TStructMemberDeclaration>(FUNCTION_PARAMETER_LIST);

		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION_LIST);

		Union();

		Add(Parsy::CFGElementType::Epsilon);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TStructMemberDeclaration>(FUNCTION_DEFINITION);

		Add(Parsy::CFGElementType::NonTerminal, TYPE_DEFINITION);
		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);
		Add(Parsy::CFGElementType::Symbol, OPENING_PARENTHESIS);
		Add(Parsy::CFGElementType::NonTerminal, FUNCTION_PARAMETER_DECLARATION_LIST);
		Add(Parsy::CFGElementType::Symbol, CLOSING_PARENTHESIS);
		Add(Parsy::CFGElementType::Symbol, OPENING_BRACE);
		Add(Parsy::CFGElementType::NonTerminal, STATEMENT_LIST);
		Add(Parsy::CFGElementType::Symbol, CLOSING_BRACE);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TStructMemberDeclaration>(FUNCTION_CALL);

		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);
		Add(Parsy::CFGElementType::Symbol, OPENING_PARENTHESIS);
		Add(Parsy::CFGElementType::NonTerminal, FUNCTION_PARAMETER_LIST);
		Add(Parsy::CFGElementType::Symbol, CLOSING_PARENTHESIS);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Utilities::Time::TimerHandle& timerHandle = Utilities::Time::BenchmarkRoutine(BIND_CALLBACK(Parse));
	m_Logger.Info("Parsing took {}ms", timerHandle.GetTimeElapsed());

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

const std::string TestParser::RuleToStr(Parsy::RuleID_t ruleID) const
{
	switch (ruleID)
	{
	case PROGRAM: return "Program";
	case STATEMENTS: return "Statements";
	case STATEMENT: return "Statement";
	case OPTIONAL_STATEMENT: return "OptionalStatement";
	case STATEMENT_LIST: return "StatementList";
	case EXPRESSION: return "Expression";
	case OPTIONAL_EXPRESSION: return "OptionalExpression";
	case EXPRESSION_LIST: return "ExpressionList";
	case EXPRESSION_LIST_NEXT: return "ExpressionListNext";
	case ASSIGNMENT: return "Assignment";
	case TERMINAL: return "Terminal";
	case TYPE_DEFINITION: return "TypeDefinition";
	case LVALUE_DECLARATION: return "LvalueDeclaration";
	case LVALUE_DECLARATION_FORMAT: return "LvalueDeclarationFormat";
	case LVALUE: return "Lvalue";
	case LVALUE_DECLARATION_LIST: return "LvalueDeclarationList";
	case LVALUE_LIST_NEXT: return "LvalueListNext";
	case IDENTIFIER_LIST: return "IdentifierList";
	case IDENTIFIER_LIST_NEXT: return "IdentifierListNext";
	case CONSTANT: return "Constant";
	case STRUCT_DEFINITION: return "StructDefinition";
	case STRUCT_MEMBER_DECLARATION_LIST: return "StructMemberDeclarationList";
	case STRUCT_MEMBER_DECLARATIONS: return "StructMemberDeclarations";
	case STRUCT_MEMBER_DECLARATION: return "StructMemberDeclaration";
	case STRUCT_MEMBER_REFERENCE: return "StructMemberReference";
	case IF_STATEMENT: return "IfStatement";
	case WHILE_STATEMENT: return "WhileStatement";
	case FOR_DECLARATION_SLOT: return "ForDeclarationSlot";
	case FOR_STATEMENT: return "ForStatement";
	case FUNCTION_PARAMETER_DECLARATION_LIST: return "FunctionParameterDeclarationList";
	case FUNCTION_PARAMETER_DECLARATION_LIST_NEXT: return "FunctionParameterDeclarationListNext";
	case FUNCTION_PARAMETER_LIST: return "FunctionParameterList";
	case FUNCTION_DEFINITION: return "FunctionDefinition";
	case FUNCTION_CALL: return "FunctionCall";
	}

	return Parser::RuleToStr(ruleID);
}

const std::string TestParser::TokenToStr(Lexy::TokenID_t tokenID) const
{
	switch (tokenID)
	{
	case INTEGER_TYPE: return "IntegerType";
	case BOOL_TYPE: return "BoolType";
	case FLOAT_TYPE: return "FloatType";
	case DOUBLE_TYPE: return "DoubleType";
	case CHARACTER_TYPE: return "CharacterType";
	case SHORT_TYPE: return "ShortType";
	case LONG_TYPE: return "LongType";
	case UNSIGNED_KEYWORD: return "Unsigned";
	case CONST_KEYWORD: return "Const";
	case IF: return "If";
	case ELSE: return "Else";
	case FOR: return "For";
	case WHILE: return "While";
	case STRUCT: return "Struct";
	case PLUS: return "Plus";
	case DASH: return "Dash";
	case STAR: return "Star";
	case SLASH: return "Slash";
	case PERCENT: return "Percent";
	case CARET: return "Caret";
	case EQUALS: return "Equals";
	case PLUS_PLUS: return "PlusPlus";
	case DASH_DASH: return "DashDash";
	case PLUS_EQUALS: return "PlusEquals";
	case DASH_EQUALS: return "DashEquals";
	case STAR_EQUALS: return "StarEquals";
	case SLASH_EQUALS: return "SlashEquals";
	case PERCENT_EQUALS: return "PercentEquals";
	case CARET_EQUALS: return "CaretEquals";
	case EQUALS_EQUALS: return "EqualsEquals";
	case EXCLAMATION_EQUALS: return "ExclamationEquals";
	case LESS: return "Less";
	case LESS_EQUALS: return "LessEquals";
	case GREATER: return "Greater";
	case GREATER_EQUALS: return "GreaterEquals";
	case DOT: return "Dot";
	case DOT_DOT: return "DotDot";
	case COMMA: return "Comma";
	case QUESTIONMARK: return "Questionmark";
	case COLON: return "Colon";
	case COLON_COLON: return "ColonColon";
	case OPENING_PARENTHESIS: return "OpeningParenthesis";
	case CLOSING_PARENTHESIS: return "ClosingParenthesis";
	case OPENING_BRACKET: return "OpeningBracket";
	case CLOSING_BRACKET: return "ClosingBracket";
	case OPENING_BRACE: return "OpeningBrace";
	case CLOSING_BRACE: return "ClosingBrace";
	case SEMICOLON: return "Semicolon";
	case IDENTIFIER: return "Identifier";
	case _INTEGER: return "Integer";
	case _BOOL: return "Bool";
	case _FLOAT: return "Float";
	case _DOUBLE: return "Double";
	case _CHARACTER: return "Character";
	case _STRING: return "String";
	case AMPERSAND: return "Ampersand";
	case ARROW: return "Arrow";
	}

	return Parser::TokenToStr(tokenID);
}

void TestParser::SyntaxErrorHandler()
{
	m_Logger.Error("Syntax error on line {}", GetLexer()->GetLineCount());
	exit(1);
}

TConstant TestParser::GetConstantValueFromExpression(const TExpression& expression)
{
	switch (expression.Type)
	{
	case EExpressionType::Constant:
	{
		return expression.ConstantValue;
	}
	case EExpressionType::Lvalue:
	{
		SymbolTableEntry* symEntry = m_SymbolTable.LookUp(expression.SymbolTableEntry, 0);
		return symEntry->ConstantValue;
	}
	}

	return TConstant();
}

TConstant TestParser::ExecuteOperation(const EOperationType& operationType, const TConstant& leftConstant, const TConstant& rightConstant)
{
	TConstant result;
	result.DataTypeProps = leftConstant.DataTypeProps;
	switch (operationType)
	{
	case EOperationType::Plus:
	{
		switch (leftConstant.DataTypeProps.Type)
		{
		case EDataType::None:
		{
			std::cout << "None";
			break;
		}
		case EDataType::Int16:
		{
			const int16_t& leftValue = leftConstant.Get<int16_t>();
			const int16_t& rightValue = rightConstant.Get<int16_t>();
			result.Data = leftValue + rightValue;
			break;
		}
		case EDataType::Int32:
		{
			const int32_t& leftValue = leftConstant.Get<int32_t>();
			const int32_t& rightValue = rightConstant.Get<int32_t>();
			result.Data = leftValue + rightValue;
			break;
		}
		case EDataType::Int64:
		{
			const int64_t& leftValue = leftConstant.Get<int64_t>();
			const int64_t& rightValue = rightConstant.Get<int64_t>();
			result.Data = leftValue + rightValue;
			break;
		}
		case EDataType::Bool:
		{
			m_Logger.Error("You cannot add booleans, on line {}!!", GetLexer()->GetLineCount());
			break;
		}
		case EDataType::Float:
		{
			const float& leftValue = leftConstant.Get<float>();
			const float& rightValue = rightConstant.Get<float>();
			result.Data = leftValue + rightValue;
			break;
		}
		case EDataType::Double:
		{
			const double& leftValue = leftConstant.Get<double>();
			const double& rightValue = rightConstant.Get<double>();
			result.Data = leftValue + rightValue;
			break;
		}
		case EDataType::Char:
		{
			const char& leftValue = leftConstant.Get<char>();
			const char& rightValue = rightConstant.Get<char>();
			result.Data = leftValue + rightValue;
			break;
		}
		case EDataType::Struct:
		{
			m_Logger.Error("You cannot add structs, on line {}!!", GetLexer()->GetLineCount());
			break;
		}
		}
		break;
	}
	}
	return result;
}