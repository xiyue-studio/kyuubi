#include "stdafx.h"
#include "kyuubi_kyuubi_generator.h"
#include "kyuubi_kyuubi.h"
#include "kyuubi_parser.h"
#include "kyuubi_exception.h"

using namespace std;
using namespace xiyue;
using namespace kyuubi;

/*
	以下模板定义在 generated_header.wanling 和 generated_code.wanling 中。
*/
const wchar_t* g_headerTemplate =
LR"({{ autoGeneratedDelcare }}
#pragma once
{{ hHeader }}
#include "xiyue_json_object.h"
#include "kyuubi_vm.h"

{% set indentLevel = 0 %}
{% if namespace %}
namespace {{ namespace }}
{
{% set indentLevel = 1 %}
{% endif %}
{% filter indent(indentLevel, '	') %}
class {{ name | camelCase(true) }}
{
public:
	static xiyue::ConstString dumpToString(xiyue::JsonObject data);
	static void dumpToFile(xiyue::JsonObject data, xiyue::ConstString fileName);

private:
	static kyuubi::VM m_vm;
};
{% endfilter %}
{% if namespace %}
}
{% endif %}
{{ hFooter }})";

const wchar_t* g_codeTemplate =
LR"({{ autoGeneratedDelcare }}
{{ contentHeader }}
#include "{{ headerFile }}"
#include <string>
#include "xiyue_string_file_writer.h"
#include "kyuubi_std_string_dumper.h"
#include "kyuubi_runtime_exception.h"

using namespace std;
using namespace xiyue;
using namespace kyuubi;
{% if namespace %}
using namespace {{ namespace }};
{% endif %}

/*
	VM Instructions
*/
static VmInstruction g_instructions[] = {
{% for ins in instructions %}
	{ {{ ins.directive }}, {{ ins.arg1 }}, L"{{ ins.arg2 | escapeCpp }}"_cs }{% if !loop.isLastItem %},{% endif %}
{% endfor %}
};

/*
	Global Virtual Machine
*/
VM {{ name | camelCase(true) }}::m_vm(g_instructions);

/*
	Dump functions
*/
ConstString {{ name | camelCase(true) }}::dumpToString(JsonObject data)
{
	StdStringDumper dumper;
	m_vm.dump(&data, &dumper);
	return dumper.getString().c_str();
}

void {{ name | camelCase(true) }}::dumpToFile(JsonObject data, ConstString fileName)
{
	ConstString result = dumpToString(data);

	StringFileWriter writer(result.length());
	if (!writer.open(fileName))
		throw RuntimeException(0, XIYUE_CONST_STRING(L"Can not write string to file `%s`.", fileName.cstr()));

	writer.writeString(result);
	writer.close();
}

{{ contentFooter }})";

/*
	自动生成说明
*/
const wchar_t* g_autoGeneratedComment =
LR"(/**
	 _                      _     _ 
	| |                    | |   (_)
	| | ___   _ _   _ _   _| |__  _ 
	| |/ / | | | | | | | | | '_ \| |
	|   <| |_| | |_| | |_| | |_) | |
	|_|\_\\__, |\__,_|\__,_|_.__/|_|
	       __/ |                    
	      |___/                     

	This file is generated automatically by Kyuubi of xiyue studio.
	Please DO NOT modify this file manually.

	Copyright (c) 2020, xiyue-studio, Tecyle
	All rights reserved.
*/)";

/*
	全局 Kyuubi 对象
*/
static Kyuubi g_headerKyuubi = ConstString::makeUnmanagedString(g_headerTemplate);
static Kyuubi g_codeKyuubi = ConstString::makeUnmanagedString(g_codeTemplate);

/*
	Kyuubi 生成器
*/
bool KyuubiGenerator::generate(const xiyue::ConstString& templateString, const xiyue::ConstString& targetFolder)
{
	JsonObject root = JsonObject::object();
	root[L"name"] = m_name;
	root[L"namespace"] = m_namespace;
	root[L"hHeader"] = m_hHeader;
	root[L"hFooter"] = m_hFooter;
	root[L"autoGeneratedDelcare"] = g_autoGeneratedComment;
	root[L"contentHeader"] = m_contentHeader;
	root[L"contentFooter"] = m_contentFooter;
	root[L"headerFile"] = XIYUE_CONST_STRING(L"%s.h", m_fileName.cstr());

	try
	{
		Parser parser;
		VM* vm = parser.generateVM(templateString);
		JsonObject jsonIns = JsonObject::list(vm->getInstructionCount());
		const VmInstruction* instructions = vm->getInstructions();
		for (size_t i = 0; i < vm->getInstructionCount(); ++i)
		{
			const VmInstruction* ins = instructions + i;
			jsonIns.append(JsonObject::object({
				{L"directive", VmDirective_toString(ins->directive)},
				{L"arg1", ins->arg1},
				{L"arg2", ins->arg2}
				}));
		}
		root[L"instructions"] = jsonIns;
		delete vm;
	}
	catch (KyuubiException e)
	{
		XIYUE_LOG_ERROR("Errors in parsing: %s.", e.message().cstr());
		return false;
	}

	if (!g_headerKyuubi.generateToFile(&root, XIYUE_CONST_STRING(L"%s/%s.h", targetFolder.cstr(), m_fileName.cstr())))
		return false;

	if (!g_codeKyuubi.generateToFile(&root, XIYUE_CONST_STRING(L"%s/%s.cpp", targetFolder.cstr(), m_fileName.cstr())))
		return false;

	return true;
}
