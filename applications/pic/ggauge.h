/*
 * ggauge.h
 *
 *  Created on: 2013年10月23日
 *      Author: salmon
 */

#ifndef GGAUGE_H_
#define GGAUGE_H_

#include <fetl/primitives.h>
#include <numeric/multi_normal_distribution.h>
#include <numeric/normal_distribution_icdf.h>
#include <numeric/rectangle_distribution.h>
#include <particle/particle.h>

namespace simpla
{

//template<int NMATE, typename TM>
//struct GGauge_s
//{
//	struct Point_s
//	{
//		typename TM::coordinates_type x;
//		Vec3 v;
//		Real f;
//		Real w[NMATE];
//	}
//	;
//
//};
//
///**
// *
// * @ingroup Particle Generator
// * @ingroup GGauge
// *
// * */
//template<int NMATE>
//class GGauge_Generator
//{
//public:
//
//	typedef GGauge_s<NMATE> value_type;
//	typedef rectangle_distribution<3> x_dist_type;
//	typedef multi_normal_distribution<3, normal_distribution_icdf> v_dist_type;
//
//	GGauge_Generator(x_dist_type const& x_dist, v_dist_type const& v_dist) :
//			x_dist_(x_dist), v_dist_(v_dist)
//	{
//	}
//	~GGauge_Generator()
//	{
//	}
//
//private:
//	x_dist_type x_dist_;
//	v_dist_type v_dist_;
//};
//
//template<typename IT, typename XDIST, typename YDIST, typename Generator>
//void GGauge_Generator(XDIST const xdsit, YDIST const ydist, Generator const & g,
//		IT const & p)
//{
//	p->x = x_dist(g)
//	p->v = v_dist(g);
//	p->f = 1.0;
//	std::fill(p->w, p->w + sizeof(p->w) / sizeof(decltype(p->w[0])), 0);
//}
//
//template<typename NMATE, typename TG>
//void InitLoad(Particle<GGauge_s<NMATE>, TG> & p, size_t pic)
//{
//	typename TG::Mesh &mesh = *p.mesh;
//	s
//
//}

} // namespace simpla

#endif /* GGAUGE_H_ */
