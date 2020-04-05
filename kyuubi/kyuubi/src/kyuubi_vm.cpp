#include "stdafx.h"
#include "kyuubi_vm.h"
#include "kyuubi_std_string_dumper.h"
#include "xiyue_json_object.h"
#include "kyuubi_vm_environment.h"

using namespace std;
using namespace xiyue;
using namespace kyuubi;

VM::VM(const VmInstruction* instructions, size_t instructionCount, bool needRelease)
{
	m_needRelease = needRelease;

	if (needRelease)
	{
		m_instructions = new VmInstruction[instructionCount];
		m_instructionCount = instructionCount;
		copy(instructions, instructions + instructionCount, m_instructions);
	}
	else
	{
		m_instructions = const_cast<VmInstruction*>(instructions);
		m_instructionCount = instructionCount;
	}

	m_data = nullptr;
	m_dumper = nullptr;
	m_filterManager = FilterManager::getBuiltinFilterManager();
}

VM::~VM()
{
	if (m_needRelease)
		delete[] m_instructions;
}

void VM::run()
{
	VMEnv vmEnv(m_data, m_dumper, m_filterManager);
	const VmInstruction* ins = m_instructions;
	JsonObject tpObj;
	auto& dataStack = vmEnv.dataStack();

	while (ins->directive != SUCC)
	{
		bool pcReseted = false;

		switch (ins->directive)
		{
		case NOOP:
			break;
		case PRNT:		///< 输出 arg2 指定的字符串到 dumper 中
			vmEnv.dumper()->dumpString(ins->arg2);
			break;
		case TVAN:		///< 按照 arg2 指定的名称，在栈顶变量中索引，取出替换栈顶
			vmEnv.loadVariableMemberByName(ins->arg2);
			break;
		case POPV:		///< 将变量栈顶弹出
			vmEnv.dataStack().popConstant();
			break;
		case TVAI:		///< 以栈顶为下标，去次栈顶变量中查找成员，并弹出栈顶，替换次栈顶
			vmEnv.loadVariableMemberByObject(dataStack.popConstant());
			break;
		case PRVA:		///< 将栈顶的变量输出到 dumper 中，并弹出栈顶
			vmEnv.dumper()->dumpString(dataStack.popConstant());
			break;
		case LODS:		///< 将一个字符串（arg2）生成常量放到栈顶
			dataStack.pushConstant(JsonObject(ins->arg2));
			break;
		case LODI:		///< 将一个整数（arg2，字符串表示）生成的常量放到栈顶
			dataStack.pushConstant(JsonObject(ins->arg2.toInt()));
			break;
		case LODA:		///< 在栈顶生成一个整型常量，表示参数的个数
			dataStack.pushConstant(JsonObject(ins->arg1));
			break;
		case LODF:		///< 将一个浮点数（arg2，字符串表示）生成的变量放到栈顶
			dataStack.pushConstant(JsonObject(ins->arg2.toDouble()));
			break;
		case LODB:		///< 将一个 boolean（arg1）生成的变量放到栈顶
			dataStack.pushConstant(JsonObject(ins->arg1 != 0));
			break;
		case ADDT:		///< 将栈顶和次栈顶出栈，相加的结果放入栈顶
			dataStack.pushConstant(dataStack.popConstant() + dataStack.popConstant());
			break;
		case SUBT:		///< 将栈顶减去次栈顶，出栈，结果入栈
			dataStack.pushConstant(dataStack.popConstant() - dataStack.popConstant());
			break;
		case MULT:		///< 将栈顶乘以次栈顶，出栈，结果入栈
			dataStack.pushConstant(dataStack.popConstant() * dataStack.popConstant());
			break;
		case DIVT:		///< 将栈顶乘以次栈顶，出栈，结果入栈
			dataStack.pushConstant(dataStack.popConstant() / dataStack.popConstant());
			break;
		case NOTT:		///< 弹出栈顶，并对栈顶值进行取反生成常量，入栈
			dataStack.pushConstant(JsonObject(!dataStack.popConstant()));
			break;
		case ANDT:		///< 对栈顶和次栈顶进行 AND 操作，出栈，结果入栈
			dataStack.pushConstant(JsonObject(dataStack.popConstant() && dataStack.popConstant()));
			break;
		case ORST:		///< 对栈顶和次栈顶进行 OR 操作，出栈，结果入栈
			dataStack.pushConstant(JsonObject(dataStack.popConstant() || dataStack.popConstant()));
			break;
		case GOZF:		///< 判定栈顶结果的真值，并弹出栈顶，如果为假，则跳转到 arg1 指定位置执行
			pcReseted = !dataStack.popConstant() ? (vmEnv.pc() = ins->arg1, true) : false;
			break;
		case GOTO:		///< 跳转到 arg1 指定位置继续执行
			pcReseted = (vmEnv.pc() = ins->arg1, true);
			break;
		case SAVE:		///< 将栈顶的变量赋值给次栈顶变量，并弹出栈顶
			vmEnv.doSaveAction();
			break;
		case LODV:		///< 根据名字，从作用域列表中取出或者创建一个变量，入栈
			dataStack.pushVariable(vmEnv.getOrCreateVariableByName(ins->arg2));
			break;
		case CKNF:
			vmEnv.checkLoopTarget(dataStack.topConstant());
			break;
		case FORS:		///< 初始化 for 循环的环境变量，在当前作用域生成 index，key，value
			vmEnv.initLoopVariables(dataStack.topConstant());
			break;
		case LDFI:		///< 将当前循环的 index 加载到栈顶
			dataStack.pushVariable(vmEnv.getVariableByName(VMEnv::LOOP_INDEX));
			break;
		case LDFK:		///< 将当前循环的 key 加载到栈顶
			dataStack.pushVariable(vmEnv.getVariableByName(VMEnv::LOOP_KEY));
			break;
		case LDFV:		///< 将当前循环的 value 加载到栈顶
			dataStack.pushVariable(vmEnv.getVariableByName(VMEnv::LOOP_VALUE));
			break;
		case FILT:		///< 在当前位置应用一个过滤器，栈内存的是参数
			vmEnv.doFilter(ins->arg2);
			break;
		case SFLT:		///< 应用过滤器的起点，所有 PRNT 指令和 PRVA 不会直接输出
			vmEnv.pushDumperLevel();
			break;
		case EFLT:		///< 应用过滤器的终点
			vmEnv.popDumperLevel();
			break;
		case SBLK:		///< 进入一个新的局部作用域
			vmEnv.pushVariableLevel();
			break;
		case EBLK:		///< 退出一个局部作用域
			vmEnv.popVariableLevel();
			break;
		case NEXT:		///< 将当前作用域内的循环迭代器推进，并将推进是否成功放置到栈顶
			vmEnv.loopNext();
			break;
		default:
			throw exception("Unsupported directive.");
		}

		if (!pcReseted)
			vmEnv.pc()++;

		ins = m_instructions + vmEnv.pc();
	}
}

void VM::dump(JsonObject* data, Dumper* dumper)
{
	m_data = data;
	m_dumper = dumper;
	run();
}
