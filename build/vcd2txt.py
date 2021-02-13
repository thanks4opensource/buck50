#!/usr/bin/env python3

# buck50: Test and measurement firmware for “Blue Pill” STM32F103 development board
# Copyright (C) 2019, 2020, 2021 Mark R. Rubin aka "thanks4opensource"
#
# This file is part of buck50.
#
# The buck50 program is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# The buck50 program is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# (LICENSE.txt) along with the buck50 program.  If not, see
# <https:#www.gnu.org/licenses/gpl.html>



import argparse
import sys



def parse_commandline():
    parser = argparse.ArgumentParser(
                formatter_class=argparse.ArgumentDefaultsHelpFormatter)


    def units(text):
        VALIDS = {'ns' : 1e9,
                  'us' : 1e6,
                  'ms' : 1e3,
                  's'  : 1.0 ,
                 }
        if not text in VALIDS:
            raise argparse.ArgumentTypeError(  "units must be one of {}"
                                             .format(VALID_SCALES)      )
        return (text, VALIDS[text])

    parser.add_argument('-u', '--units',
                        nargs='?',
                        type=units,
                        default='us')

    parser.add_argument('infile',
                        nargs='?',
                        type=argparse.FileType('r'),
                        default=sys.stdin)

    parser.add_argument('outfile',
                        nargs='?',
                        type=argparse.FileType('w'),
                        default=sys.stdout)

    return parser.parse_args()



if __name__ != '__main__':
    sys.exit(1)


UNITS = {'ns' : 1e-9,
         'us' : 1e-6,
         'ms' : 1e-3,
         's'  : 1.0 ,
        }

args = parse_commandline()

traces = {}
tick   = None
for line in args.infile:
    if line.startswith('$timescale'):
        fields = line.split()
        tick = float(fields[1]) * UNITS[fields[2].lower()]

    if line.startswith('$var'):
        fields = line.split()
        traces[fields[3]] = fields[4]

    if line.startswith('$enddefinitions'):
        break

if tick is None:
    sys.stderr.write("No $timescale in input VCD file\n")
    sys.exit(1)

for line in args.infile:
    if line[0] == '#':
        args.outfile.write(  '%.3f %s\n'
                           % (int(line[1:]) * tick * args.units[1],
                                                     args.units[0]))
    else:
        args.outfile.write("  %s  %s\n" % (line[0], traces[line[1]]))
