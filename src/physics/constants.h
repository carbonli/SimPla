/*
 * constant.h
 *
 *  Created on: 2012-3-5
 *      Author: salmon
 */

#ifndef CONSTANT_H_
#define CONSTANT_H_

static const double PI = 3.1415926535897932384626433;

static const double HALFPI = 0.50 * PI;

static const double ONEHALFPI = 1.50 * PI;

static const double TWOPI = 2.0 * PI;

/* Fundamental Physical Constants — Frequently used constants */
/* Ref: http://physics.nist.gov/cuu/Constants/ */
static const double SI_speed_of_light = 299792458.0;

static const double SI_permeability_of_free_space = 4.0e-7 * PI;

static const double SI_permittivity_of_free_space =
		1.0/ (SI_speed_of_light * SI_speed_of_light	* SI_permeability_of_free_space);

static const double SI_gravitational_constant = 6.67384e-11; /*1.2e-4*/

static const double SI_plank_constant = 6.62606957e-34;

static const double SI_plank_constant_bar = 1.054571726e-34;

static const double SI_elementary_charge = 1.60217656e-19; /*2.2e-8*/

static const double SI_electron_mass = 9.10938291e-31; /*4.4e-8*/

static const double SI_proton_mass = 1.672621777e-27;

static const double SI_proton_electron_mass_ratio = 1836.15267245;

static const double SI_electron_charge_mass_ratio = 1.7588e11;

static const double SI_fine_structure_constant = 7.2973525698e-3; /*3.23-10*/

static const double SI_Rydberg_constant = 10973731.568539; /*5e-12*/

static const double SI_Avogadro_constant = 6.02214129e23; /*4.4e-8*/

static const double SI_Faraday_constant = 96485.3365; /*2.2e-10*/

static const double SI_Boltzmann_constant = 1.3806488e-23; /*9.1e-7*/

static const double SI_electron_volt = 1.602176565e-19; /*2.2e-8*/

static const double SI_atomic_mass_unit = 1.660538921e-27; /*4.4e-8*/

#endif /* CONSTANT_H_ */
