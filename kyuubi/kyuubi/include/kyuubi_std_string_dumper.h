#pragma once
#include "kyuubi_dumper.h"

namespace kyuubi
{
	class StdStringDumper : public Dumper
	{
	public:
		const std::wstring& getString() const { return m_buffer; }

	public:
		virtual void dumpString(xiyue::ConstString str) override {
			m_buffer += str;
		}

	protected:
		std::wstring m_buffer;
	};
}
