#include "Symple/Code/Visit/Visitor.h"

namespace Symple::Code
{
	TypeVisitor::TypeVisitor(GlobalRef<CompilationUnitAst> unit)
		: m_Unit(unit) {}

	void TypeVisitor::Visit()
	{
		for (auto member : m_Unit->m_Members)
		{
			if (member->Kind == AstKind::Function)
			{
				m_Func = Cast<FunctionAst>(member);
				Visit(m_Func->m_Body);
			}
		}
	}

	void TypeVisitor::Visit(GlobalRef<StatementAst> stmt)
	{
		switch (stmt->Kind)
		{
		case AstKind::BlockStatement:
			for (auto piece : Cast<BlockStatementAst>(stmt)->m_Stmts)
				Visit(piece);
			break;
		case AstKind::ReturnStatement:
		{
			auto RetStmt = Cast<ReturnStatementAst>(stmt);
			auto val = RetStmt->m_Value;
			Visit(val);

			auto cast = MakeRef<CastExpressionAst>(WeakRef<Token>(), m_Func->m_Type, WeakRef<Token>(), val);
			cast->m_Type = cast->m_TypeAst->m_Type;
			RetStmt->m_Value = cast;
			break;
		}
		case AstKind::ExpressionStatement:
			Visit(Cast<ExpressionStatementAst>(stmt)->m_Expr);
			break;
		}
	}

	void TypeVisitor::Visit(GlobalRef<ExpressionAst> expr)
	{
		switch (expr->Kind)
		{
		case AstKind::CastExpression:
			expr->m_Type = Cast<CastExpressionAst>(expr)->m_TypeAst->m_Type;
			break;
		case AstKind::CallExpression:
			expr->m_Type = Cast<CallExpressionAst>(expr)->m_Func->m_Type->m_Type;
			break;
		case AstKind::UnaryExpression:
			Visit(Cast<UnaryExpressionAst>(expr)->m_Operand);
			expr->m_Type = Cast<UnaryExpressionAst>(expr)->m_Operand->m_Type;
			break;
		case AstKind::BinaryExpression:
		{
			auto binExpr = Cast<BinaryExpressionAst>(expr);
			Visit(binExpr->m_Left);
			Visit(binExpr->m_Right);
			binExpr->m_Type = binExpr->m_Left->m_Type;
			break;
		}
		}
	}
}