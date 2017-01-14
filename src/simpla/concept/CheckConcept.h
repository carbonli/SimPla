/**
 * @file check_concept.h
 *
 *  Created on: 2015-1-12
 *      Author: salmon
 */

#ifndef CORE_CHECK_CONCEPT_H_
#define CORE_CHECK_CONCEPT_H_

#include <type_traits>
#include <utility>  //for std::forward
namespace simpla {
namespace concept {
/**
 * @ingroup concept
 *  @addtogroup concept_check Concept Checking
 *  @{
 */

/**
 * @brief check if T has type member  T::_NAME_
 *  if T has member type _NAME_, provides the member typedef type which is the same as T::_NAME_
 *  else  provides void
 *
  EXAMPLE:

 @code{.cpp}
    #include <simpla/concept/CheckConcept.h>
    #include <iostream>
    using namespace simpla;

    struct Foo { typedef double value_type;};
    struct Goo { typedef int value_type;};
    struct Koo {};

    CHECK_MEMBER_TYPE(value_type, value_type)

    int main(int argc, char** argv) {
        std::cout << " Foo::value_type " << (has_value_type<Foo>::value ? "is" : "is not") <<
 "defined." << std::endl;
        std::cout << " Goo::value_type " << (has_value_type<Goo>::value ? "is" : "is not") <<
 "defined." << std::endl;
        std::cout << " Koo::value_type " << (has_value_type<Koo>::value ? "is" : "is not") <<
 "defined." << std::endl;

        std::cout << " Foo::value_type " << (std::is_same<value_type_t<Foo>, double>::value ? "is" :
 "is not") << " double" << std::endl;
        std::cout << " Goo::value_type " << (std::is_same<value_type_t<Goo>, double>::value ? "is" :
 "is not") << " double" << std::endl;
        std::cout << " Koo::value_type " << (std::is_same<value_type_t<Koo>, double>::value ? "is" :
 "is not") << " double" << std::endl;
    }
 @endcode

 OUTPUT:
  @verbatim
     Foo::value_type is defined
     Goo::value_type is defined
     Koo::value_type is not defined
     Foo::value_type is double
     Goo::value_type is not double
     Koo::value_type is not double
  @endverbatim

 *
 */
#define CHECK_TYPE_MEMBER(_CHECKER_NAME_, _TYPE_NAME_)                                            \
    template <typename _T>                                                                        \
    struct _CHECKER_NAME_ {                                                                       \
       private:                                                                                   \
        typedef std::true_type yes;                                                               \
        typedef std::false_type no;                                                               \
                                                                                                  \
        template <typename U>                                                                     \
        static auto test(int) -> typename U::_TYPE_NAME_;                                         \
        template <typename>                                                                       \
        static no test(...);                                                                      \
        typedef decltype(test<_T>(0)) result_type;                                                \
                                                                                                  \
       public:                                                                                    \
        typedef std::conditional_t<std::is_same<result_type, no>::value, void, result_type> type; \
    };                                                                                            \
    template <typename _T>                                                                        \
    using _CHECKER_NAME_##_t = typename _CHECKER_NAME_<_T>::type;                                 \
    template <typename _T>                                                                        \
    struct has_##_CHECKER_NAME_                                                                   \
        : public std::integral_constant<                                                          \
              bool, !std::is_same<typename _CHECKER_NAME_<_T>::type, void>::value> {};

/**
 * @brief  check if   type T has a  type member as std::true_type or std::false_type
 *
EXAMPLE:
 @code{.cpp}
    #include <simpla/concept/CheckMemberType.h>
    #include <iostream>
    using namespace simpla;

    struct Foo { typedef std::true_type is_foo;};
    struct Goo { typedef std::false_type is_foo; };
    struct Koo {};

    CHECK_BOOLEAN_TYPE_MEMBER(is_foo, is_foo)

    int main(int argc, char** argv) {
        std::cout << " Foo  " << (is_foo<Foo>::value ? "is" : "is not") << " foo." << std::endl;
        std::cout << " Goo  " << (is_foo<Goo>::value ? "is" : "is not") << " foo." << std::endl;
        std::cout << " Koo  " << (is_foo<Koo>::value ? "is" : "is not") << " foo." << std::endl;
    }
 @endcode

 OUTPUT:

 @verbatim
  Foo  is foo.
  Goo  is not foo.
  Koo  is not foo.

 @endverbatim

 */

#define CHECK_BOOLEAN_TYPE_MEMBER(_CHECKER_NAME_, _MEM_NAME_)                \
    namespace detail {                                                       \
    template <typename T>                                                    \
    struct _CHECKER_NAME_ {                                                  \
       private:                                                              \
        typedef std::true_type yes;                                          \
        typedef std::false_type no;                                          \
                                                                             \
        template <typename U>                                                \
        static auto test(int) -> typename U::_MEM_NAME_;                     \
        template <typename>                                                  \
        static no test(...);                                                 \
        typedef decltype(test<T>(0)) result_type;                            \
                                                                             \
       public:                                                               \
        typedef std::conditional_t<std::is_same<result_type, yes>::value ||  \
                                       std::is_same<result_type, no>::value, \
                                   result_type, no>                          \
            type;                                                            \
    };                                                                       \
    }                                                                        \
    template <typename...>                                                   \
    struct _CHECKER_NAME_;                                                   \
    template <typename T>                                                    \
    struct _CHECKER_NAME_<T> : public detail::_CHECKER_NAME_<T>::type {};

/**
 * @brief check if a type T has member variable _NAME_
 *
 * EXAMPLE:
 * @code{.cpp}
    struct Foo { static constexpr int iform = 2;};
    struct Goo { static constexpr double iform = 2.1; };
    struct Koo {};
    CHECK_MEMBER_STATIC_CONSTEXPR_DATA(has_iform, iform)
    TEST(CheckConceptTest, CheckMemberStaticConstexprData) {
    EXPECT_TRUE(has_iform<Foo>::value);
    EXPECT_TRUE(has_iform<Goo>::value);
    EXPECT_FALSE(has_iform<Koo>::value);
}
 * @endcode
 */

#define CHECK_STATIC_CONSTEXPR_DATA_MEMBER(_CHECKER_NAME_, _V_NAME_)                              \
    namespace detail {                                                                            \
    template <typename _T>                                                                        \
    struct _CHECKER_NAME_ {                                                                       \
       private:                                                                                   \
        typedef std::true_type yes;                                                               \
        typedef std::false_type no;                                                               \
                                                                                                  \
        template <typename U>                                                                     \
        static auto test(int) -> decltype(U::_V_NAME_);                                           \
        template <typename>                                                                       \
        static no test(...);                                                                      \
                                                                                                  \
       public:                                                                                    \
        typedef std::conditional_t<std::is_same<decltype(test<_T>(0)), no>::value, no, yes> type; \
    };                                                                                            \
    }                                                                                             \
    template <typename T>                                                                         \
    struct _CHECKER_NAME_ : public detail::_CHECKER_NAME_<T>::type {};

/**
 * @brief check if T has static  data member  T::_NAME_
 *
 *
 *   @code{.cpp}
    struct Foo { static constexpr int iform = 2;};
    struct Goo { static constexpr double iform = 2.1; };
    struct Koo {};
    CHECK_MEMBER_STATIC_CONSTEXPR_DATA_VALUE(iform_value, iform, 12)
    TEST(CheckConceptTest, CheckMemberStaticConstexprDataValue) {
    EXPECT_EQ(iform_value<Foo>::value, 2);
    EXPECT_DOUBLE_EQ(iform_value<Goo>::value, 2.1);
    EXPECT_NE(iform_value<Goo>::value, 1);
    EXPECT_EQ(iform_value<Koo>::value, 12);
   }
 * @endcode
 */

#define CHECK_VALUE_OF_STATIC_CONSTEXPR_DATA_MEMBER(_CHECKER_NAME_, _V_NAME_, _DEFAULT_VALUE_) \
    namespace detail {                                                                         \
    CHECK_STATIC_CONSTEXPR_DATA_MEMBER(check_##_CHECKER_NAME_, _V_NAME_)                       \
    template <typename T, bool ENABLE>                                                         \
    struct _CHECKER_NAME_;                                                                     \
    template <typename T>                                                                      \
    struct _CHECKER_NAME_<T, true> {                                                           \
        static constexpr decltype(T::_V_NAME_) value = T::_V_NAME_;                            \
    };                                                                                         \
    template <typename T>                                                                      \
    struct _CHECKER_NAME_<T, false> {                                                          \
        static constexpr decltype(_DEFAULT_VALUE_) value = _DEFAULT_VALUE_;                    \
    };                                                                                         \
    template <typename T>                                                                      \
    constexpr decltype(T::_V_NAME_) _CHECKER_NAME_<T, true>::value;                            \
    template <typename T>                                                                      \
    constexpr decltype(_DEFAULT_VALUE_) _CHECKER_NAME_<T, false>::value;                       \
    } /*namespace detail*/                                                                     \
    template <typename T>                                                                      \
    struct _CHECKER_NAME_                                                                      \
        : public detail::_CHECKER_NAME_<T, detail::check_##_CHECKER_NAME_<T>::value> {};

#define CHECK_STATIC_INTEGRAL_CONSTEXPR_DATA_MEMBER(_CHECKER_NAME_, _V_NAME_, _DEFAULT_VALUE_) \
    namespace detail {                                                                         \
    CHECK_STATIC_CONSTEXPR_DATA_MEMBER(has_##_CHECKER_NAME_, _V_NAME_)                         \
    template <typename T, bool ENABLE>                                                         \
    struct _CHECKER_NAME_;                                                                     \
    template <typename T>                                                                      \
    struct _CHECKER_NAME_<T, true> {                                                           \
        static constexpr int value = T::_V_NAME_;                                              \
    };                                                                                         \
    template <typename T>                                                                      \
    struct _CHECKER_NAME_<T, false> {                                                          \
        static constexpr int value = _DEFAULT_VALUE_;                                          \
    };                                                                                         \
    template <typename T>                                                                      \
    constexpr int _CHECKER_NAME_<T, true>::value;                                              \
    template <typename T>                                                                      \
    constexpr int _CHECKER_NAME_<T, false>::value;                                             \
    } /*namespace detail*/                                                                     \
    template <typename T>                                                                      \
    struct _CHECKER_NAME_                                                                      \
        : public int_const<                                                                    \
              detail::_CHECKER_NAME_<T, detail::has_##_CHECKER_NAME_<T>::value>::value> {};
/**
 * @brief check if T has data memeber _V_NAME_
 *
 @code{.cpp}
 CHECK_MEMBER_DATA(has_data, data)
 TEST(CheckConceptTest, CheckMemberData) {
     EXPECT_TRUE((has_data<Foo>::value));
     EXPECT_TRUE((has_data<Foo, int>::value));
     EXPECT_FALSE((has_data<Foo, double>::value));
     EXPECT_FALSE((has_data<Koo>::value));
     EXPECT_FALSE((has_data<Koo, double>::value));
 }
 @endcode
 */
#define CHECK_DATA_MEMBER(_F_NAME_, _V_NAME_)                                         \
    namespace detail {                                                                \
    template <typename _T, typename _D>                                               \
    struct _F_NAME_ {                                                                 \
       private:                                                                       \
        typedef std::true_type yes;                                                   \
        typedef std::false_type no;                                                   \
                                                                                      \
        template <typename U>                                                         \
        static auto test(int) -> decltype(std::declval<U>()._V_NAME_);                \
        template <typename>                                                           \
        static no test(...);                                                          \
        typedef decltype(test<_T>(0)) check_result;                                   \
                                                                                      \
       public:                                                                        \
        static constexpr bool value =                                                 \
            (!std::is_same<check_result, no>::value) &&                               \
            (std::is_same<_D, void>::value || std::is_same<check_result, _D>::value); \
    };                                                                                \
    }                                                                                 \
    template <typename T, typename D = void>                                          \
    struct _F_NAME_ : public std::integral_constant<bool, detail::_F_NAME_<T, D>::value> {};

//#define CHECK_FUNCTION(_CHECKER_NAME_, _FUN_NAME_)                                                \
//    namespace detail {                                                                            \
//    template <typename...>                                                                        \
//    struct _CHECKER_NAME_ {                                                                       \
//        static constexpr bool value = false;                                                      \
//    };                                                                                            \
//    template <typename _TRet, typename... _Args>                                                  \
//    struct _CHECKER_NAME_<_TRet(_Args...)> {                                                      \
//       private:                                                                                   \
//        typedef std::true_type yes;                                                               \
//        typedef std::false_type no;                                                               \
//        static auto test(_Args &&... args) -> decltype(_FUN_NAME_(std::forward<_Args>(args)...)); \
//        static no test(...);                                                                      \
//                                                                                                  \
//        typedef decltype(test(std::declval<_Args>()...)) check_result;                            \
//                                                                                                  \
//       public:                                                                                    \
//        static constexpr bool value = std::is_same<check_result, _TRet>::value;                   \
//    };                                                                                            \
//    }                                                                                             \
//    template <typename... _Args>                                                                  \
//    struct _CHECKER_NAME_                                                                         \
//        : public std::integral_constant<bool, detail::_CHECKER_NAME_<_Args...>::value> {};

/**
 * @brief
 */
#define CHECK_FUNCTION_MEMBER(_CHECKER_NAME_, _FUN_NAME_)                                          \
    namespace detail {                                                                             \
    template <typename...>                                                                         \
    struct _CHECKER_NAME_ {                                                                        \
        static constexpr bool value = false;                                                       \
    };                                                                                             \
    template <typename _T, typename _TRet, typename... _Args>                                      \
    struct _CHECKER_NAME_<_T, _TRet(_Args...)> {                                                   \
       private:                                                                                    \
        typedef std::true_type yes;                                                                \
        typedef std::false_type no;                                                                \
                                                                                                   \
        template <typename U>                                                                      \
        static auto test(int) ->                                                                   \
            typename std::enable_if<sizeof...(_Args) == 0,                                         \
                                    decltype(std::declval<U>()._FUN_NAME_())>::type;               \
                                                                                                   \
        template <typename U>                                                                      \
        static auto test(int) ->                                                                   \
            typename std::enable_if<(sizeof...(_Args) > 0), decltype(std::declval<U>()._FUN_NAME_( \
                                                                std::declval<_Args>()...))>::type; \
                                                                                                   \
        template <typename>                                                                        \
        static no test(...);                                                                       \
                                                                                                   \
        typedef decltype(test<_T>(0)) check_result;                                                \
                                                                                                   \
       public:                                                                                     \
        static constexpr bool value = std::is_same<decltype(test<_T>(0)), _TRet>::value;           \
    };                                                                                             \
    }                                                                                              \
    template <typename... _Args>                                                                   \
    struct _CHECKER_NAME_                                                                          \
        : public std::integral_constant<bool, detail::_CHECKER_NAME_<_Args...>::value> {};

/**
 * @brief
 */
#define CHECK_STATIC_FUNCTION_MEMBER(_CHECKER_NAME_, _FUN_NAME_)                              \
    namespace detail {                                                                        \
    template <typename...>                                                                    \
    struct _CHECKER_NAME_ {                                                                   \
        static constexpr bool value = false;                                                  \
    };                                                                                        \
    template <typename _T, typename _TRet, typename... _Args>                                 \
    struct _CHECKER_NAME_<_T, _TRet(_Args...)> {                                              \
       private:                                                                               \
        typedef std::true_type yes;                                                           \
        typedef std::false_type no;                                                           \
                                                                                              \
        template <typename U>                                                                 \
        static auto test(int) ->                                                              \
            typename std::enable_if<sizeof...(_Args) == 0, decltype(U::_FUN_NAME_())>::type;  \
                                                                                              \
        template <typename U>                                                                 \
        static auto test(int) ->                                                              \
            typename std::enable_if<(sizeof...(_Args) > 0),                                   \
                                    decltype(U::_FUN_NAME_(std::declval<_Args>()...))>::type; \
                                                                                              \
        template <typename>                                                                   \
        static no test(...);                                                                  \
                                                                                              \
        typedef decltype(test<_T>(0)) check_result;                                           \
                                                                                              \
       public:                                                                                \
        static constexpr bool value = std::is_same<decltype(test<_T>(0)), _TRet>::value;      \
    };                                                                                        \
    }                                                                                         \
    template <typename... _Args>                                                              \
    struct _CHECKER_NAME_                                                                     \
        : public std::integral_constant<bool, detail::_CHECKER_NAME_<_Args...>::value> {};

#define CHECK_OPERATOR(_NAME_, _OP_)             \
    namespace detail {                           \
    CHECK_FUNCTION_MEMBER(_NAME_, operator _OP_) \
    }                                            \
    template <typename... T>                     \
    struct _NAME_ : public std::integral_constant<bool, detail::_NAME_<T...>::value> {};

// CHECK_OPERATOR(is_indexable, [])
/**
 * @ref http://en.cppreference.com/w/cpp/types/remove_extent
 * If T is '''is_indexable''' by '''S''', provides the member typedef type equal to
 * decltyp(T[S])
 * otherwise type is T. Note that if T is a multidimensional array, only the first dimension is
 * removed.
 */
namespace detail {

template <typename _T, typename _TI>
struct is_indexable {
   private:
    typedef std::true_type yes;
    typedef std::false_type no;

