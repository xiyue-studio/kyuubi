#include "stdafx.h"
#include "kyuubi_parser.h"

using namespace std;
using namespace xiyue;
using namespace kyuubi;

Parser::Parser()
	: m_lexer(nullptr)
	, m_vm(nullptr)
{
}

Parser::~Parser()
{
}

VM* Parser::generateVM(xiyue::ConstString templateBuffer)
{
	m_vm = make_unique<VMGenerator>();
	m_lexer = make_unique<Lexer>(templateBuffer);
	parse();
	return m_vm->makeVM();
}

/*
	kyuubi            = (RAW_STRING | '{{' insert_expression '}}' | statement)+ EOF

	����������� RAW_STRING����Ҫ��������ָ�
	PRNT 0, [RAW_STRING]

	�������������ֵ���ʽ����Ҫ��������ָ�
	[expression code]
	PRVA 0, 0
*/
void Parser::parse()
{
	while (!m_lexer->peekToken().is(Token_EOF))
	{
		if (m_lexer->peekToken().is(Token_rawString))
		{
			ConstString currentRawString = normalizeRawString(m_lexer->peekToken().tokenString);
			if (!currentRawString.isEmpty())
				m_vm->generateInstruction(PRNT, 0, currentRawString);
			m_lexer->consumeToken();
		}
		else if (m_lexer->peekToken().is(Token_insertExprStart))
		{
			m_lexer->consumeToken();
			m_lexer->pushMode(LexerMode_insertionExpression);

			// ����ձ��ʽ
			if (!m_lexer->peekToken().is(Token_insertExprEnd))
			{
				parseInsertExpression();

				// ���������ֵ���ʽ�Ľ��
				m_vm->generateInstruction(PRVA);

			if (m_lexer->peekToken().isNot(Token_insertExprEnd))
					throw exception("Unexpected token, `}}` expected.");
			}

			m_lexer->consumeToken();
			m_lexer->popMode();
		}
		else if (m_lexer->peekToken().is(Token_blockStart))
		{
			m_lexer->consumeToken();
			m_lexer->pushMode(LexerMode_insertionExpression);

			parseStatement();

			m_lexer->popMode();
		}
		else
		{
			throw exception("Unexpected top level token.");
		}
	}

	m_vm->generateInstruction(SUCC);
}

/*
	insert_expression = expression ('|' ID arg_list?)*

	Filter ���õĴ��������߼���
	1. ����ǰ��ջ������ȴ�Ӧ�ù������ı�����
	2. ���������������Դ�ѹ��ջ����
	3. ѹ��һ����������ʾѹ��ջ���Ĳ���������
	4. ���� filter ָ�

	�� Filter ָ����ִ�е�ʱ�򣬰�������˳��
	1. ִ�� FILT ָ���ȡ filter ���ƣ�
	2. ��ȡջ������ȡ����������
	3. ���ζ�ȡջ�ڲ��������ݸ� filter��ע�⣬�Ǵ��ҵ����˳�򣩣�filter ������֤�����Ƿ�Ϸ���
	4. filter ȡ��ջ���ı�����Ӧ�ù�����֮�������·Ż�ջ����

	����ط�����Ҫ��������ָ�
	FILT 0, [filter name]
*/
void Parser::parseInsertExpression()
{
	parseExpression();

	while (m_lexer->peekToken().is(Token_pipe))
	{
		m_lexer->consumeToken();
		if (m_lexer->peekToken().isNot(Token_id))
		{
			throw exception("Unexpected token, filter name (ID) expected.");
		}
		ConstString filterName = m_lexer->peekToken().tokenString;
		m_lexer->consumeToken();
		if (m_lexer->peekToken().is(Token_lParenthese))
			parseArgList();
		else
			m_vm->generateInstruction(LODA, 0);
		// ���� filter ����
		m_vm->generateInstruction(FILT, 0, filterName);
	}
}

