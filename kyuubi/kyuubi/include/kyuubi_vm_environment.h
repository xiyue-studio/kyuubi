#pragma once
#include "xiyue_const_string.h"
#include "xiyue_json_object.h"
#include "kyuubi_dumper.h"
#include "kyuubi_filter_manager.h"
#include "kyuubi_std_string_dumper.h"

namespace kyuubi
{
	class DataStack
	{
	public:
		bool empty() const { return m_values.empty(); }
		void pushConstant(xiyue::JsonObject obj);
		void pushVariable(xiyue::JsonObject* obj);
		xiyue::JsonObject topConstant();
		xiyue::JsonObject* topVariable();
		xiyue::JsonObject popConstant();
		xiyue::JsonObject* popVariable();
		void replaceConstant(xiyue::JsonObject newVal) { m_values.back() = newVal; }
		xiyue::JsonObject back() const { return m_values.back(); }
		xiyue::JsonObject* backPtr() const { return m_addrs.back(); }
		xiyue::JsonObject penultimate() const { return m_values[m_values.size() - 2]; }
		xiyue::JsonObject* penultimatePtr() const { return m_addrs[m_addrs.size() - 2]; }

	private:
		std::vector<xiyue::JsonObject> m_values;
		std::vector<xiyue::JsonObject*> m_addrs;
	};

	/*
		��ǰ�Ļ����У��м��������������

		* $__loop_object__$: ��ǰ����ѭ������
		* $__loop_index__$: ��ǰѭ�����±�
		* $__loop_key__$: ��ǰѭ���� key
		* $__loop_value__$: ��ǰѭ���� value
	*/
	class VMEnv
	{
	public:
		VMEnv(xiyue::JsonObject* data, Dumper* dp, FilterManager* filterManager);
		virtual ~VMEnv();

	public:
		static const xiyue::ConstString LOOP_OBJECT;
		static const xiyue::ConstString LOOP_INDEX;
		static const xiyue::ConstString LOOP_KEY;
		static const xiyue::ConstString LOOP_VALUE;

	public:
		int& pc() { return m_pc; }
		DataStack& dataStack() { return m_dataStack; }

		// ������һ�����������
		void pushVariableLevel();
		// �˳���ǰ������������
		bool popVariableLevel();
		// �������ƻ�ȡ��ǰ׺����
		xiyue::JsonObject* getVariableByName(xiyue::ConstString name);
		// �ڵ�ǰ������������ĳ������ֵ
		xiyue::JsonObject* setVariable(xiyue::ConstString name, xiyue::JsonObject val);
		// ��ջ��������Ѱ������Ϊ name �ĳ�Ա�������滻ջ��
		void loadVariableMemberByName(xiyue::ConstString name);
		// ȥջ�������в��� index ��Ա�����滻ջ��
		void loadVariableMemberByObject(xiyue::JsonObject index);
		// ��ջ�����浽��ջ���ı�����
		void doSaveAction();
		// �������ƻ�ȡ���ߴ���һ����ǰ׺����
		xiyue::JsonObject* getOrCreateVariableByName(xiyue::ConstString name);
		// ��ʼ�� for ѭ���Ļ����������ڵ�ǰ���������� index��key��value
		void initLoopVariables(xiyue::JsonObject loopVar);
		// ��� loopVar �Ƿ��ѭ����������Ͳ��� object ���� list�����׳�����ʱ�쳣
		void checkLoopTarget(xiyue::JsonObject loopVar);
		// ����ǰ�������ڵ�ѭ���������ƽ��������ƽ��Ƿ�ɹ����õ�ջ��
		void loopNext();
		// Ӧ��һ���������������������ջ��
		void doFilter(xiyue::ConstString filterName);

		Dumper* dumper();
		void pushDumperLevel();
		bool popDumperLevel();

	private:
		int m_pc;
		std::list<xiyue::JsonObject*> m_variableStack;
		std::vector<Dumper*> m_dumpers;
		DataStack m_dataStack;
		FilterManager* m_filterManager;
	};
}
