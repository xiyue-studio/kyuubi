#pragma once
#include "kyuubi_exception.h"

namespace kyuubi
{
	class InvalidTypeException : public KyuubiException
	{
	public:
		InvalidTypeException(int errorNum, xiyue::ConstString msg)
			: KyuubiException(errorNum, msg)
		{
		}

		InvalidTypeException(xiyue::ConstString typeStr)
			: KyuubiException(0, XIYUE_CONST_STRING(L"Type `%s` is not allowed here.", typeStr.cstr()))
		{
		}
	};
}
