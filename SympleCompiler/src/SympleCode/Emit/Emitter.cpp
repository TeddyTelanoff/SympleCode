#include "SympleCode/Emit/Emitter.h"

#include <Windows.h>

#define Emit(fmt, ...) fprintf(mFile, fmt "\n", __VA_ARGS__)
#define EmitResource(fmt, ...) fprintf_s(mResourceFile, fmt "\n", __VA_ARGS__)

namespace Symple
{
#if SY_32
	const char* const Emitter::sRegisters32[Emitter::NumRegisters] = { "%eax", "%edx", "%ecx", "%ebx", };
	const char* const Emitter::sRegisters16[Emitter::NumRegisters] = { "%ax", "%dx",  "%cx",  "%bx", };
	const char* const Emitter::sRegisters8[Emitter::NumRegisters] = { "%al", "%dl",  "%cl",  "%bl", };
#else
	const char* const Emitter::sRegisters64[Emitter::NumRegisters] = { "%rax", "%rdx", "%rcx", "%rbx", "%rdi", "%rsi",
		"%r8",  "%r9",  "%r10",  "%r11",  "%r12",  "%r13", };
	const char* const Emitter::sRegisters32[Emitter::NumRegisters] = { "%eax", "%edx", "%ecx", "%ebx", "%edi", "%esi",
		"%r8d", "%r9d", "%r10d", "%r11d", "%r12d", "%r13d", };
	const char* const Emitter::sRegisters16[Emitter::NumRegisters] = { "%ax", "%dx",  "%cx",  "%bx",  "%di",  "%si" ,
		"%r8w", "%r9w", "%r10w", "%r11w", "%r12w", "%r13w", };
	const char* const Emitter::sRegisters8[Emitter::NumRegisters] = { "%al", "%dl",  "%cl",  "%bl",  "%dil", "%sil",
		"%r8b", "%r9b", "%r10b", "%r11b", "%r12b", "%r13b", };
#endif

	unsigned int Emitter::sInits = 0;

	Emitter::Emitter(const char* path)
		: mPath(path), mFile(), mResourceFile(), mData(), mReturn(), mStack(), mReturning()
	{
		while (OpenFile());
		while (OpenResourceFile());
	}

	Emitter::~Emitter()
	{
		rewind(mResourceFile);

		char c;
		while ((c = getc(mResourceFile)) != EOF)
			putc(c, mFile);

		fclose(mResourceFile);
		fclose(mFile);
	}

	void Emitter::EmitCompilationUnit(const CompilationUnitNode* unit)
	{
		for (const MemberNode* member : unit->GetMembers())
			EmitMember(member);
	}

	void Emitter::EmitStaticInitialization()
	{
		Emit(".global  ._Sy@StaticInitialization_.");
		Emit("._Sy@StaticInitialization_.:");

		for (unsigned int i = 0; i < sInits; i++)
			Emit("\tcall%c   ..%i.", Suf(), i);

		Emit("\txor%c    %s, %s", Suf(), GetReg(regax), GetReg(regax));
		Emit("\tret");
	}


	Register Emitter::AllocReg(Register reg)
	{
		if (reg != nullreg && mFreeRegisters[reg])
		{
			mFreeRegisters[reg] = false;
			return reg;
		}

		for (reg = 0; reg < NumRegisters; reg++)
			if (mFreeRegisters[reg])
			{
				mFreeRegisters[reg] = false;
				return reg;
			}

		abort();
	}

	void Emitter::FreeReg(Register reg)
	{
		if (reg == nullreg)
			return;

		if (mFreeRegisters[reg])
			abort();

		mFreeRegisters[reg] = true;
	}

	void Emitter::FreeAllRegs()
	{
		for (int i = 0; i < NumRegisters; i++)
			mFreeRegisters[i] = true;
	}

