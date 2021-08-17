# Copyright (C) 2013-2019 The ESPResSo project
# Copyright (C) 2012 Olaf Lenz
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
# This script generates the files featureconfig.h and featureconfig.c.
#
import time
import string
import inspect
import sys
import os
# find featuredefs.py
moduledir = os.path.dirname(inspect.getfile(inspect.currentframe()))
sys.path.append(os.path.join(moduledir, '..'))
import featuredefs

if len(sys.argv) != 4:
    print(f"Usage: {sys.argv[0]} DEFFILE HPPFILE CPPFILE", file=sys.stderr)
    exit(2)

deffilename, hfilename, cfilename = sys.argv[1:5]

print("Reading definitions from " + deffilename + "...")
defs = featuredefs.defs(deffilename)
print("Done.")

print("Writing " + hfilename + "...")
hfile = open(hfilename, 'w')

hfile.write("""/*
WARNING: This file was autogenerated by

   %s on %s

   Do not modify it or your changes will be overwritten!
   Modify features.def instead.
*/
#ifndef ESPRESSO_FEATURECONFIG_HPP
#define ESPRESSO_FEATURECONFIG_HPP

#include <cmake_config.hpp>
#include "myconfig-final.hpp"

""" % (sys.argv[0], time.asctime()))

# external features can only be set by the build
# system, so in case the user has defined some of
# them, we undef all external features and include
# the config from the build system again, to make
# sure only the detected ones are set.
hfile.write('/* Guards for externals */')
external_template = string.Template("""
// $feature is external
#if defined($feature)
#undef $feature
#endif
""")
for feature in defs.externals:
    hfile.write(external_template.substitute(feature=feature))

# Include definitions from CMake
hfile.write("""
/* Definitions from CMake */
#include <cmake_config.hpp>

""")

# handle implications
hfile.write('/* Handle implications */')
implication_template = string.Template("""
// $feature implies $implied
#if defined($feature) && !defined($implied)
#define $implied
#endif
""")
for feature, implied in defs.implications:
    hfile.write(implication_template.substitute(
        feature=feature, implied=implied))

# output warnings if internal features are set manually
hfile.write('/* Warn when derived switches are specified manually */')
derivation_template = string.Template("""
// $feature equals $expr
#ifdef $feature
#warning $feature is a derived switch and should not be set manually!
#elif $cppexpr
#define $feature
#endif
""")
for feature, expr, cppexpr in defs.derivations:
    hfile.write(derivation_template.substitute(
        feature=feature, cppexpr=cppexpr, expr=expr))

# write footer
# define external FEATURES and NUM_FEATURES
hfile.write("""
extern const char* FEATURES[];
extern const int NUM_FEATURES;

#endif /* of _FEATURECONFIG_HPP */""")
hfile.close()
print("Done.")

print("Writing " + cfilename + "...")
cfile = open(cfilename, 'w')

# handle requirements

cfile.write(f"""/*
WARNING: This file was autogenerated by

   {sys.argv[0]}
   on
   {time.asctime()}

   Do not modify it or your changes will be overwritten!
   Modify features.def instead.
*/

/* config.hpp includes config-features.hpp and myconfig.hpp */
#include "config.hpp"

""")

cfile.write('/* Handle requirements */')

requirement_string = """
// {feature} requires {expr}
#if defined({feature}) && !({cppexpr})
#error Feature {feature} requires {expr}
#endif
"""
for feature, expr, cppexpr in defs.requirements:
    cfile.write(
        requirement_string.format(
            feature=feature, cppexpr=cppexpr, expr=expr))

cfile.write("""

/* Feature list */
const char* FEATURES[] = {
""")

feature_string = """
#ifdef {feature}
  "{feature}",
#endif
"""

for feature in defs.externals.union(defs.features, defs.derived):
    cfile.write(feature_string.format(feature=feature))

cfile.write("""
};

const int NUM_FEATURES = sizeof(FEATURES)/sizeof(char*);
""")

cfile.close()
print("Done.")
