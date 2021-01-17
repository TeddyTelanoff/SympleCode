#include "SympleCode/Parser/Parser.h"

#include <xhash>
#include <iostream>

#include "SympleCode/Node/Statement/BreakStatementNode.h"
#include "SympleCode/Node/Statement/ExpressionStatementNode.h"

#include "SympleCode/Node/Expression/Operator/UnaryExpressionNode.h"
#include "SympleCode/Node/Expression/Operator/BinaryExpressionNode.h"

#include "SympleCode/Node/Expression/Literal/NullLiteralExpressionNode.h"
#include "SympleCode/Node/Expression/Literal/StringLiteralExpressionNode.h"
#include "SympleCode/Node/Expression/Literal/NumberLiteralExpressionNode.h"
#include "SympleCode/Node/Expression/Literal/BooleanLiteralExpressionNode.h"
#include "SympleCode/Node/Expression/Literal/CharacterLiteralExpressionNode.h"

#include "SympleCode/Node/Expression/Modifiable/AssignmentExpressionNode.h"

#include "SympleCode/Analysis/Debug.h"
#include "SympleCode/Analysis/Diagnostics.h"

#include "SympleCode/Common/Priority.h"

namespace Symple
{
	Parser::Parser(const char* source, const char* file)
		: mPreprocessor(source, file), mPosition(0), mTokens(mPreprocessor.GetTokens())
	{
		for (auto tok : mTokens)
			if (tok->Is(Token::Kind::Unknown))
				Diagnostics::ReportError(tok, "Unknown Token!");
		//	std::cout << Token::KindString(tok->GetKind()) << '#' << (int)tok->GetKind() << '|' << tok->GetLex() << '\n';
	}

	CompilationUnitNode* Parser::ParseCompilationUnit()
	{
		auto members = ParseMembers();
		Match(Token::Kind::EndOfFile);
		return new CompilationUnitNode(members);
	}

	const Token* Parser::Peek(unsigned int offset) const
	{
		unsigned int i = mPosition + offset;
		if (i >= mTokens.size())
			return mTokens.back();
		return mTokens[i];
	}

	const Token* Parser::Next()
	{
		const Token* current = Peek();
		mPosition++;
		return current;
	}

	const Token* Parser::Match(Token::Kind kind)
	{
		if (Peek()->Is(kind))
			return Next();
		Diagnostics::ReportError(Peek(), "Unexpected Token '%s' of type <%s>, Expected <%s>", std::string(Peek()->GetLex()).c_str(), Token::KindString(Peek()->GetKind()), Token::KindString(kind));
		Next();
		return new Token(kind);
	}

	bool Parser::IsType(const Token* token)
	{
		for (const Type* type : Debug::GetTypes())
			if (token->GetLex() == type->GetName())
				return true;
		return false;
	}

	const Type* Parser::GetType(const Token* token)
	{
		for (const Type* type : Debug::GetTypes())
			if (token->GetLex() == type->GetName())
				return type;
		return nullptr;
	}

	bool Parser::IsTypeNodeable(const Token* token)
	{
		return IsType(token) || TypeModifierNode::IsValid(token->GetKind());
	}

	bool Parser::IsTypeContinue(const Token* token)
	{
		return token->IsEither({ Token::Kind::Asterisk });
	}

	TypeNode* Parser::ParseType(const Type* type)
	{
		std::vector<const TypeModifierNode*> modifiers;
		const TypeContinueNode* contjnue = nullptr;

		while (IsTypeNodeable(Peek()))
		{
			if (IsType(Peek()))
			{
				if (type)
					Diagnostics::ReportError(Next(), "Type Already Specified");
				else
					type = GetType(Next());
				continue;
			}

			modifiers.push_back(new TypeModifierNode(Next()));
		}

		if (IsTypeContinue(Peek()))
			contjnue = ParseTypeContinue();

		if (type)
			return new TypeNode(type, new TypeModifiersNode(modifiers), contjnue);

		return new TypeNode(Type::PrimitiveType::Error, new TypeModifiersNode({}), contjnue);
	}

