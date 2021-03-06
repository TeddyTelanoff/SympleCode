#include "Symple/Code/Parse/Facts.h"

namespace Symple::Code
{
	uint32 Precedence::Unary(TokenKind kind)
	{
		switch (kind)
		{
		default:
			return 0;
		case TokenKind::Plus:
		case TokenKind::Minus:
			return 3;
		}
	}

	uint32 Precedence::Binary(TokenKind kind)
	{
		switch (kind)
		{
		default:
			return 0;
		case TokenKind::Plus:
		case TokenKind::Minus:
			return 1;
		case TokenKind::Star:
		case TokenKind::Slash:
			return 2;
		}
	}
}