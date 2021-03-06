//
// Created by salmon on 17-6-23.
//

#ifndef SIMPLA_NTUPLE_EXT_H
#define SIMPLA_NTUPLE_EXT_H
#include "simpla/SIMPLA_config.h"

#include <cmath>
#include <complex>
#include <iostream>

#include "simpla/utilities/type_traits.h"
#include "simpla/utilities/utility.h"

#include "nTuple.h"

namespace std {

template <typename V, int... N>
struct rank<simpla::nTuple<V, N...>> : public integral_constant<size_t, sizeof...(N)> {};
namespace detail {
template <unsigned M, int... N>
struct get_i;
template <unsigned M>
struct get_i<M> : public std::integral_constant<int, 0> {};
template <int N0, int... N>
struct get_i<0, N0, N...> : public std::integral_constant<int, N0> {};
template <unsigned M, int N0, int... N>
struct get_i<M, N0, N...> : public std::integral_constant<int, get_i<M - 1, N...>::value> {};
}
template <typename V, int... N, unsigned M>
struct extent<simpla::nTuple<V, N...>, M> : public integral_constant<size_t, detail::get_i<M, N...>::value> {};

template <typename T, int... N>
struct numeric_limits<simpla::nTuple<T, N...>> : public __numeric_limits_base {
    typedef simpla::nTuple<T, N...> _Tp;

    /** The minimum finite value, or for floating types with
    denormalization, the minimum positive normalized value.  */
    static const _Tp min() {
        _Tp res;
        res = numeric_limits<T>::min();
        return std::move(res);
    }

    /** The maximum finite value.  */
    static const _Tp max() {
        _Tp res;
        res = numeric_limits<T>::max();
        return std::move(res);
    }

    /** A finite value x such that there is no other finite value y
    *  where y < x.  */
    static const constexpr _Tp lowest() noexcept {
        _Tp res;
        res = numeric_limits<T>::lowest();
        return std::move(res);
    }

    /** The @e machine @e epsilon:  the difference between 1 and the least
    value greater than 1 that is representable.  */
    static const _Tp epsilon() {
        _Tp res;
        res = numeric_limits<T>::epsilon();
        return std::move(res);
    }

    /** The maximum rounding error measurement (see LIA-1).  */
    static const _Tp round_error() {
        _Tp res;
        res = numeric_limits<T>::round_error();
        return std::move(res);
    }

    /** The representation of positive infinity, if @c has_infinity.  */
    static const _Tp infinity() {
        _Tp res;
        res = numeric_limits<T>::infinity();
        return std::move(res);
    }

    /** The representation of a quiet Not a Number,
    if @c has_quiet_NaN. */
    static const _Tp quiet_NaN() {
        _Tp res;
        res = numeric_limits<T>::quiet_NaN();
        return std::move(res);
    }

    /** The representation of a signaling Not a Number, if
    @c has_signaling_NaN. */
    static const _Tp signaling_NaN() {
        _Tp res;
        res = numeric_limits<T>::signaling_NaN();
        return std::move(res);
    }

    /** The minimum positive denormalized value.  For types where
    @c has_denorm is false, this is the minimum positive normalized
    value.  */
    static const _Tp denorm_min() {
        _Tp res;
        res = numeric_limits<T>::denorm_min();
        return std::move(res);
    }
};
}  // namespace std {

