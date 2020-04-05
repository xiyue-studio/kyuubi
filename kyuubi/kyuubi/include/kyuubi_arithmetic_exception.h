#pragma once
#include "kyuubi_exception.h"

namespace kyuubi
{
	class ArithmeticException : public KyuubiException
	{
	public:
		ArithmeticException(int errorNum, xiyue::ConstString msg)
			: KyuubiException(errorNum, msg)
		{
		}
	};
}