	TypeContinueNode* Parser::ParseTypeContinue()
	{
		const Token* type = Next();
		std::vector<const TypeModifierNode*> modifiers;
		const TypeContinueNode* contjnue = nullptr;

		while (IsTypeNodeable(Peek()))
		{
			if (IsType(Peek()))
			{
				Diagnostics::ReportError(Next(), "Type Already Specified");
				continue;
			}

			if (IsTypeContinue(Peek()))
			{
				contjnue = ParseTypeContinue();

				break;
			}

			modifiers.push_back(new TypeModifierNode(Next()));
		}

		if (IsTypeContinue(Peek()))
			contjnue = ParseTypeContinue();

		return new TypeContinueNode(type, new TypeModifiersNode({ modifiers }), contjnue);
	}

	const std::vector<const MemberNode*> Parser::ParseMembers()
	{
		std::vector<const MemberNode*> members;
		while (!Peek()->Is(Token::Kind::EndOfFile))
		{
			const Token* start = Peek();
			members.push_back(ParseMember());

			if (start == Peek())
				Next();
		}

		return members;
	}

	MemberNode* Parser::ParseMember()
	{
		if (IsTypeNodeable(Peek()))
		{
			unsigned int pPosition = mPosition;
			ParseType();
			bool isFunction = Peek(1)->Is(Token::Kind::OpenParenthesis);
			bool isVariable = !Peek(1)->Is(Token::Kind::OpenBrace);
			mPosition = pPosition;
			if (isFunction)
				return ParseFunctionDeclaration();
			if (isVariable)
				return ParseGlobalVariableDeclaration();
		}
		if (Peek()->Is(Token::Kind::Hint))
			return ParseFunctionHint();
		if (Peek()->Is(Token::Kind::Extern))
			return ParseExternFunction();
		if (Peek()->Is(Token::Kind::Shared))
			return ParseSharedVariable();
		if (Peek()->Is(Token::Kind::Struct))
			return ParseStructDeclaration();
		if (Peek()->Is(Token::Kind::Enum))
			return ParseEnumDeclaration();

		return ParseGlobalStatement();
	}

	FunctionDeclarationNode* Parser::ParseFunctionDeclaration()
	{
		const TypeNode* type = ParseType();
		const Token* name = Next();

		Debug::BeginScope();
		FunctionArgumentsNode* arguments = ParseFunctionArguments();
		FunctionModifiersNode* modifiers = ParseFunctionModifiers();
		BlockStatementNode* body = ParseBlockStatement();
		Debug::EndScope();

		FunctionDeclarationNode* declaration = new FunctionDeclarationNode(type, name, arguments, modifiers, body);
		Debug::FunctionDeclaration(declaration);
		return declaration;
	}

	FunctionArgumentsNode* Parser::ParseFunctionArguments()
	{
		const Token* open = Match(Token::Kind::OpenParenthesis);

		std::vector<const FunctionArgumentNode*> arguments;
		while (!Peek()->Is(Token::Kind::CloseParenthesis))
		{
			if (Peek()->Is(Token::Kind::EndOfFile))
			{
				Diagnostics::ReportError(Next(), "Unexpected End Of File");
				break;
			}

			arguments.push_back(ParseFunctionArgument());

			if (Peek()->Is(Token::Kind::Comma))
				Next();
			else
			{
				if (!Peek()->Is(Token::Kind::CloseParenthesis))
					Diagnostics::ReportError(Peek(), "Expected Comma");
				break;
			}
		}

		const Token* close = Match(Token::Kind::CloseParenthesis);

		return new FunctionArgumentsNode(open, arguments, close);
	}

	FunctionModifiersNode* Parser::ParseFunctionModifiers()
	{
		std::vector<const FunctionModifierNode*> modifiers;
		while (FunctionModifierNode::IsValid(Peek()->GetKind()))
			modifiers.push_back(ParseFunctionModifier());

		return new FunctionModifiersNode(modifiers);
	}

	FunctionModifierNode* Parser::ParseFunctionModifier()
	{
		return new FunctionModifierNode(Next());
	}

