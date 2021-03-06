#pragma once

#include "Symple/Code/Common.h"
#include "Symple/Code/Util/Memory.h"
#include "Symple/Code/Util/Printable.h"

namespace Symple::Code
{
	enum class SYC_API TypeKind: uint32
	{
		Void,
		Byte,
		Short,
		Int,

		Float,

		Char,
		WChar,
		Bool,

		Count, // Count of type kinds
	};

	constexpr const char *const TypeKindNames[(uint32)TypeKind::Count] =
	{
		"Void",
		"Byte",
		"Short",
		"Int",

		"Float",

		"Char",
		"WChar",
		"Bool",
	};

	SYC_API std::ostream &operator <<(std::ostream &, TypeKind);

	class SYC_API Type: public Printable
	{
	private:
		TypeKind m_Kind;
		uint32 m_PtrCount;
		bool m_Ref;
	public:
		Type(TypeKind, uint32 pointerCount, bool isRef);
		GlobalRef<Type> Deref() const;

		virtual void Print(std::ostream &, std::string indent = "", std::string_view label = "", bool last = true) const override;

		TypeKind GetKind() const;
		uint32 GetPointerCount() const;
		bool GetIsRef() const;

		SY_PROPERTY_GET(GetKind) TypeKind Kind;
		SY_PROPERTY_GET(GetPointerCount) uint32 PointerCount;
		SY_PROPERTY_GET(GetIsRef) bool IsRef;
	};
}