	const char* Emitter::GetReg(Register reg, int sz)
	{
		if (reg == nullreg)
			return nullptr;

		if (reg == regsp)
		{
			if (sz <= 2)
				return "%sp";
			if (sz <= 4)
				return "%esp";
#if SY_64
			if (sz <= 8)
				return "%rsp";
#endif
		}

		if (reg == regbp)
		{
			if (sz <= 2)
				return "%bp";
			if (sz <= 4)
				return "%ebp";
#if SY_64
			if (sz <= 8)
				return "%rbp";
#endif
		}

		if (sz <= 1)
			return sRegisters8[reg];
		if (sz <= 2)
			return sRegisters16[reg];
		if (sz <= 4)
			return sRegisters32[reg];
#if SY_64
		if (sz <= 8)
			return sRegisters64[reg];
#endif

		return nullptr;
	}


	char Emitter::Suf(int sz)
	{
		if (sz <= 1)
			return 'b';
		if (sz <= 2)
			return 'w';
		if (sz <= 4)
			return 'l';
#if SY_64
		if (sz <= 8)
			return 'q';
#endif

		return 0;
	}

	const char* Emitter::Word(int sz)
	{
		if (sz <= 1)
			return "byte";
		if (sz <= 2)
			return "value";
		if (sz <= 4)
			return "long";
#if SY_64
		if (sz <= 8)
			return "quad";
#endif

		return nullptr;
	}


	void Emitter::Push(Register reg)
	{
		Emit("\tpush    %s", GetReg(reg));
	}

	void Emitter::Pop(Register reg)
	{
		Emit("\tpop     %s", GetReg(reg));
	}


	void Emitter::Loc(const Token* tok)
	{
		Emit(".loc 1 %i %i", tok->GetLine(), tok->GetColumn());
	}


	void Emitter::EmitMember(const MemberNode* member)
	{
		FreeAllRegs(); // In Case I forgot to Free a Register

		switch (member->GetKind())
		{
		case Node::Kind::GlobalStatement:
			return EmitGlobalStatement(member->Cast<GlobalStatementNode>());
		case Node::Kind::FunctionDeclaration:
			return EmitFunctionDeclaration(member->Cast<FunctionDeclarationNode>());
		}
	}

	void Emitter::EmitGlobalStatement(const GlobalStatementNode* member)
	{
		EmitStatement(member->GetStatement());
	}

	void Emitter::EmitFunctionDeclaration(const FunctionDeclarationNode* member)
	{
		mStack ^= mStack; // Reset stack

		const char* name = member->GetAsmName().c_str();

		if (!member->GetModifiers()->IsPrivate())
			Emit("\t.global   %s", name);
		Emit("%s:", name);

		Push(regbp);
		Emit("\tmov     %s, %s", GetReg(regsp), GetReg(regbp));

		if (member->IsMain())
			Emit("\tcall    ._Sy@StaticInitialization_.");

		Debug::BeginScope();

		mReturning = false;
		for (const StatementNode* statement : member->GetBody()->GetStatements())
			if (statement->IsReturn() == 1 && statement != member->GetBody()->GetStatements().back())
			{
				mReturning = true;
				break;
			}

		if (mReturning)
			mReturn = mData++;

		if (member->GetBody()->GetStackUsage())
			Emit("\tsub%c    $%i, %s", Suf(), member->GetBody()->GetStackUsage(), GetReg(regsp));

		for (const StatementNode* statement : member->GetBody()->GetStatements())
		{
			EmitStatement(statement);
			if (statement->IsReturn() == 2)
				break;
		}

		Debug::EndScope();

		if (mReturning)
			Emit("..%i:", mReturn);

		Emit("\tmov     %s, %s", GetReg(regbp), GetReg(regsp));
		Pop(regbp);
		Emit("\tret");
	}


	void Emitter::EmitStatement(const StatementNode* statement)
	{
		FreeAllRegs(); // In Case I forgot to Free a Register

		switch (statement->GetKind())
		{
		case Node::Kind::ExpressionStatement:
			return EmitExpressionStatement(statement->Cast<ExpressionStatementNode>());
		case Node::Kind::VariableDeclaration:
			return EmitVariableDeclaration(statement->Cast<VariableDeclarationNode>());
		}
	}

