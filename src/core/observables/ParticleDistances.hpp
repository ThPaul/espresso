/*
 * Copyright (C) 2019-2022 The ESPResSo project
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
#ifndef OBSERVABLES_PARTICLEDISTANCES_HPP
#define OBSERVABLES_PARTICLEDISTANCES_HPP

#include "BoxGeometry.hpp"
#include "PidObservable.hpp"
#include "grid.hpp"

#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

namespace Observables {

/** Calculate distances between particles in a polymer.
 *  For @f$ n @f$ bonded particles, return the @f$ n-1 @f$ distances separating
 *  them.
 */
class ParticleDistances : public PidObservable {
public:
  using PidObservable::PidObservable;
  explicit ParticleDistances(std::vector<int> ids)
      : PidObservable(std::move(ids)) {
    if (this->ids().size() < 2)
      throw std::runtime_error("At least 2 particles are required");
  }

  std::vector<double>
  evaluate(boost::mpi::communicator const &comm,
           ParticleReferenceRange const &local_particles,
           const ParticleObservables::traits<Particle> &traits) const override {
    auto const positions_sorted = detail::get_all_particle_positions(
        comm, local_particles, ids(), traits, false);

    if (comm.rank() != 0) {
      return {};
    }

    std::vector<double> res(n_values());

    for (std::size_t i = 0, end = n_values(); i < end; i++) {
      auto const v =
          box_geo.get_mi_vector(positions_sorted[i], positions_sorted[i + 1]);
      res[i] = v.norm();
    }
    return res;
  }
  std::vector<std::size_t> shape() const override {
    assert(!ids().empty());
    return {ids().size() - 1};
  }
};

} // namespace Observables

#endif
