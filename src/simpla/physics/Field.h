/**
 * @file field.h
 * @author salmon
 * @date 2015-10-16.
 */

#ifndef SIMPLA_FIELD_H
#define SIMPLA_FIELD_H

#include "simpla/SIMPLA_config.h"

#include "simpla/data/Data.h"
#include "simpla/engine/Attribute.h"
#include "simpla/utilities/type_traits.h"

#include "simpla/algebra/ExpressionTemplate.h"

namespace simpla {

namespace traits {

template <typename T>
T& GetValue(T& expr, int tag) {
    return expr;
}

template <typename T, int... N>
decltype(auto) GetValue(nTuple<T, N...> const& expr, int tag) {
    return GetValue(expr[tag & 0b111], tag >> 3);
}
template <typename T, typename... O, typename... Args>
decltype(auto) GetValue(Array<T, O...> const& expr, Args&&... args) {
    return;
}
}

template <typename TM, typename TV, int...>
class Field;
template <typename TM, typename TV, int IFORM, int... DOF>
class Field<TM, TV, IFORM, DOF...> : public engine::AttributeT<TV, IFORM, DOF...> {
   private:
    typedef Field<TM, TV, IFORM, DOF...> this_type;
    typedef engine::AttributeT<TV, IFORM, DOF...> base_type;
    static std::string FancyTypeName();
    virtual std::string TypeName() const override { return FancyTypeName(); }

   public:
    typedef TV value_type;
    typedef TM mesh_type;

    mesh_type* m_mesh_;

   public:
    template <typename... Args>
    explicit Field(mesh_type* host, Args&&... args) : base_type(host, std::forward<Args>(args)...), m_mesh_(host) {}
    ~Field() override = default;

    Field(Field const& other) = delete;
    Field(Field&& other) = delete;

    template <typename... Args>
    static std::shared_ptr<this_type> New(Args&&... args) {
        return std::shared_ptr<this_type>(new this_type(std::forward<Args>(args)...));
    }

    void DoSetUp() override { m_mesh_->InitializeAttribute(this); }
    void DoUpdate() override {}
    void DoTearDown() override {}
    template <typename Other>
    void Set(Other&& v) {
        base_type::Update();
        m_mesh_->Fill(*this, std::forward<Other>(v));
    }

    this_type& operator=(this_type const& other) {
        Set(other);
        return *this;
    }
    template <typename TR>
    this_type& operator=(TR&& rhs) {
        Set(std::forward<TR>(rhs));
        return *this;
    };

    auto& operator[](EntityId s) {
        return traits::recursive_index(this->operator[](EntityIdCoder::m_id_to_sub_index_[s.w & 0b111]), s.w >> 3)(
            s.x, s.y, s.z);
    }
    auto const& operator[](EntityId s) const {
        return traits::recursive_index(this->operator[](EntityIdCoder::m_id_to_sub_index_[s.w & 0b111]), s.w >> 3)(
            s.x, s.y, s.z);
    }

    //*****************************************************************************************************************

    template <typename... Args>
    decltype(auto) gather(Args&&... args) const {
        return m_mesh_->gather(*this, std::forward<Args>(args)...);
    }

