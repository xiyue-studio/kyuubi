#include "stdafx.h"
#include "kyuubi_filter.h"
#include "kyuubi_exception.h"

using namespace std;
using namespace xiyue;
using namespace kyuubi;

Filter::Filter(
	ConstString filterName, /* 过滤器名称 */
	const list<JsonObjectType>& inputTypes, /* 过滤器输入类型列表 */
	JsonObjectType returnType, /* 过滤处理的返回结果 */
	ConstString desciption /* 过滤器描述 */
	)
	: m_filterName(filterName)
	, m_returnType(returnType)
	, m_inputTypes(inputTypes)
	, m_description(desciption)
	, m_requiredParamsCount(-1)
{
}

static bool ObjectType_isMatched(JsonObjectType l, JsonObjectType r)
{
	if (l == Json_string)
		return true;

	if (l == Json_real)
		return r == Json_int || r == Json_real;

	return l == r;
}

int Filter::getFilterParamsCount()
{
	return static_cast<int>(m_parameters.size());
}

void Filter::pushParameter(JsonObject param)
{
	m_realParameters.push_front(param);
}

void Filter::addParameterMeta(ConstString name, JsonObjectType type, JsonObject defaultValue)
{
	// 默认参数检查
	if (!m_parameters.empty()		// 至少有一个参数
		&& defaultValue.getType() == Json_null	// 当前参数没有默认值
		&& m_parameters.back().defaultValue.getType() != Json_null)	// 但是上一个参数有默认值
		throw KyuubiException(0, L"Parameter without default value can not defined after default parameters."_cs);

	m_parameters.push_back({ name, type, defaultValue });
}

void Filter::checkParameters()
{
	// 检查参数个数是否正确
	if (m_realParameters.size() < (size_t)getRequiredParamsCount())
		throw KyuubiException(0, XIYUE_CONST_STRING(L"Filter `%s` required %d parameters, but only %u given.",
			m_filterName.cstr(), getRequiredParamsCount(), m_realParameters.size()));

	if (m_realParameters.size() > (size_t)getFilterParamsCount())
		throw KyuubiException(0, XIYUE_CONST_STRING(L"Filter `%s` only required %d parameters, but %u given.",
			m_filterName.cstr(), getFilterParamsCount(), m_realParameters.size()));

	// 检查参数类型以及必需参数是否正确
	auto formalParameter = m_parameters.begin();
	auto realParameter = m_realParameters.begin();
	while (formalParameter != m_parameters.end())
	{
		if (realParameter == m_realParameters.end())
			realParameter = m_realParameters.insert(realParameter, formalParameter->defaultValue);
		else if (!ObjectType_isMatched(formalParameter->requiredType, realParameter->getType()))
			throw KyuubiException(0, L"Parameter type not matched."_cs);
		
		formalParameter++;
		realParameter++;
	}
}

int Filter::getRequiredParamsCount()
{
	if (m_requiredParamsCount >= 0)
		return m_requiredParamsCount;

	for (auto& it : m_parameters)
	{
		if (it.defaultValue.getType() == Json_null)
			m_requiredParamsCount++;
	}
	m_requiredParamsCount++;

	return m_requiredParamsCount;
}

void Filter::checkInput(JsonObject input)
{
	for (auto it : m_inputTypes)
	{
		if (ObjectType_isMatched(it, input.getType()))
			return;
	}

	throw KyuubiException(0, L"Error input type in filter."_cs);
}

JsonObject Filter::getParameter(int index)
{
	return m_realParameters[index];
}

JsonObject Filter::getParameter(ConstString name)
{
	auto it = std::find_if(m_parameters.begin(), m_parameters.end(), [name](const FilterParameter& val) -> bool {
		return val.name == name;
	});

	if (it == m_parameters.end())
		throw KyuubiException(0, XIYUE_CONST_STRING(L"No paramter named `%s` found.", name.cstr()));

	return m_realParameters[it - m_parameters.begin()];
}