/*
	statement         = raw_statement
	                  | if_statement
	                  | for_statement
	                  | set_statement
	                  | filter_statement
*/
void Parser::parseStatement()
{
	switch (m_lexer->peekToken().type)
	{
	case Token_raw:
		parseRawStatement();
		break;
	case Token_if:
		parseIfStatement();
		break;
	case Token_for:
		parseForStatement();
		break;
	case Token_set:
		parseSetStatement();
		break;
	case Token_filter:
		parseFilterStatement();
		break;
	case Token_endRaw:
		throw exception("Unexpected `endraw` tag, no `raw` tag matched.");
	case Token_elseIf:
		throw exception("Unexpected `elseif` tag, no `if` tag matched.");
	case Token_else:
		throw exception("Unexpected `else` tag, no `if` tag matched.");
	case Token_endIf:
		throw exception("Unexpected `endif` tag, no `if` tag matched.");
	case Token_endFor:
		throw exception("Unexpected `endfor` tag, no `for` tag matched.");
	case Token_endFilter:
		throw exception("Unexpected `endfilter` tag, no `filter` tag matched.");
	default:
		throw exception("Unknown tag.");
	}
}

/*
	expression        = or_item ('||' or_item)*
*/
void Parser::parseExpression()
{
	parseOrItem();

	while (m_lexer->peekToken().is(Token_or))
	{
		m_lexer->consumeToken();
		parseOrItem();
		// ���� OR ָ��
		m_vm->generateInstruction(ORST);
	}
}

/*
	or_item           = and_item ('&&' and_item)*
*/
void Parser::parseOrItem()
{
	parseAndItem();

	while (m_lexer->peekToken().is(Token_and))
	{
		m_lexer->consumeToken();
		parseAndItem();
		// ���� AND ָ��
		m_vm->generateInstruction(ANDT);
	}
}

/*
	and_item          = sm_item (('+' | '-') sm_item)*
*/
void Parser::parseAndItem()
{
	parseSmItem();

	while (m_lexer->peekToken().is(Token_plus)
		|| m_lexer->peekToken().is(Token_minus))
	{
		bool isAdd = m_lexer->peekToken().is(Token_plus);
		m_lexer->consumeToken();
		parseSmItem();
		// ���ɼӼ�ָ��
		m_vm->generateInstruction(isAdd ? ADDT : SUBT);
	}
}

/*
	sm_item           = md_item (('*' | '/') md_item)*
*/
void Parser::parseSmItem()
{
	parseMdItem();

	while (m_lexer->peekToken().is(Token_multiply)
		|| m_lexer->peekToken().is(Token_divide))
	{
		bool isMult = m_lexer->peekToken().is(Token_multiply);
		m_lexer->consumeToken();
		parseMdItem();
		// ���ɳ˳�ָ��
		m_vm->generateInstruction(isMult ? MULT : DIVT);
	}
}

/*
	md_item           = '!'? atom
*/
void Parser::parseMdItem()
{
	bool needNotOp = false;
	if (m_lexer->peekToken().is(Token_not))
	{
		needNotOp = true;
		m_lexer->consumeToken();
	}

	parseAtom();

	if (needNotOp)
		m_vm->generateInstruction(NOTT);
}

/*
	atom              = STRING
	                  | INT
	                  | REAL
	                  | boolean
	                  | variable
	                  | '(' expression ')'

	STRING, INT, REAL, BOOLEAN ����ֱ��������ջ�� load ������ָ�
*/
void Parser::parseAtom()
{
	switch (m_lexer->peekToken().type)
	{
	case Token_string:
		m_vm->generateInstruction(LODS, 0, m_lexer->peekToken().tokenString);
		m_lexer->consumeToken();
		break;
	case Token_int:
		m_vm->generateInstruction(LODI, 0, m_lexer->peekToken().tokenString);
		m_lexer->consumeToken();
		break;
	case Token_float:
		m_vm->generateInstruction(LODF, 0, m_lexer->peekToken().tokenString);
		m_lexer->consumeToken();
		break;
	case Token_true:
		m_vm->generateInstruction(LODB, 1);
		m_lexer->consumeToken();
		break;
	case Token_false:
		m_vm->generateInstruction(LODB, 0);
		m_lexer->consumeToken();
		break;
	case Token_id:
		parseVariable();
		break;
	case Token_lParenthese:
		m_lexer->consumeToken();
		parseExpression();
		if (m_lexer->peekToken().isNot(Token_rParenthese))
			throw exception("Missing `)`.");
		m_lexer->consumeToken();
		break;
	default:
		throw exception("Unexcepted token. Atom expected.");
	}
}

