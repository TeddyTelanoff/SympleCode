#include "Symple/Code/AST/AST.h"

namespace Symple::Code
{
	using Token_t = AST::Token_t;

	ASTKind AST::GetKind() const
	{ return ASTKind::Unknown; }

	WeakRef<const Token_t> AST::GetToken() const
	{ return std::move(WeakRef<const Token_t>()); }

	std::ostream &operator <<(std::ostream &os, const AST &ast)
	{ return os << ASTKindNames[(uint32)ast.Kind] << "Ast [Token]: " << ast.Token; }

	std::ostream &operator <<(std::ostream &os, const GlobalRef<AST> &ast)
	{
		if (ast.get())
			return os << *ast.get();
		else
			return os << "Null ast";
	}

	std::ostream &operator <<(std::ostream &os, const WeakRef<AST> &ast)
	{ return os << ast; }

	std::ostream &operator <<(std::ostream &os, const Scope<AST> &ast)
	{ return os << *ast.get(); }


	CompilationUnitAST::CompilationUnitAST(const MemberList &members, const WeakRef<const Token_t> &eof)
		: m_Members(members), m_EndOfFile(eof) {}

	ASTKind CompilationUnitAST::GetKind() const
	{ return ASTKind::CompilationUnit; }

	WeakRef<const Token_t> CompilationUnitAST::GetToken() const
	{ return m_EndOfFile; }


	const MemberList &CompilationUnitAST::GetMembers() const
	{ return m_Members; }

	WeakRef<const Token_t> CompilationUnitAST::GetEndOfFile() const
	{ return m_EndOfFile; }
}