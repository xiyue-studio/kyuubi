#pragma once
#include "kyuubi_filter.h"

namespace kyuubi
{
	class FilterManager
	{
	public:
		FilterManager(const std::initializer_list<Filter*>& ilist);

		static FilterManager* getBuiltinFilterManager();

	public:
		Filter* getFilterByName(xiyue::ConstString filterName);

	protected:
		std::unordered_map<xiyue::ConstString, Filter*> m_filters;
	};
}
