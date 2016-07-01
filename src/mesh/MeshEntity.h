/**
 * @file MeshEntity.h
 * @author salmon
 * @date 2016-05-18.
 */

#ifndef SIMPLA_MESH_MESHENTITY_H
#define SIMPLA_MESH_MESHENTITY_H

#include <type_traits>
#include <cassert>

#include "../parallel/Parallel.h"

#include "MeshCommon.h"
#include "MeshEntityId.h"


namespace simpla { namespace mesh
{

class MeshEntityRange
{
    typedef MeshEntityRange this_type;

    class RangeBase;

    template<typename, bool> struct RangeHolder;

    HAS_MEMBER_FUNCTION(range)

public :
//    typedef MeshEntityIterator iterator;

    MeshEntityRange() : m_holder_(nullptr) { }

    //****************************************************************************
    // TBB RangeHolder Concept Begin
    template<typename TOther>
    MeshEntityRange(TOther const &other) :
            m_holder_(std::dynamic_pointer_cast<RangeBase>(
                    std::make_shared<RangeHolder<TOther,
                            has_member_function_range<TOther>::value>>(other)))
    {
    }

    template<typename ...Args>
    MeshEntityRange(this_type &other, parallel::tags::split) :
            m_holder_(other.m_holder_->split())
    {
    }

    MeshEntityRange(this_type const &other) : m_holder_(
            other.m_holder_ == nullptr ? nullptr : other.m_holder_->clone()) { }

    MeshEntityRange(this_type &&other) : m_holder_(other.m_holder_) { }

    ~MeshEntityRange() { }


public:
    static const bool is_splittable_in_proportion = true;

    bool is_divisible() const { return m_holder_->is_divisible(); }

    bool empty() const { return m_holder_ == nullptr || m_holder_->empty(); }

    size_t size() const { return m_holder_->size(); }

    template<typename T, typename ...Args,
            typename std::enable_if<!std::is_base_of<RangeBase, T>::value>::type * = nullptr>
    static MeshEntityRange create(Args &&...args)
    {
        MeshEntityRange res;
        res.m_holder_ = std::dynamic_pointer_cast<RangeBase>(
                RangeHolder<T, has_member_function_range<T>::value>::create(std::forward<Args>(args)...));
        return std::move(res);
    }

    template<typename T, typename ...Args,
            typename std::enable_if<std::is_base_of<RangeBase, T>::value>::type * = nullptr>
    static MeshEntityRange create(Args &&...args)
    {
        MeshEntityRange res;
        res.m_holder_ = std::dynamic_pointer_cast<RangeBase>(std::make_shared<T>(std::forward<Args>(args)...));
        return std::move(res);
    }

    this_type operator=(this_type const &other)
    {
        m_holder_ = other.m_holder_;
        return *this;
    }

    this_type &swap(this_type &other)
    {
        std::swap(m_holder_, other.m_holder_);
        return *this;
    }

    template<typename T> T &as()
    {
        return m_holder_->as<T>();

    }

    template<typename T> T const &as() const
    {
        return m_holder_->as<T>();
    }

    typedef std::function<void(MeshEntityId const &)> foreach_body_type;

    void foreach(foreach_body_type const &body) const { m_holder_->foreach(body); }


private:

    std::shared_ptr<RangeBase> m_holder_;

    struct RangeBase
    {
        virtual std::shared_ptr<RangeBase> clone() const = 0;

        virtual std::shared_ptr<RangeBase> split() = 0;

        virtual bool is_a(std::type_info const &) const = 0;

        virtual bool is_divisible() const = 0;

        virtual size_t size() const = 0;

        virtual bool empty() const = 0;

        template<typename T> T &as()
        {
            assert(is_a(typeid(T)));

            return std::dynamic_pointer_cast<RangeHolder<T, has_member_function_range<T>::value> *>(this)->self();

        }

        template<typename T> T const &as() const
        {
            assert(is_a(typeid(T)));

            return std::dynamic_pointer_cast<RangeHolder<T, has_member_function_range<T>::value> const *>(this)->self();
        }


        virtual void foreach(foreach_body_type const &body) const = 0;
    };


    template<typename TOtherRange>
    struct RangeHolder<TOtherRange, false> : public RangeBase
    {
        typedef RangeHolder<TOtherRange, false> this_type;
        TOtherRange m_range_;
    public:

        RangeHolder(TOtherRange const &other) : m_range_(other) { }

        template<typename ...Args>
        RangeHolder(this_type &other, Args &&...args) : m_range_(other.m_range_, std::forward<Args>(args)...) { }

        virtual  ~RangeHolder() { }

        TOtherRange &self() { return m_range_; }

        TOtherRange const &self() const { return m_range_; }

        template<typename ...Args>
        static std::shared_ptr<this_type> create(Args &&...args)
        {
            return std::shared_ptr<this_type>(new this_type{TOtherRange(std::forward<Args>(args)...)});
        }

        virtual std::shared_ptr<RangeBase> clone() const
        {
            return std::dynamic_pointer_cast<RangeBase>(std::make_shared<this_type>(*this));
        }

        virtual std::shared_ptr<RangeBase> split()
        {
            return std::dynamic_pointer_cast<RangeBase>(std::make_shared<this_type>(*this, parallel::tags::split()));
        };

        virtual size_t size() const { return m_range_.size(); }

        virtual bool is_a(std::type_info const &t_info) const final { return typeid(TOtherRange) == t_info; }

        virtual bool is_divisible() const { return m_range_.is_divisible(); }

        virtual bool empty() const { return m_range_.empty(); }

    public:

        virtual void foreach(foreach_body_type const &body) const
        {
            parallel::foreach(m_range_, body);
        };
    };


    template<typename TContainer>
    struct RangeHolder<TContainer, true> : public RangeBase
    {
        typedef typename TContainer::const_range_type range_type;
        typedef RangeHolder<TContainer, true> this_type;
        TContainer const *m_container_;
        range_type m_range_;
    public:

        RangeHolder(TContainer const &other) :
                m_container_(&other), m_range_(m_container_->range()) { }

        template<typename ...Args>
        RangeHolder(this_type &other, Args &&...args) :
                m_container_(other.m_container_), m_range_(other.m_range_, std::forward<Args>(args)...) { }

        virtual  ~RangeHolder() { }

        template<typename ...Args> static std::shared_ptr<this_type>
        create(Args &&...  args)
        {
            return std::shared_ptr<this_type>(new this_type{TContainer(std::forward<Args>(args)...)});
        }

        range_type &self() { return m_range_; }

        range_type const &self() const { return m_range_; }

        virtual std::shared_ptr<RangeBase> clone() const
        {
            return std::dynamic_pointer_cast<RangeBase>(std::make_shared<this_type>(*this));
        }

        virtual std::shared_ptr<RangeBase> split()
        {
            return std::dynamic_pointer_cast<RangeBase>(std::make_shared<this_type>(*this, parallel::tags::split()));
        };

        virtual size_t size() const { return m_container_->size(); }

        virtual bool is_a(std::type_info const &t_info)
        const final { return typeid(TContainer) == t_info; }

        virtual bool is_divisible() const { return m_range_.is_divisible(); }

        virtual bool empty() const { return m_range_.empty(); }

        virtual void foreach(foreach_body_type const &body) const
        {
            parallel::foreach(m_range_, body);
        };
    };
};

}} //namespace simpla { namespace get_mesh

#endif //SIMPLA_MESH_MESHENTITY_H