namespace simpla {

template <typename T>
T determinant(nTuple<T, 3> const& m) {
    return m[0] * m[1] * m[2];
}

template <typename T>
T determinant(nTuple<T, 4> const& m) {
    return m[0] * m[1] * m[2] * m[3];
}

template <typename T>
T determinant(nTuple<T, 3, 3> const& m) {
    return m[0][0] * m[1][1] * m[2][2] - m[0][2] * m[1][1] * m[2][0] + m[0][1] * m[1][2] * m[2][0] -
           m[0][1] * m[1][0] * m[2][2] + m[1][0] * m[2][1] * m[0][2] - m[1][2] * m[2][1] * m[0][0];
}
template <typename TL, int... NL, typename TR, int... NR>
auto abs(nTuple<TL, NL...> const& l, nTuple<TR, NR...> const& r) {
    return std::sqrt(inner_product(l, r));
}
template <typename T, int... N>
T abs(nTuple<T, N...> const& m) {
    return std::sqrt(inner_product(m, m));
}

template <typename T>
T determinant(nTuple<T, 4, 4> const& m) {
    return m[0][3] * m[1][2] * m[2][1] * m[3][0] - m[0][2] * m[1][3] * m[2][1] * m[3][0] -
           m[0][3] * m[1][1] * m[2][2] * m[3][0] + m[0][1] * m[1][3] * m[2][2] * m[3][0] +
           m[0][2] * m[1][1] * m[2][3] * m[3][0] - m[0][1] * m[1][2] * m[2][3] * m[3][0] -
           m[0][3] * m[1][2] * m[2][0] * m[3][1] + m[0][2] * m[1][3] * m[2][0] * m[3][1] +
           m[0][3] * m[1][0] * m[2][2] * m[3][1] - m[0][0] * m[1][3] * m[2][2] * m[3][1] -
           m[0][2] * m[1][0] * m[2][3] * m[3][1] + m[0][0] * m[1][2] * m[2][3] * m[3][1] +
           m[0][3] * m[1][1] * m[2][0] * m[3][2] - m[0][1] * m[1][3] * m[2][0] * m[3][2] -
           m[0][3] * m[1][0] * m[2][1] * m[3][2] + m[0][0] * m[1][3] * m[2][1] * m[3][2] +
           m[0][1] * m[1][0] * m[2][3] * m[3][2] - m[0][0] * m[1][1] * m[2][3] * m[3][2] -
           m[0][2] * m[1][1] * m[2][0] * m[3][3] + m[0][1] * m[1][2] * m[2][0] * m[3][3] +
           m[0][2] * m[1][0] * m[2][1] * m[3][3] - m[0][0] * m[1][2] * m[2][1] * m[3][3] -
           m[0][1] * m[1][0] * m[2][2] * m[3][3] + m[0][0] * m[1][1] * m[2][2] * m[3][3];
}

template <typename T, int... N>
auto mod(nTuple<T, N...> const& l) {
    return std::sqrt(std::abs(inner_product(l, l)));
}

template <typename T>
auto normal(T const& l, ENABLE_IF((std::rank<T>::value > 0))) {
    return (l / (std::sqrt(inner_product(l, l))));
}
template <typename T>
nTuple<T, 3> rotate(nTuple<T, 3> const& v, nTuple<T, 3> const& u, T angle) {
    Real cosA = std::cos(angle);
    Real sinA = std::sin(angle);
    return (cosA * v + sinA * cross(u, v) + (1 - cosA) * dot(u, v) * u) / dot(u, u);
}
template <typename T>
auto abs(T const& l, ENABLE_IF((std::rank<T>::value > 0))) {
    return std::sqrt(inner_product(l, l));
}
template <typename T>
auto abs(T const& l, ENABLE_IF((std::rank<T>::value == 0))) {
    return std::abs(l);
}

template <typename T>
auto NProduct(T const& v, ENABLE_IF((std::rank<T>::value == 0))) {
    return ((calculus::reduction<tags::multiplication>(v)));
}

template <typename T>
auto NSum(T const& v, ENABLE_IF((std::rank<T>::value == 0))) {
    return ((calculus::reduction<tags::addition>(v)));
}

template <typename T, int N0>
std::istream& input(std::istream& is, nTuple<T, N0>& tv) {
    for (int i = 0; i < N0 && is; ++i) { is >> tv[i]; }
    return (is);
}

template <typename T, int N0, int... N>
std::istream& input(std::istream& is, nTuple<T, N0, N...>& tv) {
    for (int i = 0; i < N0 && is; ++i) { input(is, tv[i]); }
    return (is);
}

namespace _detail {
template <typename T, int... N>
std::ostream& printNd_(std::ostream& os, T const& d, std::integer_sequence<int, N...> const&) {
    os << d;
    return os;
}

template <typename T, int M, int... N>
std::ostream& printNd_(std::ostream& os, nTuple<T, M, N...> const& d, std::integer_sequence<int, M, N...> const&) {
    os << "[";
    printNd_(os, d[0], std::integer_sequence<int, N...>());
    for (int i = 1; i < M; ++i) {
        os << " , ";
        printNd_(os, d[i], std::integer_sequence<int, N...>());
    }
    os << "]";

    return os;
}

template <typename T>
std::istream& input(std::istream& is, T& a) {
    is >> a;
    return is;
}

template <typename T, int M0, int... M>
std::istream& input(std::istream& is, nTuple<T, M0, M...>& a) {
    for (int n = 0; n < M0; ++n) { _detail::input(is, a[n]); }
    return is;
}

}  // namespace _detail

template <typename T, int... M>
std::ostream& operator<<(std::ostream& os, nTuple<T, M...> const& v) {
    _detail::printNd_(os, v, std::integer_sequence<int, M...>());
    return os;
}

template <typename T, int N0, int... M>
std::istream& operator>>(std::istream& is, nTuple<T, N0, M...>& a) {
    _detail::input(is, a);
    return is;
}
template <typename T, int... M>
std::ostream& operator<<(std::ostream& os, std::tuple<nTuple<T, M...>, nTuple<T, M...>> const& v) {
    os << "{ " << std::get<0>(v) << " ," << std::get<1>(v) << "}";
    return os;
};

namespace traits {
template <typename T>
T& RecursiveIndexing(T& expr, unsigned int n) {
    return expr;
}
template <typename T, int N0, int... N>
T& RecursiveIndexing(nTuple<T, N0, N...>& expr, unsigned int n) {
    return RecursiveIndexing(expr[n % N0], n / N0);
}
template <typename T, int N0, int... N>
T const& RecursiveIndexing(nTuple<T, N0, N...> const& expr, unsigned int n) {
    return RecursiveIndexing(expr[n % N0], n / N0);
}
}
}  // namespace simpla{

#endif  // SIMPLA_NTUPLE_EXT_H
