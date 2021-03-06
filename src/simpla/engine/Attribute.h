//
// Created by salmon on 16-10-20.
//

#ifndef SIMPLA_ATTRIBUTE_H
#define SIMPLA_ATTRIBUTE_H

#include "simpla/SIMPLA_config.h"

#include "EngineObject.h"
#include "simpla/algebra/Array.h"
#include "simpla/algebra/ExpressionTemplate.h"
#include "simpla/data/Data.h"
#include "simpla/utilities/SPDefines.h"
#include "simpla/utilities/type_traits.h"

namespace simpla {
template <typename V, typename SFC>
class Array;
namespace engine {
class Attribute;

///**
// *  permissions
// *
// *   r : readable
// *   w : writable
// *   c : create/delete
// *
// * [ 0:false 1:true ]
// * 0b0 0 0 0 0
// *   | | | | |------: is shared between different domain
// *   | | | |--------: has ghost cell
// *   | | |----------: PERSISTENT, if false then destroy data when Attribute is destructed
// *   | |------------: become unmodifiable after first write
// *   |--------------: is coordinate
// */
// enum AttributeTag {
//    SCRATCH = 0,
//    SHARED = 1,            //
//    GHOSTED = 1 << 1,      //
//    PERSISTENT = 1 << 2,   //
//    INPUT = 1 << 3,        //  can only be written once
//    COORDINATES = 1 << 4,  //  coordinate of mesh vertex
//    NO_FILL,
//    GLOBAL = SHARED | GHOSTED | PERSISTENT,
//    PRIVATE = GHOSTED | PERSISTENT,
//    DEFAULT_ATTRIBUTE_TAG = GLOBAL
//};

class AttributeGroup {
   private:
    struct pimpl_s;
    pimpl_s *m_pimpl_ = nullptr;

   public:
    typedef Attribute attribute_type;

    AttributeGroup();

    virtual ~AttributeGroup();

    virtual std::shared_ptr<data::DataEntry> Serialize() const;
    virtual void Deserialize(std::shared_ptr<const data::DataEntry> const &);

    //    virtual void Push(const std::shared_ptr<data::DataEntry> &);
    //    virtual std::shared_ptr<data::DataEntry> Pop() const;

    virtual void Push(const std::shared_ptr<Patch> &);
    virtual std::shared_ptr<Patch> Pop() const;
    virtual bool IsInitialized() const;

    std::set<Attribute *> &GetAttributes();
    std::set<Attribute *> const &GetAttributes() const;

    void Detach(Attribute *attr);
    void Attach(Attribute *attr);
};

/**
 *
 * Attribute
 *
 * @startuml
 * title Life cycle
 * actor Main
 * participant AttributeView
 * participant AttributeViewBundle
 * participant DomainView
 * participant AttributeViewAdapter as AttributeT <<T,IFORM,DOF>>
 * participant Attribute
 * Main->AttributeView: CreateDataBlock()
 * activate AttributeView
 *  AttributeView->AttributeT: CreateDataBlock(p)
 *  activate AttributeT
 *      AttributeT -> Mesh:
 *      Mesh --> AttributeT:
 *      AttributeT --> AttributeView :return DataBlock
 *  deactivate AttributeT
 *
 * AttributeT-->Main: return DataBlock
 * deactivate AttributeView
 * @enduml
 *
 *
 */
struct Attribute : public data::Configurable, public data::Serializable {
    SP_SERIALIZABLE_HEAD(data::Serializable, Attribute);
    SP_PROPERTY(std::string, Name);

   public:
    Attribute();
    ~Attribute() override;
    Attribute(this_type const &other) = delete;  // { UNIMPLEMENTED; };
    Attribute(this_type &&other) = delete;       // { UNIMPLEMENTED; };
    std::shared_ptr<Attribute> New(std::shared_ptr<simpla::data::DataEntry> const &cfg);

    template <typename THost, typename... Args>
    explicit Attribute(THost host, Args &&... args) : Attribute() {
        Register(host);
        SetProperties(std::forward<Args>(args)...);
    };

    virtual std::shared_ptr<Attribute> Copy() const = 0;
    virtual std::shared_ptr<Attribute> CreateNew() const = 0;

    void ReRegister(std::shared_ptr<Attribute> const &) const;