	FunctionArgumentNode* Parser::ParseFunctionArgument()
	{
		const TypeNode* type = ParseType();
		const Token* name = Token::Default;
		if (Peek()->Is(Token::Kind::Identifier))
			name = Next();

		FunctionArgumentNode* argument = new FunctionArgumentNode(type, name, ParseVariableModifiers());
		Debug::VariableDeclaration(argument);
		return argument;
	}

	SharedVariableNode* Parser::ParseSharedVariable(const Type* type)
	{
		if (!type)
		{
			Match(Token::Kind::Shared);
			type = GetType(Next());
		}
		const TypeNode* ty = ParseType(type);
		const Token* name = Match(Token::Kind::Identifier);

		VariableModifiersNode* modifiers = ParseVariableModifiers();

		SharedVariableNode* declaration = nullptr;
		SharedVariableNode* next = nullptr;

		if (Peek()->Is(Token::Kind::Comma))
		{
			Next();
			next = ParseSharedVariable(type);
		}

		Match(Token::Kind::Semicolon);

		declaration = new SharedVariableNode(name, ty, modifiers, next);
		Debug::VariableDeclaration(declaration);
		return declaration;
	}

	FunctionHintNode* Parser::ParseFunctionHint()
	{
		Match(Token::Kind::Hint);
		const TypeNode* type = ParseType();
		const Token* name = Next();
		Debug::BeginScope();
		FunctionArgumentsNode* arguments = ParseFunctionArguments();
		Debug::EndScope();
		FunctionModifiersNode* modifiers = ParseFunctionModifiers();
		Match(Token::Kind::Semicolon);

		FunctionHintNode* hint = new FunctionHintNode(type, name, arguments, modifiers);
		Debug::FunctionDeclaration(hint);
		return hint;
	}

	GlobalVariableDeclarationNode* Parser::ParseGlobalVariableDeclaration(const Type* type)
	{
		if (!type)
			type = GetType(Next());
		const TypeNode* ty = ParseType(type);
		const Token* name = Match(Token::Kind::Identifier);

		VariableModifiersNode* modifiers = ParseVariableModifiers();

		GlobalVariableDeclarationNode* declaration = nullptr;
		if (Peek()->Is(Token::Kind::Equal))
		{
			Next();
			ExpressionNode* expression = ParseExpression();

			GlobalVariableDeclarationNode* next = nullptr;

			if (Peek()->Is(Token::Kind::Comma))
			{
				Next();
				next = ParseGlobalVariableDeclaration(type);
			}

			Match(Token::Kind::Semicolon);

			declaration = new GlobalVariableDeclarationNode(name, ty, modifiers, expression, next);
			Debug::VariableDeclaration(declaration);
			return declaration;
		}

		GlobalVariableDeclarationNode* next = nullptr;

		if (Peek()->Is(Token::Kind::Comma))
		{
			Next();
			next = ParseGlobalVariableDeclaration(type);
		}

		Match(Token::Kind::Semicolon);

		declaration = new GlobalVariableDeclarationNode(name, ty, modifiers, nullptr, next);
		Debug::VariableDeclaration(declaration);
		return declaration;
	}

	StructDeclarationNode* Parser::ParseStructDeclaration()
	{
		Match(Token::Kind::Struct);
		const Token* name = Match(Token::Kind::Identifier);
		Match(Token::Kind::OpenBrace);
		FieldListNode* fields = ParseFieldList();
		Match(Token::Kind::CloseBrace);
		Match(Token::Kind::Semicolon);
		
		StructDeclarationNode* declaration = new StructDeclarationNode(name, fields);
		Debug::StructDeclaration(declaration);
		return declaration;
	}

	FieldListNode* Parser::ParseFieldList()
	{
		const TypeNode* type = nullptr;
		std::vector<const VariableDeclarationNode*> fields;
		while (IsTypeNodeable(Peek()))
		{
			VariableDeclarationNode* field = ParseField();
			fields.push_back(field);

			const Type* ty = field->GetType()->GetType();
			while (Peek()->Is(Token::Kind::Comma))
			{
				Next();
				fields.push_back(ParseField(ty));
			}
			Match(Token::Kind::Semicolon);
		}

		return new FieldListNode(fields);
	}

