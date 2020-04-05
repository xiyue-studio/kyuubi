#pragma once
#include "xiyue_const_string.h"
#include "xiyue_json_object.h"
#include "kyuubi_vm.h"

namespace kyuubi
{
	class Kyuubi
	{
	public:
		Kyuubi();
		Kyuubi(xiyue::ConstString templateString);
		virtual ~Kyuubi();

	public:
		bool createFromString(xiyue::ConstString templateString);
		bool createFromFile(xiyue::ConstString fileName);

	public:
		bool generateToFile(xiyue::JsonObject* data, xiyue::ConstString outputFileName);
		bool generateToStdString(xiyue::JsonObject* data, std::wstring& output);
		bool genreateToString(xiyue::JsonObject* data, xiyue::ConstString& output);

	protected:
		VM* m_vm;
	};
}
