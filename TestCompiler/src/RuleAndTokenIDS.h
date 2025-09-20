#pragma once

enum TokenID
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	IDENTIFIER,
	_INTEGER,
	_BOOL,
	_FLOAT,
	_DOUBLE,
	_CHARACTER,
	_STRING,

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PLUS,
	DASH,
	STAR,
	SLASH,
	PERCENT,
	CARET,

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	INTEGER_TYPE,
	BOOL_TYPE,
	FLOAT_TYPE,
	DOUBLE_TYPE,
	CHARACTER_TYPE,
	SHORT_TYPE,
	LONG_TYPE,
	UNSIGNED_KEYWORD,
	CONST_KEYWORD,
	IF,
	ELSE,
	FOR,
	WHILE,
	STRUCT,

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	EQUALS,
	PLUS_PLUS,
	DASH_DASH,
	PLUS_EQUALS,
	DASH_EQUALS,
	STAR_EQUALS,
	SLASH_EQUALS,
	PERCENT_EQUALS,
	CARET_EQUALS,
	EQUALS_EQUALS,
	EXCLAMATION_EQUALS,
	LESS,
	LESS_EQUALS,
	GREATER,
	GREATER_EQUALS,
	PIPE_PIPE,
	AMPERSAND_AMPERSAND,
	DOT,
	DOT_DOT,
	COMMA,
	QUESTIONMARK,
	COLON,
	COLON_COLON,
	OPENING_PARENTHESIS,
	CLOSING_PARENTHESIS,
	OPENING_BRACKET,
	CLOSING_BRACKET,
	OPENING_BRACE,
	CLOSING_BRACE,
	SEMICOLON,
	BACKSLASH,
	PIPE,
	UNDERSCORE,
	AMPERSAND,
	ARROW,

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	UMINUS,

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	TOKENID_MAX

};

enum RuleID
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PROGRAM = TOKENID_MAX,
	STATEMENTS,
	STATEMENT,
	OPTIONAL_STATEMENT,
	STATEMENT_LIST,
	EXPRESSION,
	OPTIONAL_EXPRESSION,
	EXPRESSION_LIST,
	EXPRESSION_LIST_NEXT,
	ASSIGNMENT,
	TERMINAL,
	TYPE_DEFINITION,
	LVALUE_DECLARATION,
	LVALUE_DECLARATION_FORMAT,
	LVALUE,
	LVALUE_DECLARATION_LIST,
	LVALUE_LIST_NEXT,
	IDENTIFIER_LIST,
	IDENTIFIER_LIST_NEXT,
	CONSTANT,
	STRUCT_DEFINITION,
	STRUCT_MEMBER_DECLARATION_LIST,
	STRUCT_MEMBER_DECLARATIONS,
	STRUCT_MEMBER_DECLARATION,
	STRUCT_MEMBER_REFERENCE,
	IF_STATEMENT,
	WHILE_STATEMENT,
	FOR_DECLARATION_SLOT,
	FOR_STATEMENT,
	FUNCTION_PARAMETER_DECLARATION_LIST,
	FUNCTION_PARAMETER_DECLARATION_LIST_NEXT,
	FUNCTION_PARAMETER_LIST,
	FUNCTION_DEFINITION,
	FUNCTION_CALL,

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	RULEID_MAX
};