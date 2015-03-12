/*
 * field_function.h
 *
 *  Created on: 2015年3月10日
 *      Author: salmon
 */

#ifndef CORE_FIELD_FIELD_FUNCTION_H_
#define CORE_FIELD_FIELD_FUNCTION_H_

namespace simpla
{
template<typename ...>class _Field;
namespace _impl
{

class this_is_function;

}  // namespace _impl

template<typename TM, typename TFun>
class _Field<TM, TFun, _impl::this_is_function>
{
	typedef _Field<TM, TFun, _impl::this_is_function> this_type;

	typedef TM mesh_type;
	typedef typename mesh_type::coordiantes_type coordiantes_type;
	typedef typename mesh_type::id_type id_type;
	typedef typename std::result_of<TFun(Real, coordinates_type)>::type value_type;

	typedef TFun function_type;
private:
	mesh_type m_mesh_;
	function_type m_fun_;
public:

	_Field(mesh_type const& m, function_type const & f)
			: m_mesh_(m), m_fun_(f)
	{

	}
	_Field(this_type const & other)
			: m_mesh_(other.m_mesh_), m_fun_(other.m_fun_)
	{
	}
	~_Field()
	{
	}

	void swap(this_type & other)
	{
		std::swap(m_mesh_, other.m_mesh_);
		std::swap(m_fun_, other.m_fun_);
	}

	this_type &operator=(this_type const & other)
	{
		this_type(other).swap(*this);
		return *this;
	}

	value_type operator[](id_type const &s) const
	{
		return m_mesh_.sample(m_fun_(m_mesh_.id_to_coordiantes(s)), s);
	}

	value_type operator()(coordinates_type const & x) const
	{
		return m_fun_(m_mesh_.time(), x);
	}

};

template<typename TM, typename TFun>
_Field<TM, TFun, _impl::this_is_function> make_field_function(TM const & m,
		TFun const & fun)
{
	return std::move(_Field<TM, TFun, _impl::this_is_function>(m, fun));
}

}
// namespace simpla

#endif /* CORE_FIELD_FIELD_FUNCTION_H_ */