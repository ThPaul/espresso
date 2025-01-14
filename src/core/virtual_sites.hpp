/*
 * Copyright (C) 2010-2022 The ESPResSo project
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
#ifndef VIRTUAL_SITES_HPP
#define VIRTUAL_SITES_HPP

#include "config/config.hpp"

#ifdef VIRTUAL_SITES
#include "Particle.hpp"

#ifdef VIRTUAL_SITES_RELATIVE

#include <utils/quaternion.hpp>

#include <tuple>

std::tuple<Utils::Quaternion<double>, double>
calculate_vs_relate_to_params(Particle const &p_current,
                              Particle const &p_relate_to,
                              bool override_cutoff_check = false);

/**
 * @brief Setup a virtual site to track a real particle.
 * @param[in,out] p_vs         Virtual site.
 * @param[in]     p_relate_to  Real particle to follow.
 */
inline void vs_relate_to(Particle &p_vs, Particle const &p_relate_to) {
  // Set the particle id of the particle we want to relate to, the distance
  // and the relative orientation
  auto &vs_relative = p_vs.vs_relative();
  p_vs.propagation() = p_vs.propagation() | PropagationMode::TRANS_VS_RELATIVE;
  vs_relative.to_particle_id = p_relate_to.id();
  std::tie(vs_relative.rel_orientation, vs_relative.distance) =
      calculate_vs_relate_to_params(p_vs, p_relate_to);
}

#endif // VIRTUAL_SITES_RELATIVE
#endif // VIRTUAL_SITES
#endif
