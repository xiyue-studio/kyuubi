#include "stdafx.h"
#include "kyuubi_filter_manager.h"
#include "kyuubi_format_real_filter.h"
#include "kyuubi_indent_filter.h"
#include "kyuubi_camel_case_filter.h"
#include "kyuubi_escape_cpp_filter.h"

using namespace std;
using namespace xiyue;
using namespace kyuubi;

static FormatRealFilter g_formatRealFilter;
static IndentFilter g_indentFilter;
static CamelCaseFilter g_camelCaseFilter;
static EscapeCppFilter g_escapeCppFilter;

static FilterManager g_builtinFilterManager = {
	&g_formatRealFilter,
	&g_indentFilter,
	&g_camelCaseFilter,
	&g_escapeCppFilter
};

FilterManager* FilterManager::getBuiltinFilterManager()
{
	return &g_builtinFilterManager;
}

Filter* FilterManager::getFilterByName(ConstString filterName)
{
	auto it = m_filters.find(filterName);
	if (it == m_filters.end())
		return nullptr;

	return it->second;
}

FilterManager::FilterManager(const std::initializer_list<Filter*>& ilist)
{
	for (auto filter : ilist)
	{
		m_filters[filter->getFilterName()] = filter;
	}
}
