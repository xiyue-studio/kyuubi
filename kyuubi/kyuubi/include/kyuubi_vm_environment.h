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
		当前的环境中，有几个特殊变量名：

		* $__loop_object__$: 当前块中循环对象
		* $__loop_index__$: 当前循环的下标
		* $__loop_key__$: 当前循环的 key
		* $__loop_value__$: 当前循环的 value
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

		// 进入下一层变量作用域
		void pushVariableLevel();
		// 退出当前变量的作用域
		bool popVariableLevel();
		// 根据名称获取无前缀变量
		xiyue::JsonObject* getVariableByName(xiyue::ConstString name);
		// 在当前作用域下设置某个变量值
		xiyue::JsonObject* setVariable(xiyue::ConstString name, xiyue::JsonObject val);
		// 从栈顶变量中寻找名称为 name 的成员变量，替换栈顶
		void loadVariableMemberByName(xiyue::ConstString name);
		// 去栈顶变量中查找 index 成员，并替换栈顶
		void loadVariableMemberByObject(xiyue::JsonObject index);
		// 将栈顶保存到次栈顶的变量中
		void doSaveAction();
		// 根据名称获取或者创建一个无前缀变量
		xiyue::JsonObject* getOrCreateVariableByName(xiyue::ConstString name);
		// 初始化 for 循环的环境变量，在当前作用域生成 index，key，value
		void initLoopVariables(xiyue::JsonObject loopVar);
		// 检查 loopVar 是否可循环，如果类型不是 object 或者 list，则抛出运行时异常
		void checkLoopTarget(xiyue::JsonObject loopVar);
		// 将当前作用域内的循环迭代器推进，并将推进是否成功放置到栈顶
		void loopNext();
		// 应用一个过滤器，将结果保存在栈顶
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
