/*
 * @file field.h
 *
 * @date  2013-7-19
 * @author  salmon
 */
#ifndef CORE_FIELD_FIELD_H_
#define CORE_FIELD_FIELD_H_

#include <cstdbool>
#include <cstddef>
#include <memory>
#include <type_traits>

#include "../data_structure/container_traits.h"
#include "../data_structure/data_set.h"

#include "../physics/physical_object.h"
#include "../design_pattern/expression_template.h"
#include "../diff_geometry/domain.h"
#include "../utilities/utilities.h"
#include "../parallel/parallel.h"

namespace simpla
{

/**
 *  \brief Field concept
 */
template<typename ... >struct _Field;

/**
 *
 *  \brief skeleton of Field data holder
 *   Field is a Associate container
 *     f[index_type i] => value at the discrete point/edge/face i
 *   Field is a Function
 *     f(coordinates_type x) =>  field value(scalar/vector/tensor) at the coordinates x
 *   Field is a Expression
 */
template<typename TDomain, typename Container>
struct _Field<TDomain, Container> : public PhysicalObject
{

	typedef TDomain domain_type;
	typedef typename domain_type::index_type index_type;

	typedef Container container_type;
	typedef _Field<domain_type, container_type> this_type;
	typedef typename container_traits<container_type>::value_type value_type;

private:

	domain_type domain_;

	container_type data_;

public:

	template<typename ...Args>
	_Field(domain_type const & domain, Args &&...args) :
			domain_(domain), data_(std::forward<Args>(args)...)
	{
	}

	template<typename TM>
	_Field(std::shared_ptr<TM> const & m) :
			domain_(m)
	{
	}
	_Field(this_type const & that) :
			domain_(that.domain_), data_(that.data_)
	{
	}

	~_Field()
	{
	}

	std::string get_type_as_string() const
	{
//		" + domain_type::get_type_as_string() + "
		return "Field<>";
	}

	void swap(this_type &r)
	{
		sp_swap(r.data_, data_);
		sp_swap(r.domain_, domain_);
	}

	domain_type const & domain() const
	{
		return domain_;
	}

	void domain(domain_type d)
	{
		sp_swap(d, domain_);
	}
	size_t size() const
	{
		return domain_.size();
	}
	container_type & data()
	{
		return data_;
	}

	container_type const & data() const
	{
		return data_;
	}

	void data(container_type d)
	{
		sp_swap(d, data_);
	}

	bool is_same(this_type const & r) const
	{
		return data_ == r.data_;
	}
	template<typename TR>
	bool is_same(TR const & r) const
	{
		return data_ == r.data_;
	}

	bool empty() const
	{
		return container_traits<container_type>::is_empty(data_);
	}
	bool is_valid() const
	{
		return !empty();
	}
	void allocate()
	{
		if (!is_valid())
		{
			container_traits<container_type>::allocate(size()).swap(data_);
			PhysicalObject::update();
		}
	}

	void clear()
	{
		allocate();
		container_traits<container_type>::clear(data_, size());
	}

	DataSet dataset() const
	{
		return DataSet( { data_, properties(), make_datatype<value_type>(),
				domain_.dataspace() });
	}

	/***
	 * @name Access operation
	 * @{
	 */
	value_type & get(index_type const &s)
	{
		return (container_traits<container_type>::get_value(data_,
				domain_.hash(s)));
	}

	value_type const& get(index_type const &s) const
	{
		return (container_traits<container_type>::get_value(data_,
				domain_.hash(s)));
	}

	value_type & operator[](index_type const & s)
	{
		return (container_traits<container_type>::get_value(data_,
				domain_.hash(s)));
	}

	value_type const & operator[](index_type const & s) const
	{
		return (container_traits<container_type>::get_value(data_,
				domain_.hash(s)));
	}

	/** @} */

	/**
	 * @name Assignment
	 * @{
	 */
	inline this_type &
	operator =(this_type const &that)
	{
		allocate();

		parallel_foreach(domain_, [&](index_type const & s)
		{
			(*this)[s]=that[s];
		});

		return (*this);
	}