    template <typename... Args>
    decltype(auto) scatter(Args&&... args) {
        return m_mesh_->scatter(*this, std::forward<Args>(args)...);
    }

};  // class Field

template <typename TM, typename TV, int IFORM, int... DOF>
std::string Field<TM, TV, IFORM, DOF...>::FancyTypeName() {
    return "Field<" + simpla::traits::type_name<TM>::value() + "," + simpla::traits::type_name<TV>::value() + "," +
           EntityIFORMName[IFORM] + ((sizeof...(DOF) == 0) ? "" : ("," + simpla::traits::to_string(DOF...))) + ">";
}

template <typename TM, typename TL, int... NL>
auto operator<<(Field<TM, TL, NL...> const& lhs, int n) {
    return Expression<tags::bitwise_left_shift, Field<TM, TL, NL...>, int>(lhs, n);
};

template <typename TM, typename TL, int... NL>
auto operator>>(Field<TM, TL, NL...> const& lhs, int n) {
    return Expression<tags::bitwise_right_shifit, Field<TM, TL, NL...>, int>(lhs, n);
};

#define _SP_DEFINE_FIELD_BINARY_FUNCTION(_TAG_, _FUN_)                                        \
    template <typename TM, typename TL, int... NL, typename TR>                               \
    auto _FUN_(Field<TM, TL, NL...> const& lhs, TR const& rhs) {                              \
        return Expression<tags::_TAG_, Field<TM, TL, NL...>, TR>(lhs, rhs);                   \
    };                                                                                        \
    template <typename TL, typename TM, typename TR, int... NR>                               \
    auto _FUN_(TL const& lhs, Field<TM, TR, NR...> const& rhs) {                              \
        return Expression<tags::_TAG_, TL, Field<TM, TR, NR...>>(lhs, rhs);                   \
    };                                                                                        \
    template <typename TM, typename TL, int... NL, typename... TR>                            \
    auto _FUN_(Field<TM, TL, NL...> const& lhs, Expression<TR...> const& rhs) {               \
        return Expression<tags::_TAG_, Field<TM, TL, NL...>, Expression<TR...>>(lhs, rhs);    \
    };                                                                                        \
    template <typename... TL, typename TM, typename TR, int... NR>                            \
    auto _FUN_(Expression<TL...> const& lhs, Field<TM, TR, NR...> const& rhs) {               \
        return Expression<tags::_TAG_, Expression<TL...>, Field<TM, TR, NR...>>(lhs, rhs);    \
    };                                                                                        \
    template <typename ML, typename TL, int... NL, typename MR, typename TR, int... NR>       \
    auto _FUN_(Field<ML, TL, NL...> const& lhs, Field<MR, TR, NR...> const& rhs) {            \
        return Expression<tags::_TAG_, Field<ML, TL, NL...>, Field<MR, TR, NR...>>(lhs, rhs); \
    };

#define _SP_DEFINE_FIELD_UNARY_FUNCTION(_TAG_, _FUN_)              \
    template <typename TM, typename TL, int... NL>                 \
    auto _FUN_(Field<TM, TL, NL...> const& lhs) {                  \
        return Expression<tags::_TAG_, Field<TM, TL, NL...>>(lhs); \
    }

/**
* @defgroup  vector_algebra   Linear algebra on vector fields
* @{
*   Pseudo-Signature  			| Semantics
*  -------------------------------|--------------
*  \f$\Omega^n\f$ =\f$\Omega^n\f$  	            | negate operation
*  \f$\Omega^n\f$ =\f$\Omega^n\f$  	            | positive operation
*  \f$\Omega^n\f$ =\f$\Omega^n\f$ +\f$\Omega^n\f$ 	| add
*  \f$\Omega^n\f$ =\f$\Omega^n\f$ -\f$\Omega^n\f$ 	| subtract
*  \f$\Omega^n\f$ =\f$\Omega^n\f$ *Scalar  	    | multiply
*  \f$\Omega^n\f$ = Scalar * \f$\Omega^n\f$  	    | multiply
*  \f$\Omega^n\f$ = \f$\Omega^n\f$ / Scalar  	    | divide
*
*/

_SP_DEFINE_FIELD_UNARY_FUNCTION(cos, cos)
_SP_DEFINE_FIELD_UNARY_FUNCTION(acos, acos)
_SP_DEFINE_FIELD_UNARY_FUNCTION(cosh, cosh)
_SP_DEFINE_FIELD_UNARY_FUNCTION(sin, sin)
_SP_DEFINE_FIELD_UNARY_FUNCTION(asin, asin)
_SP_DEFINE_FIELD_UNARY_FUNCTION(sinh, sinh)
_SP_DEFINE_FIELD_UNARY_FUNCTION(tan, tan)
_SP_DEFINE_FIELD_UNARY_FUNCTION(tanh, tanh)
_SP_DEFINE_FIELD_UNARY_FUNCTION(atan, atan)
_SP_DEFINE_FIELD_UNARY_FUNCTION(exp, exp)
_SP_DEFINE_FIELD_UNARY_FUNCTION(log, log)
_SP_DEFINE_FIELD_UNARY_FUNCTION(log10, log10)
_SP_DEFINE_FIELD_UNARY_FUNCTION(sqrt, sqrt)
_SP_DEFINE_FIELD_BINARY_FUNCTION(atan2, atan2)
_SP_DEFINE_FIELD_BINARY_FUNCTION(pow, pow)
_SP_DEFINE_FIELD_BINARY_FUNCTION(dot, dot)
_SP_DEFINE_FIELD_BINARY_FUNCTION(cross, cross)

_SP_DEFINE_FIELD_BINARY_FUNCTION(addition, operator+)
_SP_DEFINE_FIELD_BINARY_FUNCTION(subtraction, operator-)
_SP_DEFINE_FIELD_BINARY_FUNCTION(multiplication, operator*)
_SP_DEFINE_FIELD_BINARY_FUNCTION(division, operator/)
_SP_DEFINE_FIELD_BINARY_FUNCTION(modulo, operator%)
_SP_DEFINE_FIELD_BINARY_FUNCTION(bitwise_xor, operator^)
_SP_DEFINE_FIELD_BINARY_FUNCTION(bitwise_and, operator&)
_SP_DEFINE_FIELD_BINARY_FUNCTION(bitwise_or, operator|)
_SP_DEFINE_FIELD_BINARY_FUNCTION(logical_and, operator&&)
_SP_DEFINE_FIELD_BINARY_FUNCTION(logical_or, operator||)

_SP_DEFINE_FIELD_UNARY_FUNCTION(bitwise_not, operator~)
_SP_DEFINE_FIELD_UNARY_FUNCTION(unary_plus, operator+)
_SP_DEFINE_FIELD_UNARY_FUNCTION(unary_minus, operator-)
_SP_DEFINE_FIELD_UNARY_FUNCTION(logical_not, operator!)

#undef _SP_DEFINE_FIELD_BINARY_FUNCTION
#undef _SP_DEFINE_FIELD_UNARY_FUNCTION
/** @} */

#define _SP_DEFINE_FIELD_COMPOUND_OP(_OP_)                                                            \
    template <typename TM, typename TL, int... NL, typename TR>                                       \
    Field<TM, TL, NL...>& operator _OP_##=(Field<TM, TL, NL...>& lhs, TR const& rhs) {                \
        lhs = lhs _OP_ rhs;                                                                           \
        return lhs;                                                                                   \
    }                                                                                                 \
    template <typename TM, typename TL, int... NL, typename... TR>                                    \
    Field<TM, TL, NL...>& operator _OP_##=(Field<TM, TL, NL...>& lhs, Expression<TR...> const& rhs) { \
        lhs = lhs _OP_ rhs;                                                                           \
        return lhs;                                                                                   \
    }

