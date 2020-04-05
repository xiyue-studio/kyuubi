#include "stdafx.h"
#include "kyuubi_lexer_token.h"

using namespace std;
using namespace xiyue;
using namespace kyuubi;

int kyuubi::parseIdToken(xiyue::ConstStringPointer& p)
{
	wchar_t ch = *p;
	int startPos = p.getOffset();
	// 解析标识符
	if (!isAlpha(ch) && ch != '_')
		return 0;

	do {
		p++;
	} while (isAlphaDigit(*p) || *p == '_');

	return p.getOffset() - startPos;
}

int kyuubi::parseNumberToken(xiyue::ConstStringPointer& p, bool& isIntType)
{
	int startPos = p.getOffset();
	wchar_t ch = *p;
	// 解析数字
	if (!isDigit(ch))
		return 0;


	do {
		p++;
	} while (isDigit(*p));
	// 整数
	if (*p != '.')
	{
		isIntType = true;
	}
	else
	{
		// 浮点数
		do {
			p++;
		} while (isDigit(*p));
		isIntType = false;
	}

	return p.getOffset() - startPos;
}

int kyuubi::parseStringToken(xiyue::ConstStringPointer& p, bool& isMissingQuote)
{
	int startPos = p.getOffset();
	wchar_t quote = *p;

	if (quote != '"' && quote != '\'')
		return 0;

	p++;
	while (*p != quote)
	{
		if (*p == '\\')
		{
			p++;
			if (*p == quote)
			{
				p++;
				continue;
			}
		}

		if (*p == 0)
		{
			isMissingQuote = true;
			return p.getOffset() - startPos;
		}

		p++;
	}

	p++;
	isMissingQuote = false;
	return p.getOffset() - startPos;
}