/*
	variable          = ID (name_reference | index_reference)*

	���ڵ�һ�� ID������ LODV ָ��ȥ������ջ�ڲ��ұ��������ص�ջ�����ر�ģ�����������
	�Ҳ���������ڵ�ǰ������������һ��ͬ���� null �ֲ�������
	LODV 0, [var name]
*/
void Parser::parseVariable()
{
	assert(m_lexer->peekToken().is(Token_id));

	m_vm->generateInstruction(LODV, 0, m_lexer->peekToken().tokenString);
	m_lexer->consumeToken();

	for (;;)
	{
		if (m_lexer->peekToken().is(Token_dot))
			parseNameReference();
		else if (m_lexer->peekToken().is(Token_lBracket))
			parseIndexReference();
		else
			break;
	}
}

/*
	name_reference    = '.' ID

	�����������ã����� TVAN ָ������ջ���ı������ҳ�ԱΪ NAME �ı������滻��ջ����
	TVAN, 0, [ID]
*/
void Parser::parseNameReference()
{
	assert(m_lexer->peekToken().is(Token_dot));
	m_lexer->consumeToken();

	if (m_lexer->peekToken().isNot(Token_id))
		throw exception("Unexpected token, field name expected.");

	m_vm->generateInstruction(TVAN, 0, m_lexer->peekToken().tokenString);
	m_lexer->consumeToken();
}

/*
	index_reference   = '[' insert_expression ']'

	���ڱ��ʽ���ã����ݱ��ʽ��������� TVAI ָ���ջ��Ϊ������ȥ��ջ�����ҳ�Ա��
	�������ҽ���滻ջ���ʹ�ջ����
	TVAI, 0, 0
*/
void Parser::parseIndexReference()
{
	assert(m_lexer->peekToken().is(Token_lBracket));
	m_lexer->consumeToken();

	parseInsertExpression();
	m_vm->generateInstruction(TVAI);

	if (m_lexer->peekToken().isNot(Token_rBracket))
		throw exception("Missing `]`.");
	m_lexer->consumeToken();
}

/*
	arg_list          = '(' (insert_expression (',' insert_expression)*)? ')'

	�����б���������ָ�
	[arg1 expression]
	[arg2 expression]
	...
	LODA, [argc], 0
*/
void Parser::parseArgList()
{
	assert(m_lexer->peekToken().is(Token_lParenthese));
	m_lexer->consumeToken();

	if (m_lexer->peekToken().is(Token_rParenthese))
	{
		m_lexer->consumeToken();
		// ��ջ�����ɲ���������
		m_vm->generateInstruction(LODA, 0);
		return;
	}

	int argc = 1;
	parseInsertExpression();

	while (m_lexer->peekToken().is(Token_comma))
	{
		m_lexer->consumeToken();
		parseInsertExpression();
		argc++;
	}

	m_vm->generateInstruction(LODA, argc);

	if (m_lexer->peekToken().isNot(Token_rParenthese))
		throw exception("Missing `)`.");

	m_lexer->consumeToken();
}

/*
	raw_statement     = '{%' RAW '%}' RAW_STRING? '{%' END_RAW '%}'

	��������ָ�
	PRNT, 0, [RAW_STRING]
*/
void Parser::parseRawStatement()
{
	assert(m_lexer->peekToken().is(Token_raw));
	m_lexer->consumeToken();
	
	if (m_lexer->peekToken().isNot(Token_blockEnd))
		throw exception("Missing `%}`.");
	m_lexer->consumeToken();

	m_lexer->pushMode(LexerMode_rawString);
	if (m_lexer->peekToken().is(Token_raw))
	{
		m_vm->generateInstruction(PRNT, 0, m_lexer->peekToken().tokenString);
		m_lexer->consumeToken();
	}

	if (m_lexer->peekToken().isNot(Token_endRawTag))
		throw exception("Missing `{% endraw %}`.");

	m_lexer->consumeToken();
	m_lexer->popMode();
}

