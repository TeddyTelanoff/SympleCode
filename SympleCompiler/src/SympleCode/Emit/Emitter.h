#pragma once

#include "SympleCode/Node/CompilationUnitNode.h"

#include "SympleCode/Node/Member/GlobalStatementNode.h"
#include "SympleCode/Node/Member/FunctionDeclarationNode.h"

#include "SympleCode/Node/Statement/BlockStatementNode.h"
#include "SympleCode/Node/Statement/WhileStatementNode.h"
#include "SympleCode/Node/Statement/ReturnStatementNode.h"
#include "SympleCode/Node/Statement/ExpressionStatementNode.h"
#include "SympleCode/Node/Statement/VariableDeclarationNode.h"

#include "SympleCode/Node/Expression/CastExpressionNode.h"
#include "SympleCode/Node/Expression/ListExpressionNode.h"
#include "SympleCode/Node/Expression/FunctionCallExpressionNode.h"
#include "SympleCode/Node/Expression/ParenthesizedExpressionNode.h"

#include "SympleCode/Node/Expression/Literal/NullLiteralExpressionNode.h"
#include "SympleCode/Node/Expression/Literal/NumberLiteralExpressionNode.h"
#include "SympleCode/Node/Expression/Literal/StringLiteralExpressionNode.h"
#include "SympleCode/Node/Expression/Literal/BooleanLiteralExpressionNode.h"
#include "SympleCode/Node/Expression/Literal/CharacterLiteralExpressionNode.h"

#include "SympleCode/Node/Expression/Modifiable/FieldExpressionNode.h"
#include "SympleCode/Node/Expression/Modifiable/VariableExpressionNode.h"
#include "SympleCode/Node/Expression/Modifiable/AssignmentExpressionNode.h"
#include "SympleCode/Node/Expression/Modifiable/PointerIndexExpressionNode.h"
#include "SympleCode/Node/Expression/Modifiable/DereferencePointerExpressionNode.h"

#include "SympleCode/Node/Expression/Operator/UnaryExpressionNode.h"
#include "SympleCode/Node/Expression/Operator/BinaryExpressionNode.h"

#include <cstdio>

namespace Symple
{
	typedef int Register;

	class Emitter
	{
	private:
		friend class RegisterManager;

		RegisterManager* mRegisterManager;

		FILE* mFile;
		FILE* mLiteralFile;

		const char* mPath;

		unsigned int mData;
		unsigned int mReturn;
		unsigned int mBreak;
		unsigned int mStack;

		bool mReturning;
		bool mReturnStatement;
	public:
		Emitter(const char* path);
		~Emitter();

		void EmitCompilationUnit(const CompilationUnitNode*);
	private:
		char Suf(int sz = 4);

		void Push(Register, int sz = 4);
		void Pop(Register, int sz = 4);

		void EmitMember(const MemberNode*);
		void EmitGlobalStatement(const GlobalStatementNode*);
		void EmitFunctionDeclaration(const FunctionDeclarationNode*);

		void EmitStatement(const StatementNode*);
		void EmitBlockStatement(const BlockStatementNode*);
		void EmitExpressionStatement(const ExpressionStatementNode*);

		Register EmitExpression(const ExpressionNode*);

		Register EmitLiteralExpression(const LiteralExpressionNode*);
		Register EmitNumberLiteralExpression(const NumberLiteralExpressionNode*);

		bool OpenFile();
		bool OpenLiteralFile();
	};
}

#include "SympleCode/Emit/RegisterManager.h"