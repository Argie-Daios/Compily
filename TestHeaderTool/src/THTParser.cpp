#include "THTParser.h"

#include "THTLexer.h"
#include "RuleAndTokenIDS.h"

static const std::string RemoveExtention(const std::string& string)
{
	size_t index = string.find_last_of('.');
	if (index == std::string::npos) return string;
	return string.substr(0, index);
}

THTParser::THTParser(const std::string& sourceCodePath)
	: CLRParser(Parsy::CLRParserFlags_ForcePrecedence)
{
	AttachLexer<THTLexer>(sourceCodePath);
	m_Logger = Utilities::Logger::Register("TestParser", sourceCodePath);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	EnableDebugTools();
	DeclareTokenIDValidationCheck([](int32_t id) { return id >= 0 && id < TOKENID_MAX; });
	DeclareRuleIDValidationCheck([](int32_t id) { return id >= TOKENID_MAX && id < RULEID_MAX; });
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

		Add(Parsy::CFGElementType::NonTerminal, CLASS_DEFINITION, 
			[this](Parsy::EntryValue& entry) {
				m_Logger.Warn("Found class");
			});

		Union();

		Add(Parsy::CFGElementType::Symbol, SEMICOLON);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BeginRule(CLASS_DEFINITION);

		Add(Parsy::CFGElementType::Symbol, CLASS);
		Add(Parsy::CFGElementType::Symbol, IDENTIFIER);
		Add(Parsy::CFGElementType::Symbol, LEFT_BRACE);
		Add(Parsy::CFGElementType::Symbol, RIGHT_BRACE);

	EndRule();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Utilities::Time::TimerHandle& timerHandle = Utilities::Time::BenchmarkRoutine(BIND_CALLBACK(Parse));
	m_Logger.Info("Parsing took {}ms", timerHandle.GetTimeElapsed());

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

const std::string THTParser::RuleToStr(Parsy::RuleID_t ruleID) const
{
	switch (ruleID)
	{
	case CLASS_DEFINITION: return "ClassDefinition";
	}

	return Parser::RuleToStr(ruleID);
}

const std::string THTParser::TokenToStr(Lexy::TokenID_t tokenID) const
{
	switch (tokenID)
	{
	case CLASS: return "Class";
	case IDENTIFIER: return "Identifier";
	case LEFT_BRACE: return "LeftBrace";
	case RIGHT_BRACE: return "RightBrace";
	case LEFT_PARENTHESIS: return "LeftParenthesis";
	case RIGHT_PARENTHESIS: return "RightParenthesis";
	case SEMICOLON: return "Semicolon";
	}

	return Parser::TokenToStr(tokenID);
}

void THTParser::SyntaxErrorHandler()
{
	m_Logger.Error("Syntax error on line {}", GetLexer()->GetLineCount());
	exit(1);
}