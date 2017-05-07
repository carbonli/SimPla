/**
 * @file field.h
 * @author salmon
 * @date 2015-10-16.
 */

#ifndef SIMPLA_FIELD_H
#define SIMPLA_FIELD_H

#include <simpla/SIMPLA_config.h>
#include <simpla/engine/Attribute.h>
#include <simpla/engine/Domain.h>
#include <simpla/engine/MeshBlock.h>
#include <simpla/utilities/Array.h>
#include <simpla/utilities/EntityId.h>
#include <simpla/utilities/FancyStream.h>
#include <simpla/utilities/Range.h>
#include <simpla/utilities/nTuple.h>
#include <simpla/utilities/sp_def.h>
#include <cstring>  // for memset
#include "Algebra.h"
#include "CalculusPolicy.h"
namespace simpla {
namespace algebra {
namespace calculus {
template <typename...>
class calculator;
}

template <typename TM, typename TV, int IFORM, int DOF>
class FieldView : public engine::Attribute {
   private:
    typedef FieldView<TM, TV, IFORM, DOF> this_type;

   public:
    typedef TV value_type;
    typedef TM mesh_type;

    static constexpr int iform = IFORM;
    static constexpr int dof = DOF;
    static constexpr int NDIMS = mesh_type::NDIMS;
    static constexpr int NUMBER_OF_SUB = ((IFORM == VERTEX || IFORM == VOLUME) ? 1 : 3) * DOF;

    typedef std::true_type prefer_pass_by_reference;
    typedef std::false_type is_expression;
    typedef std::true_type is_field;
    typedef std::conditional_t<DOF == 1, value_type, nTuple<value_type, DOF>> cell_tuple;
    typedef std::conditional_t<(IFORM == VERTEX || IFORM == VOLUME), cell_tuple, nTuple<cell_tuple, 3>>
        field_value_type;

   private:
    typedef Array<value_type, NDIMS> array_type;
    std::vector<array_type> m_data_;
    mesh_type* m_mesh_ = nullptr;
    EntityRange m_range_;

   public:
    template <typename... Args>
    explicit FieldView(engine::Domain* d, Args&&... args)
        : engine::Attribute(IFORM, DOF, typeid(value_type), d,
                            std::make_shared<data::DataTable>(std::forward<Args>(args)...)),
          m_mesh_(dynamic_cast<mesh_type*>(engine::Attribute::GetDomain()->GetMesh())),
          m_data_(NUMBER_OF_SUB){};
    template <typename... Args>
    explicit FieldView(engine::MeshBase* d, Args&&... args)
        : engine::Attribute(IFORM, DOF, typeid(value_type), d,
                            std::make_shared<data::DataTable>(std::forward<Args>(args)...)),
          m_mesh_(dynamic_cast<mesh_type*>(engine::Attribute::GetDomain()->GetMesh())),
          m_data_(NUMBER_OF_SUB){};

    FieldView(this_type const& other)
        : engine::Attribute(other), m_mesh_(other.m_mesh_), m_data_(other.m_data_), m_range_(other.m_range_) {}

    FieldView(this_type&& other)
        : engine::Attribute(other), m_mesh_(other.m_mesh_), m_data_(other.m_data_), m_range_(other.m_range_) {}

    FieldView(this_type const& other, EntityRange const& r)
        : engine::Attribute(other), m_mesh_(other.m_mesh_), m_data_(other.m_data_), m_range_(r) {
        CHECK(m_range_.size());
    }

    ~FieldView() override = default;

    size_type size() const override { return m_range_.size() * DOF; }

    void Clear() {
        SetUp();
        for (int i = 0; i < m_data_.size(); ++i) { m_data_[i].Clear(); }
    }

    bool empty() const override { return m_data_.size() == 0; }

    this_type& operator=(this_type const& other) {
        Assign(other);
        return *this;
    }

    template <typename TR>
    this_type& operator=(TR const& rhs) {
        Assign(rhs);
        return *this;
    }

    void Push(std::shared_ptr<data::DataBlock> d, EntityRange const& r) override {
        Click();
        m_range_ = r;

        if (d != nullptr) {
            auto& t = d->cast_as<data::DataMultiArray<value_type, NDIMS>>();
            for (int i = 0; i < m_data_.size(); ++i) { array_type(t.GetArray(i)).swap(m_data_[i]); }
            Tag();
        }
    }
    std::shared_ptr<data::DataBlock> Pop() override {
        auto res = std::make_shared<data::DataMultiArray<value_type, NDIMS>>(m_data_.size());
        for (int i = 0; i < m_data_.size(); ++i) { array_type(m_data_[i]).swap(res->GetArray(i)); }
        return res;
    }

    array_type const& operator[](int i) const { return m_data_[i]; }
    array_type& operator[](int i) { return m_data_[i]; }

