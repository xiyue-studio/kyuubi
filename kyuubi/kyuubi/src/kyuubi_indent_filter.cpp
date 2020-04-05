#include "stdafx.h"
#include "kyuubi_indent_filter.h"
#include "kyuubi_runtime_exception.h"

using namespace std;
using namespace xiyue;
using namespace kyuubi;

JsonObject IndentFilter::doFilter(JsonObject input)
{
	JsonObject indentLevelParam = getParameter(0);
	JsonObject indentCharParam = getParameter(1);

	ConstString chs = indentCharParam;
	if (chs.length() != 1)
		throw RuntimeException(0, L"indent filter only accepted 1 character as indentChar parameter."_cs);
	wchar_t ch = chs[0];

	ConstString content = input.toString();
	wstring result;
	auto p = &content;
	while (*p)
	{
		result.append(indentLevelParam.intValue(), ch);
		while (!isLineBreak(*p) && *p != 0)
			result.push_back(*p++);
		// 一行加载结束
		while (!result.empty() && !isLineBreak(*p) && isSpace(result.back()))
			result.pop_back();
		// 将换行符追加
		while (isLineBreak(*p))
			result.push_back(*p++);
	}

	return JsonObject(result.c_str());
}
