#include "stdafx.h"
#include "xiyue_string_format.h"
#include "kyuubi_camel_case_filter.h"

using namespace std;
using namespace xiyue;
using namespace kyuubi;

JsonObject CamelCaseFilter::doFilter(JsonObject input)
{
	return xiyue_makeCamelCaseName(input, getParameter(0));
}