	void Emitter::EmitExpressionStatement(const ExpressionStatementNode* statement)
	{
		Register reg = EmitExpression(statement->GetExpression());
		if (reg != regax)
			Emit("\tmov     %s, %s", GetReg(reg), GetReg(regax));

		FreeAllRegs(); // In Case I forgot to Free a Register
	}

	void Emitter::EmitVariableDeclaration(const VariableDeclarationNode* statement)
	{
		Debug::VariableDeclaration(statement);

		std::string nameStr(statement->GetName()->GetLex());
		const char* name = nameStr.c_str();
		unsigned int depth = Debug::GetDepth();
		unsigned int sz = statement->GetType()->GetSize();

		Emit("_%s$%i = -%i", name, depth, mStack += sz);

		if (statement->GetInitializer())
		{
			if (statement->GetInitializer()->CanEvaluate())
			{
				Emit("\tmov%c    $%i, _%s$%i(%s)", Suf(sz), statement->GetInitializer()->Evaluate(), name, depth, GetReg(regbp));
			}
			else
			{
				Register init = EmitExpression(statement->GetInitializer());
				Emit("\tmov     %s, _%s$%i(%s)", GetReg(init), name, depth, GetReg(regbp));
				FreeReg(init);
			}
		}
	}


	Register Emitter::EmitExpression(const ExpressionNode* expression)
	{
		if (expression->CanEvaluate())
		{
			Register reg = AllocReg();
			if (expression->Evaluate())
				Emit("\tmov     $%i, %s", expression->Evaluate(), GetReg(reg));
			else
				Emit("\txor     %s, %s", GetReg(reg), GetReg(reg));
			return reg;
		}

		if (expression->Is<ModifiableExpressionNode>())
			return EmitModifiableExpression(expression->Cast<ModifiableExpressionNode>());

		switch (expression->GetKind())
		{
		case Node::Kind::CastExpression:
			return EmitCastExpression(expression->Cast<CastExpressionNode>());
		case Node::Kind::StallocExpression:
			return EmitStallocExpression(expression->Cast<StallocExpressionNode>());
		case Node::Kind::FunctionCallExpression:
			return EmitFunctionCallExpression(expression->Cast<FunctionCallExpressionNode>());
		case Node::Kind::StringLiteralExpression:
			return EmitStringLiteralExpression(expression->Cast<StringLiteralExpressionNode>());
		case Node::Kind::VariableAddressExpression:
			return EmitVariableAddressExpression(expression->Cast<VariableAddressExpressionNode>());
		}

		return nullreg;
	}

	Register Emitter::EmitCastExpression(const CastExpressionNode* expression)
	{
		return EmitExpression(expression->GetExpression());
	}

	Register Emitter::EmitStallocExpression(const StallocExpressionNode* expression)
	{
		Register reg;

		if (expression->GetSize()->CanEvaluate())
		{
			reg = AllocReg();
			Emit("\tsub     $%i, %s", expression->GetSize()->Evaluate(), GetReg(regsp));
		}
		else
		{
			reg = EmitExpression(expression->GetSize());
			Emit("\tsub     %s, %s", GetReg(reg), GetReg(regsp));
		}

		Emit("\tmov     %s, %s", GetReg(regsp), GetReg(reg));
		return reg;
	}

	Register Emitter::EmitFunctionCallExpression(const FunctionCallExpressionNode* expression)
	{
		bool axUsed = !mFreeRegisters[regax];
		if (axUsed)
			Push(regax);

		for (unsigned int i = expression->GetArguments()->GetArguments().size(); i > 0; i--)
		{
			Register reg = EmitExpression(expression->GetArguments()->GetArguments()[i - 1]);
			Push(reg);
			FreeReg(reg);
		}

		Emit("\tcall    %s", Debug::GetFunction(expression->GetName()->GetLex(), expression->GetArguments())->GetAsmName().c_str());
		Emit("\tadd     $%i, %s", expression->GetArguments()->GetArguments().size() * platsize, GetReg(regsp));
		Register reg = AllocReg(regax);
		if (reg != regax)
			Emit("\tmov     %s, %s", GetReg(regax), GetReg(reg));

		if (axUsed)
			Pop(regax);

		return reg;
	}

