/**
 * @file field_function.h
 *
 *  Created on: 2015-3-10
 *      Author: salmon
 */

#ifndef COREFieldField_FUNCTION_H_
#define COREFieldField_FUNCTION_H_

#include <stddef.h>
#include <cstdbool>
#include <functional>

#include "../gtl/primitives.h"
#include "field_traits.h"

namespace simpla
{
template<typename ...> class Field;

template<typename ...> class Domain;

template<typename ...TDomain, typename TV, typename TFun>
class Field<Domain<TDomain...>, TV, tags::function, TFun>
{
public:
	typedef Domain<TDomain...> domain_type;
	typedef TV value_type;
	typedef TFun function_type;

	typedef Field<domain_type, value_type, tags::function, function_type> this_type;

	typedef typename domain_type::mesh_type mesh_type;

	typedef typename mesh_type::id_type id_type;

	typedef typename mesh_type::point_type point_type;

	typedef traits::field_value_t <this_type> field_value_type;

	static constexpr int iform = domain_type::iform;
	static constexpr int ndims = domain_type::ndims;

private:
	function_type m_fun_;
	domain_type m_domain_;
	mesh_type const &m_mesh_;
public:


	Field(domain_type const &d) :
			m_domain_(d), m_mesh_(d.mesh())
	{
	}

	template<typename TF>
	Field(domain_type const &d, TF const &fun) :
			m_domain_(d), m_mesh_(d.mesh()), m_fun_(fun)
	{
	}

	Field(this_type const &other) :
			m_domain_(other.m_domain_), m_mesh_(other.m_mesh_), m_fun_(other.m_fun_)
	{
	}

	Field(this_type &&other) :
			m_domain_(other.m_domain_), m_mesh_(other.m_mesh_), m_fun_(other.m_fun_)
	{
	}

	~Field()
	{
	}

	bool is_valid() const
	{
		return (!!m_fun_) && m_domain_.is_valid();
	}

	operator bool() const
	{
		return !!m_fun_;
	}

	domain_type const &domain() const
	{
		return m_domain_;
	}

	value_type operator[](id_type s) const
	{
		Real t = m_mesh_.time();

		return m_mesh_.template sample<iform>(s,
				static_cast<field_value_type>(m_fun_(m_mesh_.point(s), t)));
	}

	field_value_type operator()(point_type const &x, Real t) const
	{
		return static_cast<field_value_type>(m_fun_(x, t));
	}

	template<typename ...Others>
	field_value_type operator()(point_type const &x, Real t,
			Others &&... others) const
	{
		return static_cast<field_value_type>(m_fun_(x, t,
				std::forward<Others>(others)...));
	}

	/**
	 *
	 * @param args
	 * @return (x,t) -> m_fun_(x,t,args(x,t))
	 */
	template<typename ...Args>
	Field<domain_type, value_type, tags::function,
			std::function<field_value_type(point_type const &, Real)>> op_on(
			Args const &...args) const
	{
		typedef std::function<field_value_type(point_type const &, Real)> res_function_type;
		typedef Field<domain_type, value_type, tags::function,
				res_function_type> res_type;

		res_function_type fun = [&](point_type const &x, Real t)
		{
			return static_cast<field_value_type>(m_fun_(x, t,
					static_cast<field_value_type>((args)(x))...));
		};

		return res_type(m_domain_, fun);

	}

};
namespace traits
{
template<typename TV, typename TDomain, typename TFun>
Field<TDomain, TV, tags::function, TFun> make_field_function(
		TDomain const &domain, TFun const &fun)
{
	return std::move(Field<TDomain, TV, tags::function, TFun>(domain, fun));
}

template<typename TV, typename TD, typename TDict>
Field<TD, TV, tags::function, TDict> //
make_function_by_config(TDict const &dict, TD domain)
{
	typedef TV value_type;

	typedef TD domain_type;

	typedef Field<domain_type, value_type, tags::function, TDict> field_type;

	// TODO create null filed

	if (dict["Domain"])
	{

//		filter_by_config(dict["Domain"], &domain);

		return field_type(domain, dict["Value"]);
	}
	else
	{
		domain.clear();
		return field_type(domain);

	}

}
}//namespace traits
}
// namespace simpla

#endif /* COREFieldField_FUNCTION_H_ */
