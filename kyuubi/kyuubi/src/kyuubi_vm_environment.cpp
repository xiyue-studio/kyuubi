#include "stdafx.h"
#include "kyuubi_vm_environment.h"
#include "kyuubi_runtime_exception.h"

using namespace xiyue;
using namespace xiyue;
using namespace kyuubi;

const ConstString VMEnv::LOOP_OBJECT = L"$__loop_object__$"_cs;
const ConstString VMEnv::LOOP_INDEX = L"$__loop_index__$"_cs;
const ConstString VMEnv::LOOP_KEY = L"$__loop_key__$"_cs;
const ConstString VMEnv::LOOP_VALUE = L"$__loop_value__$"_cs;

void kyuubi::DataStack::pushConstant(JsonObject obj)
{
	m_values.push_back(obj);
	m_addrs.push_back(nullptr);
}

JsonObject* DataStack::popVariable()
{
	JsonObject* val = topVariable();
	m_values.pop_back();
	m_addrs.pop_back();
	return val;
}

JsonObject DataStack::popConstant()
{
	JsonObject val = topConstant();
	m_values.pop_back();
	m_addrs.pop_back();
	return val;
}

JsonObject* DataStack::topVariable()
{
	return m_addrs.back();
}

JsonObject DataStack::topConstant()
{
	return m_values.back();
}

void DataStack::pushVariable(JsonObject* obj)
{
	m_values.push_back(*obj);
	m_addrs.push_back(obj);
}

VMEnv::VMEnv(JsonObject* data, Dumper* dp, FilterManager* filterManager)
{
	m_pc = 0;
	m_variableStack.push_front(data);
	m_dumpers.push_back(dp);
	m_filterManager = filterManager;
}

VMEnv::~VMEnv()
{
	while (m_variableStack.size() > 1)
	{
		delete m_variableStack.front();
		m_variableStack.pop_front();
	}

	while (m_dumpers.size() > 1)
	{
		delete m_dumpers.back();
		m_dumpers.pop_back();
	}
}

void VMEnv::pushVariableLevel()
{
	JsonObject* obj = new JsonObject();
	*obj = JsonObject::object();
	m_variableStack.push_front(obj);
}

bool VMEnv::popVariableLevel()
{
	if (m_variableStack.size() < 2)
		return false;

	delete m_variableStack.front();
	m_variableStack.pop_front();
	return true;
}

JsonObject* VMEnv::getVariableByName(xiyue::ConstString name)
{
	// 沿着变量作用域一路往上找
	for (auto it : m_variableStack)
	{
		if (it->containsMember(name))
			return &((*it)[name]);
	}

	return nullptr;
}

void VMEnv::loadVariableMemberByName(xiyue::ConstString name)
{
	JsonObject obj = m_dataStack.popConstant();
	
	if (obj.getType() != Json_object)
		throw RuntimeException(0, L"Current variable is not map type."_cs);

	if (!obj.containsMember(name))
		throw RuntimeException(0, ConstString::makeByFormat(L"Member named `%s` does not exists.", name.cstr()));

	m_dataStack.pushVariable(&obj[name]);
}

void VMEnv::loadVariableMemberByObject(JsonObject index)
{
	if (index.getType() != Json_int && index.getType() != Json_string)
		throw RuntimeException(0, L"Index can only be integer or string."_cs);

	JsonObject obj = m_dataStack.popConstant();
	if (index.getType() == Json_int)
	{
		if (obj.getType() != Json_list)
			throw RuntimeException(0, L"Only list can index by integer.");

		m_dataStack.pushVariable(&obj[index.intValue()]);
	}
	else
	{
		if (obj.getType() != Json_object)
			throw RuntimeException(0, L"Only map can index by string.");

		m_dataStack.pushVariable(&obj[index]);
	}
}

void VMEnv::doSaveAction()
{
	JsonObject value = m_dataStack.popConstant();
	JsonObject* var = m_dataStack.topVariable();

	if (var == nullptr)
		throw RuntimeException(0, L"Can not save value to temporary variables.");

	*var = value;
	m_dataStack.replaceConstant(value);
}

