#pragma once
#include "xiyue_const_string.h"

namespace kyuubi
{
	enum VmDirective
	{
		NOOP = 0,
		PRNT,		///< 输出 arg2 指定的字符串到 dumper 中
		TVAN,		///< 按照 arg2 指定的名称，在栈顶变量中索引，取出替换栈顶
		POPV,		///< 将变量栈顶弹出
		TVAI,		///< 以栈顶为下标，去次栈顶变量中查找成员，并弹出栈顶，替换次栈顶
		PRVA,		///< 将栈顶的变量输出到 dumper 中，并弹出栈顶
		LODS,		///< 将一个字符串（arg2）生成常量放到栈顶
		LODI,		///< 将一个整数（arg2，字符串表示）生成的常量放到栈顶
		LODA,		///< 在栈顶生成一个整型常量，表示参数的个数
		LODF,		///< 将一个浮点数（arg2，字符串表示）生成的变量放到栈顶
		LODB,		///< 将一个 boolean（arg1）生成的变量放到栈顶
		ADDT,		///< 将栈顶和次栈顶出栈，相加的结果放入栈顶
		SUBT,		///< 将栈顶减去次栈顶，出栈，结果入栈
		MULT,		///< 将栈顶乘以次栈顶，出栈，结果入栈
		DIVT,		///< 将栈顶乘以次栈顶，出栈，结果入栈
		NOTT,		///< 弹出栈顶，并对栈顶值进行取反生成常量，入栈
		ANDT,		///< 对栈顶和次栈顶进行 AND 操作，出栈，结果入栈
		ORST,		///< 对栈顶和次栈顶进行 OR 操作，出栈，结果入栈
		GOZF,		///< 判定栈顶结果的真值，并弹出栈顶，如果为假，则跳转到 arg1 指定位置执行
		GOTO,		///< 跳转到 arg1 指定位置继续执行
		SAVE,		///< 将栈顶的变量赋值给次栈顶变量，并弹出栈顶
		LODV,		///< 根据名字，从作用域列表中取出或者创建一个变量，入栈
		CKNF,		///< 检查栈顶变量是不是可循环的 map 或 list，并生成一个 bool 放置栈顶
		FORS,		///< 初始化 for 循环的环境变量，在当前作用域生成 index，key，value
		LDFI,		///< 将当前循环的 index 加载到栈顶
		LDFK,		///< 将当前循环的 key 加载到栈顶
		LDFV,		///< 将当前循环的 value 加载到栈顶
		FILT,		///< 在当前位置应用一个过滤器，栈内存的是参数
		SFLT,		///< 应用过滤器的起点，所有 PRNT 指令和 PRVA 不会直接输出
		EFLT,		///< 应用过滤器的终点
		SBLK,		///< 进入一个新的局部作用域
		EBLK,		///< 退出一个局部作用域
		NEXT,		///< 将当前作用域内的循环迭代器推进，并将推进是否成功放置到栈顶
		SUCC
	};

	const wchar_t* VmDirective_toString(VmDirective d);

	struct VmInstruction
	{
		VmDirective directive;
		int arg1;
		xiyue::ConstString arg2;
	};
}