	VariableDeclarationNode* Parser::ParseField(const Type* type)
	{
		if (!type)
			type = GetType(Next());
		TypeNode* ty = ParseType(type);
		const Token* name = Next();

		VariableModifiersNode* modifiers = ParseVariableModifiers();

		if (Peek()->Is(Token::Kind::Equal))
		{
			Next();
			ExpressionNode* expression = ParseExpression();

			return new VariableDeclarationNode(name, ty, modifiers, expression, nullptr);
		}

		return new VariableDeclarationNode(name, ty, modifiers, nullptr, nullptr);
	}

	ExternFunctionNode* Parser::ParseExternFunction()
	{
		Match(Token::Kind::Extern);
		const TypeNode* type = ParseType();
		const Token* name = Next();
		Debug::BeginScope();
		FunctionArgumentsNode* arguments = ParseFunctionArguments();
		Debug::EndScope();
		FunctionModifiersNode* modifiers = ParseFunctionModifiers();
		Match(Token::Kind::Semicolon);

		ExternFunctionNode* exjern = new ExternFunctionNode(type, name, arguments, modifiers);
		Debug::FunctionDeclaration(exjern);
		return exjern;
	}

	StatementNode* Parser::ParseStatement(bool matchSemicolon)
	{
		if (Peek()->Is(Token::Kind::Semicolon))
			return new StatementNode; // Empty Statement;
		if (Peek()->Is(Token::Kind::Return))
			return ParseReturnStatement(matchSemicolon);
		if (Peek()->Is(Token::Kind::While))
			return ParseWhileStatement(matchSemicolon);
		if (Peek()->Is(Token::Kind::For))
			return ParseForLoopStatement(matchSemicolon);
		if (Peek()->Is(Token::Kind::If))
			return ParseIfStatement(matchSemicolon);
		if (Peek()->Is(Token::Kind::Else))
		{
			Diagnostics::ReportError(Next(), "Else Statement Without Matching If");
			return new StatementNode;
		}
		if (Peek()->Is(Token::Kind::Asm))
			return ParseAsmStatement(matchSemicolon);
		if (Peek()->Is(Token::Kind::Break))
			return ParseBreakStatement(matchSemicolon);
		if (Peek()->Is(Token::Kind::OpenBrace))
			return ParseBlockStatement(matchSemicolon);
		if (IsTypeNodeable(Peek()))
			return ParseVariableDeclaration();

		ExpressionNode* expression = ParseExpression();
		if (matchSemicolon)
			Match(Token::Kind::Semicolon);
		return new ExpressionStatementNode(expression);
	}

	IfStatementNode* Parser::ParseIfStatement(bool matchSemicolon)
	{
		const Token* open = Match(Token::Kind::If);
		ExpressionNode* condition = ParseExpression();
		BlockStatementNode* then  = ParseBlockStatement(false);
		BlockStatementNode* elze = nullptr;
		if (Peek()->Is(Token::Kind::Else))
		{
			Next();
			elze = ParseBlockStatement(matchSemicolon);
		}
		else if (matchSemicolon && then->GetOpen() != then->GetClose())
			Match(Token::Kind::Semicolon);

		return new IfStatementNode(open, condition, then, elze);
	}

	AsmStatementNode* Parser::ParseAsmStatement(bool matchSemicolon)
	{
		Match(Token::Kind::Asm);
		const Token* instructions = Match(Token::Kind::String);
		if (matchSemicolon)
			Match(Token::Kind::Semicolon);

		return new AsmStatementNode(instructions);
	}

	BreakStatementNode* Parser::ParseBreakStatement(bool matchSemicolon)
	{
		const Token* dreak = Match(Token::Kind::Break);
		BreakStatementNode* statement = new BreakStatementNode(dreak);
		if (matchSemicolon)
			Match(Token::Kind::Semicolon);

		return statement;
	}

