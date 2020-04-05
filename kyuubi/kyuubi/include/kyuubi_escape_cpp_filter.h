#pragma once
#include "kyuubi_filter.h"

namespace kyuubi
{
	class EscapeCppFilter : public Filter
	{
	public:
		BEGIN_FILTER(EscapeCppFilter)
			FILTER_NAME("escapeCpp")
			FILTER_INPUT(xiyue::Json_string)
			FILTER_RETURN(xiyue::Json_string)
			FILTER_DESCRIPTION(L"��һ���ַ�������C�ķ�ʽ����ת�塣")
		END_FILTER()

			virtual ~EscapeCppFilter() = default;

	public:
		virtual xiyue::JsonObject doFilter(xiyue::JsonObject input) override;
	};
}
