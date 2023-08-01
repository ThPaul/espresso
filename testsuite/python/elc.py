#
# Copyright (C) 2010-2022 The ESPResSo project
#
# This file is part of ESPResSo.
#
# ESPResSo is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ESPResSo is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import unittest as ut
import unittest_decorators as utx
import espressomd.electrostatics

import numpy as np

GAP = np.array([0., 0., 3.])
BOX_L = np.array(3 * [10.]) + GAP
TIME_STEP = 1e-100
POTENTIAL_DIFFERENCE = -3.


class ElcTest:
    system = espressomd.System(box_l=BOX_L, time_step=TIME_STEP)
    system.cell_system.skin = 0.0

    def tearDown(self):
        self.system.part.clear()
        self.system.electrostatics.clear()

    def test_finite_potential_drop(self):
        system = self.system

        p1 = system.part.add(pos=[0, 0, 1], q=+1)
        p2 = system.part.add(pos=[0, 0, 9], q=-1)

        p3m = self.p3m_class(
            # zero is not allowed
            prefactor=1e-100,
            mesh=32,
            cao=5,
            accuracy=1e-3,
        )
        elc = espressomd.electrostatics.ELC(
            actor=p3m,
            gap_size=GAP[2],
            maxPWerror=1e-3,
            delta_mid_top=-1,
            delta_mid_bot=-1,
            const_pot=True,
            pot_diff=POTENTIAL_DIFFERENCE,
        )

        system.electrostatics.solver = elc

        # Calculated energy
        U_elc = system.analysis.energy()['coulomb']

        # Expected E-Field is voltage drop over the box
        E_expected = POTENTIAL_DIFFERENCE / (BOX_L[2] - GAP[2])
        # Expected potential is -E_expected * z, so
        U_expected = -E_expected * (p1.pos[2] * p1.q + p2.pos[2] * p2.q)

        system.integrator.run(0)

        self.assertAlmostEqual(U_elc, U_expected)
        self.assertAlmostEqual(p1.f[2] / p1.q, E_expected)
        self.assertAlmostEqual(p2.f[2] / p2.q, E_expected)

        # Check if error is thrown when particles enter the ELC gap
        # positive direction
        p1.pos = [BOX_L[0] / 2, BOX_L[1] / 2, BOX_L[2] - GAP[2] / 2]
        with self.assertRaises(Exception):
            self.system.analysis.energy()
        with self.assertRaisesRegex(Exception, 'entered ELC gap region'):
            self.system.integrator.run(2)
        # negative direction
        p1.pos = [BOX_L[0] / 2, BOX_L[1] / 2, -GAP[2] / 2]
        with self.assertRaises(Exception):
            self.system.analysis.energy()
        with self.assertRaisesRegex(Exception, 'entered ELC gap region'):
            self.system.integrator.run(2)


@utx.skipIfMissingFeatures(["P3M"])
class ElcTestCPU(ElcTest, ut.TestCase):

    p3m_class = espressomd.electrostatics.P3M
    rtol = 1e-7


@utx.skipIfMissingGPU()
@utx.skipIfMissingFeatures(["P3M"])
class ElcTestGPU(ElcTest, ut.TestCase):

    p3m_class = espressomd.electrostatics.P3MGPU
    rtol = 4e-6


if __name__ == "__main__":
    ut.main()
