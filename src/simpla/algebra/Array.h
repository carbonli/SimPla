//
// Created by salmon on 16-12-28.
//

#ifndef SIMPLA_ARRAY_H
#define SIMPLA_ARRAY_H
#include "simpla/SIMPLA_config.h"

#include <simpla/utilities/FancyStream.h>
#include <initializer_list>
#include <limits>
#include <memory>
#include <tuple>

#include "simpla/utilities/Log.h"
#include "simpla/utilities/type_traits.h"

#include "ExpressionTemplate.h"
#include "SFC.h"
#include "nTuple.h"

namespace simpla {
template <typename V, typename SFC>
class Array;
typedef nTuple<index_type, 3> IdxShift;

struct ArrayBase {
    virtual std::type_info const& value_type_info() const = 0;
    virtual size_type size() const = 0;
    virtual void* pointer() = 0;
    virtual void const* pointer() const = 0;
    virtual int GetNDIMS() const = 0;
    virtual int GetIndexBox(index_type* lo, index_type* hi) const = 0;
    virtual bool empty() const = 0;
    virtual bool isNull() const = 0;
    virtual size_type CopyIn(ArrayBase const& other) = 0;
    virtual size_type CopyOut(ArrayBase& other) const { return other.CopyIn(*this); };
    virtual void Clear() = 0;
    virtual void reset(void*, index_type const* lo, index_type const* hi) = 0;
};

template <typename V, typename SFC = ZSFC<3>>
class Array : public ArrayBase {
   public:
    typedef V value_type;
    typedef typename SFC::array_index_box_type array_index_box_type;

   private:
    typedef Array<value_type, SFC> this_type;
    SFC m_sfc_;
    std::shared_ptr<value_type> m_holder_ = nullptr;
    value_type* m_data_ = nullptr;

   public:
    Array() = default;
    virtual ~Array(){};

    Array(SFC const& sfc) : m_sfc_(sfc), m_data_(nullptr), m_holder_(nullptr) {}

    Array(this_type const& other) : m_sfc_(other.m_sfc_), m_data_(other.m_data_), m_holder_(other.m_holder_) {}
    Array(this_type& other) : m_sfc_(other.m_sfc_), m_data_(other.m_data_), m_holder_(other.m_holder_) {}

    Array(this_type&& other) noexcept
        : m_sfc_(other.m_sfc_), m_data_(other.m_data_), m_holder_(std::shared_ptr<value_type>(other.m_holder_)) {}

    Array(this_type const& other, IdxShift s) : Array(other) { Shift(s); }
    template <typename... Args>
    explicit Array(Args&&... args) : m_sfc_(std::forward<Args>(args)...) {}

    template <typename... Args>
    explicit Array(value_type* d, Args&&... args)
        : m_holder_(d, tags::do_nothing()), m_sfc_(std::forward<Args>(args)...) {}

    template <typename... Args>
    explicit Array(std::shared_ptr<value_type> const& d, Args&&... args)
        : m_holder_(d), m_sfc_(std::forward<Args>(args)...) {}

    void swap(this_type& other) {
        std::swap(m_holder_, other.m_holder_);
        std::swap(m_data_, other.m_data_);
        m_sfc_.swap(other.m_sfc_);
    }

    std::ostream& Print(std::ostream& os, int indent = 0,
#ifndef NDEBUG
                        bool verbose = true
#else
                        bool verbose = false
#endif
                        ) const;

    void SetUp() { alloc(); }
    void TearDown() {
        m_holder_.reset();
        m_data_ = nullptr;
    }

    Array& operator=(this_type&& other) {
        this_type(std::forward<this_type>(other)).swap(*this);
        return *this;
    };

    this_type Sub(index_box_type const& b) { return this_type(m_data_, m_sfc_.Sub(b)); }

    std::type_info const& value_type_info() const override { return typeid(value_type); };
    size_type size() const override { return m_sfc_.size(); }
    void* pointer() override { return m_data_; }
    void const* pointer() const override { return m_data_; }
    int GetNDIMS() const override { return m_sfc_.GetNDIMS(); }
    int GetIndexBox(index_type* lo, index_type* hi) const override { return m_sfc_.GetIndexBox(lo, hi); }
    bool empty() const override { return m_data_ == nullptr || size() == 0; }
    bool isNull() const override { return m_data_ == nullptr || size() == 0; }

    auto GetIndexBox() const { return m_sfc_.GetIndexBox(); }

