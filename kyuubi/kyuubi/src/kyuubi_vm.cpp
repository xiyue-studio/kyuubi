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
		case PRNT:		///< ��� arg2 ָ�����ַ����� dumper ��
			vmEnv.dumper()->dumpString(ins->arg2);
			break;
		case TVAN:		///< ���� arg2 ָ�������ƣ���ջ��������������ȡ���滻ջ��
			vmEnv.loadVariableMemberByName(ins->arg2);
			break;
		case POPV:		///< ������ջ������
			vmEnv.dataStack().popConstant();
			break;
		case TVAI:		///< ��ջ��Ϊ�±꣬ȥ��ջ�������в��ҳ�Ա��������ջ�����滻��ջ��
			vmEnv.loadVariableMemberByObject(dataStack.popConstant());
			break;
		case PRVA:		///< ��ջ���ı�������� dumper �У�������ջ��
			vmEnv.dumper()->dumpString(dataStack.popConstant());
			break;
		case LODS:		///< ��һ���ַ�����arg2�����ɳ����ŵ�ջ��
			dataStack.pushConstant(JsonObject(ins->arg2));
			break;
		case LODI:		///< ��һ��������arg2���ַ�����ʾ�����ɵĳ����ŵ�ջ��
			dataStack.pushConstant(JsonObject(ins->arg2.toInt()));
			break;
		case LODA:		///< ��ջ������һ�����ͳ�������ʾ�����ĸ���
			dataStack.pushConstant(JsonObject(ins->arg1));
			break;
		case LODF:		///< ��һ����������arg2���ַ�����ʾ�����ɵı����ŵ�ջ��
			dataStack.pushConstant(JsonObject(ins->arg2.toDouble()));
			break;
		case LODB:		///< ��һ�� boolean��arg1�����ɵı����ŵ�ջ��
			dataStack.pushConstant(JsonObject(ins->arg1 != 0));
			break;
		case ADDT:		///< ��ջ���ʹ�ջ����ջ����ӵĽ������ջ��
			dataStack.pushConstant(dataStack.popConstant() + dataStack.popConstant());
			break;
		case SUBT:		///< ��ջ����ȥ��ջ������ջ�������ջ
			dataStack.pushConstant(dataStack.popConstant() - dataStack.popConstant());
			break;
		case MULT:		///< ��ջ�����Դ�ջ������ջ�������ջ
			dataStack.pushConstant(dataStack.popConstant() * dataStack.popConstant());
			break;
		case DIVT:		///< ��ջ�����Դ�ջ������ջ�������ջ
			dataStack.pushConstant(dataStack.popConstant() / dataStack.popConstant());
			break;
		case NOTT:		///< ����ջ��������ջ��ֵ����ȡ�����ɳ�������ջ
			dataStack.pushConstant(JsonObject(!dataStack.popConstant()));
			break;
		case ANDT:		///< ��ջ���ʹ�ջ������ AND ��������ջ�������ջ
			dataStack.pushConstant(JsonObject(dataStack.popConstant() && dataStack.popConstant()));
			break;
		case ORST:		///< ��ջ���ʹ�ջ������ OR ��������ջ�������ջ
			dataStack.pushConstant(JsonObject(dataStack.popConstant() || dataStack.popConstant()));
			break;
		case GOZF:		///< �ж�ջ���������ֵ��������ջ�������Ϊ�٣�����ת�� arg1 ָ��λ��ִ��
			pcReseted = !dataStack.popConstant() ? (vmEnv.pc() = ins->arg1, true) : false;
			break;
		case GOTO:		///< ��ת�� arg1 ָ��λ�ü���ִ��
			pcReseted = (vmEnv.pc() = ins->arg1, true);
			break;
		case SAVE:		///< ��ջ���ı�����ֵ����ջ��������������ջ��
			vmEnv.doSaveAction();
			break;
		case LODV:		///< �������֣����������б���ȡ�����ߴ���һ����������ջ
			dataStack.pushVariable(vmEnv.getOrCreateVariableByName(ins->arg2));
			break;
		case CKNF:
			vmEnv.checkLoopTarget(dataStack.topConstant());
			break;
		case FORS:		///< ��ʼ�� for ѭ���Ļ����������ڵ�ǰ���������� index��key��value
			vmEnv.initLoopVariables(dataStack.topConstant());
			break;
		case LDFI:		///< ����ǰѭ���� index ���ص�ջ��
			dataStack.pushVariable(vmEnv.getVariableByName(VMEnv::LOOP_INDEX));
			break;
		case LDFK:		///< ����ǰѭ���� key ���ص�ջ��
			dataStack.pushVariable(vmEnv.getVariableByName(VMEnv::LOOP_KEY));
			break;
		case LDFV:		///< ����ǰѭ���� value ���ص�ջ��
			dataStack.pushVariable(vmEnv.getVariableByName(VMEnv::LOOP_VALUE));
			break;
		case FILT:		///< �ڵ�ǰλ��Ӧ��һ����������ջ�ڴ���ǲ���
			vmEnv.doFilter(ins->arg2);
			break;
		case SFLT:		///< Ӧ�ù���������㣬���� PRNT ָ��� PRVA ����ֱ�����
			vmEnv.pushDumperLevel();
			break;
		case EFLT:		///< Ӧ�ù��������յ�
			vmEnv.popDumperLevel();
			break;
		case SBLK:		///< ����һ���µľֲ�������
			vmEnv.pushVariableLevel();
			break;
		case EBLK:		///< �˳�һ���ֲ�������
			vmEnv.popVariableLevel();
			break;
		case NEXT:		///< ����ǰ�������ڵ�ѭ���������ƽ��������ƽ��Ƿ�ɹ����õ�ջ��
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
