#include "stdafx.h"
#include "kyuubi_format_real_filter.h"
#include "xiyue_string_format.h"

using namespace std;
using namespace xiyue;
using namespace kyuubi;

JsonObject FormatRealFilter::doFilter(JsonObject input)
{
	JsonObject formatParam = getParameter(0);
	wstring result;
	if (!xiyue_formatNumber(result, formatParam.stringValue(), input.realValue()))
		return input.toString();

	return JsonObject(result.c_str());
}
