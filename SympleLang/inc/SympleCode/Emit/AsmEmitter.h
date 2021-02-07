#pragma once

#include <cstdio>
#include "SympleCode/Binding/BoundStatement.h"
#include "SympleCode/Binding/BoundCallExpression.h"
#include "SympleCode/Binding/BoundCompilationUnit.h"
#include "SympleCode/Binding/BoundReturnStatement.h"
#include "SympleCode/Binding/BoundBinaryExpression.h"
#include "SympleCode/Binding/BoundVariableExpression.h"
#include "SympleCode/Binding/BoundExpressionStatement.h"

#include "SympleCode/Symbol/FunctionSymbol.h"
#include "SympleCode/Emit/Scope.h"

namespace Symple::Emit
{
	class AsmEmitter
	{
	protected:
		char* mFile;
		FILE* mTextStream;
		FILE* mDataStream;

		shared_ptr<Symbol::FunctionSymbol> mFunction;
		shared_ptr<Binding::BoundCompilationUnit> mCompilationUnit;
		shared_ptr<Scope> mScope;

		void BeginScope();
		void EndScope();

		char* RegAx(unsigned sz);
		char Suf(unsigned sz);

		bool mClosed = false;
		bool mReturning;
	public:
		AsmEmitter(char* file = nullptr);
		~AsmEmitter();

		void Compile();
		void Emit(shared_ptr<Binding::BoundCompilationUnit>);
		void EmitFunction(shared_ptr<Symbol::FunctionSymbol>, shared_ptr<Binding::BoundStatement>);

		void EmitStatement(shared_ptr<Binding::BoundStatement>);
		void EmitBlockStatement(shared_ptr<Binding::BoundBlockStatement>);
		void EmitReturnStatement(shared_ptr<Binding::BoundReturnStatement>);
		void EmitExpressionStatement(shared_ptr<Binding::BoundExpressionStatement>);

		void EmitConstant(shared_ptr<Binding::BoundConstant>);
		void EmitExpression(shared_ptr<Binding::BoundExpression>);
		void EmitCallExpression(shared_ptr<Binding::BoundCallExpression>);
		void EmitBinaryExpression(shared_ptr<Binding::BoundBinaryExpression>);
		void EmitVariableExpression(shared_ptr<Binding::BoundVariableExpression>);
	private:
		void CloseStreams();
	};
}