#pragma once
#include "kyuubi_filter.h"

namespace kyuubi
{
	class CamelCaseFilter : public Filter
	{
	public:
		BEGIN_FILTER(CamelCaseFilter)
			FILTER_NAME("camelCase")
			FILTER_INPUT(xiyue::Json_string)
			FILTER_RETURN(xiyue::Json_string)
			FILTER_DESCRIPTION(L"将一个字符串转换成驼峰命名方式。")
			FILTER_OPTIONAL_PARAM("firstUppercase", xiyue::Json_boolean, false)
		END_FILTER()

			virtual ~CamelCaseFilter() = default;

	public:
		virtual xiyue::JsonObject doFilter(xiyue::JsonObject input) override;
	};
}
