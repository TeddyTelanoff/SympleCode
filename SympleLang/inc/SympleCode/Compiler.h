#pragma once

#include <string_view>

#include "SympleCode/DiagnosticBag.h"
#include "SympleCode/Syntax/Token.h"
#include "SympleCode/Syntax/TranslationUnitSyntax.h"
#include "SympleCode/Binding/Binder.h"
#include "SympleCode/Binding/BoundCompilationUnit.h"

namespace Symple
{
	class Compiler
	{
	private:
		std::string mPath, mAsmPath;
		Syntax::TokenList mTokens;
		shared_ptr<Syntax::TranslationUnitSyntax> mAST;
		shared_ptr<Binding::BoundCompilationUnit> mTree;
		std::vector<shared_ptr<Compiler>> mUnits;
		static std::vector<std::string> sLibraries;

		friend class Binding::Binder;
	public:
		Compiler(char *path);

		shared_ptr<DiagnosticBag> Lex();
		shared_ptr<DiagnosticBag> Parse();
		shared_ptr<DiagnosticBag> Bind();
		shared_ptr<Binding::BoundCompilationUnit> BindSymbols();
		void Emit();
		int Exec();

		/// <summary>
		/// Print Diagnostics
		/// </summary>
		/// <param name="diagnostics">Diagnostic Bag</param>
		/// <param name="step">Step in witch diagnostics are for</param>
		/// <returns>Returns true if there is an error</returns>
		bool PrintDiagnosticBag(shared_ptr<DiagnosticBag> diagnostics, char step[] = "Null Step");
	};
}