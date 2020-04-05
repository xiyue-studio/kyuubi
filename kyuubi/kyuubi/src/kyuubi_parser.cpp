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

	在最外层遇到 RAW_STRING，需要生成以下指令：
	PRNT 0, [RAW_STRING]

	在最外层遇到插值表达式，需要生成以下指令：
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

			// 处理空表达式
			if (!m_lexer->peekToken().is(Token_insertExprEnd))
			{
				parseInsertExpression();

				// 生成输出插值表达式的结果
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

	Filter 调用的代码生成逻辑：
	1. 调用前，栈顶保存等待应用过滤器的变量；
	2. 将参数从左往右以此压入栈顶；
	3. 压入一个整数，表示压入栈顶的参数个数；
	4. 生成 filter 指令。

	而 Filter 指令在执行的时候，按照以下顺序：
	1. 执行 FILT 指令，获取 filter 名称；
	2. 读取栈顶，获取参数个数；
	3. 依次读取栈内参数，传递给 filter（注意，是从右到左的顺序），filter 自行验证参数是否合法；
	4. filter 取出栈顶的变量，应用过滤器之后，再重新放回栈顶。

	这个地方，需要生成以下指令：
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
		// 生成 filter 调用
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
		// 生成 OR 指令
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
		// 生成 AND 指令
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
		// 生成加减指令
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
		// 生成乘除指令
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

	STRING, INT, REAL, BOOLEAN 都是直接生成在栈顶 load 常量的指令。
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

	对于第一个 ID，生成 LODV 指令去变量堆栈内查找变量并加载到栈顶。特别的，如果这个变量
	找不到，则会在当前作用域下生成一个同名的 null 局部变量。
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

	对于名称引用，生成 TVAN 指定，从栈顶的变量中找成员为 NAME 的变量，替换到栈顶。
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

	对于表达式引用，根据表达式结果，生成 TVAI 指令，以栈顶为索引，去次栈顶中找成员，
	并将查找结果替换栈顶和次栈顶。
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

	参数列表，生成以下指令：
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
		// 在栈顶生成参数的数量
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

	生成如下指令：
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

	IF 语句块的指令如下：
	[insert_expression_1]			; if insert_expression_1
	GOZF, L1, 0
	SBLK			; 进入一个作用域
	[kyuubi_1]
	EBLK			; 离开一个作用域
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

	// 解析 if 块
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

	// 回填跳转地址
	if (!hasElse)
	{
		// 没有 else 块，则最后一个 if 块无需使用 GOTO
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

	FOR 语句的指令逻辑：

	[variables]							; 生成一个循环变量在栈顶
	CKNF								; 检查循环是否可以进行
	GOZF L2								; 判定栈顶变量是否可循环，不可循环则跳到循环结束位置
	SBLK								; 进入 for 的第一层语句块
	FORS								; 初始化 for 循环的环境变量
	LODV 0, [index]					; L1: 生成一个用来接收 index 得到变量（如果声明了的话）
	LOFI								; 将当前循环的索引放到栈顶
	SAVE								; 将栈顶保存到次栈顶，栈顶出栈
	POPV								; 栈顶出栈
	[...]								; key 和 value 也用这种方式加载
	SBLK								; 进入循环体
	[kyuubi]
	EBLK								; 循环体结束
	NEXT								; 推进循环的下一个迭代，将推进结果放置到栈顶
	GOZF L1								; 循环还未结束，则继续下一次循环
	EBLK								; end for
	POPV							; L2: 平衡栈顶
*/
void Parser::parseForStatement()
{
	assert(m_lexer->peekToken().is(Token_for));
	m_lexer->consumeToken();

	// 获取循环变量名
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

	// 生成循环变量
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

	赋值语句生成如下指令：
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

	Filter 的指令如下：
	SFLT					; 切换 dumper，将后续输出重定向到一个内存缓存中
	SBLK
	[kyuubi]
	EBLK
	EFLT					; 将 dumper 中收集到的内容输出到栈顶上，并切回 dumper
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
		为了避免单行指令造成的输出空行，对于遇到的 rawString，做出如下特殊处理：
		1. 记录下 {% 之前的 rawString 内容；
		2. 记录下当前的 rawString 是不是跟在 %}；
		3. 如果 {% 之前的 rawString 最后一行全是空格，而且当前的 rawString 第二行之前全是空格，
			删除这一行。
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
