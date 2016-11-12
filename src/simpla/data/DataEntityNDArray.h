//
// Created by salmon on 16-10-31.
//

#ifndef SIMPLA_ARRAYPATCH_H
#define SIMPLA_ARRAYPATCH_H

#include <type_traits>
#include <simpla/toolbox/PrettyStream.h>
#include <simpla/toolbox/Memory.h>
#include "DataEntity.h"
#include "DataBase.h"

namespace simpla { namespace data
{


template<typename V, unsigned int NDIMS>
class DataEntityNDArray : public DataEntityHeavy
{
public:
    static constexpr int ndims = NDIMS;
    typedef DataEntityNDArray<V, 4> this_type;
    typedef V value_type;

    DataEntityNDArray() {}

    DataEntityNDArray(index_type const *lo, index_type const *hi, value_type *p)
            : m_data_(p), m_holder_(nullptr) { initialize(lo, hi); }

    DataEntityNDArray(index_type const *lo, index_type const *hi, std::shared_ptr<value_type> const &p)
            : m_holder_(p), m_data_(p.get()) { initialize(lo, hi); };
private:
    void initialize(index_type const *lo, index_type const *hi)
    {
        m_ndims_ = 0;
        m_size_ = 1;
        for (int i = 0; i < ndims; ++i)
        {
            m_start_[i] = lo[i];
            if (hi[i] > lo[i])
            {
                m_count_[i] = static_cast<size_type>(hi[i] - lo[i]);
                ++m_ndims_;
            } else
            {
                m_count_[i] = 1;
            }
            m_size_ *= m_count_[i];
        }

        // C-order
        m_strides_[ndims - 1] = 1;
        for (int j = ndims - 2; j >= 0; --j)
        {
            m_strides_[j] = m_count_[j + 1] * m_strides_[j + 1];
        }
        CHECK("strides") << m_strides_[0] << "," << m_strides_[1] << "," << m_strides_[2] << "," << m_strides_[3]
                         << std::endl;
    }

public:
    DataEntityNDArray(index_box_type const &b, std::shared_ptr<value_type> const &p = nullptr)
            : DataEntityNDArray(&std::get<0>(b)[0], &std::get<1>(b)[0], p) {}

    DataEntityNDArray(this_type const &other) = delete;

    virtual ~DataEntityNDArray() {};

    virtual bool is_valid() const { return !empty(); };

    virtual bool empty() const { return m_data_ == nullptr; }

    virtual std::string get_class_name() const { return class_name(); }

    static std::string class_name()
    {
        return std::string("DataEntityNDArray<") + traits::type_id<value_type>::name() + std::string(",4>");
    }

    virtual std::ostream &print(std::ostream &os, int indent = 1) const
    {
        os << "-- dims=[" << m_count_[0] << "," << m_count_[1] << "," << m_count_[2] << "," << m_count_[3] << "," << "]"
           << std::endl;
        size_type r_count[ndims];
        int r_ndims = 0;
        for (int i = 0; i < ndims; ++i)
        {
            if (m_count_[i] > 1)
            {
                r_count[r_ndims] = m_count_[i];
                ++r_ndims;
            }
        }
        printNdArray(os, m_data_, r_ndims, r_count);
        return os;

    }

    virtual void load(DataBase const &, std::string const & = "") {};

    virtual void save(DataBase *, std::string const & = "") const {};

    virtual bool is_a(std::type_info const &t_info) const
    {
        return t_info == typeid(this_type) || DataEntityHeavy::is_a(t_info);
    };

    virtual bool is_null() const { return false; };

    virtual bool is_deployed() const { return m_data_ != nullptr; }

    virtual void deploy()
    {
        if (m_data_ == nullptr)
        {
            if (m_holder_ == nullptr) { m_holder_ = toolbox::MemoryHostAllocT<value_type>(m_size_); }

            m_data_ = m_holder_.get();
        }
    };

    virtual void destroy()
    {
        m_holder_.reset();
        m_data_ = nullptr;
    }

    virtual void clear()
    {
        deploy();
        toolbox::MemorySet(m_data_, 0, m_size_ * sizeof(value_type));
    }


    virtual void *data() { return m_data_; }

    virtual void const *data() const { return m_data_; }

    template<typename ...Args>
    value_type &get(Args &&...args) { return m_data_[hash(std::forward<Args>(args)...)]; }

    template<typename ...Args>
    value_type const &get(Args &&...args) const { return m_data_[hash(std::forward<Args>(args)...)]; }

private:

    int m_ndims_;

    index_type m_start_[ndims];
    size_type m_count_[ndims];
    size_type m_strides_[ndims];
    size_type m_size_;

    std::shared_ptr<value_type> m_holder_;
    value_type *m_data_;

    inline constexpr size_type hash(index_type x0) const
    {
        return (x0 - m_start_[0] + m_count_[0] * 2) % m_count_[0] * m_strides_[0];
    }

    inline constexpr size_type hash(index_type x0, index_type x1) const
    {
        return (x0 - m_start_[0] + m_count_[0] * 2) % m_count_[0] * m_strides_[0]
               + (x1 - m_start_[1] + m_count_[1] * 2) % m_count_[1] * m_strides_[1];
    }

    inline constexpr size_type hash(index_type x0, index_type x1, index_type x2) const
    {
        return (x0 - m_start_[0] + m_count_[0] * 2) % m_count_[0] * m_strides_[0]
               + (x1 - m_start_[1] + m_count_[1] * 2) % m_count_[1] * m_strides_[1]
               + (x2 - m_start_[2] + m_count_[2] * 2) % m_count_[2] * m_strides_[2];
    }

    inline constexpr size_type hash(index_type x0, index_type x1, index_type x2, index_type x3) const
    {
        return (x0 - m_start_[0] + m_count_[0] * 2) % m_count_[0] * m_strides_[0]
               + (x1 - m_start_[1] + m_count_[1] * 2) % m_count_[1] * m_strides_[1]
               + (x2 - m_start_[2] + m_count_[2] * 2) % m_count_[2] * m_strides_[2]
               + (x3 - m_start_[3] + m_count_[3] * 2) % m_count_[3] * m_strides_[3];
    }


    inline constexpr size_type
    hash2(index_type const *start, size_type const *count) const { return 0; }

    template<typename ...Args>
    inline constexpr size_type
    hash2(index_type const *start, size_type const *count, size_type const *stride, index_type first,
          Args const &...others) const
    {
        return (first - start[0] + count[0] * 2) % count[0] * stride[0] +
               hash2(start + 1, count + 1, stride + 1, std::forward<Args>(others)...);
    }

    template<typename ...Args>
    inline constexpr size_type
    hash(index_type x0, index_type x1, index_type x2, index_type x3, index_type s4, Args const &...args) const
    {
        return hash(x0, x1, x2, x3) +
               hash2(m_start_ + 4, m_count_ + 4, m_strides_ + 4, s4, std::forward<Args>(args)...);
    }

    inline constexpr size_type hash(nTuple<index_type, ndims> const idx) const
    {
        return hash(idx[0], idx[1], idx[2], idx[3]);
    }
};
}}//namespace simpla { namespace data
#endif //SIMPLA_ARRAYPATCH_H
