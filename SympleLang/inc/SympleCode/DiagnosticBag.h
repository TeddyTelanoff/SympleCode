#pragma once

#include <vector>

#include "SympleCode/Diagnostic.h"

#include "SympleCode/Syntax/Token.h"
#include "SympleCode/Syntax/CallExpressionSyntax.h"
#include "SympleCode/Syntax/LiteralExpressionSyntax.h"

#include "SympleCode/Symbol/TypeSymbol.h"

namespace Symple
{
	class DiagnosticBag
	{
	private:
		std::vector<shared_ptr<Diagnostic>> mDiagnostics;
	public:
		void ReportMessage(shared_ptr<Syntax::Token>, std::string_view msg);
		void ReportWarning(shared_ptr<Syntax::Token>, std::string_view msg);
		void ReportError(shared_ptr<Syntax::Token>, std::string_view msg);

		std::vector<shared_ptr<Diagnostic>>& GetDiagnostics();

		void ReportUnimplimentedMessage(shared_ptr<Syntax::Token>);
		void ReportUnimplimentedWarning(shared_ptr<Syntax::Token>);
		void ReportUnimplimentedError(shared_ptr<Syntax::Token>);

		void ReportBindError(shared_ptr<Syntax::Node>);
		void ReportExpressionMustHaveValue(shared_ptr<Syntax::ExpressionSyntax>);

		void ReportNoDefaultArgument(shared_ptr<Syntax::CallExpressionSyntax>, unsigned paramIndex);
		void ReportTooFewArguments(shared_ptr<Syntax::CallExpressionSyntax>);
		void ReportTooManyArguments(shared_ptr<Syntax::CallExpressionSyntax>, unsigned paramIndex);

		void ReportUnexpectedEndOfFile(shared_ptr<Syntax::Token>);
		void ReportUnexpectedToken(shared_ptr<Syntax::Token>, Syntax::Token::Kind expectedKind);
		void ReportUnknownToken(shared_ptr<Syntax::Token>);

		void ReportInvalidOperation(shared_ptr<Syntax::Token>, shared_ptr<Symbol::TypeSymbol> left, shared_ptr<Symbol::TypeSymbol> right);
		void ReportInvalidOperation(shared_ptr<Syntax::Token>, shared_ptr<Symbol::TypeSymbol>);

		void ReportExpectedUnqualifiedID(shared_ptr<Syntax::Token>);
		void ReportInvalidLiteral(shared_ptr<Syntax::LiteralExpressionSyntax>);
	};
}