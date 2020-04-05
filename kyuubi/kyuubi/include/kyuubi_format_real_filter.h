#pragma once
#include "kyuubi_filter.h"

namespace kyuubi
{
	class FormatRealFilter : public Filter
	{
	public:
		BEGIN_FILTER(FormatRealFilter)
			FILTER_NAME("format_real")
			FILTER_INPUT(xiyue::Json_real, xiyue::Json_int)
			FILTER_RETURN(xiyue::Json_string)
			FILTER_DESCRIPTION(L"格式化一个数字。")
			FILTER_PARAM("format", xiyue::Json_string)
		END_FILTER()

		virtual ~FormatRealFilter() = default;

	public:
		virtual xiyue::JsonObject doFilter(xiyue::JsonObject input) override;
	};
}