/*
	if_statement      = '{%' IF insert_expression '%}'
	                    kyuubi?
	                    (else_if_statement)*
	                    else_statement?
	                    '{%' END_IF '%}'

	IF �����ָ�����£�
	[insert_expression_1]			; if insert_expression_1
	GOZF, L1, 0
	SBLK			; ����һ��������
	[kyuubi_1]
	EBLK			; �뿪һ��������
	GOTO L4, 0
	[insert_expression_2]			; L1: elseif insert_expression_2
	GOZF L2, 0
	SBLK
	[kyuubi_2]
	EBLK
	GOTO L4, 0
	[insert_expression_3]			; L2: elseif insert_expression_3
	GOZF L3, 0
	SBLK
	[kyuubi_3]
	EBLK
	GOTO L4, 0
	SBLK							; L3: else
	[kyuubi_4]
	EBLK
	[other codes]					; L4: endif
*/
void Parser::parseIfStatement()
{
	unordered_set<TokenType> firstSets = {
		Token_elseIf,
		Token_else,
		Token_endIf
	};

	assert(m_lexer->peekToken().is(Token_if));
	m_lexer->consumeToken();

	parseInsertExpression();
	uint32_t jumpAddr = m_vm->generateInstruction(GOZF, 0);
	if (m_lexer->peekToken().isNot(Token_blockEnd))
		throw exception("Missing `%}`.");
	m_lexer->consumeToken();

	// ���� if ��
	m_vm->generateInstruction(SBLK);
	m_lexer->popMode();
	parseSubStatement(firstSets);
	m_vm->generateInstruction(EBLK);
	bool hasElse = true;
	vector<uint32_t> jumpOutAddrs;
	jumpOutAddrs.push_back(m_vm->generateInstruction(GOTO, 0));
	while (m_lexer->peekToken().is(Token_elseIf))
	{
		m_lexer->consumeToken();
		m_vm->getInstruction(jumpAddr).arg1 = m_vm->getCurrentAddress();
		parseInsertExpression();
		jumpAddr = m_vm->generateInstruction(GOZF, 0);
		if (m_lexer->peekToken().isNot(Token_blockEnd))
			throw exception("Missing `%}`.");
		m_lexer->consumeToken();
		m_lexer->popMode();

		m_vm->generateInstruction(SBLK);
		parseSubStatement(firstSets);
		m_vm->generateInstruction(EBLK);
		jumpOutAddrs.push_back(m_vm->generateInstruction(GOTO, 0));
	}

	firstSets.erase(Token_elseIf);
	firstSets.erase(Token_else);
	m_vm->getInstruction(jumpAddr).arg1 = m_vm->getCurrentAddress();
	if (m_lexer->peekToken().is(Token_else))
	{
		m_lexer->consumeToken();
		if (m_lexer->peekToken().isNot(Token_blockEnd))
			throw exception("Missing `%}`.");
		m_lexer->consumeToken();
		m_lexer->popMode();

		m_vm->generateInstruction(SBLK);
		parseSubStatement(firstSets);
		m_vm->generateInstruction(EBLK);
	}
	else
	{
		hasElse = false;
	}

	assert(m_lexer->peekToken().is(Token_endIf));
	m_lexer->consumeToken();
	if (m_lexer->peekToken().isNot(Token_blockEnd))
		throw exception("Missing `%}`.");
	m_lexer->consumeToken();

	// ������ת��ַ
	if (!hasElse)
	{
		// û�� else �飬�����һ�� if ������ʹ�� GOTO
		m_vm->popInstruction();
		jumpOutAddrs.pop_back();
		m_vm->getInstruction(jumpAddr).arg1--;
	}

	for (auto addr : jumpOutAddrs)
	{
		m_vm->getInstruction(addr).arg1 = m_vm->getCurrentAddress();
	}
}

void Parser::parseSubStatement(const unordered_set<kyuubi::TokenType>& expectedCommands)
{
	for (;;)
	{
		if (m_lexer->peekToken().is(Token_rawString))
		{
			ConstString currentRawString = normalizeRawString(m_lexer->peekToken().tokenString);
			if (!currentRawString.isEmpty())
				m_vm->generateInstruction(PRNT, 0, currentRawString);
			m_lexer->consumeToken();
		}
		else if (m_lexer->peekToken().is(Token_insertExprStart))
		{
			m_lexer->consumeToken();
			m_lexer->pushMode(LexerMode_insertionExpression);
			parseInsertExpression();
			m_vm->generateInstruction(PRVA);
			if (m_lexer->peekToken().isNot(Token_insertExprEnd))
				throw exception("Missing `}}`.");
			m_lexer->consumeToken();
			m_lexer->popMode();
		}
		else if (m_lexer->peekToken().is(Token_blockStart))
		{
			m_lexer->consumeToken();
			m_lexer->pushMode(LexerMode_insertionExpression);
			if (expectedCommands.find((TokenType)m_lexer->peekToken().type) != expectedCommands.end())
				break;

			parseStatement();
			m_lexer->popMode();
		}
		else
		{
			throw exception("Unexpected token in block.");
		}
	}
}

