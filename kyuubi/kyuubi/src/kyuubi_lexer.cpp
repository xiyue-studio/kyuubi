#include "stdafx.h"
#include "kyuubi_lexer.h"
#include "kyuubi_exception.h"

using namespace std;
using namespace xiyue;
using namespace kyuubi;

Lexer::Lexer(xiyue::ConstString templateString)
	: m_text(templateString)
	, m_cursor(&templateString)
{
	m_token.offset = uint32_t(-1);
	m_modeStack.push_back(LexerMode_raw);
	m_cacheStartPos = -1;
	m_cacheStartMode = LexerMode_raw;
	m_isLastTokenBlockEnd = false;
	m_isLastBlockStartInLineHead = false;
}

bool Lexer::tryParseRawToken()
{
	int startPos = m_cursor.getOffset();

	if (tryParseStartMarkups())
		return true;

	wchar_t ch = 0;
	while (*m_cursor != 0)
	{
		// 检查当前位置是否遇到了标记 {{, {# 或 {%
		if (*m_cursor == '{')
		{
			ch = *(m_cursor + 1);
			if (ch == '{' || ch == '%' || ch == '#')
				break;
		}

		m_cursor++;
	}

	fillToken(Token_rawString, startPos, m_cursor.getOffset());
	if (*m_cursor != 0 && ch != '{')
		m_token.tokenString = m_token.tokenString.rTrim(L" \t"_cs);
	if (m_isLastTokenBlockEnd)
		m_token.tokenString = m_token.tokenString.lTrim(L" \t"_cs);

	return true;
}

bool Lexer::tryParseStartMarkups()
{
	int startPos = m_cursor.getOffset();

	if (*m_cursor != '{')
		return false;

	wchar_t ch = *(m_cursor + 1);

	// 解析插值表达式
	if (ch == '{')
	{
		fillToken(Token_insertExprStart, startPos, startPos + 2);
		m_cursor += 2;
		return true;
	}

	// 解析语句表达式
	if (ch == '%')
	{
		fillToken(Token_blockStart, startPos, startPos + 2);
		m_cursor += 2;
		return true;
	}

	// 解析注释
	if (ch == '#')
	{
		m_cursor += 2;
		while (*m_cursor != 0)
		{
			if (*m_cursor == '#' && *(m_cursor + 1) == '}')
				break;
			m_cursor++;
		}

		// 如果没有遇到注释末尾就结束了，报告一个错误
		if (*m_cursor == 0)
		{
			XIYUE_LOG_ERROR("Missing #} in comment.");
		}
		else
		{
			m_cursor += 2;
		}

		fillToken(Token_comment, startPos, m_cursor.getOffset());
		return true;
	}

	return false;
}

bool Lexer::tryParseInsertionToken()
{
	static const unordered_map<wchar_t, TokenType> tokenMap = {
		{'.', Token_dot},
		{'(', Token_lParenthese},
		{')', Token_rParenthese},
		{'[', Token_lBracket},
		{']', Token_rBracket},
		{'|', Token_pipe},
		{',', Token_comma},
		{'+', Token_plus},
		{'-', Token_minus},
		{'*', Token_multiply},
		{'/', Token_divide},
		{'!', Token_not},
		{'=', Token_equal}
	};

	while (tryParseWS());
	int startPos = m_cursor.getOffset();
	wchar_t ch = *m_cursor;

	if (tryParseEndMarkups())
		return true;

	if (tryParseID())
		return true;

	if (tryParseNumber())
		return true;

	if (tryParseString())
		return true;

	if (ch == '&' && *(m_cursor + 1) == '&')
	{
		fillToken(Token_and, startPos, startPos + 2);
		m_cursor += 2;
		return true;
	}

	if (ch == '|' && *(m_cursor + 1) == '|')
	{
		fillToken(Token_or, startPos, startPos + 2);
		m_cursor += 2;
		return true;
	}

	auto it = tokenMap.find(ch);
	if (it == tokenMap.end())
		return false;

	fillToken(it->second, startPos, startPos + 1);
	m_cursor++;
	return true;
}

bool Lexer::tryParseEndMarkups()
{
	int startPos = m_cursor.getOffset();
	wchar_t ch = *m_cursor;
	wchar_t nextCh = *(m_cursor + 1);

	if (nextCh != '}')
		return false;

	// 匹配 }}
	if (ch == '}')
	{
		fillToken(Token_insertExprEnd, startPos, startPos + 2);
		m_cursor += 2;
		return true;
	}

	// 匹配 %}
	if (ch == '%')
	{
		fillToken(Token_blockEnd, startPos, startPos + 2);
		m_cursor += 2;
		return true;
	}

	// 匹配 #}
	if (ch == '#')
	{
		fillToken(Token_commentEnd, startPos, startPos + 2);
		m_cursor += 2;
		return true;
	}

	return false;
}

void Lexer::fillToken(kyuubi::TokenType type, int startPos, int endPos)
{
	m_token.type = type;
	m_token.offset = startPos;
	m_token.length = endPos - startPos;
	if (type != Token_EOF)
		m_token.tokenString = m_text.substr(startPos, m_token.length);
}

bool Lexer::tryParseWS()
{
	if (!isSpace(*m_cursor))
		return false;

	m_cursor++;
	while (isSpace(*m_cursor))
	{
		m_cursor++;
	}

	return true;
}