    virtual void Update() = 0;

    virtual bool CheckType(Attribute const &other) const = 0;
    virtual bool isNull() const = 0;
    virtual bool empty() const { return isNull(); };
    virtual std::type_info const &value_type_info() const = 0;
    virtual int GetIFORM() const = 0;
    virtual int GetDOF() const = 0;
    virtual int GetRank() const = 0;

    int GetNumOfSub() const { return GetIFORM() == NODE || GetIFORM() == CELL ? GetDOF() : 3 * GetDOF(); }

    void Register(AttributeGroup *p = nullptr);
    void Deregister(AttributeGroup *p = nullptr);

    virtual void Push(const std::shared_ptr<data::DataEntry> &) = 0;
    virtual std::shared_ptr<data::DataEntry> Pop() = 0;

    virtual void Clear() = 0;

   private:
    struct pimpl_s;
    pimpl_s *m_pimpl_ = nullptr;
};

template <typename V, int IFORM, int... DOF>
struct attribute_traits {
    typedef std::conditional_t<(IFORM == EDGE || IFORM == FACE), nTuple<Array<V>, 3, DOF...>,
                               std::conditional_t<sizeof...(DOF) == 0, Array<V>, nTuple<Array<V>, DOF...>>>
        data_type;
};
template <typename V>
struct attribute_traits<V, NODE> {
    typedef Array<V> data_type;
};
template <typename V>
struct attribute_traits<V, CELL> {
    typedef Array<V> data_type;
};

template <typename V, int IFORM, int... DOF>
struct AttributeT : public Attribute, public attribute_traits<V, IFORM, DOF...>::data_type {
   public:
    static bool _is_registered;

   private:
    typedef Attribute base_type;
    typedef AttributeT this_type;

   public:
    typedef typename attribute_traits<V, IFORM, DOF...>::data_type data_type;
    typedef V value_type;
    typedef Array<value_type> array_type;

    static constexpr int iform = IFORM;

    AttributeT();

    template <typename... Args>
    explicit AttributeT(Args &&... args) : Attribute(std::forward<Args>(args)...) {}
    ~AttributeT() override;

    template <typename... Args>
    static std::shared_ptr<this_type> New(Args &&... args) {
        return std::shared_ptr<this_type>(new this_type(std::forward<Args>(args)...));
    }

    static std::shared_ptr<this_type> New(std::shared_ptr<simpla::data::DataEntry> const &cfg) {
        auto res = std::shared_ptr<this_type>(new this_type());
        res->Deserialize(cfg);
        return res;
    }

    void Deserialize(std::shared_ptr<const simpla::data::DataEntry> const &cfg) override;
    std::shared_ptr<simpla::data::DataEntry> Serialize() const override;

    void Update() override;

    void Push(const std::shared_ptr<data::DataEntry> &) override;
    std::shared_ptr<data::DataEntry> Pop() override;

    std::shared_ptr<Attribute> Copy() const override {
        std::shared_ptr<this_type> res(new this_type);
        ReRegister(res);
        return res;
    }
    std::shared_ptr<Attribute> CreateNew() const override { return std::make_shared<this_type>(); }

    void Clear() override;

    bool CheckType(Attribute const &other) const override { return dynamic_cast<this_type const *>(&other) != nullptr; }
    bool isNull() const override;
    bool empty() const override { return isNull(); };
    std::type_info const &value_type_info() const override { return typeid(V); };
    int GetIFORM() const override { return IFORM; };
    int GetDOF() const override { return simpla::utility::NProduct(DOF...); };
    int GetRank() const override { return sizeof...(DOF); };
    auto GetDOFExtents() const { return nTuple<int, sizeof...(DOF)>{DOF...}; };

    auto &GetData(int n) { return traits::index(dynamic_cast<data_type &>(*this), n); }
    auto const &GetData(int n) const { return traits::index(dynamic_cast<data_type const &>(*this), n); }

