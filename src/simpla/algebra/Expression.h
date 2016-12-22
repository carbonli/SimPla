/**
 * @file expression_template.h
 *
 *  Created on: 2014-9-26
 *      Author: salmon
 */

#ifndef EXPRESSION_TEMPLATE_H_
#define EXPRESSION_TEMPLATE_H_

#include <cmath>
#include <limits>
#include <type_traits>
#include <complex>
#include <tuple>
#include <cmath>
#include "../mpl/type_traits.h"
#include "../mpl/integer_sequence.h"

namespace simpla { namespace algebra
{
/**
 *  @ingroup calculus
 *  @addtogroup expression_template  Expression Template
 *  @{
 */

template<typename ...> class Expression;

template<typename ...> struct BooleanExpression;

template<typename ...> struct AssignmentExpression;

template<typename TOP, typename ...Args>
struct Expression<TOP, Args...>
{
    typedef Expression<TOP, Args...> this_type;

    typename std::tuple<typename simpla::traits::reference<Args>::type...> m_args_;

    TOP m_op_;

    Expression(this_type const &that) : m_args_(that.m_args_), m_op_(that.m_op_) {}

    Expression(this_type &&that) : m_args_(that.m_args_), m_op_(that.m_op_) {}

    Expression(Args const &... args) : m_args_(args ...), m_op_() {}

    Expression(TOP op, Args const &... args) : m_args_(args ...), m_op_(op) {}

    virtual ~Expression() {}

};

template<typename TOP, typename TL, typename TR>
struct BooleanExpression<TOP, TL, TR> : public Expression<TOP, TL, TR>
{
    typedef Expression<TOP, TL, TR> base_type;

    using Expression<TOP, TL, TR>::Expression;

    operator bool() const { return false; }

    BooleanExpression(TL const &l, TR const &r) : base_type(l, r) {}
};

template<typename TOP, typename TL>
struct BooleanExpression<TOP, TL> : public Expression<TOP, TL>
{
    typedef Expression<TOP, TL> base_type;

    using Expression<TOP, TL>::Expression;

    BooleanExpression(TL const &l) : base_type(l) {}

    operator bool() const { return false; }
};
//
//template<typename TOP, typename TL, typename TR>
//struct AssignmentExpression<TOP, TL, TR>
//{
//    typedef AssignmentExpression<TOP, TL, TR> this_type;
//    TL &lhs;
//    typename traits::reference<TR>::type rhs;
//    TOP op_;
//
//    AssignmentExpression(this_type const &that) : lhs(that.lhs), rhs(that.rhs), op_(that.op_) {}
//
//    AssignmentExpression(this_type &&that) : lhs(that.lhs), rhs(that.rhs), op_(that.op_) {}
//
//    AssignmentExpression(TL &l, TR const &r) : lhs(l), rhs(r), op_() {}
//
//    AssignmentExpression(TOP op, TL &l, TR const &r) : lhs(l), rhs(r), op_(op) {}
//
//    virtual   ~AssignmentExpression() {}
//
//    template<typename IndexType>
//    inline auto operator[](IndexType const &s) const
//    DECL_RET_TYPE (((op_(traits::get_value(lhs, s), traits::get_value(rhs, s)))))
//
//};
}}  // namespace simpla

#endif /* EXPRESSION_TEMPLATE_H_ */
