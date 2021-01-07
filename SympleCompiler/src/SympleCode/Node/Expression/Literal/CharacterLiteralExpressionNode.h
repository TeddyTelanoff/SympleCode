#pragma once

#include "SympleCode/Node/Expression/Literal/LiteralExpressionNode.h"

namespace Symple
{
	class CharacterLiteralExpressionNode : public LiteralExpressionNode
	{
	public:
		CharacterLiteralExpressionNode(const Token* literal)
			: LiteralExpressionNode(CharType, literal) {}

		Kind GetKind() const override
		{
			return Kind::CharacterLiteralExpression;
		}

		std::string ToString(const std::string& indent = "", bool last = true) const override
		{
			std::stringstream ss;
			ss << indent;
			if (last)
				ss << "L--\t";
			else
				ss << "|--\t";
			ss << "Character Literal Expression (" << mLiteral->GetLex() << ")";

			return ss.str();
		}

		int Evaluate() const override
		{
			return mLiteral->GetLex()[0];
		}
	};
}