#include "TestParser.h"

#include "TestLexer.h"
#include "RuleAndTokenIDS.h"

#include "RuleTypes/Constant.h"
#include "RuleTypes/DataType.h"
#include "RuleTypes/Lvalue.h"
#include "RuleTypes/Struct.h"

TestParser::TestParser(const std::ifstream& inputStream)
	: CLRParser(inputStream, Parsy::CLRParserFlags_ForcePrecedence)
{
	m_Lexer = new TestLexer(inputStream);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

		Add(Parsy::CFGElementType::NonTerminal, ASSIGNMENT_LIST);
		Add(Parsy::CFGElementType::Symbol, SEMICOLON);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, LVALUE_DECLARATION_LIST);
		Add(Parsy::CFGElementType::Symbol, SEMICOLON);

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

	BeginRule(EXPRESSION);

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

		Add(Parsy::CFGElementType::NonTerminal, TERMINAL);

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

		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);
		Add(Parsy::CFGElementType::Symbol, EQUALS);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(ASSIGNMENT_LIST);

	Add(Parsy::CFGElementType::NonTerminal, ASSIGNMENT);
	Add(Parsy::CFGElementType::NonTerminal, ASSIGNMENT_LIST_NEXT);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(ASSIGNMENT_LIST_NEXT);

		Add(Parsy::CFGElementType::Symbol, COMMA);
		Add(Parsy::CFGElementType::NonTerminal, ASSIGNMENT);
		Add(Parsy::CFGElementType::NonTerminal, ASSIGNMENT_LIST_NEXT);

		Union();

		Add(Parsy::CFGElementType::Epsilon);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(TERMINAL);

		Add(Parsy::CFGElementType::Symbol, OPENING_PARENTHESIS);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);
		Add(Parsy::CFGElementType::Symbol, CLOSING_PARENTHESIS);

		Union();

		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);
		Add(Parsy::CFGElementType::Symbol, PLUS);
		Add(Parsy::CFGElementType::Symbol, PLUS);

		Union();

		Add(Parsy::CFGElementType::Symbol, PLUS);
		Add(Parsy::CFGElementType::Symbol, PLUS);
		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);

		Union();

		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);
		Add(Parsy::CFGElementType::Symbol, DASH);
		Add(Parsy::CFGElementType::Symbol, DASH);

		Union();

		Add(Parsy::CFGElementType::Symbol, DASH);
		Add(Parsy::CFGElementType::Symbol, DASH);
		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);

		Union();

		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);

		Union();

		Add(Parsy::CFGElementType::Symbol, STAR);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);

		Union();

		Add(Parsy::CFGElementType::Symbol, AMPERSAND);
		Add(Parsy::CFGElementType::NonTerminal, EXPRESSION);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, FUNCTION_CALL);

		Union();
	
		Add(Parsy::CFGElementType::NonTerminal, CONSTANT);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TDataType>(TYPE_DEFINITION);

		Add(Parsy::CFGElementType::Symbol, INTEGER_TYPE);

		Union();

		Add(Parsy::CFGElementType::Symbol, FLOAT_TYPE);

		Union();

		Add(Parsy::CFGElementType::Symbol, DOUBLE_TYPE);

		Union();

		Add(Parsy::CFGElementType::Symbol, CHARACTER_TYPE);

		Union();

		Add(Parsy::CFGElementType::Symbol, SHORT_TYPE);

		Union();

		Add(Parsy::CFGElementType::Symbol, LONG_TYPE);

		Union();

		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, TYPE_DEFINITION);
		Add(Parsy::CFGElementType::Symbol, STAR, [this](Parsy::EntryValue& entry) {
			TDataType& entryValue = Get<TDataType>(entry);
			TDataType& dataTypeValue = Get<TDataType>(0);
			dataTypeValue.PointerDepth++;
			entryValue = dataTypeValue;
		});

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TLvalue>(LVALUE_DECLARATION);

		Add(Parsy::CFGElementType::NonTerminal, TYPE_DEFINITION);
		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, TYPE_DEFINITION);
		Add(Parsy::CFGElementType::NonTerminal, ASSIGNMENT);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<TLvalue>(LVALUE);

		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);

		Union();

		Add(Parsy::CFGElementType::NonTerminal, ASSIGNMENT);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<std::vector<TLvalue>>(LVALUE_DECLARATION_LIST);

		Add(Parsy::CFGElementType::NonTerminal, LVALUE_DECLARATION);
		Add(Parsy::CFGElementType::NonTerminal, LVALUE_LIST_NEXT);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<std::vector<TLvalue>>(LVALUE_LIST);

	Add(Parsy::CFGElementType::NonTerminal, LVALUE);
	Add(Parsy::CFGElementType::NonTerminal, LVALUE_LIST_NEXT);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule<std::vector<TLvalueData>>(LVALUE_LIST_NEXT);

		Add(Parsy::CFGElementType::Symbol, COMMA);
		Add(Parsy::CFGElementType::NonTerminal, LVALUE_DECLARATION);
		Add(Parsy::CFGElementType::NonTerminal, LVALUE_LIST_NEXT);

		Union();

		Add(Parsy::CFGElementType::Symbol, COMMA);
		Add(Parsy::CFGElementType::NonTerminal, ASSIGNMENT);
		Add(Parsy::CFGElementType::NonTerminal, LVALUE_LIST_NEXT);

		Union();

		Add(Parsy::CFGElementType::Symbol, COMMA);
		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);
		Add(Parsy::CFGElementType::NonTerminal, LVALUE_LIST_NEXT);

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

		Add(Parsy::CFGElementType::Symbol, INTEGER, [this](Parsy::EntryValue& entry) {
			TConstant& constantValue = Get<TConstant>(entry);
			int32_t integerValue = Get<int32_t>(0);
			constantValue.Type = ConstantType::Int32;
			constantValue.Data = integerValue;
		});

		Union();

		Add(Parsy::CFGElementType::Symbol, _FLOAT, [this](Parsy::EntryValue& entry) {
			TConstant& constantValue = Get<TConstant>(entry);
			float floatValue = Get<float>(0);
			constantValue.Type = ConstantType::Float;
			constantValue.Data = floatValue;
		});

		Union();

		Add(Parsy::CFGElementType::Symbol, DOUBLE, [this](Parsy::EntryValue& entry) {
			TConstant& constantValue = Get<TConstant>(entry);
			double doubleValue = Get<double>(0);
			constantValue.Type = ConstantType::Double;
			constantValue.Data = doubleValue;
		});

		Union();

		Add(Parsy::CFGElementType::Symbol, CHARACTER, [this](Parsy::EntryValue& entry) {
			TConstant& constantValue = Get<TConstant>(entry);
			char characterValue = Get<char>(0);
			constantValue.Type = ConstantType::Char;
			constantValue.Data = characterValue;
		});

		Union();

		Add(Parsy::CFGElementType::Symbol, STRING, [this](Parsy::EntryValue& entry) {
			TConstant& constantValue = Get<TConstant>(entry);
			std::string& stringValue = Get<std::string>(0);
			constantValue.Type = ConstantType::String;
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

		Add(Parsy::CFGElementType::NonTerminal, LVALUE_LIST);

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
	case ASSIGNMENT_LIST: return "AssignmentList";
	case ASSIGNMENT_LIST_NEXT: return "AssignmentListNext";
	case TERMINAL: return "Terminal";
	case TYPE_DEFINITION: return "TypeDefinition";
	case LVALUE_DECLARATION: return "LvalueDeclaration";
	case LVALUE: return "Lvalue";
	case LVALUE_DECLARATION_LIST: return "LvalueDeclarationList";
	case LVALUE_LIST: return "LvalueList";
	case LVALUE_LIST_NEXT: return "LvalueListNext";
	case IDENTIFIER_LIST: return "IdentifierList";
	case IDENTIFIER_LIST_NEXT: return "IdentifierListNext";
	case CONSTANT: return "Constant";
	case STRUCT_DEFINITION: return "StructDefinition";
	case STRUCT_MEMBER_DECLARATION_LIST: return "StructMemberDeclarationList";
	case STRUCT_MEMBER_DECLARATIONS: return "StructMemberDeclarations";
	case STRUCT_MEMBER_DECLARATION: return "StructMemberDeclaration";
	case IF_STATEMENT: return "IfStatement";
	case WHILE_STATEMENT: return "WhileStatement";
	case FOR_DECLARATION_SLOT: return "ForDeclarationSlot";
	case FOR_STATEMENT: return "ForStatement";
	case FUNCTION_PARAMETER_DECLARATION_LIST: return "FunctionParameterDeclarationList";
	case FUNCTION_PARAMETER_DECLARATION_LIST_NEXT: return "FunctionParameterDeclarationListNext";
	case FUNCTION_PARAMETER_LIST: return "FunctionParameterList";
	case FUNCTION_DEFINITION: return "FunctionDefinition";
	}

	return Parser::RuleToStr(ruleID);
}

const std::string TestParser::TokenToStr(Lexy::TokenID_t tokenID) const
{
	switch (tokenID)
	{
	case INTEGER_TYPE: return "IntegerType";
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
	case STRING: return "String";
	case CHARACTER: return "Character";
	case IDENTIFIER: return "Identifier";
	case INTEGER: return "Integer";
	case _FLOAT: return "Float";
	case DOUBLE: return "Double";
	case AMPERSAND: return "Ampersand";
	}

	return Parser::TokenToStr(tokenID);
}