/*
	for_statement     = '{%' FOR loop_vars IN variable '%}' kyuubi? '{%' END_FOR '%}'

	FOR ����ָ���߼���

	[variables]							; ����һ��ѭ��������ջ��
	CKNF								; ���ѭ���Ƿ���Խ���
	GOZF L2								; �ж�ջ�������Ƿ��ѭ��������ѭ��������ѭ������λ��
	SBLK								; ���� for �ĵ�һ������
	FORS								; ��ʼ�� for ѭ���Ļ�������
	LODV 0, [index]					; L1: ����һ���������� index �õ���������������˵Ļ���
	LOFI								; ����ǰѭ���������ŵ�ջ��
	SAVE								; ��ջ�����浽��ջ����ջ����ջ
	POPV								; ջ����ջ
	[...]								; key �� value Ҳ�����ַ�ʽ����
	SBLK								; ����ѭ����
	[kyuubi]
	EBLK								; ѭ�������
	NEXT								; �ƽ�ѭ������һ�����������ƽ�������õ�ջ��
	GOZF L1								; ѭ����δ�������������һ��ѭ��
	EBLK								; end for
	POPV							; L2: ƽ��ջ��
*/
void Parser::parseForStatement()
{
	assert(m_lexer->peekToken().is(Token_for));
	m_lexer->consumeToken();

	// ��ȡѭ��������
	ConstString keyVar, valueVar, indexVar;
	if (m_lexer->peekToken().is(Token_id))
	{
		keyVar = m_lexer->peekToken().tokenString;
		m_lexer->consumeToken();
	}
	else if (m_lexer->peekToken().is(Token_lParenthese))
	{
		m_lexer->consumeToken();
		if (m_lexer->peekToken().isNot(Token_id))
			throw exception("Missing loop variables.");
		keyVar = m_lexer->peekToken().tokenString;

		if (m_lexer->peekToken().is(Token_comma))
		{
			m_lexer->consumeToken();
			if (m_lexer->peekToken().isNot(Token_id))
				throw exception("Missing loop variables.");
			indexVar = m_lexer->peekToken().tokenString;
			m_lexer->consumeToken();
		}

		if (m_lexer->peekToken().is(Token_comma))
		{
			m_lexer->consumeToken();
			valueVar = indexVar;
			if (m_lexer->peekToken().isNot(Token_id))
				throw exception("Missing loop variables.");
			indexVar = m_lexer->peekToken().tokenString;
			m_lexer->consumeToken();
		}

		if (m_lexer->peekToken().isNot(Token_rParenthese))
			throw exception("Missing `)`.");
		m_lexer->consumeToken();
	}
	else
	{
		throw exception("Missing loop variables.");
	}

	if (m_lexer->peekToken().isNot(Token_in))
		throw exception("Missing `in`.");
	m_lexer->consumeToken();

	parseVariable();
	m_vm->generateInstruction(CKNF);
	uint32_t noLoopJumpAddr = m_vm->generateInstruction(GOZF, 0);
	m_vm->generateInstruction(SBLK);
	m_vm->generateInstruction(FORS);
	uint32_t forCheckAddr = m_vm->getCurrentAddress();

	// ����ѭ������
	if (!indexVar.isEmpty())
	{
		m_vm->generateInstruction(LODV, 0, indexVar);
		m_vm->generateInstruction(LDFI);
		m_vm->generateInstruction(SAVE);
		m_vm->generateInstruction(POPV);
	}
	if (!valueVar.isEmpty())
	{
		m_vm->generateInstruction(LODV, 0, valueVar);
		m_vm->generateInstruction(LDFV);
		m_vm->generateInstruction(SAVE);
		m_vm->generateInstruction(POPV);
	}
	if (!keyVar.isEmpty())
	{
		m_vm->generateInstruction(LODV, 0, keyVar);
		m_vm->generateInstruction(LDFK);
		m_vm->generateInstruction(SAVE);
		m_vm->generateInstruction(POPV);
	}

	if (m_lexer->peekToken().isNot(Token_blockEnd))
		throw exception("Missing `%}`.");
	m_lexer->consumeToken();
	m_lexer->popMode();

	m_vm->generateInstruction(SBLK);
	parseSubStatement({ Token_endFor });
	m_vm->generateInstruction(EBLK);

	assert(m_lexer->peekToken().is(Token_endFor));
	m_lexer->consumeToken();
	if (m_lexer->peekToken().isNot(Token_blockEnd))
		throw exception("Missing `%}`.");
	m_lexer->consumeToken();

	m_vm->generateInstruction(NEXT);
	m_vm->generateInstruction(GOZF, forCheckAddr);
	m_vm->generateInstruction(EBLK);
	m_vm->getInstruction(noLoopJumpAddr).arg1 = m_vm->getCurrentAddress();
	m_vm->generateInstruction(POPV);
}

