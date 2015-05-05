/**
 * @file particle_container.h
 *
 *  Created on: 2015年3月26日
 *      Author: salmon
 */

#ifndef CORE_PARTICLE_PARTICLE_CONTAINER_H_
#define CORE_PARTICLE_PARTICLE_CONTAINER_H_

#include <stddef.h>
#include <algorithm>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
#include "../application/sp_object.h"
#include "../dataset/dataset.h"
#include "../utilities/log.h"
#include "../utilities/memory_pool.h"
#include "../gtl/enable_create_from_this.h"
#include "../gtl/primitives.h"
#include "../gtl/properties.h"
#include "../gtl/iterator/sp_iterator.h"
#include "../parallel/mpi_update.h"
#include "../parallel/mpi_aux_functions.h"

/** @ingroup physical_object
 *  @addtogroup particle Particle
 *  @{
 *	  @brief  @ref particle  is an abstraction from  physical particle or "phase-space sample".
 *	  @details
 * ## Summary
 *  - @ref particle is used to  describe trajectory in  @ref phase_space_7d  .
 *  - @ref particle is used to  describe the behavior of  discrete samples of
 *    @ref phase_space_7d function  \f$ f\left(t,x,y,z,v_x,v_y,v_z \right) \f$.
 *  - @ref particle is a @ref container;
 *  - @ref particle is @ref splittable;
 *  - @ref particle is a @ref field
 * ### Data Structure
 *  -  @ref particle is  `unorder_set<Point_s>`
 *
 * ## Requirements
 *- The following table lists the requirements of a Particle type  '''P'''
 *	Pseudo-Signature    | Semantics
 * -------------------- |----------
 * ` struct Point_s `   | data  type of sample point
 * ` P( ) `             | Constructor
 * ` ~P( ) `            | Destructor
 * ` void  next_timestep(dt, args ...) const; `  | push  particles a time interval 'dt'
 * ` void  next_timestep(num_of_steps,t0, dt, args ...) const; `  | push  particles from time 't0' to 't1' with time step 'dt'.
 * ` flush_buffer( ) `  | flush input buffer to internal data container
 *
 *- @ref particle meets the requirement of @ref container,
 * Pseudo-Signature                 | Semantics
 * -------------------------------- |----------
 * ` push_back(args ...) `          | Constructor
 * ` foreach(TFun const & fun)  `   | Destructor
 * ` dataset dump() `               | dump/copy 'data' into a DataSet
 *
 *- @ref particle meets the requirement of @ref physical_object
 *   Pseudo-Signature           | Semantics
 * ---------------------------- |----------
 * ` print(std::ostream & os) ` | print decription of object
 * ` update() `                 | update internal data storage and prepare for execute 'next_timestep'
 * ` sync()  `                  | sync. internal data with other processes and threads
 *
 *
 * ## Description
 * @ref particle   consists of  @ref particle_container and @ref particle_engine .
 *   @ref particle_engine  describes the individual behavior of one sample. @ref particle_container
 *	  is used to manage these samples.
 *
 *
 * ## Example
 *
 *  @}
 */
