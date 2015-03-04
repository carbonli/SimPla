/**
 * @file field_shared_ptr.h
 *
 *  Created on: @date{ 2015-1-30}
 *      @author: salmon
 */

#ifndef CORE_FIELD_FIELD_SHARED_PTR_H_
#define CORE_FIELD_FIELD_SHARED_PTR_H_

#include <stddef.h>
#include <cstdbool>
#include <memory>
#include <string>

#include "../application/sp_object.h"
#include "../dataset/dataset.h"
#include "../gtl/properties.h"
#include "../gtl/type_traits.h"
#include "../utilities/log.h"

namespace simpla
{
template<typename ...> struct _Field;

/**
 * @ingroup field
 * @{
 */

/**
 *  Simple Field
 */
template<typename TM, typename TV, typename ...Others>
struct _Field<TM, std::shared_ptr<TV>, Others...> : public SpObject
{

	typedef TM mesh_type;

	typedef typename mesh_type::id_type id_type;
	typedef typename mesh_type::coordinates_type coordinates_type;

	typedef std::shared_ptr<TV> container_type;
	typedef TV value_type;

	typedef _Field<mesh_type, container_type, Others...> this_type;

private:

	mesh_type m_mesh_;

	std::shared_ptr<TV> m_data_;

public:

	_Field(mesh_type const & d)
			: m_mesh_(d), m_data_(nullptr)
	{

	}
	_Field(this_type const & that)
			: m_mesh_(that.m_mesh_), m_data_(that.m_data_)
	{
	}
	~_Field()
	{
	}

	std::string get_type_as_string() const
	{
		return "Field<" + m_mesh_.get_type_as_string() + ">";
	}
	mesh_type const & mesh() const
	{
		return m_mesh_;
	}

	template<typename TU> using clone_field_type=
	_Field<TM,std::shared_ptr<TU>,Others... >;

	template<typename TU>
	clone_field_type<TU> clone() const
	{
		return clone_field_type<TU>(m_mesh_);
	}

	void clear()
	{
		deploy();
		*this = 0;
	}
	template<typename T>
	void fill(T const &v)
	{
		deploy();
		CHECK(m_mesh_.max_hash());
		CHECK(v);
		std::fill(m_data_.get(), m_data_.get() + m_mesh_.max_hash(), v);
	}

	/** @name range concept
	 * @{
	 */

	template<typename ...Args>
	_Field(this_type & that, Args && ...args)
			: m_mesh_(that.m_mesh_, std::forward<Args>(args)...), m_data_(
					that.m_data_)
	{
	}
	bool empty() const
	{
		return m_data_ == nullptr;
	}
	bool is_divisible() const
	{
		return m_mesh_.is_divisible();
	}

	/**@}*/

	/**
	 * @name assignment
	 * @{
	 */

//	inline _Field<AssignmentExpression<_impl::_assign, this_type, this_type>> operator =(
//			this_type const &that)
//	{
//		deploy();
//
//		return std::move(
//				_Field<
//						AssignmentExpression<_impl::_assign, this_type,
//								this_type>>(*this, that));
//	}
	inline this_type & operator =(this_type const &that)
	{
		deploy();

		for (auto s : m_mesh_.range())
		{
			this->operator[](s) = m_mesh_.calculate(that, s);
		}

		return *this;
	}

	template<typename TR>
	inline this_type & operator =(TR const &that)
	{
//		deploy();
//		return std::move(
//				_Field<AssignmentExpression<_impl::_assign, this_type, TR>>(
//						*this, that));
		deploy();

//		parallel_for(mesh_.range(), [&](typename mesh_type::range_type s_range)
//		{
		auto s_range = m_mesh_.range();

		for (auto s : s_range)
		{
			CHECK(m_mesh_.hash(s));
			this->operator[](s) = m_mesh_.calculate(that, s);
		}
//		});

		return *this;
	}

	template<typename TFun> void pull_back(TFun const &fun)
	{
		deploy();
		m_mesh_.pull_back(*this, fun);
	}

	/** @} */

	/** @name access
	 *  @{*/

	typedef typename mesh_type::template field_value_type<value_type> field_value_type;

	field_value_type gather(coordinates_type const& x) const
	{
		return std::move(m_mesh_.gather(*this, x));
	}

	template<typename ...Args>
	void scatter(Args && ... args)
	{
		m_mesh_.scatter(*this, std::forward<Args>(args)...);
	}

	/**@}*/

//	DataSet dump_data() const
//	{
//		return DataSet();
//	}
	void deploy()
	{

		if (m_data_ == nullptr)
		{
			m_data_ = sp_make_shared_array<value_type>(m_mesh_.max_hash());
		}
	}

public:
	template<typename IndexType>
	value_type & operator[](IndexType const & s)
	{
		return m_data_.get()[m_mesh_.hash(s)];
	}
	template<typename IndexType>
	value_type const & operator[](IndexType const & s) const
	{
		return m_data_.get()[m_mesh_.hash(s)];
	}

	template<typename ...Args>
	auto id(Args && ... args)
	DECL_RET_TYPE((m_data_.get()[m_mesh_.hash(std::forward<Args>(args)...)]))

	template<typename ...Args>
	auto id(Args && ... args) const
	DECL_RET_TYPE((m_data_.get()[m_mesh_.hash(std::forward<Args>(args)...)]))

	template<typename ...Args>
	auto operator()(Args && ... args) const
	DECL_RET_TYPE((m_mesh_.gather(*this,std::forward<Args>(args)...)))

	DataSet dataset()
	{
		return std::move(DataSet { m_data_, DataType::create<value_type>(),
				m_mesh_.dataspace(), properties });
	}

	DataSet dataset() const
	{
		size_t num = m_mesh_.max_hash();
		auto data = sp_make_shared_array<value_type>(num);
		std::copy(m_data_.get(), m_data_.get() + num, data.get());
		return std::move(
				DataSet { data, DataType::create<value_type>(),
						m_mesh_.dataspace(), properties });
	}
}
;
namespace _impl
{
class is_sequence_container;
template<typename TContainer> struct field_selector;
template<typename TV>
struct field_selector<std::shared_ptr<TV>>
{
	typedef is_sequence_container type;
};

}  // namespace _impl
/**@} */

template<typename TM, typename TV>
auto make_field(TM const & mesh)
DECL_RET_TYPE(( _Field<TM,std::shared_ptr<TV>>(mesh)))
}
// namespace simpla

#endif /* CORE_FIELD_FIELD_SHARED_PTR_H_ */
