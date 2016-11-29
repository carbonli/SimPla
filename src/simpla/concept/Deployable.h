//
// Created by salmon on 16-11-29.
//

#ifndef SIMPLA_DEPLOYABLE_H
#define SIMPLA_DEPLOYABLE_H

#include <simpla/toolbox/Log.h>

namespace simpla { namespace concept
{
struct Deployable
{

    Deployable() : m_is_deployed_(false), m_is_valid_(false) {}

    virtual ~Deployable() { destroy(); }

    virtual bool is_deployed() const { return m_is_deployed_; }

    virtual void deploy()
    {
        if (is_deployed()) { RUNTIME_ERROR << "Repeat deploying!" << std::endl; }
        m_is_deployed_ = true;
    };

    virtual void destroy() { m_is_deployed_ = false; };

    virtual bool is_valid() const { return m_is_valid_; }

    virtual void preprocess() { m_is_valid_ = true; /*add sth here*/}

    virtual void postprocess() { /*add sth here*/ m_is_valid_ = false; }

    virtual void initialize(Real data_time = 0) { preprocess(); }

    virtual void finalize(Real data_time = 0) { postprocess(); }


private:
    bool m_is_deployed_ = false;
    bool m_is_valid_ = false;

};
}}//namespace simpla{namespace concept{
#endif //SIMPLA_DEPLOYABLE_H
