/**
 * @file context.h
 *
 * @date    2014-9-18  AM9:33:53
 * @author salmon
 */

#ifndef CORE_APPLICATION_CONTEXT_H_
#define CORE_APPLICATION_CONTEXT_H_

#include <list>
#include "task_flow_base.h"
#include "../mesh/Mesh.h"
#include "../mesh/MeshAtlas.h"
#include "../mesh/MeshAttribute.h"
#include "../field/Field.h"
#include "../manifold/Calculus.h"

namespace simpla
{
namespace particle { template<typename ...> class Particle; }
namespace field { template<typename ...> class Field; }
}

namespace simpla { namespace task_flow
{

class Context
{
    typedef Context this_type;
public:


    template<typename TV, typename TM, int IFORM> using
    field_t=field::Field<TV, TM, std::integral_constant<int, IFORM>, Attribute>;

    template<typename TEngine, typename TM> using
    particle_t=particle::Particle<TEngine, TM, Attribute>;

    class Attribute;

    class Worker;

    void apply(Worker &w, mesh::uuid const &id, Real dt);

    void sync(Worker &w, mesh::uuid const &oid);


private:
    mesh::MeshAtlas m_mesh_atlas_;

    std::map<std::string, std::weak_ptr<Attribute>> m_attributes_;

public:


//    void next_time_step() { m_time_ += m_dt_; }
//
//    double time() const { return m_time_; }
//
//    void time(double t) { m_time_ = t; }
//
//    double dt() const { return m_dt_; }
//
//    void dt(double p_dt) { m_dt_ = p_dt; }


    template<typename TF>
    std::shared_ptr<TF> get_attribute(std::string const &s_name)
    {
        static_assert(std::is_base_of<Attribute, TF>::value);

        auto it = m_attributes_.find(s_name);

        if (it == m_attributes_.end())
        {
            return create_attribute<TF>(s_name);
        }
        else if (it->second.lock()->is_a(typeid(TF)))
        {
            return std::dynamic_pointer_cast<TF>(it->second.lock());
        }
        else
        {
            return nullptr;
        }

    }

    template<typename TF>
    std::shared_ptr<TF> create_attribute(std::string const &s_name = "")
    {
        static_assert(std::is_base_of<Attribute, TF>::value);

        auto res = std::make_shared<TF>(*this);

        if (s_name != "") { enroll(s_name, std::dynamic_pointer_cast<Attribute>(res)); }

        return res;
    }

    template<typename TF>
    std::shared_ptr<TF> create_attribute() const
    {
        return std::make_shared<TF>(*this);
    }

    template<typename TF>
    void enroll(std::string const &name, std::shared_ptr<TF> p)
    {
        static_assert(std::is_base_of<Attribute, TF>::value);

        m_attributes_.insert(std::make_pair(name, std::dynamic_pointer_cast<Attribute>(p)));
    };


};

template<typename TManifold>
struct Context<TManifold>::Attribute : public base::Object
{
    SP_OBJECT_HEAD(Attribute, base::Object);

    mesh::MeshAtlas const &m_mesh_tree_;

    std::map<mesh::uuid, std::shared_ptr<void>> m_data_tree_;

    mesh::uuid m_id_;

    Attribute(mesh::MeshAtlas const &mesh_tree, std::string const &name)
            : m_mesh_tree_(mesh_tree)
    {
    }

    virtual void view(mesh::uuid const &id)
    {
        std::assert(m_data_tree_.find(id) != m_data_tree_.end());
        std::assert(m_data_tree_.find(id) != m_data_tree_.end());
        m_id_ = id;

    };

    void *data() { return m_data_tree_.at(m_id_).get(); }

    void const *data() const { return m_data_tree_.at(m_id_).get(); }

    mesh_type const *mesh() const { return m_mesh_tree_.at(m_id_); }


};

template<typename TManifold>
struct Context<TManifold>::Worker
{
    typedef Context<TManifold> context_type;

    std::list<std::string> m_attributes_;

    context_type &m_ctx_;

    Worker(context_type &ctx) : m_ctx_(ctx) { }

    virtual ~Worker() { }

    void view(mesh::uuid const &id)
    {
        for (auto &item:m_attributes_)
        {
            m_ctx_.m_attributes_.at(item).lock()->view(id);
        }
    }

    virtual void work(Real dt) = 0;

    /**
    * copy data from lower level
    */
    virtual void coarsen(mesh::uuid const &) { }

    /**
     * copy data to lower level
     */
    virtual void refine(mesh::uuid const &) { }

    /**
     * copy data from same level neighbour
     */
    virtual void sync(std::list<mesh::uuid> const &) { }
};


class EM : public Context::Worker
{

    typedef Context context_type;

    EM(context_type &ctx) { }

    ~EM() { }

    typedef Real scalar_type;
    typedef nTuple<scalar_type, 3> vector_type;


    context_type &m_ctx_;

    context_type::field_t <scalar_type, mesh::EDGE> E{m_ctx_, "E0"};
    context_type::field_t <scalar_type, mesh::FACE> B{m_ctx_, "B0"};
    context_type::field_t <vector_type, mesh::VERTEX> Bv{m_ctx_, "B0v"};
    context_type::field_t <scalar_type, mesh::VERTEX> BB{m_ctx_, "BB"};


    void work(Real dt)
    {
        E += curl(B) * dt;
        B -= curl(E) * dt;

        sync(E);
        sync(B);
    };
};


}//namespace task_flow
}// namespace simpla

#endif /* CORE_APPLICATION_CONTEXT_H_ */