namespace simpla
{
template<typename ... T> class Particle;

namespace _impl
{

template<typename TContainer>
struct particle_container_traits
{
	template<typename ...Args>
	static std::shared_ptr<TContainer> create(Args && ...args)
	{
		return std::make_shared<TContainer>();
	}
};

}  // namespace _impl

template<typename TDomain, typename Engine, typename TContainer>
class Particle<TDomain, Engine, TContainer> //
:	public SpObject,
	public Engine,
	public TContainer,
	public enable_create_from_this<Particle<TDomain, Engine, TContainer> >
{
public:

	typedef TDomain domain_type;

	typedef typename domain_type::mesh_type mesh_type;

	typedef Engine engine_type;

	typedef TContainer container_type;

	typedef Particle<domain_type, engine_type, container_type> this_type;

	typedef typename container_type::value_type value_type;

	typedef typename mesh_type::index_type index_type;
	typedef typename mesh_type::id_type id_type;
	typedef typename mesh_type::coordinates_type coordinates_type;

	static constexpr size_t iform = domain_type::iform;

//private:

	domain_type m_domain_;
	mesh_type const & m_mesh_;
public:

	Particle(domain_type const & d)
			: m_domain_(d), m_mesh_(d.mesh())
	{
	}

	Particle(this_type const& other)
			: engine_type(other), container_type(other), m_domain_(
					other.m_domain_), m_mesh_(other.m_mesh_)
	{
	}

	template<typename ... Args>
	Particle(this_type & other, Args && ...args)
			: engine_type(other), container_type(other,
					std::forward<Args>(args)...), m_domain_(other.m_domain_), m_mesh_(
					other.m_mesh_)
	{
	}

	~Particle()
	{
	}

	using SpObject::properties;

	this_type & self()
	{
		return *this;
	}
	this_type const& self() const
	{
		return *this;
	}

	static std::string get_type_as_string_static()
	{
		return engine_type::get_type_as_string();
	}
	std::string get_type_as_string() const
	{
		return get_type_as_string_static();
	}
	mesh_type const & mesh() const
	{
		return m_mesh_;
	}

	domain_type const & domain() const
	{
		return m_domain_;
	}

	template<typename TDict, typename ...Others>
	void load(TDict const & dict, Others && ...others)
	{
		engine_type::load(dict, std::forward<Others>(others)...);

		if (dict["DataSrc"])
		{
			UNIMPLEMENTED2("load particle from [DataSrc]");
		}
	}
	template<typename OS>
	OS & print(OS & os) const
	{
		engine_type::print(os);
		os << " num= " << size() << ",";
		return os;
	}
	bool empty() const
	{
		return TContainer::empty();
	}

	bool is_divisible() const
	{
		return TContainer::is_divisible();
	}

	bool is_valid() const
	{
		return engine_type::is_valid();
	}

	void deploy()
	{
		engine_type::deploy();

		SpObject::properties.append(engine_type::properties);
	}

	template<typename TRange>
	size_t size(TRange const & r) const
	{
		return container_type::size_all(r);
	}

	size_t size() const
	{
		return container_type::size_all(m_domain_);
	}
	void sync()
	{

		auto ghost_list = m_domain_.mesh().template ghost_shape<iform>();

		for (auto const & item : ghost_list)
		{
			mpi_send_recv_buffer_s send_recv_s;

			send_recv_s.datatype = MPIDataType::create<value_type>();

			std::tie(send_recv_s.dest, send_recv_s.send_tag,
					send_recv_s.recv_tag) = GLOBAL_COMM.make_send_recv_tag(SpObject::object_id(),
					&item.coord_shift[0]);

			//   collect send data

//			auto send_range = m_domain_.mesh().template range<VERTEX>(
//					item.send_offset, item.send_offset + item.send_count);

			auto send_range = m_domain_.select(item.send_offset,
					item.send_offset + item.send_count);

			CHECK(size(send_range));
			send_recv_s.send_size = container_type::size_all(send_range);

			send_recv_s.send_data = sp_alloc_memory(
					send_recv_s.send_size * send_recv_s.datatype.size());

			value_type *data =
					reinterpret_cast<value_type*>(send_recv_s.send_data.get());

			// FIXME need parallel optimize
			for (auto const & key : send_range)
			{
				for (auto const & p : container_type::operator[](key))
				{
					*data = p;
					++data;
				}
			}

			send_recv_s.recv_size = 0;
			send_recv_s.recv_data = nullptr;
			m_send_recv_buffer_.push_back(std::move(send_recv_s));

			//  clear ghosts cell

			auto recv_range = m_domain_.select(item.recv_offset,
					item.recv_offset + item.recv_count);

			CHECK(size(recv_range));
			container_type::erase(recv_range);

		}

		sync_update_varlength(&m_send_recv_buffer_,
				&(SpObject::m_mpi_requests_));
	}

	void wait()
	{
		SpObject::wait();

		for (auto const & item : m_send_recv_buffer_)
		{

			value_type *data =
					reinterpret_cast<value_type*>(item.recv_data.get());

			container_type::insert(data, data + item.recv_size);
		}
		m_send_recv_buffer_.clear();
	}

	template<typename TD>
	DataSet dataset(TD const & pdomain) const
	{

		DataSet res;

		res.datatype = DataType::create<value_type>();
		res.properties = SpObject::properties;

		index_type count = 0;

		pdomain.for_each([&](id_type const &s)
		{
			auto it = container_type::find(s);
			if (it != container_type::end())
			{
				count += std::distance(it->second.begin(), it->second.end());
			}
		});

		res.data = sp_alloc_memory(count * sizeof(value_type));

		value_type * p = reinterpret_cast<value_type *>(res.data.get());

		//TODO need parallel optimize

		pdomain.for_each([&](id_type const &s)
		{
			auto it = container_type::find(s);

			if (it != container_type::end())
			{
				for (auto const & pit : it->second)
				{
					*p = pit;
					++p;
				}
			}
		});

		ASSERT(std::distance(data.get(), p) == count);

		index_type offset = 0;
		index_type total_count = count;

		std::tie(offset, total_count) = sync_global_location(count);

		DataSpace(1, &total_count).swap(res.dataspace);

		res.dataspace.select_hyperslab(&offset, nullptr, &count, nullptr);

		VERBOSE << "dump particles [" << count << "/" << total_count << "] "
				<< std::endl;

		return std::move(res);
	}

	DataSet dataset() const
	{
		return std::move(dataset(m_domain_));
	}

//! @}

	template<typename TFun>
	void for_each(TFun const& fun)
	{
		wait();

		for (auto const & bucket : *this)
		{
			for (auto &p : bucket.second)
			{
				fun(p);
			}
		}

	}
	template<typename TFun>
	void for_each(TFun const& fun) const
	{
		ASSERT(is_ready());

		for (auto const & bucket : *this)
		{
			for (auto &p : bucket.second)
			{
				fun(p);
			}
		}
	}
	template<typename TFun, typename ...Args>
	void for_each(TFun const& fun, Args && ...args) const
	{
		ASSERT(is_ready());

		m_domain_.for_each([&](id_type s)
		{
			for (auto &p : container_type::operator[](s))
			{
				fun(p);
			}
		});
	}
	/**
	 *
	 * @param args arguments
	 *
	 * - Semantics
	 @code
	 for( Point_s & point: all particle)
	 {
	 engine_type::next_timestep(& point,std::forward<Args>(args)... );
	 }
	 @endcode
	 *
	 */
	template<typename ...Args>
	void next_timestep(Args && ...args)
	{

		wait();

		for (auto & bucket : *this)
		{
			for (auto &p : bucket.second)
			{
				engine_type::next_timestep(&p, std::forward<Args>(args)...);
			}
		}
	}

	/**
	 *
	 * @param num_of_steps number of time steps
	 * @param t0 start time point
	 * @param dt delta time step
	 * @param args other arguments
	 * @return t0+num_of_steps*dt
	 *
	 *-Semantics
	 @code
	 for(s=0;s<num_of_steps;++s)
	 {
	 for( Point_s & point: all particle)
	 {
	 engine_type::next_timestep(& point,t0+s*dt,dt,std::forward<Args>(args)... );
	 }
	 }
	 return t0+num_of_steps*dt;
	 @endcode
	 */
	template<typename ...Args>
	Real next_n_timesteps(size_t num_of_steps, Real t0, Real dt,
			Args && ...args)
	{

		wait();

		for (auto & bucket : *this)
		{
			for (auto &p : bucket.second)
			{
				for (int s = 0; s < num_of_steps; ++s)
				{
					engine_type::next_timestep(&p, t0 + dt * s, dt,
							std::forward<Args>(args)...);
				}
			}
		}
	}

}
;
}  // namespace simpla

#endif /* CORE_PARTICLE_PARTICLE_CONTAINER_H_ */