    template <typename... Args>
    auto &Get(index_type i0, Args &&... args) {
        return traits::invoke(traits::index(dynamic_cast<data_type &>(*this), i0), std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto const &Get(index_type i0, Args &&... args) const {
        return traits::invoke(traits::index(dynamic_cast<data_type const &>(*this), i0), std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto &at(Args &&... args) {
        return Get(std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto const &at(Args &&... args) const {
        return Get(std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto &operator()(index_type i0, Args &&... args) {
        return Get(i0, std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto const &operator()(index_type i0, Args &&... args) const {
        return Get(i0, std::forward<Args>(args)...);
    }

    template <typename RHS, typename TRange>
    void Assign(RHS const &rhs, TRange const &range);
    template <typename RHS>
    void Assign(RHS const &rhs) {
        Assign(rhs, std::true_type());
    };

    template <int I0, typename RHS, typename TRange>
    void AssignSub(RHS const &rhs, TRange const &range);
    template <int I0, typename RHS>
    void AssignSub(RHS const &rhs) {
        AssignSub<I0>(rhs, std::true_type());
    };

    template <typename RHS>
    this_type &operator=(RHS const &rhs) {
        Assign(rhs);
        return *this;
    }

    template <typename... RHS>
    this_type &operator=(Expression<RHS...> const &rhs) {
        Update();
        data_type::operator=(rhs);
        return *this;
    }
};

template <typename V, int IFORM, int... DOF>
AttributeT<V, IFORM, DOF...>::AttributeT() = default;

template <typename V, int IFORM, int... DOF>
AttributeT<V, IFORM, DOF...>::~AttributeT() = default;

// template <typename V, int IFORM, int... DOF>
// std::shared_ptr<data::DataEntry> AttributeT<V, IFORM, DOF...>::GetDescription() const {
//    auto res = data::DataEntry::New(data::DataEntry::DN_TABLE);
//    res->Set(backend());
//    res->SetValue("Name", GetName());
//    res->SetValue("IFORM", IFORM);
//    if (sizeof...(DOF) > 0) {
//        res->SetValue("DOF", DOF...);
//    } else {
//        res->SetValue("DOF", 1);
//    }
//    res->SetValue("ValueType", traits::type_name<V>::value());
//    return res;
//};

namespace detail {
template <typename U>
std::shared_ptr<data::DataEntry> pop_data(Array<U> const &v) {
    auto d = data::DataBlock<U>::New();
    Array<U>(v).swap(*d);
    return data::DataEntry::New(d);
}

template <typename U, int N0, int... N>
std::shared_ptr<data::DataEntry> pop_data(nTuple<Array<U>, N0, N...> const &v) {
    auto res = data::DataEntry::New(data::DataEntry::DN_ARRAY);
    for (int i = 0; i < N0; ++i) { res->Add(pop_data(v[i])); }
    return res;
}

template <typename U>
size_type push_data(Array<U> &dest, std::shared_ptr<data::DataEntry> const &src) {
    size_type count = 0;
    if (src == nullptr) {
    } else if (auto p = std::dynamic_pointer_cast<Array<U>>(src->GetEntity())) {
        Array<U>(*p).swap(dest);
        count = 1;
    }

    return count;
}

template <typename U, int N0, int... N>
size_type push_data(nTuple<Array<U>, N0, N...> &v, std::shared_ptr<data::DataEntry> const &src) {
    size_type count = 0;

    for (int i = 0; i < N0; ++i) { count += push_data(v[i], src == nullptr ? nullptr : src->Get(i)); }

    return count;
}

template <typename U>
bool is_null(Array<U> const &d) {
    return d.Array<U>::isNull();
}

template <typename U, int N0, int... N>
bool is_null(nTuple<Array<U>, N0, N...> const &v) {
    bool res = false;
    for (int i = 0; i < N0; ++i) { res = res || is_null(v[i]); }
    return res;
}

template <typename U>
void tear_down(Array<U> &d) {
    d.Array<U>::free();
}

template <typename U, int N0, int... N>
void tear_down(nTuple<Array<U>, N0, N...> &v) {
    for (int i = 0; i < N0; ++i) { tear_down(v[i]); }
}

template <typename U>
void clear(Array<U> &d) {
    d.Array<U>::Clear();
}

template <typename U, int N0, int... N>
void clear(nTuple<Array<U>, N0, N...> &v) {
    for (int i = 0; i < N0; ++i) { clear(v[i]); }
}

template <typename U>
void update(Array<U> &d) {
    d.alloc();
}

template <typename U, int N0, int... N>
void update(nTuple<Array<U>, N0, N...> &v) {
    for (int i = 0; i < N0; ++i) { update(v[i]); }
}
}  // namespace detail{

template <typename V, int IFORM, int... DOF>
void AttributeT<V, IFORM, DOF...>::Update() {
    detail::update(*this);
};

template <typename V, int IFORM, int... DOF>
bool AttributeT<V, IFORM, DOF...>::isNull() const {
    return detail::is_null(*this);
};

template <typename V, int IFORM, int... DOF>
void AttributeT<V, IFORM, DOF...>::Clear() {
    Update();
    detail::clear(*this);
};

template <typename V, int IFORM, int... DOF>
std::shared_ptr<data::DataEntry> AttributeT<V, IFORM, DOF...>::Serialize() const {
    auto res = base_type::Serialize();
    res->SetValue("IFORM", IFORM);

    if (sizeof...(DOF) > 0) {
        res->SetValue("DOF", DOF...);
    } else {
        res->SetValue("DOF", 1);
    }
    res->SetValue("ValueType", traits::type_name<V>::value());
    return res;
};

template <typename V, int IFORM, int... DOF>
void AttributeT<V, IFORM, DOF...>::Deserialize(std::shared_ptr<const data::DataEntry> const &cfg) {
    base_type::Deserialize(cfg);
};

template <typename V, int IFORM, int... DOF>
void AttributeT<V, IFORM, DOF...>::Push(const std::shared_ptr<data::DataEntry> &d) {
    if (d != nullptr) detail::push_data(*this, d->Get("_DATA_"));
};

template <typename V, int IFORM, int... DOF>
std::shared_ptr<data::DataEntry> AttributeT<V, IFORM, DOF...>::Pop() {
    auto res = data::DataEntry::New(data::DataEntry::DN_TABLE);
    res->SetValue<int>("IFORM", GetIFORM());
    res->Set("_DATA_", detail::pop_data(*this));
    detail::tear_down(*this);
    return res;
};

namespace detail {

template <size_type I0, typename RHS, typename... Args>
auto try_invoke_(std::true_type, RHS const &rhs, Args &&... args) {
    return (rhs(std::forward<Args>(args)...));
};

template <size_type I0, typename RHS, typename... Args>
auto try_invoke_(std::false_type, RHS const &rhs, Args &&... args) {
    return (rhs);
};
template <size_type I0, typename RHS, typename... Args>
auto try_invoke(RHS const &rhs, Args &&... args) {
    return traits::nt_get_r<I0>(
        try_invoke_<I0>(traits::is_invocable<RHS, Args...>(), rhs, std::forward<Args>(args)...));
};

template <size_type I0, typename... V, typename RHS>
void Assign_(std::true_type, Array<V...> &lhs, RHS const &rhs) {
    lhs.Foreach([&](auto &v, auto &&... idx) { v = try_invoke<I0>(rhs, std::forward<decltype(idx)>(idx)...); });
};
template <size_type I0, typename... V, typename... U, int... N>
void Assign_(std::true_type, Array<V...> &lhs, nTuple<Array<U...>, N...> const &rhs) {
    lhs.Assign(traits::nt_get_r<I0>(rhs));
};
template <size_type I0, typename... V, typename... RHS>
void Assign_(std::true_type, Array<V...> &lhs, Expression<RHS...> const &rhs) {
    lhs.Assign(rhs);
};
template <typename... V, typename RHS>
void Assign(std::true_type const &, Array<V...> &lhs, RHS const &rhs) {
    lhs.Assign(rhs);
};
template <typename... V, int... N, typename... RHS>
void Assign(std::true_type const &, nTuple<Array<V...>, N...> &lhs, Expression<RHS...> const &rhs) {
    lhs = rhs;
};

template <size_type I0, typename TRange, typename... V, typename RHS>
void Assign_(TRange const &range, Array<V...> &lhs, RHS const &rhs) {
    range.Foreach([&](auto &&... idx) {
        lhs.Set(::simpla::detail::array_parser(rhs, std::forward<decltype(idx)>(idx)...),
                std::forward<decltype(idx)>(idx)...);
    });
};
template <size_type I0, typename TRange, typename... V, typename... U, int... N>
void Assign_(TRange const &range, Array<V...> &lhs, nTuple<Array<U...>, N...> const &rhs) {
    range.Foreach([&](auto &&... idx) {
        lhs.Set(simpla::traits::nt_get_r<I0>(lhs).Get(std::forward<decltype(idx)>(idx)...),
                std::forward<decltype(idx)>(idx)...);
    });
};
template <size_type I0, typename TRange, typename... V, typename... RHS>
void Assign_(TRange const &range, Array<V...> &lhs, Expression<RHS...> const &rhs) {
    range.Foreach([&](auto &&... idx) {
        lhs.Set(::simpla::detail::array_parser(rhs, std::forward<decltype(idx)>(idx)...),
                std::forward<decltype(idx)>(idx)...);
    });
};

template <typename TRange, typename... V, typename RHS>
void Assign(TRange const &range, Array<V...> &lhs, RHS const &rhs) {
    range.Foreach([&](auto &&... idx) {
        lhs.Set(try_invoke<0>(rhs, std::forward<decltype(idx)>(idx)...), std::forward<decltype(idx)>(idx)...);
    });
};

template <typename TRange, typename LHS, typename RHS>
void Assign(TRange const &, std::index_sequence<>, LHS &lhs, RHS const &rhs){};

template <typename TRange, size_type I0, size_type... I, typename... V, int... N, typename RHS>
void Assign(TRange const &range, std::index_sequence<I0, I...>, nTuple<Array<V...>, N...> &lhs, RHS const &rhs) {
    Assign_<I0>(range, simpla::traits::nt_get_r<I0>(lhs), rhs);
    Assign(range, std::index_sequence<I...>(), lhs, rhs);
};

template <typename TRange, typename... V, int... N, typename RHS>
void Assign(TRange const &range, nTuple<Array<V...>, N...> &lhs, RHS const &rhs) {
    Assign(range, std::make_index_sequence<simpla::traits::nt_size<nTuple<Array<V...>, N...>>::value>(), lhs, rhs);
};

}  // namespace detail

template <typename V, int IFORM, int... DOF>
template <typename RHS, typename TRange>
void AttributeT<V, IFORM, DOF...>::Assign(RHS const &rhs, TRange const &range) {
    Update();
    detail::Assign(range, *this, rhs);
};

template <typename V, int IFORM, int... DOF>
template <int I0, typename RHS, typename TRange>
void AttributeT<V, IFORM, DOF...>::AssignSub(RHS const &rhs, TRange const &range) {
    detail::Assign_<I0>(range, simpla::traits::nt_get_r<I0>(dynamic_cast<data_type &>(*this)), rhs);
};
}  // namespace engine

namespace traits {
template <typename>
struct reference;
template <typename TV, int... I>
struct reference<engine::AttributeT<TV, I...>> {
    typedef const engine::AttributeT<TV, I...> &type;
};

template <typename TV, int... I>
struct reference<const engine::AttributeT<TV, I...>> {
    typedef const engine::AttributeT<TV, I...> &type;
};
template <typename>
struct iform;

template <typename T>
struct iform<const T> : public std::integral_constant<int, iform<T>::value> {};
template <typename TV, int IFORM, int... DOF>
struct iform<engine::AttributeT<TV, IFORM, DOF...>> : public std::integral_constant<int, IFORM> {};
template <typename TF>
struct dof;
template <typename TV, int IFORM, int... DOF>
struct dof<engine::AttributeT<TV, IFORM, DOF...>>
    : public std::integral_constant<int, reduction_v(tags::multiplication(), 1, DOF...)> {};

template <typename TV, int... DOF>
struct value_type<engine::AttributeT<TV, DOF...>> {
    typedef TV type;
};
}  // namespace traits {
}  // namespace simpla

namespace std {
template <typename V, int... N>
struct rank<simpla::engine::AttributeT<V, N...>>
    : public integral_constant<size_t, rank<typename simpla::engine::AttributeT<V, N...>::data_type>::value> {};

template <typename V, int... N, unsigned I>
struct extent<simpla::engine::AttributeT<V, N...>, I>
    : public integral_constant<size_t, extent<typename simpla::engine::AttributeT<V, N...>::data_type, I>::value> {};
}
#endif  // SIMPLA_ATTRIBUTE_H