/*
	set_statement     = '{%' SET variable '=' insert_expression '%}'

	��ֵ�����������ָ�
	[variable]
	[insert_expression]
	SAVE 0, 0
	POPV 0, 0
*/
void Parser::parseSetStatement()
{
	assert(m_lexer->peekToken().is(Token_set));
	m_lexer->consumeToken();
	parseVariable();

	if (m_lexer->peekToken().isNot(Token_equal))
		throw exception("Missing `=`.");
	m_lexer->consumeToken();

	parseInsertExpression();

	m_vm->generateInstruction(SAVE);
	m_vm->generateInstruction(POPV);

	if (m_lexer->peekToken().isNot(Token_blockEnd))
		throw exception("Missing `%}`.");
	m_lexer->consumeToken();
}

/*
	filter_statement  = '{%' FILTER ID arg_list? '%}' kyuubi? '{%' END_FILTER '%}'

	Filter ��ָ�����£�
	SFLT					; �л� dumper������������ض���һ���ڴ滺����
	SBLK
	[kyuubi]
	EBLK
	EFLT					; �� dumper ���ռ��������������ջ���ϣ����л� dumper
	[arg list]
	FILT 0, [filter name]
	PRVA
*/
void Parser::parseFilterStatement()
{
	assert(m_lexer->peekToken().is(Token_filter));
	m_lexer->consumeToken();

	vector<VmInstruction> instructions;
	m_vm->swapInstructions(&instructions);
	if (m_lexer->peekToken().isNot(Token_id))
		throw exception("Missing filter name.");

	ConstString filterName = m_lexer->peekToken().tokenString;
	m_lexer->consumeToken();
	if (m_lexer->peekToken().is(Token_lParenthese))
		parseArgList();
	else
		m_vm->generateInstruction(LODA, 0);
	m_vm->generateInstruction(FILT, 0, filterName);
	m_vm->swapInstructions(nullptr);

	if (m_lexer->peekToken().isNot(Token_blockEnd))
		throw exception("Missing `%}`.");
	m_lexer->consumeToken();
	m_lexer->popMode();

	m_vm->generateInstruction(SFLT);
	m_vm->generateInstruction(SBLK);
	parseSubStatement({ Token_endFilter });
	m_vm->generateInstruction(EBLK);
	m_vm->generateInstruction(EFLT);

	assert(m_lexer->peekToken().is(Token_endFilter));
	m_lexer->consumeToken();
	if (m_lexer->peekToken().isNot(Token_blockEnd))
		throw exception("Missing `%}`.");
	m_lexer->consumeToken();

	m_vm->appendInstructions(&instructions);
	m_vm->generateInstruction(PRVA);
}

ConstString Parser::normalizeRawString(const ConstString& str)
{
	/*
		Ϊ�˱��ⵥ��ָ����ɵ�������У����������� rawString�������������⴦��
		1. ��¼�� {% ֮ǰ�� rawString ���ݣ�
		2. ��¼�µ�ǰ�� rawString �ǲ��Ǹ��� %}��
		3. ��� {% ֮ǰ�� rawString ���һ��ȫ�ǿո񣬶��ҵ�ǰ�� rawString �ڶ���֮ǰȫ�ǿո�
			ɾ����һ�С�
	*/
	ConstString currentRawString = str;
	if (m_lexer->isLastTokenBlockEnd() && m_lexer->isLastBlockStartInLineHead())
	{
		if (currentRawString[0] == '\r')
			currentRawString = currentRawString.substr(1);
		if (currentRawString[0] == '\n')
			currentRawString = currentRawString.substr(1);
	}

	return currentRawString;
}
