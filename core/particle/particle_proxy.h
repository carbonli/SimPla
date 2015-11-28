/**
 * @file particle_proxy.h
 * @author salmon
 * @date 2015-11-26.
 */

#ifndef SIMPLA_PARTICLE_PROXY_H
#define SIMPLA_PARTICLE_PROXY_H

#include "../dataset/dataset.h"

namespace simpla
{
template<typename...> struct Particle;
template<typename...> struct ParticleProxyBase;
template<typename...> struct ParticleProxy;

template<typename TE, typename TB, typename TJ, typename TRho>
struct ParticleProxyBase<TE, TB, TJ, TRho>
{
    ParticleProxyBase()
    {
    }

    virtual ~ParticleProxyBase()
    {
    }

    virtual void deploy() = 0;

    virtual void rehash() = 0;

    virtual void sync() = 0;

    virtual DataSet dataset() = 0;

    virtual void dataset(DataSet const &) = 0;

    virtual void push(Real dt, Real t, TE const &E, TB const &B) = 0;

    virtual void integral(TJ *J) = 0;

};

template<typename ...T, typename TE, typename TB, typename TJ, typename TRho>
struct ParticleProxy<Particle<T...>, TE, TB, TJ, TRho> : public ParticleProxyBase<TE, TB, TJ, TRho>
{
    typedef Particle<T...> particle_type;

    std::shared_ptr<particle_type> m_self_;

    template<typename ...Args>
    ParticleProxy(Args &&... args) : m_self_(std::make_shared<particle_type>(std::forward<Args>(args)...))
    {
    }

    ParticleProxy(particle_type &other) : m_self_(other.shared_from_this())
    {
    }

    virtual  ~ParticleProxy()
    {
    }

    virtual void deploy() { m_self_->deploy(); }

    virtual void rehash() { m_self_->rehash(); }

    virtual void sync() { m_self_->sync(); }

    virtual DataSet dataset() const { return m_self_->dataset(); }

    virtual void dataset(DataSet const &ds) { m_self_->dataset(ds); }

    virtual void push(Real dt, Real t, TE const &E, TB const &B)
    {
        m_self_->push(dt, t, E, B);
    };

    virtual void integral(TJ *J) const
    {
        m_self_->integral(J);
    };
};

}//namespace simpla
#endif //SIMPLA_PARTICLE_PROXY_H