	Register Emitter::EmitVariableAddressExpression(const VariableAddressExpressionNode* expression)
	{
		return EmitModifiableExpression(expression->GetVariable(), true);
	}


	Register Emitter::EmitModifiableExpression(const ModifiableExpressionNode* expression, bool retptr)
	{
		switch (expression->GetKind())
		{
		case Node::Kind::VariableExpression:
			return EmitVariableExpression(expression->Cast<VariableExpressionNode>(), retptr);
		}

		return nullreg;
	}

	Register Emitter::EmitVariableExpression(const VariableExpressionNode* expression, bool retptr)
	{
		const VariableDeclaration* decl = Debug::GetVariable(expression->GetName()->GetLex());

		std::string nameStr(decl->GetName()->GetLex());
		const char* name = nameStr.c_str();
		unsigned int depth = Debug::GetVariableDepth(expression->GetName()->GetLex());
		unsigned int sz = decl->GetType()->GetSize();

		Register reg = AllocReg();
		if (retptr)
			Emit("\tlea     _%s$%i(%s), %s", name, depth, GetReg(regbp), GetReg(reg));
		else
			Emit("\tmov     _%s$%i(%s), %s", name, depth, GetReg(regbp), GetReg(reg, sz));

		return reg;
	}


	Register Emitter::EmitStringLiteralExpression(const StringLiteralExpressionNode* literal)
	{
		EmitResource("..%i:", mData);
		EmitResource("\t.string \"%s\"", literal->Stringify().c_str());

		Register reg = AllocReg();
		Emit("\tmov     $..%i, %s", mData++, GetReg(reg));
		return reg;
	}


	bool Emitter::OpenFile()
	{
		errno_t err = fopen_s(&mFile, mPath, "w");
		if (err && !mFile)
		{
			char msg[32];
			if (!strerror_s(msg, err))
			{
				char realMsg[56];
				sprintf_s(realMsg, "[!]: Error opening temp file: %.25s", msg);
				MessageBeep(MB_ICONSTOP);
				int instruct = MessageBoxA(NULL, realMsg, "Error Opening File!", MB_ABORTRETRYIGNORE);
				if (instruct == IDABORT)
					exit(IDABORT);
				else if (instruct == IDIGNORE)
					exit(IDIGNORE);
				else if (instruct == IDRETRY)
					return true;
			}
			else
			{
				char msg[37] = "[!]: Unknown error opening temp file";
				MessageBeep(MB_ICONSTOP);
				int instruct = MessageBoxA(NULL, msg, "Error Opening File!", MB_ABORTRETRYIGNORE);
				if (instruct == IDABORT)
					exit(IDABORT);
				else if (instruct == IDIGNORE)
					exit(IDIGNORE);
				else if (instruct == IDRETRY)
					return true;
			}
		}
		else
		{
			return false;
		}

		return true;
	}

	bool Emitter::OpenResourceFile()
	{
		errno_t err = tmpfile_s(&mResourceFile);
		if (err && !mResourceFile)
		{
			char msg[32];
			if (!strerror_s(msg, err))
			{
				char realMsg[56];
				sprintf_s(realMsg, "[!]: Error opening temp file: %.25s", msg);
				MessageBeep(MB_ICONSTOP);
				int instruct = MessageBoxA(NULL, realMsg, "Error Opening File!", MB_ABORTRETRYIGNORE);
				if (instruct == IDABORT)
					exit(IDABORT);
				else if (instruct == IDIGNORE)
					exit(IDIGNORE);
				else if (instruct == IDRETRY)
					return true;
			}
			else
			{
				char msg[37] = "[!]: Unknown error opening temp file";
				MessageBeep(MB_ICONSTOP);
				int instruct = MessageBoxA(NULL, msg, "Error Opening File!", MB_ABORTRETRYIGNORE);
				if (instruct == IDABORT)
					exit(IDABORT);
				else if (instruct == IDIGNORE)
					exit(IDIGNORE);
				else if (instruct == IDRETRY)
					return true;
			}
		}
		else
		{
			return false;
		}

		return true;
	}
}