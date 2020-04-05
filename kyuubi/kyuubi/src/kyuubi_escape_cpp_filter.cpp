#include "stdafx.h"
#include "kyuubi_escape_cpp_filter.h"
#include "xiyue_string_format.h"

using namespace std;
using namespace xiyue;
using namespace kyuubi;

JsonObject EscapeCppFilter::doFilter(JsonObject input)
{
	return xiyue_escapeCppStyleChars(input).c_str();
}
