#pragma once

#include <vector>

#include "SympleCode/Parser/Lexer.h"
#include "SympleCode/Common/Node/Node.h"
#include "SympleCode/Common/Node/CompilationUnitNode.h"
#include "SympleCode/Common/Node/FunctionArgumentsNode.h"
#include "SympleCode/Common/Node/FunctionCallArgumentsNode.h"

#include "SympleCode/Common/Node/Statement/IfStatementNode.h"
#include "SympleCode/Common/Node/Statement/BlockStatementNode.h"
#include "SympleCode/Common/Node/Statement/WhileStatementNode.h"
#include "SympleCode/Common/Node/Statement/ReturnStatementNode.h"
#include "SympleCode/Common/Node/Statement/VariableDeclarationNode.h"

#include "SympleCode/Common/Node/Member/FunctionDeclarationNode.h"
#include "SympleCode/Common/Node/Member/ExternFunctionNode.h"
#include "SympleCode/Common/Node/Member/FunctionHintNode.h"
#include "SympleCode/Common/Node/Member/GlobalStatementNode.h"

#include "SympleCode/Common/Node/Expression/ExpressionNode.h"
#include "SympleCode/Common/Node/Expression/LiteralExpressionNode.h"
#include "SympleCode/Common/Node/Expression/FunctionCallExpressionNode.h"
#include "SympleCode/Common/Node/Expression/ParenthesizedExpressionNode.h"

#include "SympleCode/Common/Analysis/Type.h"
#include "SympleCode/Common/Analysis/Diagnostics.h"

namespace Symple
{
	class Parser
	{
	private:
		Lexer mLexer;
		std::vector<const Token*> mTokens;
		std::vector<const Type*> mTypes;
		Diagnostics* mDiagnostics;
		size_t mPosition;
	public:
		Parser(const char* source = "");

		CompilationUnitNode* ParseCompilationUnit();
		Diagnostics* GetDiagnostics() const;
	private:
		void Preprocess(const Token* cmd);

		const Token* Peek(size_t offset = 0);
		const Token* Next();
		const Token* Match(Token::Kind kind);

		bool IsType(const Token* token);
		const Type* GetType(const Token* token);

		const std::vector<const MemberNode*> ParseMembers();
		MemberNode* ParseMember();

		FunctionDeclarationNode* ParseFunctionDeclaration();
		FunctionArgumentsNode* ParseFunctionArguments();
		FunctionArgumentNode* ParseFunctionArgument();
		ExternFunctionNode* ParseExternFunction();
		FunctionHintNode* ParseFunctionHint();

		StatementNode* ParseStatement();
		IfStatementNode* ParseIfStatement();
		WhileStatementNode* ParseWhileStatement();
		BlockStatementNode* ParseBlockStatement();
		ReturnStatementNode* ParseReturnStatement();
		GlobalStatementNode* ParseGlobalStatement();
		VariableDeclarationNode* ParseVariableDeclaration();
	
		ExpressionNode* ParseExpression();

		ExpressionNode* ParseUnaryExpression(int parentPriority = -1);
		ExpressionNode* ParseBinaryExpression(int parentPriority = -1);
		ExpressionNode* ParsePrimaryExpression();

		ExpressionNode* ParseNameOrCallExpression();
		FunctionCallArgumentsNode* ParseFunctionCallArguments();

		FunctionCallExpressionNode* ParseFunctionCallExpression();
		ParenthesizedExpressionNode* ParseParenthesizedExpression();
	};
}