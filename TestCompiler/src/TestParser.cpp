#include "TestParser.h"

#include "TestLexer.h"
#include "RuleAndTokenIDS.h"

#include "RuleTypes/Constant.h"
#include "RuleTypes/Lvalue.h"
#include "RuleTypes/Struct.h"

TestParser::TestParser(const std::ifstream& inputStream)
	: CLRParser(inputStream, Parsy::CLRParserFlags_ForcePrecedence)
{
	m_Lexer = new TestLexer(inputStream);

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
				std::cout << "Lvalue type: ";
				switch (lvalue.Constant.DataTypeProps.Type)
				{
				case EDataType::None:
				{
					std::cout << "None";
					break;
				}
				case EDataType::Int16:
				{
					std::cout << "Int16";
					break;
				}
				case EDataType::Int32:
				{
					std::cout << "Int32";
					break;
				}
				case EDataType::Int64:
				{
					std::cout << "Int64";
					break;
				}
				case EDataType::Bool:
				{
					std::cout << "Bool";
					break;
				}
				case EDataType::Float:
				{
					std::cout << "Float";
					break;
				}
				case EDataType::Double:
				{
					std::cout << "Double";
					break;
				}
				case EDataType::Char:
				{
					std::cout << "Character";
					break;
				}
				case EDataType::Struct:
				{
					std::cout << "Struct";
					break;
				}
				}
				std::cout << ", Depth: " << lvalue.Constant.DataTypeProps.PointerDepth << std::endl;
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

	BeginRule<TConstant>(EXPRESSION);

		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, PLUS);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);

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
			TConstant& entryValue = Get<TConstant>(entry);
			TConstant& constantValue = Get<TConstant>(0);
			entryValue = constantValue;
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

	BeginRule<TConstant>(TERMINAL);

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

		Add(Parsy::CFGElementType::NonTerminal, LVALUE);

		Union();
	
		Add(Parsy::CFGElementType::NonTerminal, CONSTANT, [this](Parsy::EntryValue& entry) {
			TConstant& entryValue = Get<TConstant>(entry);
			TConstant& constantValue = Get<TConstant>(0);
			entryValue = constantValue;
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
			entryValue.Constant.DataTypeProps = dataProperties;
		});

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TLvalue>(LVALUE_DECLARATION_FORMAT);

	Add(Parsy::CFGElementType::Symbol, IDENTIFIER);

	Union();

	Add(Parsy::CFGElementType::Symbol, IDENTIFIER);
	Add(Parsy::CFGElementType::Symbol, EQUALS);
	Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TLvalue>(LVALUE);

		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);

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
			for (TLvalue& lvalue : lvalueNextValue)
			{
				if (lvalue.Constant.DataTypeProps.Type == EDataType::None)
					lvalue.Constant.DataTypeProps.Type = lvalueDeclarationValue.Constant.DataTypeProps.Type;
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
			for (TLvalue& lvalue : lvalueNextValue)
			{
				if (lvalue.Constant.DataTypeProps.Type == EDataType::None)
					lvalue.Constant.DataTypeProps.Type = lvalueDeclarationValue.Constant.DataTypeProps.Type;
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
	Utilities::Logger::Info("TestParser", "Parsing took {}ms", timerHandle.GetTimeElapsed());

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
	Utilities::Logger::Error("TestCompiler", "Syntax error on line {}", m_Lexer->GetLineCount());
	exit(1);
}