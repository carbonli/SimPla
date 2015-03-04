/**
 * @file kinetic_particle_test.h
 *
 *  created on: 2014-6-30
 *      Author: salmon
 */

#ifndef KINETIC_PARTICLE_TEST_H_
#define KINETIC_PARTICLE_TEST_H_

#include <gtest/gtest.h>
#include "../kinetic_particle.h"
#include "../particle_engine.h"
#include "../../mesh/simple_mesh.h"
#include "../../numeric/rectangle_distribution.h"

using namespace simpla;

class TestKineticParticle: public testing::TestWithParam<SimpleMesh>
{
protected:
	virtual void SetUp()
	{
		LOGGER.set_stdout_visable_level(LOG_DEBUG);
	}
public:
	typedef SimpleMesh mesh_type;
	typedef SimpleParticleEngine engine_type;
	mesh_type m_mesh;

	typedef KineticParticle<mesh_type, engine_type> particle_type;

	mesh_type mesh;

	nTuple<Real, 3> xmin, xmax;

	nTuple<size_t, 3> dims;

};

TEST_P(TestKineticParticle,Add)
{

	particle_type p(mesh);

	auto extents = mesh.extents();

	rectangle_distribution<mesh_type::m_ndims_> x_dist(extents.first,
			extents.second);

	std::mt19937 rnd_gen(mesh_type::m_ndims_);

	nTuple<Real, 3> v = { 0, 0, 0 };

	int pic = 100;

	for (auto s : mesh.range())
	{
		for (int i = 0; i < pic; ++i)
		{
			p.emplace(x_dist(rnd_gen), v, 1.0);
		}
	}

//	INFORM << "Add particle DONE " << p.size() << std::endl;
//
//	EXPECT_EQ(p.size(), mesh.get_local_memory_size(VERTEX) * pic);

	sync(&p);

//	VERBOSE << "update_ghosts particle DONE " << p.size() << std::endl;

}

TEST_P(TestKineticParticle, scatter_n)
{

	Field<mesh_type, Real> n(mesh), n0(mesh);

	particle_type ion(mesh);

	Field<mesh_type, Real> E(mesh);
	Field<mesh_type, Real> B(mesh);

	E.Clear();
	B.Clear();
	n0.Clear();
	n.Clear();

	scatter(ion, &n, E, B);

	Real q = ion.q;
	Real variance = 0.0;

	Real average = 0.0;

	for (auto s : mesh.range())
	{
		coordinates_type x = mesh.id_to_coordinates(s);

		Real expect = q * n(x[0], x[1], x[2]).template as<Real>();

		n0[s] = expect;

		Real actual = n.get(s);

		average += abs(actual);

		variance += std::pow(abs(expect - actual), 2.0);
	}

//	if (std::is_same<engine_type, PICEngineFullF<mesh_type> >::value)
//	{
//		Real relative_error = std::sqrt(variance) / abs(average);
//		CHECK(relative_error);
//		EXPECT_LE(relative_error, 1.0 / std::sqrt(pic));
//	}
//	else
	{
		Real error = 1.0 / std::sqrt(static_cast<double>(ion.size()));

		EXPECT_LE(abs(average), error);
	}

}

//TYPED_TEST_P(TestParticle,move){
//{
//	GLOBAL_DATA_STREAM.cd("ParticleTest.h5:/");
//	typedef mesh_type mesh_type;
//
//	typedef particle_pool_type pool_type;
//
//	typedef Point_s Point_s;
//
//	typedef iterator iterator;
//
//	typedef coordinates_type coordinates_type;
//
//	typedef scalar_type scalar_type;
//
//	mesh_type const & mesh = mesh;
//
//	LuaObject cfg;
//	cfg.ParseString(cfg_str);
//
//	Field<mesh_type,VERTEX,scalar_type> n0(mesh);
//
//	pool_type ion(mesh,cfg["ion"]);
//	ion.SetParticleSorting(enable_sorting);
//	Field<mesh_type,EDGE,Real> E(mesh);
//	Field<mesh_type,FACE,Real> B(mesh);
//
//	Field<mesh_type,EDGE,scalar_type> J0(mesh);
//
//	n0.Clear();
//	J0.Clear();
//	E.Clear();
//	B.Clear();
//
//	constexpr Real PI=3.141592653589793;
//
//	nTuple<3,Real> E0=
//	{	1.0e-4,1.0e-4,1.0e-4};
//	nTuple<3,Real> Bv=
//	{	0,0,1.0};
//	nTuple<3,Real> k=
//	{	2.0*PI,4.0*PI,2.0*PI};
//
//	Real q=ion.get_charge();
//
//	auto n0_cfg= cfg["ion"]["Density"];
//
//	Real pic =cfg["ion"]["PIC"].template as<Real>();
//
//	for(auto s:mesh.select(VERTEX))
//	{
//		auto x =mesh.get_coordinates(s);
//		n0[s]=q* n0_cfg(x[0],x[1],x[2]).template as<Real>();
//	}
//
//	for (auto s : mesh.select(EDGE))
//	{
//		auto x=mesh.get_coordinates(s);
//
//		nTuple<3,Real> Ev;
//
//		Ev=E0*std::sin(Dot(k,mesh.get_coordinates(s)));
//
//		E[s]=mesh.Sample(std::integral_constant<unsigned int ,EDGE>(),s,Ev);
//	}
//
//	for (auto s : mesh.select(FACE))
//	{
//		B[s]= mesh.Sample(std::integral_constant<unsigned int ,FACE>(),s,Bv);
//	}
//
//	Real dt=1.0e-12;
//	Real a=0.5*(dt*q/ion.get_mass());
//
//	J0=2*n0*a*(E+a* Cross(E,B)+a*a* Dot(E,B)*B)/(1.0+Dot(Bv,Bv)*a*a);
//
//	LOG_CMD(ion.next_timestep(dt,E, B));
//
//	LOGGER<<SAVE1(E);
//	LOGGER<<SAVE1(B);
//	LOGGER<<SAVE1(n0 );
//	LOGGER<<SAVE1(J0 );
//	LOGGER<<SAVE1(ion.J);
//	LOGGER<<SAVE1(ion.n);
//	Real variance=0.0;
//
//	Real average=0.0;
//
//	for(auto s:mesh.select(VERTEX))
//	{
//		auto expect=J0[s];
//
//		auto actual=ion.J[s];
//
//		average+=abs(expect);
//
//		variance+=std::pow(abs (expect-actual),2.0);
//	}
//
//	{
//		Real relative_error=std::sqrt(variance)/abs(average);
//
//		CHECK(relative_error);
//		EXPECT_LE(relative_error,1.0/std::sqrt(pic))<<mesh.get_dimensions();
//	}
//
//}
//}

#endif /* KINETIC_PARTICLE_TEST_H_ */
