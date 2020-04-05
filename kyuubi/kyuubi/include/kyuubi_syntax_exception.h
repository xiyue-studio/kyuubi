#pragma once
#include "kyuubi_exception.h"

namespace kyuubi
{
	class SyntaxException : public KyuubiException
	{
	public:
		SyntaxException(int errorNum, xiyue::ConstString msg)
			: KyuubiException(errorNum, msg)
		{
		}
	};
}
