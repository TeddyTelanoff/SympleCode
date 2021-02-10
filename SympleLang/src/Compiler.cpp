#include "SympleCode/Compiler.h"

#include <spdlog/spdlog.h>

#include "SympleCode/Syntax/Lexer.h"
#include "SympleCode/Syntax/Parser.h"
#include "SympleCode/Binding/Binder.h"
#include "SympleCode/Emit/AsmEmitter.h"
#include "SympleCode/Util/ConsoleColor.h"

namespace Symple
{
	Compiler::Compiler(char *path)
		: mPath(path)
	{
		AsmPath = mPath.substr(0, mPath.find_last_of('.')) + ".S";
	}

	shared_ptr<DiagnosticBag> Compiler::Lex()
	{
		unique_ptr<Syntax::Lexer> lexer = make_unique<Syntax::Lexer>((char*)mPath.c_str());
		mTokens.clear();

		do
			mTokens.push_back(lexer->Lex());
		while (!mTokens.back()->Is(Syntax::Token::EndOfFile));

		if (PrintDiagnosticBag(lexer->GetDiagnosticBag(), "Lexing"))
			return lexer->GetDiagnosticBag();

#if __SY_DEBUG
		spdlog::debug("Lex Tokens:");
		for (auto tok : mTokens)
		{
			tok->Print(std::cout, "", tok->Is(Syntax::Token::EndOfFile));
			putchar('\n');
		}
#endif

		return lexer->GetDiagnosticBag();
	}

	shared_ptr<DiagnosticBag> Compiler::Parse()
	{
		unique_ptr<Syntax::Parser> parser = make_unique<Syntax::Parser>(mTokens);
		mAST = parser->Parse();
		putchar('\n');
		if (PrintDiagnosticBag(parser->GetDiagnosticBag(), "Parsing"))
			return parser->GetDiagnosticBag();

#if __SY_DEBUG
		spdlog::debug("Parse Tree:");
		mAST->Print();
		putchar('\n');
#endif

		return parser->GetDiagnosticBag();
	}

	shared_ptr<DiagnosticBag> Compiler::Bind()
	{
		shared_ptr<Binding::Binder> binder = make_shared<Binding::Binder>();
		mTree = binder->Bind(mAST);
		putchar('\n');
		if (PrintDiagnosticBag(binder->GetDiagnosticBag(), "Binding"))
			return binder->GetDiagnosticBag();

		spdlog::info("Bound Tree:");
		mTree->Print();
		putchar('\n');

		using namespace Symple::Util;
		ResetConsoleColor();

		return binder->GetDiagnosticBag();
	}

	shared_ptr<Binding::BoundCompilationUnit> Compiler::BindSymbols()
	{
		shared_ptr<Binding::Binder> binder = make_shared<Binding::Binder>();
		mTree = binder->BindSymbols(mAST);
		putchar('\n');
		if (PrintDiagnosticBag(binder->GetDiagnosticBag(), "Importing"))
			return mTree;

		spdlog::info("Bound Tree:");
		mTree->Print();
		putchar('\n');

		using namespace Symple::Util;
		ResetConsoleColor();

		return mTree;
	}

	void Compiler::Emit()
	{
		shared_ptr<Emit::AsmEmitter> emitter = make_shared<Emit::AsmEmitter>((char*)AsmPath.c_str());
		emitter->Emit(mTree);
		emitter->Compile();
	}
	
	int Compiler::Exec()
	{
		system(("clang -m32 --optimize -o sy/Main.exe " + AsmPath).c_str());
		puts("Executing program...");
		int ec = system("sy\\Main.exe");
		printf("\nProgram Exited with code %i (0x%x)", ec, ec);

		return ec;
	}

	bool Compiler::PrintDiagnosticBag(shared_ptr<DiagnosticBag> diagnostics, char *step)
	{
		using namespace Symple::Util;
		SetConsoleColor(Yellow);

		unsigned errCount = 0, warningCount = 0, messageCount = 0;

		for (shared_ptr<Diagnostic> diagnostic : diagnostics->GetDiagnostics())
			switch (diagnostic->GetLevel())
			{
			case Diagnostic::Message:
				spdlog::info("'{}' {}:{} \"{}\"", diagnostic->GetToken()->GetFile(), diagnostic->GetToken()->GetLine(), diagnostic->GetToken()->GetColumn(), diagnostic->GetMessage());
				messageCount++;
				break;
			case Diagnostic::Warning:
				spdlog::warn("'{}' {}:{} \"{}\"", diagnostic->GetToken()->GetFile(), diagnostic->GetToken()->GetLine(), diagnostic->GetToken()->GetColumn(), diagnostic->GetMessage());
				warningCount++;
				break;
			case Diagnostic::Error:
				spdlog::error("'{}' {}:{} \"{}\"", diagnostic->GetToken()->GetFile(), diagnostic->GetToken()->GetLine(), diagnostic->GetToken()->GetColumn(), diagnostic->GetMessage());
				errCount++;
				break;
			}

		if (errCount)
		{
			spdlog::info("{} failed with {} errors, {} warnings, {} messages", step, errCount, warningCount, messageCount);

			return true;
		}
		else
		{
			spdlog::info("{} completed with {} errors, {} warnings, {} messages", step, errCount, warningCount, messageCount);

			return false;
		}
	}
}