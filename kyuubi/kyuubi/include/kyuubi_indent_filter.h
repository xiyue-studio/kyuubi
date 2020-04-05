#pragma once
#include "kyuubi_filter.h"

namespace kyuubi
{
	class IndentFilter : public Filter
	{
	public:
		BEGIN_FILTER(IndentFilter)
			FILTER_NAME("indent")
			FILTER_INPUT(xiyue::Json_string)
			FILTER_RETURN(xiyue::Json_string)
			FILTER_DESCRIPTION(L"根据参数，给文本添加缩进。")
			FILTER_PARAM("indentLevel", xiyue::Json_int)
			FILTER_OPTIONAL_PARAM("indentChar", xiyue::Json_string, xiyue::JsonObject(L" "))
		END_FILTER()

			virtual ~IndentFilter() = default;

	public:
		virtual xiyue::JsonObject doFilter(xiyue::JsonObject input) override;
	};
}
