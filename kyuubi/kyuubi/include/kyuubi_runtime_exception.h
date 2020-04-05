#pragma once
#include "kyuubi_exception.h"

namespace kyuubi
{
	class RuntimeException : public KyuubiException
	{
	public:
		RuntimeException(int errorNum, xiyue::ConstString msg)
			: KyuubiException(errorNum, msg)
		{
		}
	};
}
