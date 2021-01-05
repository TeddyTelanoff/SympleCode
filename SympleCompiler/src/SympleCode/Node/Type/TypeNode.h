#pragma once

#include "SympleCode/Common/Type.h"
#include "SympleCode/Node/Type/TypeContinueNode.h"
#include "SympleCode/Node/Type/TypeModifiersNode.h"

#include <iostream>

namespace Symple
{
	class TypeNode : public Node, public TypeNodes
	{
	private:
		const Type* mType;
		const TypeModifiersNode* mModifiers;
		const TypeContinueNode* mContinue;
	public:
		TypeNode(const Type* type, const TypeModifiersNode* modifiers, const TypeContinueNode* contjnue)
			: mType(type), mModifiers(modifiers), mContinue(contjnue)
		{}

		Kind GetKind() const override
		{
			return Kind::Type;
		}

		std::string ToString(const std::string& indent = "", bool last = true) const override
		{
			std::stringstream ss;
			ss << indent;
			if (last)
				ss << "L--\t";
			else
				ss << "|--\t";
			ss << "Type (" << mType->GetName() << ")";

			const char* newIndent = " \t";
			if (!last)
				newIndent = "|\t";
			ss << '\n' << mModifiers->ToString(indent + newIndent);
			return ss.str();
		}

		const Type* GetType() const
		{
			return mType;
		}

		const TypeModifiersNode* GetModifiers() const
		{
			return mModifiers;
		}

		const TypeContinueNode* GetContinue() const
		{
			return mContinue;
		}

		unsigned int GetSize() const
		{
			return HasContinue(Token::Kind::Asterisk) ? 4 : mType->GetSize();
		}

		bool HasContinue(Token::Kind kind) const
		{
			if (mContinue)
				return mContinue->HasContinue(kind);
			return false;
		}

		bool SameAs(const TypeNode* other) const
		{
			bool rawType = mType == other->mType && (mContinue == other->mContinue || (mContinue && other->mContinue && mContinue->SameAs(other->mContinue)));
			bool modifiers = mModifiers->IsMutable() == other->mModifiers->IsMutable();

			return rawType && modifiers;
		}

		bool CanImplicitlyCastTo(const TypeNode* other) const
		{
			bool rawType = mType == other->mType && (mContinue == other->mContinue || (mContinue && other->mContinue && mContinue->CanImplicitlyCastTo(other->mContinue)));
			bool modifiers = mModifiers->IsMutable() || !(mModifiers->IsMutable() || other->mModifiers->IsMutable());

			return rawType && modifiers;
		}

		bool CanCastTo(const TypeNode* other) const
		{
			bool type = GetSize() == other->GetSize() || (mType == other->mType && (mContinue == other->mContinue || (mContinue && other->mContinue && mContinue->CanImplicitlyCastTo(other->mContinue))));
			bool modifiers = mModifiers->IsMutable() || !(mModifiers->IsMutable() || other->mModifiers->IsMutable());

			return type && modifiers;
		}
	};

	extern TypeNode* VoidType;
	extern TypeNode* ByteType;
	extern TypeNode* ShortType;
	extern TypeNode* IntType;

	extern TypeNode* BoolType;
	extern TypeNode* CharType;
	extern TypeNode* WCharType;

	extern TypeNode* PtrType;
	extern TypeNode* StringType;

	extern TypeNode* ErrorType;
}