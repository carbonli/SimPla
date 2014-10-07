/*
 * manifold.h
 *
 *  Created on: 2014年9月23日
 *      Author: salmon
 */

#ifndef MANIFOLD_H_
#define MANIFOLD_H_

#include <utility>
#include <vector>
#include "../utilities/sp_type_traits.h"
#include "../utilities/primitives.h"
#include <valarray>
namespace simpla
{

template<typename > class FiniteDiffMethod;
template<typename > class InterpolatorLinear;
template<typename, unsigned int> class Domain;
template<typename ... > class Field;

template<typename TG, //
		template<typename > class Policy1 = FiniteDiffMethod, //
		template<typename > class Policy2 = InterpolatorLinear>
class Manifold: public TG, public Policy1<TG>, public Policy2<TG>
{
public:

	typedef Manifold<TG, Policy1, Policy2> this_type;
	typedef TG geometry_type;
	typedef Policy1<geometry_type> policy1;
	typedef Policy2<geometry_type> policy2;
	typedef typename geometry_type::topology_type topology_type;
	typedef typename geometry_type::coordinates_type coordiantes_type;
	typedef typename geometry_type::index_type index_type;

	template<typename ...Args>
	Manifold(Args && ... args) :
			geometry_type(std::forward<Args>(args)...), //
			policy1(dynamic_cast<geometry_type const &>(*this)), //
			policy2(dynamic_cast<geometry_type const &>(*this))
	{
	}

	~Manifold() = default;

	Manifold(this_type const & r) :
			geometry_type(dynamic_cast<geometry_type const &>(r)), //
			policy1(dynamic_cast<policy1 const &>(r)), //
			policy2(dynamic_cast<policy2 const &>(r))
	{
	}
	this_type & operator=(this_type const &) = delete;

	template<unsigned int IFORM = VERTEX>
	Domain<this_type, IFORM> domain() const
	{
		return std::move(Domain<this_type, IFORM>(*this));
	}
	template<unsigned int IFORM, typename TV> using
	field=Field<Domain<this_type,IFORM>,std::vector<TV> >;

	template<typename TF> TF make_field() const
	{
		return std::move(TF(domain<TF::iform>()));
	}
	template<typename T>
	auto get_value(T const & expr, index_type const & s) const
	DECL_RET_TYPE((simpla::get_value(expr,s)))

	template<typename M, unsigned int I, typename TL>
	auto get_value(Field<Domain<M, I>, TL> const & expr,
			index_type const & s) const
			DECL_RET_TYPE((expr[s]))

	template<typename TOP, typename TL>
	auto get_value(Field<Expression<TOP, TL>> const & expr,
			index_type const & s) const
			DECL_RET_TYPE((calcluate(expr.op_, expr.lhs, s)))

	template<typename TOP, typename TL, typename TR>
	auto get_value(Field<Expression<TOP, TL, TR>> const & expr,
			index_type const & s) const
			DECL_RET_TYPE((calcluate(expr.op_, expr.lhs, expr.rhs, s)))

};

}
// namespace simpla

#endif /* MANIFOLD_H_ */
