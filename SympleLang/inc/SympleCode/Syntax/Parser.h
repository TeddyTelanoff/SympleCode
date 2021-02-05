#pragma once

#include <vector>

#include "SympleCode/Syntax/Lexer.h"
#include "SympleCode/Syntax/Token.h"

#include "SympleCode/Syntax/Node.h"
#include "SympleCode/Syntax/TranslationUnitSyntax.h"

#include "SympleCode/Syntax/MemberSyntax.h"
#include "SympleCode/Syntax/FunctionDeclarationSyntax.h"
#include "SympleCode/Syntax/FunctionDeclarationSyntax.h"

#include "SympleCode/Syntax/StatementSyntax.h"
#include "SympleCode/Syntax/TypeSyntax.h"
#include "SympleCode/Syntax/TypeReferenceSyntax.h"
#include "SympleCode/Syntax/BlockStatementSyntax.h"
#include "SympleCode/Syntax/ReturnStatementSyntax.h"
#include "SympleCode/Syntax/ExpressionStatementSyntax.h"
#include "SympleCode/Syntax/VariableDeclarationSyntax.h"

#include "SympleCode/Syntax/ExpressionSyntax.h"
#include "SympleCode/Syntax/UnaryExpressionSyntax.h"
#include "SympleCode/Syntax/BinaryExpressionSyntax.h"
#include "SympleCode/Syntax/LiteralExpressionSyntax.h"
#include "SympleCode/Syntax/ParenthesizedExpressionSyntax.h"

#include "SympleCode/Syntax/NameExpressionSyntax.h"
#include "SympleCode/Syntax/CallExpressionSyntax.h"

#include "SympleCode/DiagnosticBag.h"

namespace Symple::Syntax
{
	class Parser
	{
	private:
		std::vector<shared_ptr<Token>> mTokens;
		unsigned mPosition = 0;

		shared_ptr<DiagnosticBag> mDiagnosticBag = make_shared<DiagnosticBag>();
	public:
		Parser(shared_ptr<Lexer>);
		Parser(shared_ptr<Lexer>, std::vector<shared_ptr<Token>>);
		Parser(std::vector<shared_ptr<Token>>);

		shared_ptr<TranslationUnitSyntax> Parse();

		shared_ptr<MemberSyntax> ParseMember();
		shared_ptr<FunctionDeclarationSyntax> ParseFunctionDeclaration();

		shared_ptr<StatementSyntax> ParseStatement(bool matchSemicolon = true);
		shared_ptr<BlockStatementSyntax> ParseBlockStatement();
		shared_ptr<ReturnStatementSyntax> ParseReturnStatement();
		shared_ptr<ExpressionStatementSyntax> ParseExpressionStatement();
		shared_ptr<VariableDeclarationSyntax> ParseVariableDeclaration();
		VariableDeclarationList ParseVariableDeclarationList();
		shared_ptr<TypeSyntax> ParseType(shared_ptr<TypeSyntax> base = nullptr);

		shared_ptr<ExpressionSyntax> ParseExpression();
		ExpressionList ParseExpressionList();
		shared_ptr<ExpressionSyntax> ParseUnaryExpression(unsigned parentPrecedence = 0);
		shared_ptr<ExpressionSyntax> ParseBinaryExpression(unsigned parentPrecedence = 0);

		shared_ptr<ExpressionSyntax> ParsePrimaryExpression();
		shared_ptr<NameExpressionSyntax> ParseNameExpression();
		shared_ptr<CallExpressionSyntax> ParseCallExpression();
		shared_ptr<LiteralExpressionSyntax> ParseLiteralExpression();
		shared_ptr<ParenthesizedExpressionSyntax> ParseParenthesizedExpression();

		shared_ptr<DiagnosticBag> GetDiagnosticBag();
	private:
		shared_ptr<Token> Peek(unsigned off = 0);
		shared_ptr<Token> Next();
		shared_ptr<Token> Match(Token::Kind);

		bool IsType();
	};
}