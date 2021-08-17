# Copyright (C) 2010-2019 The ESPResSo project
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
"""Code shared by charge and dipole methods based on the SCAFACOS library."""


from .actors cimport Actor
from libcpp.string cimport string  # import std::string
from . cimport electrostatics
from . cimport magnetostatics
from .utils import to_char_pointer, to_str, handle_errors


include "myconfig.pxi"

# Interface to the scafacos library. These are the methods shared between
# dipolar and electrostatics methods

IF SCAFACOS == 1:
    class ScafacosConnector(Actor):

        """Scafacos interface class shared by charge and dipole methods. Do not use directly.

        """

        def get_params(self):
            return self._get_params_from_es_core()

        def set_params(self, params):
            self._params = params
            self._set_params_in_es_core()

        def valid_keys(self):
            tmp = self.required_keys().copy()
            if not self.dipolar:
                tmp.add("check_neutrality")
            return tmp

        def required_keys(self):
            return {"method_name", "method_params", "prefactor"}

        def validate_params(self):
            if self._params["method_name"] not in available_methods():
                raise ValueError(
                    f"method '{self._params['method_name']}' is unknown or not compiled in ScaFaCoS")
            if not self._params["method_params"]:
                raise ValueError(
                    "ScaFaCoS methods require at least 1 parameter")

        def _get_params_from_es_core(self):
            # Parameters are returned as strings
            # First word is method name, rest are key value pairs
            # which we convert to a dict
            p = to_str(get_method_and_parameters(self.dipolar)).split(" ")
            res = {}
            res["method_name"] = p.pop(0)

            method_params = {}
            while p:
                pname = p.pop(0)

                # The first item after the parameter name is always a value
                # But in the case of array-like properties, there can also
                # follow several values. Therefore, we treat the next
                # words as part of the value, if they begin with a digit
                pvalues = [p.pop(0)]
                while p:
                    if p[0][0] in "-0123456789":
                        pvalues.append(p.pop(0))
                    else:
                        break

                # If there is one value, cast away the list
                if len(pvalues) == 1:
                    pvalues = pvalues[0]
                else:
                    # Cast array elements to strings and join them by commas
                    # to achieve consistency with setting array-likes
                    # such as "pnfft_n":"128,128,128"
                    pvalues = ",".join(pvalues)
                method_params[pname] = pvalues

            res["method_params"] = method_params

            # Re-add the prefactor to the parameter set
            if self.dipolar:
                IF DIPOLES == 1:
                    res["prefactor"] = magnetostatics.dipole.prefactor
                pass
            else:
                IF ELECTROSTATICS == 1:
                    res["prefactor"] = electrostatics.coulomb.prefactor
                pass
            return res

        def _set_params_in_es_core(self):
            # Convert the parameter dictionary to a list of strings
            method_params = self._params["method_params"]
            param_string = ""
            for k in method_params:
                param_string += f"{k} {method_params[k]} "
            # Format list as a single string
            param_string = param_string.rstrip().replace(" ", ",")

            set_parameters(to_char_pointer(self._params["method_name"]),
                           to_char_pointer(param_string), self.dipolar)
            handle_errors("Scafacos not initialized.")

        def default_params(self):
            if not self.dipolar:
                return {"check_neutrality": True}
            return {}

    def available_methods():
        """Lists long range methods available in the Scafacos library.

        """
        methods = available_methods_core()
        res = []
        for m in methods:
            res.append(to_str(m))
        return res
