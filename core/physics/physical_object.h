/*
 * object.h
 *
 *  Created on: 2014年11月18日
 *      Author: salmon
 */

#ifndef CORE_PHYSICS_PHYSICAL_OBJECT_H_
#define CORE_PHYSICS_PHYSICAL_OBJECT_H_

#include <iostream>

namespace simpla
{
class DataSet;
class Properties;

struct PhysicalObject
{
	//! Default constructor
	PhysicalObject() :
			is_valid_(false)
	{
	}
	//! destroy.
	virtual ~PhysicalObject()
	{
	}

	PhysicalObject(const PhysicalObject&); // copy constructor.

	virtual std::string get_type_as_string() const=0;

	virtual DataSet dataset() const =0; //!< return the data set of PhysicalObject

	virtual bool is_valid()
	{
		return is_valid_;
	}

	virtual void sync()
	{

	}
	virtual void load()
	{
	}
	virtual bool update()
	{
		is_valid_ = true;
		return is_valid_;
	}

	virtual Properties const & properties(std::string const & name = "") const
	{
		return prop_[name];
	}

	virtual Properties & properties(std::string const & name = "")
	{
		return prop_[name];
	}

	virtual std::ostream& print(std::ostream & os) const
	{
		return prop_.print(os);

	}
private:
	Properties prop_;

	bool is_valid_ = false;

};
}  // namespace simpla

#endif /* CORE_PHYSICS_PHYSICAL_OBJECT_H_ */