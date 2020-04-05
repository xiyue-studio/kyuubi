#pragma once
#include "xiyue_const_string.h"
#include "kyuubi_lexer.h"
#include "kyuubi_vm.h"
#include "kyuubi_vm_generator.h"

namespace kyuubi
{
	/*
		kyuubi            = (RAW_STRING | '{{' insert_expression '}}' | statement)+
		insert_expression = expression ('|' ID arg_list?)*
		statement         = raw_statement
		                  | if_statement
		                  | for_statement
		                  | set_statement
		                  | filter_statement
		expression        = or_item ('||' or_item)*
		or_item           = and_item ('&&' and_item)*
		and_item          = sm_item (('+' | '-') sm_item)*
		sm_item           = md_item (('*' | '/') md_item)*
		md_item           = '!'? atom
		atom              = STRING
		                  | INT
		                  | REAL
		                  | boolean
		                  | variable
		                  | '(' expression ')'
		boolean           = TRUE | FALSE
		variable          = ID (name_reference | index_reference)*
		name_reference    = '.' ID
		index_reference   = '[' insert_expression ']'
		arg_list          = '(' (insert_expression (',' insert_expression)*)? ')'
		raw_statement     = '{%' RAW '%}' RAW_STRING? '{%' END_RAW '%}'
		if_statement      = '{%' IF insert_expression '%}'
		                    kyuubi?
		                    (else_if_statement)*
		                    else_statement?
		                    '{%' END_IF '%}'
		else_if_statement = '{%' ELSE_IF insert_expression '%}' kyuubi?
		else_statement    = '{%' ELSE '%}' kyuubi?
		for_statement     = '{%' FOR loop_vars IN variable '%}' kyuubi? '{%' END_FOR '%}'
		set_statement     = '{%' SET variable '=' insert_expression '%}'
		filter_statement  = '{%' FILTER ID arg_list? '%}' kyuubi? '{%' END_FILTER '%}'
		loop_vars         = ID | '(' ID (',' ID)<0,2> ')'
	*/
	class Parser
	{
	public:
		Parser();
		virtual ~Parser();

	public:
		VM* generateVM(xiyue::ConstString templateBuffer);

	protected:
		void parse();
		void parseInsertExpression();
		void parseStatement();
		void parseExpression();
		void parseOrItem();
		void parseAndItem();
		void parseSmItem();
		void parseMdItem();
		void parseAtom();
		void parseVariable();
		void parseNameReference();
		void parseIndexReference();
		void parseArgList();
		void parseRawStatement();
		void parseIfStatement();
		void parseSubStatement(const std::unordered_set<TokenType>& expectedCommands);
		void parseForStatement();
		void parseSetStatement();
		void parseFilterStatement();

		xiyue::ConstString normalizeRawString(const xiyue::ConstString& str);

	protected:
		std::unique_ptr<Lexer> m_lexer;
		std::unique_ptr<VMGenerator> m_vm;
	};
}