    template <typename _U>
    static auto test(int) -> decltype(std::declval<_U>()[std::declval<_TI>()]);

    template <typename>
    static no test(...);
    typedef decltype(test<_T>(0)) check_result;

   public:
    static constexpr bool value = !std::is_same<check_result, no>::value;
    typedef std::conditional_t<std::is_same<check_result, no>::value, void, check_result> type;
};
}
template <typename T, typename TI = int>
struct is_indexable : public std::integral_constant<bool, detail::is_indexable<T, TI>::value> {
    typedef typename detail::is_indexable<T, TI>::type type;
};
// CHECK_OPERATOR(is_callable, ())

template <typename...>
struct is_callable;
template <typename _T, typename... _Args>
struct is_callable<_T, _Args...> {
   private:
    typedef std::true_type yes;
    typedef std::false_type no;

    template <typename _U>
    static auto test(int) -> typename std::result_of<_U(_Args...)>::type;

    template <typename>
    static no test(...);

    typedef decltype(test<_T>(0)) check_result;

   public:
    static constexpr bool value = !std::is_same<check_result, no>::value;
};

template <typename _T, typename _TRet, typename... _Args>
struct is_callable<_T, _TRet(_Args...)> {
   private:
    typedef std::true_type yes;
    typedef std::false_type no;

    template <typename _U>
    static auto test(int) -> typename std::result_of<_U(_Args...)>::type;

    template <typename>
    static no test(...);

    typedef decltype(test<_T>(0)) check_result;

   public:
    static constexpr bool value = std::is_same<check_result, _TRet>::value;
};

template <typename TFun, typename... Args>
auto try_invoke(TFun const &fun, Args &&... args) ->
    typename std::result_of<TFun(Args &&...)>::type {
    return (fun(std::forward<Args>(args)...));
}

template <typename TFun, typename... Args>
auto try_invoke(TFun const &fun, Args &&... args) ->
    typename std::enable_if<!is_callable<TFun, Args &&...>::value, TFun>::type {
    return fun;
}

}  // namespace concept
}  // namespace simpla

#endif /* CORE_CHECK_CONCEPT_H_ */