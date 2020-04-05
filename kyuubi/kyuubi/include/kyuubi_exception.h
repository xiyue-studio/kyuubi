#pragma once
#include "xiyue_const_string.h"

namespace kyuubi
{
	class KyuubiException
	{
	public:
		KyuubiException(int errorNum, xiyue::ConstString errorMsg)
			: m_errorNum(errorNum)
			, m_msg(errorMsg)
		{
		}

	public:
		xiyue::ConstString message() const { return m_msg; }
		int errorNum() const { return m_errorNum; }

	protected:
		int m_errorNum;
		xiyue::ConstString m_msg;
	};
}