    size_type CopyIn(ArrayBase const& other) override {
        size_type count = 0;
        if (auto* p = dynamic_cast<this_type const*>(&other)) { count = CopyIn(*p); }
        return count;
    }
    size_type CopyOut(ArrayBase& other) const override {
        size_type count = 0;
        if (auto* p = dynamic_cast<this_type*>(&other)) { count = CopyOut(*p); }
        return count;
    };
    void reset(void* d, index_type const* lo, index_type const* hi) override {
        m_data_ = reinterpret_cast<value_type*>(d);
        m_holder_.reset();
        m_sfc_.reset(lo, hi);
    };
    template <typename... Args>
    void reset(value_type* d, Args&&... args) {
        m_data_ = d;
        m_holder_.reset();
        m_sfc_.reset(std::forward<Args>(args)...);
    }
    template <typename... Args>
    void reset(std::shared_ptr<value_type> const& d, Args&&... args) {
        m_holder_ = d;
        m_data_ = m_holder_.get();
        m_sfc_.reset(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void reset(Args&&... args) {
        m_sfc_.reset(std::forward<Args>(args)...);
        if (m_sfc_.empty()) {
            m_data_ = nullptr;
            m_holder_.reset();
        }
    }
    template <typename... Args>
    bool in_box(Args&&... args) const {
        return m_sfc_.in_box(std::forward<Args>(args)...);
    }

    SFC const& GetSpaceFillingCurve() const { return m_sfc_; }

    std::shared_ptr<value_type>& GetData() { return m_holder_; }
    std::shared_ptr<value_type> const& GetData() const { return m_holder_; }

    value_type* get() { return m_data_; }
    value_type const* get() const { return m_data_; }

    template <typename... Args>
    void Shift(Args&&... args) {
        m_sfc_.Shift(std::forward<Args>(args)...);
    }

    size_type CopyIn(this_type const& other) {
        SetUp();
        return m_sfc_.Overlap(other.m_sfc_).Foreach([&] __host__ __device__(auto&&... s) {
            this->at(std::forward<decltype(s)>(s)...) = other.at(std::forward<decltype(s)>(s)...);
        });
    };
    size_type CopyOut(this_type& other) const { return other.CopyIn(*this); };

    void DeepCopy(value_type const* other) {
        SetUp();
        m_sfc_.Copy(m_data_, other);
    }
    void FillNaN() { Fill(std::numeric_limits<value_type>::signaling_NaN()); }
    void Fill(value_type v) {
        SetUp();
        m_sfc_.Foreach([&] __host__ __device__(auto&&... s) { this->at(std::forward<decltype(s)>(s)...) = v; });
    }
    void Clear() override {
        SetUp();
        memset(m_data_, 0, size() * sizeof(value_type));
    }

    this_type& operator=(this_type const& rhs) {
        Assign(rhs);
        return (*this);
    }

    template <typename TR>
    this_type& operator=(TR const& rhs) {
        Assign(rhs);
        return (*this);
    }

    this_type operator()(IdxShift const& idx) const { return this_type(*this, idx); }

    __host__ __device__ value_type& operator[](size_type s) { return m_data_[s]; }

    __host__ __device__ value_type const& operator[](size_type s) const { return m_data_[s]; }

    template <typename... Args>
    __host__ __device__ value_type& at(Args&&... args) {
        ASSERT(m_sfc_.in_box(std::forward<Args>(args)...));
        return m_data_[m_sfc_.hash(std::forward<Args>(args)...)];
    }
    template <typename... Args>
    __host__ __device__ value_type const& at(Args&&... args) const {
        ASSERT(m_sfc_.in_box(std::forward<Args>(args)...));
        return m_data_[m_sfc_.hash(std::forward<Args>(args)...)];
    }
    template <typename... Args>
    __host__ __device__ value_type& operator()(index_type s0, Args&&... args) {
        return at(s0, std::forward<Args>(args)...);
    }
    template <typename... Args>
    __host__ __device__ value_type const& operator()(index_type s0, Args&&... args) const {
        return at(s0, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void Set(value_type const& v, Args&&... args) {
        if (m_sfc_.in_box(std::forward<Args>(args)...)) { m_data_[m_sfc_.hash(std::forward<Args>(args)...)] = v; }
    }

    template <typename... Args>
    value_type Get(Args&&... args) const {
        if (m_sfc_.in_box(std::forward<Args>(args)...)) {
            return m_data_[m_sfc_.hash(std::forward<Args>(args)...)];
        } else {
            return std::numeric_limits<value_type>::signaling_NaN();
        }
    }

    template <typename RHS>
    void Assign(RHS const& rhs) {
        SetUp();
        m_sfc_.Overlap(rhs).Foreach([&] __host__ __device__(auto&&... s) {
            this->at(std::forward<decltype(s)>(s)...) = calculus::getValue(rhs, std::forward<decltype(s)>(s)...);
        });
    }

    template <typename RHS, typename... Args>
    void Assign(RHS const& rhs, Args&&... args) {
        if (GetSpaceFillingCurve().in_box(std::forward<Args>(args)...)) {
            at(std::forward<Args>(args)...) = calculus::getValue(rhs, std::forward<Args>(args)...);
        }
    }

    void alloc();
};
template <typename V, typename SFC>
void Array<V, SFC>::alloc() {
    if (m_data_ == nullptr && m_sfc_.size() > 0) {
        if (m_holder_ == nullptr) { m_holder_ = spMakeShared<value_type>(m_data_, m_sfc_.size()); }
        m_data_ = m_holder_.get();
#ifndef NDEBUG
        Fill(std::numeric_limits<V>::signaling_NaN());
#else
        Fill(0);
#endif
    }
}
template <typename V, typename SFC>
std::ostream& Array<V, SFC>::Print(std::ostream& os, int indent, bool verbose) const {
    os << "Array<" << simpla::traits::type_name<value_type>::value() << ">" << GetIndexBox() << std::endl;

    //    if (size() < 20 || verbose) {
    int ndims = GetNDIMS();
    index_type lo[ndims], hi[ndims];
    GetIndexBox(lo, hi);
    FancyPrintNd<3>(os, *this, lo, hi, indent);
    //    }
    return os;
}
namespace traits {
template <typename... T>
struct reference<Array<T...>> {
    typedef Array<T...> type;
};

template <typename... T, typename TFun>
auto foreach (Array<T...>& v, TFun const& f) {
    v.GetSpaceFillingCurve().Foreach(
        [&](auto&&... s) { f(v(std::forward<decltype(s)>(s)...), std::forward<decltype(s)>(s)...); });
}

template <typename... T, typename TFun>
auto foreach (Array<T...> const& v, TFun const& f) {
    v.GetSpaceFillingCurve().Foreach(
        [&](auto&&... s) { f(v(std::forward<decltype(s)>(s)...), std::forward<decltype(s)>(s)...); });
}
}

template <typename... TL>
std::ostream& operator<<(std::ostream& os, Array<TL...> const& lhs) {
    return lhs.Print(os, 0);
};

template <typename... TL>
std::istream& operator>>(std::istream& is, Array<TL...>& lhs) {
    UNIMPLEMENTED;
    return is;
};

#define _SP_DEFINE_ARRAY_BINARY_OPERATOR(_OP_, _NAME_)                                      \
    template <typename... TL, typename TR>                                                  \
    auto operator _OP_(Array<TL...> const& lhs, TR const& rhs) {                            \
        return Expression<simpla::tags::_NAME_, Array<TL...>, TR>(lhs, rhs);                \
    };                                                                                      \
    template <typename TL, typename... TR>                                                  \
    auto operator _OP_(TL const& lhs, Array<TR...> const& rhs) {                            \
        return Expression<simpla::tags::_NAME_, TL, Array<TR...>>(lhs, rhs);                \
    };                                                                                      \
    template <typename... TL, typename... TR>                                               \
    auto operator _OP_(Array<TL...> const& lhs, Expression<TR...> const& rhs) {             \
        return Expression<simpla::tags::_NAME_, Array<TL...>, Expression<TR...>>(lhs, rhs); \
    };                                                                                      \
    template <typename... TL, typename... TR>                                               \
    auto operator _OP_(Expression<TL...> const& lhs, Array<TR...> const& rhs) {             \
        return Expression<simpla::tags::_NAME_, Expression<TL...>, Array<TR...>>(lhs, rhs); \
    };                                                                                      \
    template <typename... TL, typename... TR>                                               \
    auto operator _OP_(Array<TL...> const& lhs, Array<TR...> const& rhs) {                  \
        return Expression<simpla::tags::_NAME_, Array<TL...>, Array<TR...>>(lhs, rhs);      \
    };

#define _SP_DEFINE_ARRAY_UNARY_OPERATOR(_OP_, _NAME_)               \
    template <typename... TL>                                       \
    auto operator _OP_(Array<TL...> const& lhs) {                   \
        return Expression<simpla::tags::_NAME_, Array<TL...>>(lhs); \
    };

_SP_DEFINE_ARRAY_BINARY_OPERATOR(+, addition)
_SP_DEFINE_ARRAY_BINARY_OPERATOR(-, subtraction)
_SP_DEFINE_ARRAY_BINARY_OPERATOR(*, multiplication)
_SP_DEFINE_ARRAY_BINARY_OPERATOR(/, division)
_SP_DEFINE_ARRAY_BINARY_OPERATOR(%, modulo)

_SP_DEFINE_ARRAY_UNARY_OPERATOR(~, bitwise_not)
_SP_DEFINE_ARRAY_BINARY_OPERATOR (^, bitwise_xor)
_SP_DEFINE_ARRAY_BINARY_OPERATOR(&, bitwise_and)
_SP_DEFINE_ARRAY_BINARY_OPERATOR(|, bitwise_or)

template <typename... TL>
auto operator<<(Array<TL...> const& lhs, unsigned int n) {
    return Expression<simpla::tags::bitwise_left_shift, Array<TL...>, int>(lhs, n);
};

template <typename... TL>
auto operator>>(Array<TL...> const& lhs, unsigned int n) {
    return Expression<simpla::tags::bitwise_right_shifit, Array<TL...>, int>(lhs, n);
};
//_SP_DEFINE_ARRAY_BINARY_OPERATOR(bitwise_left_shift, <<)
//_SP_DEFINE_ARRAY_BINARY_OPERATOR(bitwise_right_shift, >>)

_SP_DEFINE_ARRAY_UNARY_OPERATOR(+, unary_plus)
_SP_DEFINE_ARRAY_UNARY_OPERATOR(-, unary_minus)
_SP_DEFINE_ARRAY_UNARY_OPERATOR(!, logical_not)
_SP_DEFINE_ARRAY_BINARY_OPERATOR(&&, logical_and)
_SP_DEFINE_ARRAY_BINARY_OPERATOR(||, logical_or)

#undef _SP_DEFINE_ARRAY_BINARY_OPERATOR
#undef _SP_DEFINE_ARRAY_UNARY_OPERATOR

#define _SP_DEFINE_ARRAY_BINARY_FUNCTION(_NAME_)                                            \
    template <typename... TL, typename TR>                                                  \
    auto _NAME_(Array<TL...> const& lhs, TR const& rhs) {                                   \
        return Expression<simpla::tags::_NAME_, Array<TL...>, TR>(lhs, rhs);                \
    };                                                                                      \
    template <typename TL, typename... TR>                                                  \
    auto _NAME_(TL const& lhs, Array<TR...> const& rhs) {                                   \
        return Expression<simpla::tags::_NAME_, TL, Array<TR...>>(lhs, rhs);                \
    };                                                                                      \
    template <typename... TL, typename... TR>                                               \
    auto _NAME_(Array<TL...> const& lhs, Expression<TR...> const& rhs) {                    \
        return Expression<simpla::tags::_NAME_, Array<TL...>, Expression<TR...>>(lhs, rhs); \
    };                                                                                      \
    template <typename... TL, typename... TR>                                               \
    auto _NAME_(Expression<TL...> const& lhs, Array<TR...> const& rhs) {                    \
        return Expression<simpla::tags::_NAME_, Expression<TL...>, Array<TR...>>(lhs, rhs); \
    };                                                                                      \
    template <typename... TL, typename... TR>                                               \
    auto _NAME_(Array<TL...> const& lhs, Array<TR...> const& rhs) {                         \
        return Expression<simpla::tags::_NAME_, Array<TL...>, Array<TR...>>(lhs, rhs);      \
    };

#define _SP_DEFINE_ARRAY_UNARY_FUNCTION(_NAME_)                    \
    template <typename... T>                                       \
    auto _NAME_(Array<T...> const& lhs) {                          \
        return Expression<simpla::tags::_NAME_, Array<T...>>(lhs); \
    }

_SP_DEFINE_ARRAY_UNARY_FUNCTION(cos)
_SP_DEFINE_ARRAY_UNARY_FUNCTION(acos)
_SP_DEFINE_ARRAY_UNARY_FUNCTION(cosh)
_SP_DEFINE_ARRAY_UNARY_FUNCTION(sin)
_SP_DEFINE_ARRAY_UNARY_FUNCTION(asin)
_SP_DEFINE_ARRAY_UNARY_FUNCTION(sinh)
_SP_DEFINE_ARRAY_UNARY_FUNCTION(tan)
_SP_DEFINE_ARRAY_UNARY_FUNCTION(tanh)
_SP_DEFINE_ARRAY_UNARY_FUNCTION(atan)
_SP_DEFINE_ARRAY_UNARY_FUNCTION(exp)
_SP_DEFINE_ARRAY_UNARY_FUNCTION(log)
_SP_DEFINE_ARRAY_UNARY_FUNCTION(log10)
_SP_DEFINE_ARRAY_UNARY_FUNCTION(sqrt)
_SP_DEFINE_ARRAY_BINARY_FUNCTION(atan2)
_SP_DEFINE_ARRAY_BINARY_FUNCTION(pow)

#undef _SP_DEFINE_ARRAY_BINARY_FUNCTION
#undef _SP_DEFINE_ARRAY_UNARY_FUNCTION

#define _SP_DEFINE_ARRAY_COMPOUND_OP(_OP_)                                            \
    template <typename... TL, typename TR>                                            \
    Array<TL...>& operator _OP_##=(Array<TL...>& lhs, TR const& rhs) {                \
        lhs = lhs _OP_ rhs;                                                           \
        return lhs;                                                                   \
    }                                                                                 \
    template <typename... TL, typename... TR>                                         \
    Array<TL...>& operator _OP_##=(Array<TL...>& lhs, Expression<TR...> const& rhs) { \
        lhs = lhs _OP_ rhs;                                                           \
        return lhs;                                                                   \
    }

_SP_DEFINE_ARRAY_COMPOUND_OP(+)
_SP_DEFINE_ARRAY_COMPOUND_OP(-)
_SP_DEFINE_ARRAY_COMPOUND_OP(*)
_SP_DEFINE_ARRAY_COMPOUND_OP(/)
_SP_DEFINE_ARRAY_COMPOUND_OP(%)
_SP_DEFINE_ARRAY_COMPOUND_OP(&)
_SP_DEFINE_ARRAY_COMPOUND_OP(|)
_SP_DEFINE_ARRAY_COMPOUND_OP (^)
_SP_DEFINE_ARRAY_COMPOUND_OP(<<)
_SP_DEFINE_ARRAY_COMPOUND_OP(>>)

#undef _SP_DEFINE_ARRAY_COMPOUND_OP

#define _SP_DEFINE_ARRAY_BINARY_BOOLEAN_OPERATOR(_OP_, _NAME_, _REDUCTION_)                                    \
    template <typename... TL, typename TR>                                                                     \
    bool operator _OP_(Array<TL...> const& lhs, TR const& rhs) {                                               \
        return calculus::reduction<_REDUCTION_>(Expression<simpla::tags::_NAME_, Array<TL...>, TR>(lhs, rhs)); \
    };                                                                                                         \
    template <typename TL, typename... TR>                                                                     \
    bool operator _OP_(TL const& lhs, Array<TR...> const& rhs) {                                               \
        return calculus::reduction<_REDUCTION_>(Expression<simpla::tags::_NAME_, TL, Array<TR...>>(lhs, rhs)); \
    };                                                                                                         \
    template <typename... TL, typename... TR>                                                                  \
    bool operator _OP_(Array<TL...> const& lhs, Expression<TR...> const& rhs) {                                \
        return calculus::reduction<_REDUCTION_>(                                                               \
            Expression<simpla::tags::_NAME_, Array<TL...>, Expression<TR...>>(lhs, rhs));                      \
    };                                                                                                         \
    template <typename... TL, typename... TR>                                                                  \
    bool operator _OP_(Expression<TL...> const& lhs, Array<TR...> const& rhs) {                                \
        return calculus::reduction<_REDUCTION_>(                                                               \
            Expression<simpla::tags::_NAME_, Expression<TL...>, Array<TR...>>(lhs, rhs));                      \
    };                                                                                                         \
    template <typename... TL, typename... TR>                                                                  \
    bool operator _OP_(Array<TL...> const& lhs, Array<TR...> const& rhs) {                                     \
        return calculus::reduction<_REDUCTION_>(                                                               \
            Expression<simpla::tags::_NAME_, Array<TL...>, Array<TR...>>(lhs, rhs));                           \
    };

_SP_DEFINE_ARRAY_BINARY_BOOLEAN_OPERATOR(!=, not_equal_to, simpla::tags::logical_or)
_SP_DEFINE_ARRAY_BINARY_BOOLEAN_OPERATOR(==, equal_to, simpla::tags::logical_and)
_SP_DEFINE_ARRAY_BINARY_BOOLEAN_OPERATOR(<=, less_equal, simpla::tags::logical_and)
_SP_DEFINE_ARRAY_BINARY_BOOLEAN_OPERATOR(>=, greater_equal, simpla::tags::logical_and)
_SP_DEFINE_ARRAY_BINARY_BOOLEAN_OPERATOR(<, less, simpla::tags::logical_and)
_SP_DEFINE_ARRAY_BINARY_BOOLEAN_OPERATOR(>, greater, simpla::tags::logical_and)
#undef _SP_DEFINE_ARRAY_BINARY_BOOLEAN_OPERATOR

}  // namespace simpla{
#endif  // SIMPLA_ARRAY_H