JsonObject* VMEnv::getOrCreateVariableByName(ConstString name)
{
	JsonObject* result = getVariableByName(name);
	if (result != nullptr)
		return result;

	// 作用域中无此变量，在当前创建一个
	return setVariable(name, JsonObject());
}

void VMEnv::initLoopVariables(JsonObject loopVar)
{
	if (loopVar.getType() != Json_list && loopVar.getType() != Json_object)
		throw RuntimeException(0, L"Only list or map can be looped."_cs);

	setVariable(LOOP_OBJECT, loopVar);
	setVariable(LOOP_INDEX, JsonObject(0));
	if (loopVar.getType() == Json_list)
	{
		setVariable(LOOP_KEY, loopVar[0]);
	}
	else
	{
		setVariable(LOOP_KEY, JsonObject(loopVar.getKeyAtIndex(0)));
		setVariable(LOOP_VALUE, loopVar[loopVar.getKeyAtIndex(0)]);
	}

	// 添加 loop 变量
	JsonObject loop = JsonObject::object();
	loop[L"isLastItem"] = loopVar.getMemberCount() == 1;
	setVariable(L"loop", loop);
}

void VMEnv::loopNext()
{
	if (!m_variableStack.front()->containsMember(LOOP_INDEX))
		throw RuntimeException(0, L"No loop in current level can be next."_cs);

	int index = getVariableByName(LOOP_INDEX)->intValue() + 1;
	JsonObject* loopVar = getVariableByName(LOOP_OBJECT);
	if (index >= (int)loopVar->getMemberCount())
	{
		// 循环结束
		m_dataStack.pushConstant(JsonObject(true));
		return;
	}

	// 修改循环变量
	setVariable(LOOP_INDEX, JsonObject(index));
	if (loopVar->getType() == Json_list)
	{
		setVariable(LOOP_KEY, (*loopVar)[index]);
	}
	else
	{
		setVariable(LOOP_KEY, JsonObject(loopVar->getKeyAtIndex(index)));
		setVariable(LOOP_VALUE, (*loopVar)[loopVar->getKeyAtIndex(index)]);
	}

	// 修改 loop 变量
	JsonObject loop = JsonObject::object();
	loop[L"isLastItem"] = (int)loopVar->getMemberCount() == index + 1;
	setVariable(L"loop", loop);

	m_dataStack.pushConstant(JsonObject(false));
}

void VMEnv::doFilter(ConstString filterName)
{
	Filter* filter = m_filterManager->getFilterByName(filterName);
	if (filter == nullptr)
		throw RuntimeException(0, ConstString::makeByFormat(L"Filter `%s` does not defined.", filterName.cstr()));

	int argc = m_dataStack.popConstant().intValue();
	for (int i = 0; i < argc; ++i)
		filter->pushParameter(m_dataStack.popConstant());
	
	filter->checkParameters();
	filter->checkInput(m_dataStack.back());
	m_dataStack.pushConstant(filter->doFilter(m_dataStack.popConstant()));
	filter->clearParameters();
}

JsonObject* VMEnv::setVariable(xiyue::ConstString name, JsonObject val)
{
	m_variableStack.front()->setMember(name, val);
	return &m_variableStack.front()->operator[](name);
}

Dumper* VMEnv::dumper()
{
	return m_dumpers.back();
}

void VMEnv::pushDumperLevel()
{
	m_dumpers.push_back(new StdStringDumper());
}

bool VMEnv::popDumperLevel()
{
	if (m_dumpers.size() < 2)
		return false;

	StdStringDumper* dp = dynamic_cast<StdStringDumper*>(m_dumpers.back());
	// 将 dumper 的结果放入栈顶
	m_dataStack.pushConstant(dp->getString().c_str());
	m_dumpers.pop_back();
	delete dp;

	return true;
}

void VMEnv::checkLoopTarget(JsonObject loopVar)
{
	if (!loopVar.isObject() && !loopVar.isList())
		throw RuntimeException(0, L"Only list or object can loop."_cs);

	m_dataStack.pushConstant((bool)loopVar);
}
