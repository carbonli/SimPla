/**
 * @file field.h
 *
 * @date  2013-7-19
 * @author  salmon
 */
#ifndef CORE_FIELD_FIELD_H_
#define CORE_FIELD_FIELD_H_

#include <stddef.h>
#include <memory>


#include "field_expression.h"
#include "field_dense.h"
#include "field_function.h"
#include "load_field.h"
namespace simpla
{

/**
 * @ingroup physical_object
 *
 * @addtogroup field Field
 * @brief @ref field is an abstraction from physical field on 4d or 3d @ref configuration_space
 *
 * ## Summary
 *  - @ref field  assigns a scalar/vector/tensor to each point of a
 *     mathematical space (typically a Euclidean space or manifold).
 *
 *  - @ref field  is function, \f$ y=f(x)\f$ , on the
 *     @ref configuration_space, where \f$x\in D\f$ is
 *       coordinates defined in @ref domain \f$D\f$, and  \f$ y \f$
 *       is a scalar/vector/tensor.     f(coordinates_type x) =>  field value(scalar/vector/tensor) at the coordinates x
 *
 *  - @ref field is an associate container, i.e.'unordered_map<id_type,value_type>'
 *     f[id_type i] => value at the discrete point/edge/face/volume i
 *
 *  - @ref field is an @ref expression
 *
 * ## Member types
 *  Member type	 				    | Semantics
 *  --------------------------------|--------------
 *  coordinates_type				| Datatype of coordinates
 *  index_type						| Datatype of of grid points index
 *  value_type 					    | Datatype of value
 *  domain_type					    | Domain
 *  storage_type					| container type
 *
 *
 * ## Member functions
 *
 * ###Constructor
 *
 *  Pseudo-Signature 	 			| Semantics
 *  --------------------------------|--------------
 *  `Field()`						| Default constructor
 *  `~Field() `					    | destructor.
 *  `Field( const Field& ) `	    | copy constructor.
 *  `Field( Field && ) `			| move constructor.
 *
 *
 * ### Domain &  Split
 *
 *  Pseudo-Signature 	 			        | Semantics
 *  ----------------------------------------|--------------
 *  `Field( Domain & D ) `			        | Construct a field on domain \f$D\f$.
 *  `Field( Field &r,split)`			    | Split field into two part,  see @ref concept_domain
 *  `domain_type const &domain() const `	| Get define domain of field
 *  `void domain(domain_type cont&) ` 	    | Reset define domain of field
 *  `Field split(domain_type d)`			| Sub-field on  domain \f$D \cap D_0\f$
 *  `Field boundary()`				        | Sub-field on  boundary \f${\partial D}_0\f$
 *
 * ###   Capacity
 *  Pseudo-Signature 	 			| Semantics
 *  --------------------------------|--------------
 *  `bool empty() `  				| _true_ if memory is not allocated.
 *  `allocate()`					| allocate memory
 *  `data()`						| direct access to the underlying memory
 *  `clear()`						| set value to zero, allocate memory if empty() is _true_
 *
 *
 * ### Element access
 *  Pseudo-Signature 				            | Semantics
 *  --------------------------------------------|--------------
 *  `value_type & at(index_type s)`   			| access element on the grid points _s_ with bounds checking
 *  `value_type & operator[](index_type s) `  	| access element on the grid points _s_as
 *  `field_value_type  operator()(coordiantes_type x) const` | field value on coordinates \f$x\f$, which is interpolated from discrete points
 *
 * ### Assignment
 *   Pseudo-Signature 	 				         | Semantics
 *  ---------------------------------------------|--------------
 *  `Field & operator=(Function const & f)`  	 | assign values as \f$y[s]=f(x)\f$
 *  `Field & operator=(FieldExpression const &)` | Assign operation,
 *  `Field operator=( const Field& )`            | copy-assignment operator.
 *  `Field operator=( Field&& )`		         | move-assignment operator.
 *
 * ## Non-member functions
 *  Pseudo-Signature  				| Semantics
 *  --------------------------------|--------------
 *  `swap(Field &,Field&)`			| swap
 *
 * ## See also
 *  - @ref FETL
 *  - @ref mesh_concept
 *  - @ref diff_geometry
 *
 */
/**
 * @ingroup field
 * @{
 */

/**
 * Field Class
 */
template<typename, size_t> struct Domain;
template<typename ... >struct _Field;

//template<typename TV, typename TD>
//_Field<TD, TV, _impl::is_sequence_container> make_field(TD const & mesh)
//{
//	return std::move(_Field<TD, TV, _impl::is_sequence_container>(mesh));
//}
//template<typename TV, typename TD>
//_Field<TD, TV, _impl::is_sequence_container> make_field(
//		std::shared_ptr<TD> mesh)
//{
//	return std::move(_Field<TD, TV, _impl::is_sequence_container>(*mesh));
//}
//template<typename TD, typename TV>
//using Field=_Field<TD,TV, _impl::is_sequence_container >;

//template<size_t IFORM, typename TV, typename TM>
//_Field<Domain<TM, IFORM>, TV, _impl::is_sequence_container> make_form(
//		TM const & mesh)
//{
//	return std::move(
//			_Field<Domain<TM, IFORM>, TV, _impl::is_sequence_container>(
//					Domain<TM, IFORM>(mesh)));
//}
template<size_t IFORM, typename TV, typename TM>
_Field<Domain<TM, IFORM>, TV, _impl::is_sequence_container> make_form(
		std::shared_ptr<TM> const & pmesh)
{
	return std::move(
			_Field<Domain<TM, IFORM>, TV, _impl::is_sequence_container>(
					Domain<TM, IFORM>(*pmesh)));
}

template<size_t IFORM, typename TV, typename TM>
_Field<Domain<TM, IFORM>, TV, _impl::is_sequence_container> make_form(
		TM const & mesh)
{
	return std::move(
			_Field<Domain<TM, IFORM>, TV, _impl::is_sequence_container>(
					Domain<TM, IFORM>(mesh)));
}
/** @} */
}
// namespace simpla

#endif /* CORE_FIELD_FIELD_H_ */
