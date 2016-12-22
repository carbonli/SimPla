//
// Created by salmon on 16-12-22.
//

#ifndef SIMPLA_ARITHMETIC_H
#define SIMPLA_ARITHMETIC_H

#include <simpla/SIMPLA_config.h>
#include <cmath>
#include <type_traits>

namespace simpla { namespace algebra
{
template<typename ...> class Expression;

template<typename ...> struct BooleanExpression;

template<typename ...> struct AssignmentExpression;

#define DEF_BOP(_NAME_, _OP_)                                                                \
namespace tags{struct _NAME_{ template<typename TL,typename TR> auto operator()( TL const & l,TR const & r   )const ->decltype(l _OP_ r) {return l _OP_ r;};};}

#define DEF_UOP(_NAME_, _OP_)                                                                \
namespace tags{struct _NAME_{ template<typename TL> auto operator()(TL const & l )const ->decltype(  _OP_ l){return  _OP_ l;};};}   \


DEF_BOP(plus, +)
DEF_BOP(minus, -)
DEF_BOP(multiplies, *)
DEF_BOP(divides, /)
DEF_BOP(modulus, %)
DEF_UOP(negate, -)
DEF_UOP(unary_plus, +)
DEF_BOP(bitwise_and, &)
DEF_BOP(bitwise_or, |)
DEF_BOP(bitwise_xor, ^)
DEF_UOP(bitwise_not, ~)
DEF_BOP(shift_left, <<)
DEF_BOP(shift_right, >>)
DEF_UOP(logical_not, !)
DEF_BOP(logical_and, &&)
DEF_BOP(logical_or, ||)
DEF_BOP(equal_to, ==)
DEF_BOP(not_equal_to, !=)
DEF_BOP(greater, >)
DEF_BOP(less, <)
DEF_BOP(greater_equal, >=)
DEF_BOP(less_equal, <=)

#undef DEF_UOP
#undef DEF_BOP

using namespace std;

#define DEF_BI_FUN(_NAME_)                                                                \
namespace tags{struct _NAME_{ template<typename TL,typename TR> auto operator()( TL const & l,TR const & r )const ->decltype(_NAME_(l , r)){return _NAME_(l , r);};};}

#define DEF_UN_FUN(_NAME_)                                                                \
namespace tags{struct _NAME_{template<typename TL> auto operator()(TL const & l )const ->decltype( _NAME_ (l)){return  _NAME_(l) ;};};}                                                             \

DEF_UN_FUN(cos)
DEF_UN_FUN(acos)
DEF_UN_FUN(cosh)
DEF_UN_FUN(sin)
DEF_UN_FUN(asin)
DEF_UN_FUN(sinh)
DEF_UN_FUN(tan)
DEF_UN_FUN(tanh)
DEF_UN_FUN(atan)
DEF_UN_FUN(exp)
DEF_UN_FUN(log)
DEF_UN_FUN(log10)
DEF_UN_FUN(sqrt)
DEF_UN_FUN(real)
DEF_UN_FUN(imag)

DEF_BI_FUN(atan2)
DEF_BI_FUN(pow)
#undef DEF_UN_FUN
#undef DEF_BI_FUN

/** @name Constant Expressions
 * @{*/


template<typename value_type> struct Constant { value_type value; };
struct Zero {};
struct One {};
struct Infinity {};
struct Undefine {};
struct Identity {};

template<typename TE> inline TE const &operator+(TE const &e, Zero const &) { return (e); }

template<typename TE> inline TE const &operator+(Zero const &, TE const &e) { return (e); }

template<typename TE> inline TE const &operator-(TE const &e, Zero const &) { return (e); }

//template<typename TE> inline auto operator -(Zero const &, TE const &e)
//DECL_RET_TYPE (((-e)))

inline Zero operator+(Zero const &, Zero const &e) { return (Zero()); }

template<typename TE> inline TE const &operator*(TE const &e, One const &) { return (e); }

template<typename TE> inline TE const &operator*(One const &, TE const &e) { return (e); }

template<typename TE> inline Zero operator*(TE const &, Zero const &) { return (Zero()); }

template<typename TE> inline Zero operator*(Zero const &, TE const &) { return (Zero()); }

template<typename TE> inline Infinity operator/(TE const &e, Zero const &) { return (Infinity()); }

template<typename TE> inline Zero operator/(Zero const &, TE const &e) { return (Zero()); }

template<typename TE> inline Zero operator/(TE const &, Infinity const &) { return (Zero()); }

template<typename TE> inline Infinity operator/(Infinity const &, TE const &e) { return (Infinity()); }

//template<typename TL> inline auto operator==(TL const &lhs, Zero)DECL_RET_TYPE ((lhs))
//
//template<typename TR> inline auto operator==(Zero, TR const &rhs)DECL_RET_TYPE ((rhs))

constexpr Identity operator&(Identity, Identity) { return Identity(); }

template<typename TL> TL const &operator&(TL const &l, Identity) { return l; }

template<typename TR> TR const &operator&(Identity, TR const &r) { return r; }

template<typename TL> constexpr Zero operator&(TL const &l, Zero) { return std::move(Zero()); }

template<typename TR> constexpr Zero operator&(Zero, TR const &l) { return std::move(Zero()); }

template<typename TR> constexpr Zero operator&(Zero, Zero) { return std::move(Zero()); }

/** @} */

/**
 * ### Assignment Operator
 *
 *   Pseudo-Signature 	 				         | Semantics
 *  ---------------------------------------------|--------------
 *  `operator+=(GeoObject &,Expression const &)`     | Assign operation +
 *  `operator-=(GeoObject & ,Expression const &)`     | Assign operation -
 *  `operator/=(GeoObject & ,Expression const &)`     | Assign operation /
 *  `operator*=(GeoObject & ,Expression const &)`     | Assign operation *
 */

template<typename TL, typename TR>
void assign(std::nullptr_t, TL &l, TR const &r, typename std::enable_if<
        std::is_arithmetic<TL>::value &&
        std::is_arithmetic<TR>::value, void>::type *_p = nullptr) { l = static_cast<TL>(r); }


//#define DEF_ASSIGN_OP(_NAME_, _OP_)                                                                \
//template<typename TL,typename TR> void assign(tags::_NAME_ const *, TL   & l,TR const & r                                \
//,ENABLE_IF(std::is_arithmetic<TL>::value && std::is_arithmetic<TR>::value> ){  l _OP_##= r;};
//
//DEF_ASSIGN_OP(plus, +)
//DEF_ASSIGN_OP(minus, -)
//DEF_ASSIGN_OP(multiplies, *)
//DEF_ASSIGN_OP(divides, /)
//DEF_ASSIGN_OP(modulus, %)
//
//#undef DEF_ASSIGN_OP
#define  DEFINE_EXPRESSION_TEMPLATE_BASIC_ALGEBRA2(_CONCEPT_)                                              \
_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_OPERATOR(+, plus)                                      \
_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_OPERATOR(-, minus)                                     \
_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_OPERATOR(*, multiplies)                                \
_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_OPERATOR(/, divides)                                   \
_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_OPERATOR(%, modulus)                                   \
_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_OPERATOR(^, bitwise_xor)                               \
_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_OPERATOR(&, bitwise_and)                               \
_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_OPERATOR(|, bitwise_or)                                \
_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_OPERATOR(~, bitwise_not)                                \
_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_OPERATOR(+, unary_plus)                                 \
_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_OPERATOR(-, negate)                                     \
_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_FUNCTION(cos)                                           \
_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_FUNCTION(acos)                                          \
_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_FUNCTION(cosh)                                          \
_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_FUNCTION(sin)                                           \
_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_FUNCTION(asin)                                          \
_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_FUNCTION(sinh)                                          \
_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_FUNCTION(tan)                                           \
_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_FUNCTION(tanh)                                          \
_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_FUNCTION(atan)                                          \
_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_FUNCTION(exp)                                           \
_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_FUNCTION(log)                                           \
_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_FUNCTION(log10)                                         \
_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_FUNCTION(sqrt)                                          \
_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_FUNCTION(real)                                          \
_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_FUNCTION(imag)                                          \
_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_FUNCTION(atan2)                                      \
_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_FUNCTION(pow)                                         \
_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_BOOLEAN_OPERATOR(!,  logical_not)                       \
_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_BOOLEAN_OPERATOR(&&, logical_and)                      \
_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_BOOLEAN_OPERATOR(||, logical_or)                       \
_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_BOOLEAN_OPERATOR(!=, not_equal_to)                     \
_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_BOOLEAN_OPERATOR(==, equal_to)                         \
_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_BOOLEAN_OPERATOR(<, less)                              \
_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_BOOLEAN_OPERATOR(>, greater)                           \
_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_BOOLEAN_OPERATOR(<=, less_equal)                       \
_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_BOOLEAN_OPERATOR(>=, greater_equal)                    \


//_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_RIGHT_OPERATOR(<<, shift_left)                        \
//_SP_DEFINE_##_CONCEPT_##_EXPR_BINARY_RIGHT_OPERATOR(>>  , shift_right)                     \

#define _SP_DEFINE_Expression_EXPR_BINARY_OPERATOR(_OP_, _NAME_)                           \
    template< typename  T1,typename T2>   Expression< tags::_NAME_,T1,T2 >                    \
    operator _OP_(T1 const & l,T2 const &r)   {return (Expression< tags::_NAME_,T1,T2 > (l,r));}  \

#define _SP_DEFINE_Expression_EXPR_BINARY_FUNCTION(_NAME_)                                       \
    template< typename   T1,typename  T2> constexpr    Expression< tags::_NAME_,T1 , T2>       \
    _NAME_(T1 const & l,T2  const &r) {return ( Expression< tags::_NAME_,T1,T2> (l,r));} \

#define _SP_DEFINE_Expression_EXPR_BINARY_RIGHT_OPERATOR(_OP_, _NAME_)                      \
    template<typename T1,typename  T2> constexpr  Expression<tags::_NAME_,T1,T2>                             \
    operator _OP_(T1 const & l,T2 const &r){return ( Expression<tags::_NAME_,T1,T2> (l,r)) ;}                    \

#define _SP_DEFINE_Expression_EXPR_UNARY_OPERATOR(_OP_, _NAME_)                           \
        template<typename T>  Expression<tags::_NAME_,T >                                      \
        operator _OP_(T const &l){return (Expression<tags::_NAME_,T >(l));}                       \

#define _SP_DEFINE_Expression_EXPR_UNARY_FUNCTION(_NAME_)                                  \
        template<typename T> constexpr   Expression<tags::_NAME_,T>                        \
        _NAME_(T const &r)  {return ( Expression<tags::_NAME_,T > (r));}                   \

#define _SP_DEFINE_Expression_EXPR_BINARY_BOOLEAN_OPERATOR(_OP_, _NAME_)                            \
        template< typename T1,typename T2>  constexpr BooleanExpression< tags::_NAME_,T1,T2>                     \
        operator _OP_(T1 const & l,T2 const &r) {return ( BooleanExpression< tags::_NAME_,T1,T2> (l,r));}    \


#define _SP_DEFINE_Expression_EXPR_UNARY_BOOLEAN_OPERATOR(_OP_, _NAME_)                           \
        template<typename T>    constexpr  BooleanExpression<tags::_NAME_,T >                             \
        operator _OP_(T const &l)     {return ( BooleanExpression<tags::_NAME_,T > (l)) ;}                      \


DEFINE_EXPRESSION_TEMPLATE_BASIC_ALGEBRA2(Expression)

#undef _SP_DEFINE_Expression_EXPR_BINARY_OPERATOR
#undef _SP_DEFINE_Expression_EXPR_BINARY_FUNCTION
#undef _SP_DEFINE_Expression_EXPR_BINARY_RIGHT_OPERATOR
#undef _SP_DEFINE_Expression_EXPR_UNARY_OPERATOR
#undef _SP_DEFINE_Expression_EXPR_UNARY_FUNCTION
#undef _SP_DEFINE_Expression_EXPR_BINARY_BOOLEAN_OPERATOR
#undef _SP_DEFINE_Expression_EXPR_UNARY_BOOLEAN_OPERATOR
}//namespace algebra
}//namespace simpla{namespace algebra
#endif //SIMPLA_ARITHMETIC_H