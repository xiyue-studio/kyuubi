#pragma once
#include "xiyue_const_string.h"

namespace kyuubi
{
	enum TokenType
	{
		Token_EOF,
		Token_error,
		Token_rawString,			///< ��ͨ�ַ���
		Token_insertExprStart,		///< {{
		Token_insertExprEnd,		///< }}
		Token_id,					///< ������
		Token_dot,					///< .
		Token_lParenthese,			///< (
		Token_rParenthese,			///< )
		Token_lBracket,				///< [
		Token_rBracket,				///< ]
		Token_string,				///< "str" �� 'str'
		Token_int,					///< ����
		Token_float,				///< ������
		Token_true,					///< true
		Token_false,				///< false
		Token_pipe,					///< |
		Token_comma,				///< ,
		Token_plus,					///< +
		Token_minus,				///< -
		Token_multiply,				///< *
		Token_divide,				///< /
		Token_comment,				///< {# ... #}
		Token_not,					///< !
		Token_and,					///< &&
		Token_or,					///< ||
		Token_blockStart,			///< {%
		Token_blockEnd,				///< %}
		Token_commentEnd,			///< #}
		Token_raw,					///< raw
		Token_endRaw,				///< endraw
		Token_if,					///< if
		Token_elseIf,				///< elseif
		Token_else,					///< else
		Token_endIf,				///< endif
		Token_for,					///< for
		Token_endFor,				///< endfor
		Token_in,					///< in
		Token_set,					///< set
		Token_equal,				///< =
		Token_filter,				///< filter
		Token_endFilter,			///< endFilter
		Token_ws,					///< �հ��ַ�
		Token_endRawTag				///< {% endraw %}
	};

	enum LexerMode
	{
		LexerMode_raw,
		LexerMode_insertionExpression,
		LexerMode_rawString,
	};
}
