#pragma once
#include "xiyue_json_object.h"

namespace kyuubi
{
	struct FilterParameter
	{
		xiyue::ConstString name;
		xiyue::JsonObjectType requiredType;
		xiyue::JsonObject defaultValue;
	};

	class Filter
	{
	public:
		Filter(
			xiyue::ConstString filterName,							///< 过滤器名称
			const std::list<xiyue::JsonObjectType>& inputTypes,		///< 过滤器输入类型列表
			xiyue::JsonObjectType returnType,						///< 过滤处理的返回结果
			xiyue::ConstString desciption							///< 过滤器描述
		);
		virtual ~Filter() = default;

	public:
		xiyue::ConstString getFilterName() const { return m_filterName; }
		xiyue::ConstString getFilterDescription() const { return m_description; }

		int getFilterParamsCount();
		int getRequiredParamsCount();
		virtual void pushParameter(xiyue::JsonObject param);
		virtual void addParameterMeta(xiyue::ConstString name, xiyue::JsonObjectType type, xiyue::JsonObject defaultValue);

		virtual void checkParameters();
		virtual void checkInput(xiyue::JsonObject input);
		virtual xiyue::JsonObject getParameter(int index);
		virtual xiyue::JsonObject getParameter(xiyue::ConstString name);
		virtual void clearParameters() { m_realParameters.clear(); }

		virtual xiyue::JsonObject doFilter(xiyue::JsonObject input) = 0;

	protected:
		int m_requiredParamsCount;
		xiyue::ConstString m_filterName;
		xiyue::ConstString m_description;
		std::list<xiyue::JsonObjectType> m_inputTypes;
		xiyue::JsonObjectType m_returnType;
		std::vector<FilterParameter> m_parameters;
		std::deque<xiyue::JsonObject> m_realParameters;
	};
}

#define BEGIN_FILTER(name)		name() : kyuubi::Filter(
#define FILTER_NAME(name)		xiyue::ConstString::makeUnmanagedString(L ## name),
#define FILTER_INPUT(type, ...)	{ type, __VA_ARGS__ },
#define FILTER_RETURN(type)		type,
#define FILTER_DESCRIPTION(d)	xiyue::ConstString::makeUnmanagedString(d) ) {
#define FILTER_OPTIONAL_PARAM(name, type, defaultValue) \
	kyuubi::Filter::addParameterMeta(xiyue::ConstString::makeUnmanagedString(L##name), type, defaultValue);
#define FILTER_PARAM(name, type)	FILTER_OPTIONAL_PARAM(name, type, xiyue::JsonObject())
#define END_FILTER()			}