bool Lexer::tryParseID()
{
	static const unordered_map<ConstString, TokenType> keywords = {
		{L"true"_cs, Token_true},
		{L"false"_cs, Token_false},
		{L"raw"_cs, Token_raw},
		{L"endraw"_cs, Token_endRaw},
		{L"if"_cs, Token_if},
		{L"elseif"_cs, Token_elseIf},
		{L"else"_cs, Token_else},
		{L"endif"_cs, Token_endIf},
		{L"for"_cs, Token_for},
		{L"endfor"_cs, Token_endFor},
		{L"in"_cs, Token_in},
		{L"set"_cs, Token_set},
		{L"filter"_cs, Token_filter},
		{L"endfilter"_cs, Token_endFilter}
	};

	int startPos = m_cursor.getOffset();
	if (parseIdToken(m_cursor) == 0)
		return false;

	fillToken(Token_id, startPos, m_cursor.getOffset());
	// 判断是不是关键字
	auto it = keywords.find(m_token.tokenString);
	if (it != keywords.end())
		m_token.type = it->second;

	return true;
}

bool Lexer::tryParseNumber()
{
	bool isInt = false;

	int startPos = m_cursor.getOffset();
	if (parseNumberToken(m_cursor, isInt) == 0)
		return false;

	fillToken(isInt ? Token_int : Token_float, startPos, m_cursor.getOffset());
	return true;
}

bool Lexer::tryParseString()
{
	int startPos = m_cursor.getOffset();
	bool isMissingQuote = false;
	if (parseStringToken(m_cursor, isMissingQuote) == 0)
		return false;

	fillToken(Token_string, startPos, m_cursor.getOffset());
	if (isMissingQuote)
		throw KyuubiException(0, ConstString::makeByFormat(L"Missing %c.", m_token.tokenString[0]));

	m_token.tokenString = m_token.tokenString.substr(1, m_token.tokenString.length() - 2);
	return true;
}

void Lexer::parseToken()
{
	if (*m_cursor == 0)
	{
		fillToken(Token_EOF, m_cursor.getOffset(), m_cursor.getOffset() + 1);
		return;
	}

	LexerMode mode = m_modeStack.back();
	switch (mode)
	{
	case LexerMode_raw:
		do 
		{
			if (!tryParseRawToken())
			{
				fillToken(Token_error, m_cursor.getOffset(), m_cursor.getOffset() + 1);
				m_cursor++;
				return;
			}
		} while (m_token.type == Token_comment);
		break;
	case LexerMode_insertionExpression:
		if (!tryParseInsertionToken())
		{
			fillToken(Token_error, m_cursor.getOffset(), m_cursor.getOffset() + 1);
			m_cursor++;
			return;
		}
		break;
	case LexerMode_rawString:
		tryParseRawString();
		break;
	default:
		assert(!"Unsupported channel.");
		break;
	}
}

bool Lexer::tryParseRawString()
{
	int startPos = m_cursor.getOffset();

	if (tryParseEndRawTag())
		return true;

	if (*m_cursor == 0)
	{
		fillToken(Token_EOF, startPos, startPos + 1);
		return true;
	}

	m_cursor++;
	while (*m_cursor != 0)
	{
		int endPos = m_cursor.getOffset();
		if (tryParseEndRawTag())
		{
			// 遇到了结束标记
			m_cursor.reset(endPos);
			fillToken(Token_rawString, startPos, endPos);
			return true;
		}

		m_cursor++;
	}

	fillToken(Token_rawString, startPos, m_cursor.getOffset());
	return true;
}

#define checkChar(c)	if (*m_cursor != c) return false
bool Lexer::tryParseEndRawTag()
{
	int startPos = m_cursor.getOffset();
	checkChar('{');

	m_cursor++;
	checkChar('%');

	while (tryParseWS());

	checkChar('e');

	m_cursor++;
	checkChar('n');

	m_cursor++;
	checkChar('d');
	
	m_cursor++;
	checkChar('r');

	m_cursor++;
	checkChar('a');

	m_cursor++;
	checkChar('w');

	while (tryParseWS());

	checkChar('%');

	m_cursor++;
	checkChar('}');

	m_cursor++;
	fillToken(Token_endRawTag, startPos, m_cursor.getOffset());
	return true;
}

Token Lexer::peekToken()
{
	if (m_cacheStartPos == m_cursor.getOffset()
		&& m_cacheStartMode == m_modeStack.back())
		return m_token;

	m_cacheStartPos = m_cursor.getOffset();
	m_cacheStartMode = m_modeStack.back();
	parseToken();
	m_cursor.reset(m_cacheStartPos);

	// 检查当前如果是 {% 的话，那这个符号是不是在行首
	if (m_token.is(Token_blockStart))
	{
		ConstStringPointer p = &m_text;
		p += m_token.offset - 1;
		while (!isLineBreak(*p) && isSpace(*p))
		{
			--p;
		}
		m_isLastBlockStartInLineHead = *p == 0 || isLineBreak(*p);
	}

	return m_token;
}

bool Lexer::consumeToken()
{
	Token token = peekToken();
	if (token.is(Token_EOF))
		return false;

	m_isLastTokenBlockEnd = token.is(Token_blockEnd);
	m_cursor.reset(token.offset + token.length);
	return true;
}

void Lexer::pushMode(LexerMode mode)
{
	m_modeStack.push_back(mode);
}

bool Lexer::popMode()
{
	if (m_modeStack.size() < 2)
		return false;

	m_modeStack.pop_back();
	return true;
}
