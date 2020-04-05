#pragma once
#include "xiyue_const_string.h"

namespace kyuubi
{
	class Dumper
	{
	public:
		virtual ~Dumper() = default;

	public:
		virtual void dumpString(xiyue::ConstString str) = 0;
	};
}