	template<typename TR> inline this_type &
	operator =(TR const&that)
	{
		allocate();

		parallel_foreach(domain_, [&](index_type const & s)
		{
			(*this)[s]= domain_.calculate( (that), s);
		});

		return (*this);
	}

	template<typename TR>
	inline this_type & operator +=(TR const &that)
	{
		allocate();

		parallel_foreach(domain_, [&](index_type const & s)
		{
			(*this)[s]+= domain_.calculate( (that), s);
		});
		return (*this);

	}

	template<typename TR>
	inline this_type & operator -=(TR const &that)
	{
		allocate();

		parallel_foreach(domain_, [&](index_type const & s)
		{
			(*this)[s] -= domain_.calculate(that, s);
		});

		return (*this);
	}

	template<typename TR>
	inline this_type & operator *=(TR const &that)
	{
		allocate();

		parallel_foreach(domain_, [&](index_type const & s)
		{
			(*this)[s] *= domain_.calculate(that, s);
		});

		return (*this);
	}

	template<typename TR>
	inline this_type & operator /=(TR const &that)
	{
		allocate();

		parallel_foreach(domain_, [&](index_type const & s)
		{
			(*this)[s] /= domain_.calculate(that, s);
		});

		return (*this);
	}
	/** @} */

	template<typename TFun> void pull_back(TFun const &fun)
	{
		pull_back(domain_, fun);
	}

	template<typename TD, typename TFun> void pull_back(TD const & domain,
			TFun const &fun)
	{
		allocate();

		parallel_foreach(domain,

		[&](index_type const & s)
		{

			//FIXME geometry coordinates convert

				(*this)[s] = domain_.sample( s,fun(
								//domain.MapTo(domain_.InvMapTo(
								domain_.coordinates(s)
								//))
						)
				);
			});

	}

	typedef typename std::conditional<
			domain_type::iform == VERTEX || domain_type::iform == VOLUME,
			value_type, nTuple<value_type, 3>>::type field_value_type;

	field_value_type operator()(
			typename domain_type::coordinates_type const& x) const
	{
		return std::move(domain_.gather(*this, x));
	}

	field_value_type gather(typename domain_type::coordinates_type const& x)
	{
		return std::move(domain_.gather(*this, x));

	}

	template<typename ...Args>
	void scatter(Args && ... args)
	{
		domain_.scatter(const_cast<this_type&>(*this),
				std::forward<Args>(args)...);
	}

}
;

template<typename TD, typename TC>
struct reference_traits<_Field<TD, TC> >
{
	typedef _Field<TD, TC> const & type;
};

template<typename TDomain, typename TV> using Field= _Field< TDomain,std::shared_ptr<TV> >;

template<typename > struct is_field
{
	static constexpr bool value = false;
};

template<typename ...T> struct is_field<_Field<T...>>
{
	static constexpr bool value = true;
};

template<typename ...> struct field_traits;

template<typename T> struct field_traits<T>
{

	static constexpr size_t ndims = 0;

	static constexpr size_t iform = VERTEX;

	typedef T value_type;

	typedef T field_value_type;

	static constexpr bool is_field = false;

};

template<typename TD, typename TC>
struct field_traits<_Field<TD, TC>>
{
	static constexpr bool is_field = true;

	static constexpr size_t ndims = TD::ndims;

	static constexpr size_t iform = TD::iform;

	typedef typename _Field<TD, TC>::value_type value_type;

	typedef typename std::conditional<iform == EDGE || iform == FACE,
			nTuple<value_type, 3>, value_type>::type field_value_type;

};

/// @name  Field Expression
/// @{

template<typename TOP, typename TL, typename TR> struct Expression;

template<typename TOP, typename TL>
struct field_traits<_Field<Expression<TOP, TL, std::nullptr_t> >>
{
private:
	typedef typename field_traits<TL>::value_type l_type;
public:
	static constexpr bool is_field = true;

	static constexpr size_t ndims = field_traits<TL>::ndims;

	static constexpr size_t iform = field_traits<TL>::iform;

