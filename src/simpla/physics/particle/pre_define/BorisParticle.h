/**
 * @file PICBoris.h
 * @author salmon
 * @date 2015-11-29.
 */

#ifndef SIMPLA_PIC_BORIS_H
#define SIMPLA_PIC_BORIS_H

namespace simpla {
namespace particle {

struct BorisYeeCXXWrap {
    SP_DEFINE_PARTICLE(point_s, Vec3, x, Vec3, v, Real, f, Real, w);

    virtual Properties &properties() = 0;

    virtual Properties const &properties() const = 0;

    DEFINE_PROPERTIES(Real, mass);

    DEFINE_PROPERTIES(Real, charge);

    DEFINE_PROPERTIES(Real, temperature);

    void deploy() {
        m_mass_ = properties()["mass"].template as<Real>(1.0);
        m_charge_ = properties()["charge"].template as<Real>(1.0);
        m_temperature_ = properties()["temperature"].template as<Real>(1.0);
    }

    point_type project(point_s const &z) const { return z.x; }

    template <typename TE, typename TB>
    void push(point_s *p0, Real dt, TE const &E1, TB const &B0) const {
        auto E = E1(project(*p0));

        auto B = B0(project(*p0));

        Real cmr = m_charge_ / m_mass_;

        p0->x += p0->v * dt * 0.5;

        Vec3 v_, t;

        t = B * (cmr * dt * 0.5);

        p0->v += E * (cmr * dt * 0.5);

        v_ = p0->v + cross(p0->v, t);

        p0->v += cross(v_, t * 2.0) / (inner_product(t, t) + 1.0);

        p0->v += E * (cmr * dt * 0.5);

        p0->x += p0->v * dt * 0.5;
    }
};
}
}  // namespace simpla { namespace particle

#endif  // SIMPLA_PIC_BORIS_H
