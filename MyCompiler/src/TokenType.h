#pragma once

enum TokenType
{
	/////GENERAL/////////////////////////////////////////////////////////////////////////////////////////

	COMMENT,
	STRING,
	IDENTIFIER,
	DOUBLE,
	INTEGER,

	/////KEYWORDS////////////////////////////////////////////////////////////////////////////////////////

	IF,
	ELSE,
	WHILE,
	FOR,

	/////OPERATORS///////////////////////////////////////////////////////////////////////////////////////

	PLUS,
	MINUS,
	MULTIPLY,
	DIVIDE,
	MOD,

	/////PUNCTUATION/////////////////////////////////////////////////////////////////////////////////////

	EQUAL,
	EQUAL_EQUAL,
	NOT_EQUAL,
	LESS,
	LESS_EQUAL,
	GREATER,
	GREATER_EQUAL,
	LEFT_BRACE,
	RIGHT_BRACE,
	LEFT_BRACKET,
	RIGHT_BRACKET,
	LEFT_PARENTHESIS,
	RIGHT_PARENTHESIS,
	SEMICOLON

	/////////////////////////////////////////////////////////////////////////////////////////////////////
};