	typedef typename sp_result_of<TOP(l_type)>::type value_type;

	typedef typename std::conditional<iform == EDGE || iform == FACE,
			nTuple<value_type, 3>, value_type>::type field_value_type;

};

template<typename TOP, typename TL, typename TR>
struct field_traits<_Field<Expression<TOP, TL, TR> > >
{
private:
	static constexpr size_t NDIMS = sp_max<size_t, field_traits<TL>::ndims,
			field_traits<TL>::ndims>::value;

	static constexpr size_t IL = field_traits<TL>::iform;
	static constexpr size_t IR = field_traits<TR>::iform;

	typedef typename field_traits<TL>::value_type l_type;
	typedef typename field_traits<TR>::value_type r_type;

public:
	static const size_t ndims = NDIMS;
	static const size_t iform = IL;
	static constexpr bool is_field = true;

	typedef typename sp_result_of<TOP(l_type, r_type)>::type value_type;

	typedef typename std::conditional<iform == EDGE || iform == FACE,
			nTuple<value_type, 3>, value_type>::type field_value_type;

};

template<typename TOP, typename TL, typename TR>
struct _Field<Expression<TOP, TL, TR>> : public Expression<TOP, TL, TR>
{
	typedef _Field<Expression<TOP, TL, TR>> this_type;

	using Expression<TOP, TL, TR>::Expression;
};

template<typename TOP, typename TL, typename TR>
struct _Field<BooleanExpression<TOP, TL, TR>> : public Expression<TOP, TL, TR>
{
	typedef _Field<BooleanExpression<TOP, TL, TR>> this_type;

	using Expression<TOP, TL, TR>::Expression;

	operator bool() const
	{
		UNIMPLEMENTED;
		return false;
	}
};

DEFINE_EXPRESSOPM_TEMPLATE_BASIC_ALGEBRA(_Field)

#define SP_DEF_BINOP_FIELD_NTUPLE(_OP_,_NAME_)                                                 \
template<typename ...T1, typename T2, size_t ... N>                                            \
_Field<Expression<_impl::plus, _Field<T1...>, nTuple<T2, N...> > > operator _OP_(              \
		_Field<T1...> const & l, nTuple<T2, N...> const &r)                                    \
{return (_Field<Expression<_impl::_NAME_, _Field<T1...>, nTuple<T2, N...> > >(l, r));}         \
template<typename T1, size_t ... N, typename ...T2>                                            \
_Field<Expression<_impl::plus, nTuple<T1, N...>, _Field<T2...> > > operator _OP_(              \
		nTuple<T1, N...> const & l, _Field< T2...>const &r)                                    \
{	return (_Field<Expression< _impl::_NAME_,T1,_Field< T2...>>>(l,r));}                       \


SP_DEF_BINOP_FIELD_NTUPLE(+, plus)
SP_DEF_BINOP_FIELD_NTUPLE(-, minus)
SP_DEF_BINOP_FIELD_NTUPLE(*, multiplies)
SP_DEF_BINOP_FIELD_NTUPLE(/, divides)
SP_DEF_BINOP_FIELD_NTUPLE(%, modulus)
SP_DEF_BINOP_FIELD_NTUPLE(^, bitwise_xor)
SP_DEF_BINOP_FIELD_NTUPLE(&, bitwise_and)
SP_DEF_BINOP_FIELD_NTUPLE(|, bitwise_or)
#undef SP_DEF_BINOP_FIELD_NTUPLE

template<typename TV, typename TD>
auto make_field(TD const& d)
DECL_RET_TYPE((_Field<TD, std::shared_ptr<TV>>( (d) )))

template<typename, size_t> class Domain;

template<size_t IFORM, typename TV, typename TM>
_Field<Domain<TM, IFORM>, std::shared_ptr<TV>> make_form(
		std::shared_ptr<TM> manifold)
{
	return std::move(
			_Field<Domain<TM, IFORM>, std::shared_ptr<TV>>(
					Domain<TM, IFORM>(manifold->shared_from_this())));
}

}
// namespace simpla

#endif /* CORE_FIELD_FIELD_H_ */