	WhileStatementNode* Parser::ParseWhileStatement(bool matchSemicolon)
	{
		const Token* open = Match(Token::Kind::While);
		ExpressionNode* condition = ParseExpression();
		BlockStatementNode* body = ParseBlockStatement(matchSemicolon);

		return new WhileStatementNode(open, condition, body);
	}

	BlockStatementNode* Parser::ParseBlockStatement(bool matchSemicolon)
	{
		Debug::BeginScope();
		if (!Peek()->Is(Token::Kind::OpenBrace))
		{
			StatementNode* statement = ParseStatement();
			Debug::EndScope();
			return new BlockStatementNode(Peek(), { statement }, Peek());
		}
		const Token* open = Match(Token::Kind::OpenBrace);
		std::vector<const StatementNode*> statements;
		while (!Peek()->Is(Token::Kind::CloseBrace))
		{
			if (Peek()->Is(Token::Kind::EndOfFile))
			{
				Diagnostics::ReportError(Next(), "Unexpected End Of File");
				break;
			}

			const Token* start = Peek();

			statements.push_back(ParseStatement());

			if (start == Peek())
				Next();
		}
		const Token* close = Match(Token::Kind::CloseBrace);
		if (matchSemicolon)
			Match(Token::Kind::Semicolon);
		Debug::EndScope();

		return new BlockStatementNode(open, statements, close);
	}

	ReturnStatementNode* Parser::ParseReturnStatement(bool matchSemicolon)
	{
		Match(Token::Kind::Return);
		ExpressionNode* expression = ParseExpression();
		if (matchSemicolon)
			Match(Token::Kind::Semicolon);
		return new ReturnStatementNode(expression);
	}

	GlobalStatementNode* Parser::ParseGlobalStatement(bool matchSemicolon)
	{
		return new GlobalStatementNode(ParseStatement());
	}

	ForLoopStatementNode* Parser::ParseForLoopStatement(bool matchSemicolon)
	{
		const Token* open = Match(Token::Kind::For);

		bool parentithized = Peek()->Is(Token::Kind::OpenParenthesis);
		if (parentithized)
			Next();

		Debug::BeginScope();
		StatementNode* initializer = ParseStatement();
		ExpressionNode* condition = ParseExpression();
		Match(Token::Kind::Semicolon);
		StatementNode* step = ParseStatement(false);

		if (parentithized)
			Match(Token::Kind::CloseParenthesis);

		BlockStatementNode* body = ParseBlockStatement(matchSemicolon);
		Debug::EndScope();

		return new ForLoopStatementNode(open, initializer, condition, step, body);
	}

	GlobalVariableDeclarationNode* Parser::ParseEnumDeclaration(bool matchSemicolon)
	{
		Match(Token::Kind::Enum);
		const Type* type = Type::PrimitiveType::Int;
		if (Peek()->Is(Token::Kind::Colon))
		{
			Next();
			type = GetType(Next());
		}

		Match(Token::Kind::OpenBracket);
		GlobalVariableDeclarationNode* declaration = ParseEnumField(new TypeNode(type, EmptyModifiers, nullptr));
		Match(Token::Kind::CloseBracket);
		if (matchSemicolon)
			Match(Token::Kind::Semicolon);

		return declaration;
	}

	static VariableModifiersNode* EmptyVarModifiers = new VariableModifiersNode({});

