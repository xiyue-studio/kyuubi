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
			�ո�������
			1. {{ }} ǰ������пո񶼻ᱣ����
			2. {% %} ǰ�����еĿո񶼻�ɾ����ֱ���������з���
			3. {# #} ǰ�����еĿո񶼻ᱣ����

			�����
			1. ��һ�� rawString �Ի��з���β��Ȼ�������� {%��%} ����ȥ���ո�֮��ֱ�������˻��з�����ɾ��������з���
			2. �����һ�� rawString �ǿհ׵ģ����߲�������һ�� rawString �� 1 һ������
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
