/*
 * Copyright (C) 2010-2022 The ESPResSo project
 * Copyright (C) 2002,2003,2004,2005,2006,2007,2008,2009,2010
 *   Max-Planck-Institute for Polymer Research, Theory Group
 *
 * This file is part of ESPResSo.
 *
 * ESPResSo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ESPResSo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CORE_NB_IA_THOLE_HPP
#define CORE_NB_IA_THOLE_HPP
/** \file
 *  Routines to calculate the Thole damping potential between particle pairs.
 *  See @cite thole81a.
 */

#include "config/config.hpp"

#ifdef THOLE
#include "Particle.hpp"
#include "bonded_interactions/bonded_interaction_data.hpp"
#include "bonded_interactions/bonded_interaction_utils.hpp"
#include "electrostatics/coulomb.hpp"
#include "electrostatics/coulomb_inline.hpp"
#include "nonbonded_interactions/nonbonded_interaction_data.hpp"

#include <utils/Vector.hpp>

#include <cmath>

/** Calculate Thole force */
inline Utils::Vector3d
thole_pair_force(Particle const &p1, Particle const &p2,
                 IA_parameters const &ia_params, Utils::Vector3d const &d,
                 double dist,
                 Coulomb::ShortRangeForceKernel::kernel_type const *kernel) {
  auto const thole_q1q2 = ia_params.thole.q1q2;
  auto const thole_s = ia_params.thole.scaling_coeff;

  if (thole_s != 0. and thole_q1q2 != 0. and kernel != nullptr and
      !(pair_bond_enum_exists_between<ThermalizedBond>(p1, p2))) {
    // Calc damping function (see @cite thole81a)
    // S(r) = 1.0 - (1.0 + thole_s*r/2.0) * exp(-thole_s*r);
    // Calc F = - d/dr ( S(r)*q1q2/r) =
    // -(1/2)*(-2+(r^2*s^2+2*r*s+2)*exp(-s*r))*q1q2/r^2 Everything before
    // q1q2/r^2 can be used as a factor for the Coulomb::central_force method
    auto const sr = thole_s * dist;
    auto const dS_r = 0.5 * (2.0 - (exp(-sr) * (sr * (sr + 2.0) + 2.0)));
    return (*kernel)(thole_q1q2 * (-1. + dS_r), d, dist);
  }
  return {};
}

/** Calculate Thole energy */
inline double
thole_pair_energy(Particle const &p1, Particle const &p2,
                  IA_parameters const &ia_params, Utils::Vector3d const &d,
                  double dist,
                  Coulomb::ShortRangeEnergyKernel::kernel_type const *kernel) {

  auto const thole_s = ia_params.thole.scaling_coeff;
  auto const thole_q1q2 = ia_params.thole.q1q2;

  if (thole_s != 0. and thole_q1q2 != 0. and kernel != nullptr and
      dist < Coulomb::get_coulomb().cutoff() and
      !(pair_bond_enum_exists_between<ThermalizedBond>(p1, p2))) {
    auto const sd = thole_s * dist;
    auto const S_r = 1.0 - (1.0 + sd / 2.0) * exp(-sd);
    // Subtract p3m short-range energy and add thole energy
    return (*kernel)(p1, p2, thole_q1q2 * (-1.0 + S_r), d, dist);
  }
  return 0.0;
}
#endif // THOLE
#endif