	GlobalVariableDeclarationNode* Parser::ParseEnumField(const TypeNode* ty, int before)
	{
		const Token* name = Match(Token::Kind::Identifier);
		GlobalVariableDeclarationNode* declaration = nullptr;
		if (Peek()->Is(Token::Kind::Equal))
		{
			Next();
			ExpressionNode* expression = ParseExpression();
			if (expression->CanEvaluate())
				before = expression->Evaluate();
			else
				Diagnostics::ReportError(name, "Must be Constant Value");

			GlobalVariableDeclarationNode* next = nullptr;

			if (Peek()->Is(Token::Kind::Comma))
			{
				Next();
				next = ParseEnumField(ty, before+1);
			}

			declaration = new GlobalVariableDeclarationNode(name, ty, EmptyVarModifiers, expression, next);
			Debug::VariableDeclaration(declaration);
			return declaration;
		}

		GlobalVariableDeclarationNode* next = nullptr;

		if (Peek()->Is(Token::Kind::Comma))
		{
			Next();
			next = ParseEnumField(ty, before+1);
		}

		std::stringstream ss;
		ss << before;
		std::string* str = new std::string(ss.str());

		declaration = new GlobalVariableDeclarationNode(name, ty, EmptyVarModifiers, new NumberLiteralExpressionNode(new Token(Token::Kind::Number, str->c_str(), str->length(), "")), next);
		Debug::VariableDeclaration(declaration);
		return declaration;
	}

	VariableDeclarationNode* Parser::ParseVariableDeclaration(bool matchSemicolon, const Type* type)
	{
		if (!type)
			type = GetType(Next());
		const TypeNode* ty = ParseType(type);
		const Token* name = Match(Token::Kind::Identifier);

		VariableModifiersNode* modifiers = ParseVariableModifiers();

		VariableDeclarationNode* declaration = nullptr;
		if (Peek()->Is(Token::Kind::Equal))
		{
			Next();
			ExpressionNode* expression = ParseExpression();

			VariableDeclarationNode* next = nullptr;

			if (Peek()->Is(Token::Kind::Comma))
			{
				Next();
				next = ParseVariableDeclaration(false, type);
			}

			if (matchSemicolon)
				Match(Token::Kind::Semicolon);

			declaration = new VariableDeclarationNode(name, ty, modifiers, expression, next);
			Debug::VariableDeclaration(declaration);
			return declaration;
		}

		VariableDeclarationNode* next = nullptr;

		if (Peek()->Is(Token::Kind::Comma))
		{
			Next();
			next = ParseVariableDeclaration(false, type);
		}

		if (matchSemicolon)
			Match(Token::Kind::Semicolon);

		declaration = new VariableDeclarationNode(name, ty, modifiers, nullptr, next);
		Debug::VariableDeclaration(declaration);
		return declaration;
	}

	VariableModifiersNode* Parser::ParseVariableModifiers()
	{
		std::vector<const VariableModifierNode*> modifiers;

		while (VariableModifierNode::IsValid(Peek()->GetKind()))
			modifiers.push_back(ParseVariableModifier());

		return new VariableModifiersNode(modifiers);
	}

	VariableModifierNode* Parser::ParseVariableModifier()
	{
		return new VariableModifierNode(Next());
	}

	ExpressionNode* Parser::ParseExpression()
	{
		return ParseAssignmentExpression();
	}

	ExpressionNode* Parser::ParseAssignmentExpression()
	{
		unsigned int pPosition = mPosition;
		ModifiableExpressionNode* left = ParseModifiableExpression();
		if (left)
			while (Peek()->IsEither({ Token::Kind::OpenBracket }))
			{
				if (Peek()->Is(Token::Kind::OpenBracket))
					left = ParsePointerIndexExpression(left);
			}

		const Token* oqerator = Next();
		int prority = Priority::AssignmentOperatorPriority(oqerator);
		if (left && !prority)
		{
			ExpressionNode* right = ParseExpression();
			while (Peek()->IsEither({ Token::Kind::OpenBracket }))
			{
				if (Peek()->Is(Token::Kind::OpenBracket))
					right = ParsePointerIndexExpression(right);
			}

			return new AssignmentExpressionNode(oqerator, left, right);
		}
		mPosition = pPosition;

		ExpressionNode* expression = ParseBinaryExpression();

		while (Peek()->IsEither({ Token::Kind::QuestionMark }))
		{
			if (Peek()->Is(Token::Kind::QuestionMark))
				expression = ParseTernaryExpression(expression);
		}

		return expression;
	}

