#pragma once
#include "kyuubi_lexer_types.h"
#include "kyuubi_lexer_token.h"

namespace kyuubi
{
	class Lexer
	{
	public:
		explicit Lexer(xiyue::ConstString templateString);
		virtual ~Lexer() = default;

	public:
		Token peekToken();
		bool consumeToken();

		void pushMode(LexerMode mode);
		bool popMode();

		bool isLastTokenBlockEnd() const { return m_isLastTokenBlockEnd; }

		/*
			空格保留规则：
			1. {{ }} 前后的所有空格都会保留；
			2. {% %} 前后所有的空格都会删除，直到遇到换行符；
			3. {# #} 前后所有的空格都会保留。

			如果：
			1. 上一个 rawString 以换行符结尾，然后遇到了 {%，%} 后面去掉空格之后直接遇到了换行符，则删除这个换行符。
			2. 如果上一个 rawString 是空白的，或者不存在上一个 rawString 和 1 一样处理。
		*/
		bool isLastBlockStartInLineHead() const { return m_isLastBlockStartInLineHead; }

	protected:
		void parseToken();

		bool tryParseRawToken();
		bool tryParseStartMarkups();
		bool tryParseEndMarkups();
		bool tryParseInsertionToken();
		bool tryParseWS();
		bool tryParseID();
		bool tryParseNumber();
		bool tryParseString();
		bool tryParseRawString();
		bool tryParseEndRawTag();

		void fillToken(TokenType type, int startPos, int endPos);

	private:
		xiyue::ConstString m_text;
		xiyue::ConstString m_rawStringBeforeBlock;
		bool m_isLastTokenBlockEnd;
		bool m_isLastBlockStartInLineHead;
		Token m_token;
		xiyue::ConstStringPointer m_cursor;
		std::vector<LexerMode> m_modeStack;
		int m_cacheStartPos;
		LexerMode m_cacheStartMode;
	};
}