    value_type& operator()(index_type i, index_type j = 0, index_type k = 0, index_type w = 0) {
        return m_data_[w](i, j, k);
    }
    value_type const& operator()(index_type i, index_type j = 0, index_type k = 0, index_type w = 0) const {
        return m_data_[w](i, j, k);
    }

    //*****************************************************************************************************************

    typedef calculus::template calculator<mesh_type> calculus_policy;

    value_type const& at(EntityId s) const { return calculus_policy::getValue(*m_mesh_, *this, s); }
    value_type& at(EntityId s) { return calculus_policy::getValue(*m_mesh_, *this, s); }
    value_type const& operator[](EntityId s) const { return at(s); }
    value_type& operator[](EntityId s) { return at(s); }

    template <typename... Args>
    auto gather(Args&&... args) const {
        return calculus_policy::gather(*m_mesh_, *this, std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto scatter(Args&&... args) {
        return calculus_policy::scatter(*m_mesh_, *this, std::forward<Args>(args)...);
    }

    //        decltype(auto) operator()(point_type const& x) const { return gather(x); }

    void SetUp() override {
        engine::Attribute::SetUp();
        static constexpr int id_2_sub_edge[3] = {1, 2, 4};
        static constexpr int id_2_sub_face[3] = {6, 5, 3};

        for (int i = 0; i < NUMBER_OF_SUB; ++i) {
            if (!m_data_[i].empty()) { continue; }
            switch (IFORM) {
                case VERTEX:
                    array_type(m_mesh_->GetIndexBox(0)).swap(m_data_[i]);
                    break;
                case EDGE:
                    array_type(m_mesh_->GetIndexBox(id_2_sub_edge[(i / DOF) % 3])).swap(m_data_[i]);
                    break;
                case FACE:
                    array_type(m_mesh_->GetIndexBox(id_2_sub_face[(i / DOF) % 3])).swap(m_data_[i]);
                    break;
                case VOLUME:
                    array_type(m_mesh_->GetIndexBox(7)).swap(m_data_[i]);
                    break;
                default:
                    break;
            }
        }
        if (m_range_.empty() && GetDomain() != nullptr) { m_range_ = GetDomain()->GetBodyRange(GetIFORM()); }
        Tag();
    }

    template <typename Other>
    void Assign(Other const& other) {
        SetUp();
        static constexpr int id_2_sub_edge[3] = {1, 2, 4};
        static constexpr int id_2_sub_face[3] = {6, 5, 3};

        if (m_range_.empty()) {
            for (int i = 0; i < NUMBER_OF_SUB; ++i) {
                int16_t w = 0;
                switch (IFORM) {
                    case VERTEX:
                        w = static_cast<int16_t>(i << 3);
                        break;
                    case EDGE:
                        w = static_cast<int16_t>(((i % DOF) << 3) | id_2_sub_edge[(i / DOF) % 3]);
                        break;
                    case FACE:
                        w = static_cast<int16_t>(((i % DOF) << 3) | id_2_sub_face[(i / DOF) % 3]);
                        break;
                    case VOLUME:
                        w = static_cast<int16_t>((i << 3) | 0b111);
                        break;
                    default:
                        break;
                }
                m_data_[i].Foreach([&](index_tuple const& idx, value_type& v) {
                    EntityId s;
                    s.w = w;
                    s.x = static_cast<int16_t>(idx[0]);
                    s.y = static_cast<int16_t>(idx[1]);
                    s.z = static_cast<int16_t>(idx[2]);
                    v = calculus_policy::getValue(*m_mesh_, other, s);
                });
            }
        } else {
            for (int i = 0; i < DOF; ++i) {
                m_range_.foreach ([&](EntityId s) {
                    s.w = s.w | static_cast<int16_t>(i << 3);
                    at(s) = calculus_policy::getValue(*m_mesh_, other, s);
                });
            }
        }
    }
};  // class FieldView

namespace declare {

template <typename TM, typename TV, int IFORM, int DOF>
class Field_ : public FieldView<TM, TV, IFORM, DOF> {
    typedef Field_<TM, TV, IFORM, DOF> this_type;
    typedef FieldView<TM, TV, IFORM, DOF> base_type;

   public:
    template <typename... Args>
    explicit Field_(Args&&... args) : base_type(std::forward<Args>(args)...) {}

    Field_(this_type const& other) : base_type(other){};
    //    Field_(this_type&& other) = delete;
    ~Field_() {}

    using base_type::operator[];
    using base_type::operator=;
    using base_type::operator();

    this_type operator[](EntityRange const& d) const { return this_type(*this, d); }
};

}  // namespace declare

}  // namespace algebra

template <typename TM, typename TV, int IFORM = VERTEX, int DOF = 1>
using Field = algebra::declare::Field_<TM, TV, IFORM, DOF>;

}  // namespace simpla

#endif  // SIMPLA_FIELD_H