	ExpressionNode* Parser::ParseUnaryExpression(int parentPriority)
	{
		int priority = Priority::UnaryOperatorPriority(Peek());
		if (priority >= 0 && priority >= parentPriority)
		{
			const Token* oqerator = Next();
			ExpressionNode* value = ParseUnaryExpression(parentPriority);
			return new UnaryExpressionNode(oqerator, value);
		}

		ExpressionNode* expression = ParsePrimaryExpression();
		while (Peek()->IsEither({ Token::Kind::OpenBracket }))
		{
			if (Peek()->Is(Token::Kind::OpenBracket))
				expression = ParsePointerIndexExpression(expression);
		}
		return expression;
	}

	ExpressionNode* Parser::ParseBinaryExpression(int parentPriority)
	{
		ExpressionNode* left = ParseUnaryExpression();
		if (!left)
		{
			Diagnostics::ReportError(Peek(), "Left Hand Side is Null");
			return new BinaryExpressionNode(Token::Default, new ExpressionNode(ErrorType), new ExpressionNode(ErrorType));
		}

		while (!Peek()->Is(Token::Kind::EndOfFile))
		{
			int priority = Priority::BinaryOperatorPriority(Peek());
			if (priority < 0 || priority <= parentPriority)
				break;
			const Token* oqerator = Next();
			ExpressionNode* right = ParseBinaryExpression(priority);
			left = new BinaryExpressionNode(oqerator, left, right);
		}

		return left;
	}

	ExpressionNode* Parser::ParsePrimaryExpression()
	{
		switch (Peek()->GetKind())
		{
		case Token::Kind::OpenParenthesis:
			if (IsTypeNodeable(Peek(1)))
				return ParseCastExpression();
			return ParseParenthesizedExpression();
		case Token::Kind::Stalloc:
			return ParseStallocExpression();
		case Token::Kind::Null:
			return new NullLiteralExpressionNode(Next());
		case Token::Kind::True:
		case Token::Kind::False:
			return new BooleanLiteralExpressionNode(Next());
		case Token::Kind::Number:
			return new NumberLiteralExpressionNode(Next());
		case Token::Kind::String:
			return new StringLiteralExpressionNode(Next());
		case Token::Kind::Character:
			return new CharacterLiteralExpressionNode(Next());
		case Token::Kind::At:
			return ParseVariableAddressExpression();
		case Token::Kind::OpenBracket:
			return ParseListExpression();
		}

		if (IsType(Peek()) && Peek(1)->Is(Token::Kind::OpenBrace))
			return ParseStructInitializerExpression();
		if (Peek()->Is(Token::Kind::Identifier) && Peek(1)->Is(Token::Kind::OpenParenthesis))
			return ParseFunctionCallExpression();
		return ParseModifiableExpression();
	}

	CastExpressionNode* Parser::ParseCastExpression()
	{
		const Token* open = Match(Token::Kind::OpenParenthesis);
		const TypeNode* type = ParseType();
		const Token* close = Match(Token::Kind::CloseParenthesis);

		ExpressionNode* expression = ParsePrimaryExpression();

		return new CastExpressionNode(open, type, close, expression);
	}

	ListExpressionNode* Parser::ParseListExpression()
	{
		const Token* open = Match(Token::Kind::OpenBracket);
		
		std::vector<const ExpressionNode*> expressions;
		while (!Peek()->Is(Token::Kind::CloseBracket))
		{
			expressions.push_back(ParseExpression());
			if (Peek()->Is(Token::Kind::Comma))
				Next();
		}

		const Token* close = Next();

		return new ListExpressionNode(open, expressions, close);
	}

	StallocExpressionNode* Parser::ParseStallocExpression()
	{
		Match(Token::Kind::Stalloc);
		const Token* open = Match(Token::Kind::OpenParenthesis);
		ExpressionNode* size = ParseExpression();
		const Token* close = Match(Token::Kind::CloseParenthesis);

		return new StallocExpressionNode(open, size, close);
	}

	TernaryExpressionNode* Parser::ParseTernaryExpression(ExpressionNode* condition)
	{
		const Token* questionMark = Match(Token::Kind::QuestionMark);
		ExpressionNode* then = ParseExpression();
		const Token* colon = Match(Token::Kind::Colon);
		ExpressionNode* elze = ParseExpression();

		return new TernaryExpressionNode(condition, questionMark, then, colon, elze);
	}

