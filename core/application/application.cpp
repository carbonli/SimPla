/*
 * application.cpp
 *
 *  Created on: 2015年1月7日
 *      Author: salmon
 */
#include "application.h"
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include "../utilities/utilities.h"

namespace simpla
{

std::string SpAppList::add(std::string const & name,
		std::shared_ptr<SpApp> const & p)
{
	(*this)[name] = p;
	return "SpApp" + ToString(this->size()) + "_" + name;
}

std::ostream & SpAppList::print(std::ostream & os)
{
	for (auto const & item : (*this))
	{
		os << item.first << std::endl;
	}
	return os;
}

void SpAppList::run(int argc, char ** argv)
{
	for (auto const & item : (*this))
	{

		LOGGER << "Case [" << item.first << "] initialize ." << std::endl;

		item.second->setup(argc, argv);

		LOGGER << "Case [" << item.first << "] start." << std::endl;

		item.second->body();

		LOGGER << "Case [" << item.first << "] done." << std::endl;
	}
}

}  // namespace simpla