_SP_DEFINE_FIELD_COMPOUND_OP(+)
_SP_DEFINE_FIELD_COMPOUND_OP(-)
_SP_DEFINE_FIELD_COMPOUND_OP(*)
_SP_DEFINE_FIELD_COMPOUND_OP(/)
_SP_DEFINE_FIELD_COMPOUND_OP(%)
_SP_DEFINE_FIELD_COMPOUND_OP(&)
_SP_DEFINE_FIELD_COMPOUND_OP(|)
_SP_DEFINE_FIELD_COMPOUND_OP (^)
_SP_DEFINE_FIELD_COMPOUND_OP(<<)
_SP_DEFINE_FIELD_COMPOUND_OP(>>)
#undef _SP_DEFINE_FIELD_COMPOUND_OP

#define _SP_DEFINE_FIELD_BINARY_BOOLEAN_OPERATOR(_TAG_, _REDUCTION_, _OP_)                                    \
    template <typename TM, typename TL, int... NL, typename TR>                                               \
    bool operator _OP_(Field<TM, TL, NL...> const& lhs, TR const& rhs) {                                      \
        return calculus::reduction<_REDUCTION_>(Expression<tags::_TAG_, Field<TM, TL, NL...>, TR>(lhs, rhs)); \
    };                                                                                                        \
    template <typename TL, typename TM, typename TR, int... NR>                                               \
    bool operator _OP_(TL const& lhs, Field<TM, TR, NR...> const& rhs) {                                      \
        return calculus::reduction<_REDUCTION_>(Expression<tags::_TAG_, TL, Field<TM, TR, NR...>>(lhs, rhs)); \
    };                                                                                                        \
    template <typename TM, typename TL, int... NL, typename... TR>                                            \
    bool operator _OP_(Field<TM, TL, NL...> const& lhs, Expression<TR...> const& rhs) {                       \
        return calculus::reduction<_REDUCTION_>(                                                              \
            Expression<tags::_TAG_, Field<TM, TL, NL...>, Expression<TR...>>(lhs, rhs));                      \
    };                                                                                                        \
    template <typename... TL, typename TM, typename TR, int... NR>                                            \
    bool operator _OP_(Expression<TL...> const& lhs, Field<TM, TR, NR...> const& rhs) {                       \
        return calculus::reduction<_REDUCTION_>(                                                              \
            Expression<tags::_TAG_, Expression<TL...>, Field<TM, TR, NR...>>(lhs, rhs));                      \
    };                                                                                                        \
    template <typename TM, typename TL, int... NL, typename TR, int... NR>                                    \
    bool operator _OP_(Field<TM, TL, NL...> const& lhs, Field<TM, TR, NR...> const& rhs) {                    \
        return calculus::reduction<_REDUCTION_>(                                                              \
            Expression<tags::_TAG_, Field<TM, TL, NL...>, Field<TM, TR, NR...>>(lhs, rhs));                   \
    };

_SP_DEFINE_FIELD_BINARY_BOOLEAN_OPERATOR(not_equal_to, tags::logical_or, !=)
_SP_DEFINE_FIELD_BINARY_BOOLEAN_OPERATOR(equal_to, tags::logical_and, ==)
_SP_DEFINE_FIELD_BINARY_BOOLEAN_OPERATOR(less, tags::logical_and, <)
_SP_DEFINE_FIELD_BINARY_BOOLEAN_OPERATOR(greater, tags::logical_and, >)
_SP_DEFINE_FIELD_BINARY_BOOLEAN_OPERATOR(less_equal, tags::logical_and, <=)
_SP_DEFINE_FIELD_BINARY_BOOLEAN_OPERATOR(greater_equal, tags::logical_and, >=)

#undef _SP_DEFINE_FIELD_BINARY_BOOLEAN_OPERATOR
}  // namespace simpla//namespace algebra
#endif  // SIMPLA_FIELD_H