	ParenthesizedExpressionNode* Parser::ParseParenthesizedExpression()
	{
		const Token* open = Match(Token::Kind::OpenParenthesis);
		ExpressionNode* expression = ParseExpression();
		const Token* close = Match(Token::Kind::CloseParenthesis);

		return new ParenthesizedExpressionNode(open, expression, close);
	}

	StructInitializerExpressionNode* Parser::ParseStructInitializerExpression()
	{
		const Token* name = Match(Token::Kind::Identifier);
		const Token* open = Match(Token::Kind::OpenBrace);

		std::vector<const ExpressionNode*> expressions;
		while (!Peek()->Is(Token::Kind::CloseBrace))
		{
			ExpressionNode* expression = ParseExpression();
			expressions.push_back(expression);
			if (Peek()->Is(Token::Kind::Comma))
				Next();
		}

		const Token* close = Next();

		return new StructInitializerExpressionNode(name, open, expressions, close);
	}

	ModifiableExpressionNode* Parser::ParseModifiableExpression()
	{
		ModifiableExpressionNode* expression = nullptr;

		switch (Peek()->GetKind())
		{
		case Token::Kind::Ampersand:
			expression = ParseDereferencePointerExpression();
			goto CheckIfField;
		}

		if (Debug::GetVariable(Peek()->GetLex()))
		{
			expression = ParseVariableExpression();
			goto CheckIfField;
		}
		
	CheckIfField:
		while (Peek()->Is(Token::Kind::Period))
			expression = ParseFieldExpression(expression);
		return expression;
	}

	VariableExpressionNode* Parser::ParseVariableExpression()
	{
		return new VariableExpressionNode(Next());
	}

	FieldExpressionNode* Parser::ParseFieldExpression(ModifiableExpressionNode* callee)
	{
		Match(Token::Kind::Period);
		const Token* name = Match(Token::Kind::Identifier);

		return new FieldExpressionNode(callee, name);
	}

	VariableAddressExpressionNode* Parser::ParseVariableAddressExpression()
	{
		const Token* symbol = Match(Token::Kind::At);
		ModifiableExpressionNode* variable = ParseModifiableExpression();

		return new VariableAddressExpressionNode(symbol, variable);
	}

	PointerIndexExpressionNode* Parser::ParsePointerIndexExpression(ExpressionNode* address)
	{
		const Token* bracket = Match(Token::Kind::OpenBracket);
		ExpressionNode* index = ParseExpression();
		Match(Token::Kind::CloseBracket);

		return new PointerIndexExpressionNode(address, bracket, index);
	}

	DereferencePointerExpressionNode* Parser::ParseDereferencePointerExpression()
	{
		const Token* symbol = Match(Token::Kind::Ampersand);
		ExpressionNode* address = ParseExpression();

		return new DereferencePointerExpressionNode(symbol, address);
	}

	FunctionCallArgumentsNode* Parser::ParseFunctionCallArguments()
	{
		const Token* open = Match(Token::Kind::OpenParenthesis);

		std::vector<const ExpressionNode*> arguments;
		while (!Peek()->Is(Token::Kind::CloseParenthesis))
		{
			if (Peek()->Is(Token::Kind::EndOfFile))
			{
				Diagnostics::ReportError(Next(), "Unexpected End Of File");
				break;
			}

			arguments.push_back(ParseExpression());

			if (Peek()->Is(Token::Kind::Comma))
				Next();
			else
				break;
		}

		const Token* close = Match(Token::Kind::CloseParenthesis);

		return new FunctionCallArgumentsNode(open, arguments, close);
	}

	FunctionCallExpressionNode* Parser::ParseFunctionCallExpression()
	{
		const Token* name = Match(Token::Kind::Identifier);
		FunctionCallArgumentsNode* arguments = ParseFunctionCallArguments();

		return new FunctionCallExpressionNode(name, arguments);
	}
}