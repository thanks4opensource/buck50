#!/usr/bin/env python3

# buck50: Test and measurement firmware for “Blue Pill” STM32F103 development board
# Copyright (C) 2019,2020 Mark R. Rubin aka "thanks4opensource"
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


#!/usr/bin/env python3

import argparse
import collections
import copy
import fcntl
import functools
import math
import os
import re
import random
import readline
import select
import shlex
import shutil
import socket
import string
import struct
import subprocess
import sys
import termios
import time
import tty


### copyright and version
#
#

VERSION = (0, 9, 5)

COPYRIGHT = '''%s %d.%d.%d
Copyright 2020 Mark R. Rubin aka "thanks4opensource"''' \
% (os.path.basename(sys.argv[0]), *VERSION)

BANNER  = COPYRIGHT + """
This is free software with ABSOLUTELY NO WARRANTY.
For details type "warranty" or "help warranty".
Type "using" for program usage.
Type "help" for commands, configurations, parameters, values.
"""

WARRANTY="""
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License , or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, write to

       The Free Software Foundation, Inc.
       51 Franklin Street, Fifth Floor
       Boston, MA 02110-1335  USA

"""




### preliminary constants (must precede utils, actions, etc)
#
#

CPU_MHZ     = 72
CPU_HZ      = float(CPU_MHZ) * 1e6

# python select() time type limit (C _PyTime_t):
#   (1<<63)/1e9 == 9223372035.854776 seconds == 292.47120864582627 years
#   72e6 * (1<<63)/1e9 = 6.640827866535438e+17
#   int(72e6 * (1<<63)/1e9) = 664082786653543808
#   hex(int(72e6 * (1<<63)/1e9)) = 0x9374bc6a7ef9d80
#                                  0x7ffffffffffffff (largest mask)
MAX_DURATION = 0x7ffffffffffffff
MAX_PYTIME   = (1 << 63) / 1e9




### preliminary utils
# (used by classes and helpers, must precede both for module-level
#  init of objects)
#
#

def quoted_sequence(sequence):
    return ['"%s"' % element for element in sequence]


def comma_or_concat(sequence, conjunction='or'):
    if len(sequence) == 2:
        return (" %s " % conjunction).join(quoted_sequence(sequence))
    elif len(sequence) > 2:
        return "%s, %s \"%s\"" % (", ".join(quoted_sequence(sequence[:-1])),
                                  conjunction                              ,
                                  sequence[-1]                             )
    elif len(sequence) == 1:
        return sequence[0]
    else:
        return ''




### action utils (must precede configs/actions below)
#
#

# reset_config actions
#

def adjust_cpu_hz():
    global CPU_HZ
    CPU_HZ = float(CPU_MHZ) * 1e6 * configure_config['trim'].val


def config_set(configuration, parameter, value):
    configuration[parameter].val = value


def usb_connect():
    global dev_acm, usb_fd
    if usb_fd != -1:
        os.close(usb_fd)
    try:
        usb_fd = os.open(dev_acm, os.O_RDWR)
    except Exception as error:
        sys.stderr.write(  "Can't open CDC/ACM file %s: %s\n"
                         % (dev_acm, str(error))             )
        sys.exit(1)
    if os.isatty(usb_fd):  # for testing
        tty.setraw(usb_fd)



def check_version(quiet):
    global firmware_version
    vers_rsp = cmd_rsp(VERS_CMD, 3, "VERS")
    if vers_rsp is not None:
        firmware_version = struct.unpack('BBB', vers_rsp)
        if firmware_version == VERSION:
            if not quiet:
                sys.stdout.write(  "Firmware version  match: %s\n"
                                 % '.'.join([str(elem) for elem in VERSION]))
        else:
            if    firmware_version[0] != VERSION[0]\
               or firmware_version[1]  < VERSION[1]:
                mismatch = "ERROR"
            else:
                mismatch = "Warning"
            sys.stdout.write(  "%s: Firmware version mismatch: "
                               "firmware: %s  software: %s\n"
                             % (mismatch,
                                '.'.join([str(elem) for elem in
                                                    firmware_version]  ),
                                '.'.join([str(elem) for elem in VERSION])))



def firmware_connect(quiet=False):
    while True:
        if not os.isatty(usb_fd): break   # for testing
        if not quiet:
            sys.stdout.write("Connecting to buck50 device "
                             "(press CTRL-C to abort ...  )\n")
        os.write(usb_fd, struct.pack('20B', *SIGNATURE))
        response = wait_read(4, 1)
        if response is WAIT_READ_STDIN:
            sys.exit(1)
        elif response is not None:
            response = struct.unpack('I', response)[0]
            if response == IDENTITY:
                if not quiet:
                    sys.stdout.write(  "Firmware identity match: 0x%08x\n"
                                     % IDENTITY)
            else:
                sys.stdout.write(  "buck50 device identity mismatch: "
                                   "hardware: 0x%08x  "
                                   "software: 0x%08x\n"
                                 % (response, IDENTITY)                    )
            break
        elif not os.isatty(usb_fd):   # for testing
            break
        else:
            sys.stderr.write("Will retry sending signature data in 1 second\n")
            time.sleep(1)
    if not os.isatty(usb_fd): return  # for testing
    # check version and report serial number
    check_version(quiet)
    if not quiet:
        serial_number()



def reset_connect():
    global dev_acm, usb_fd
    if r_u_sure(  "Disconnect USB port/driver (currently "
                  "\"%s\") and connect to (use usb=... to "
                  "change) \"%s\"?"
                % (dev_acm, reset_config['usb'].val)       ):
        dev_acm = reset_config['usb'].val
        usb_connect()   # will exit program on failure



def serial_number():
    serial   = None
    response = cmd_rsp(SRNO_CMD, 1, "serial_number")
    if response:
        serial = wait_read(response[0], 2)
    if serial in (WAIT_READ_STDIN, None):
        sys.stderr.write("Failure receiving serial number\n")
        return
    Pager()("Device serial number:    %s\n" % serial.decode('utf-8'),
            immed=True, one_line=True)



def blink_user_led():
    '''
    Pager()("Blinking device LED, <ENTER> to halt ...",
            immed=True, one_line=True                 )
    '''
    cmnd_cmd(BLNK_CMD)
    safe_input("Blinking device LED, <ENTER> to halt ... ",
               "blink_user_led"                           )
    cmnd_cmd(HALT_CMD)



# trigger_config actions
#

def check_triggers():
    max_trig = max([ndx for ndx in triggers_config])
    errors   = 0

    for (num, trig) in triggers_config.items():
        if trig.mach() not in triggers_config:
            sys.stderr.write(  "trigger %d=%s: pass %d isn't in triggers %s\n"
                             % (num, trig, trig.mach(), list(triggers_config)))
            errors += 1

        if trig.fail() not in triggers_config:  #  and trig.mask() != 0x00:
            sys.stderr.write(  "trigger %d=%s: fail %d isn't in triggers %s\n"
                             % (num, trig, trig.fail(), list(triggers_config)))
            errors += 1

        if      trig.mach     () != num             \
            and trig.mask     () != 0               \
            and trig.mach     () in triggers_config \
            and trig.mask_bits() == triggers_config[trig.mach()].mask_bits():
            sys.stderr.write(  "trigger %d=%s: pass -> %d has "
                               "same test (%s) -- disallowd\n"
                             % (num, trig, trig.mach(), trig.mask_bits()))
            errors += 1

        if      trig.fail     () != num             \
            and trig.mask     () != 0               \
            and trig.fail     () in triggers_config \
            and trig.mask_bits() == triggers_config[trig.fail()].mask_bits():
            sys.stderr.write(  "trigger %d=%s: fail -> %d has "
                               "same test (%s) -- disallowd\n"
                             % (num, trig, trig.fail(), trig.mask_bits()))
            errors += 1

        if num == 0:
            continue

        if trig.mach() == trig.fail() and trig.fail() != 0 and trig.mask() != 0:
            sys.stderr.write(  "trigger %d=%s: not allowed pass same as fail "
                               "(%d)\n"
                             % (num, trig, trig.mach())                       )
            errors += 1

    for trigger in triggers_config:
        machs = []
        check = trigger
        while True:
            machs.append(check)
            next = triggers_config[check].mach()
            if not next in triggers_config:  # already checked above
                break
            if next == 0:
                break
            if next in machs:
                machs.append(next)
                sys.stderr.write(  "Infinite triggers \"pass\" loop: %s\n"
                                 % " -> ".join([str(mach) for mach in machs]))
                errors += 1
                break
            check = next

    for trigger in triggers_config:
        fails = []
        start = trigger
        check = trigger
        while True:
            if triggers_config[check].mask() == 0:
                break
            fails.append(check)
            next = triggers_config[check].fail()
            if not next in triggers_config:  # already checked above
                break
            # if triggers_config[next].mask() == 0:
            #   break
            if next in fails:
                if next == start:
                    break
                else:
                    sys.stderr.write(  "Bad triggers fail chain, doesn't end "
                                       "at start or at \"xxxxxxxx-X-Y\": %s\n"
                                     % " -> ".join([    str(fail)
                                                    for fail
                                                    in fails]    )            )
                errors += 1
                break
            check = next

    if errors:
        sys.stderr.write(  "%d trigger error%s\n"
                         % (errors, '' if errors == 1 else 's'))
        return False
    else:
        return True


def triggers_undo():
    global triggers_config, triggers_backup
    if r_u_sure("Restore triggers to before last change or delete?"):
        triggers_config = copy.deepcopy(triggers_backup)
        sys.stdout.write("Triggers restored\n")



# configure_config actions
#

def configure_save():
    global configure_loadsave_error
    if configure_config['file'].val is None:
        sys.stderr.write("Set \"file=\" parameter\n")
        configure_loadsave_error = True
        return
    filename = configure_config['file'].val
    if not filename.endswith(CONFIG_FILE_EXTENSION):
        filename += CONFIG_FILE_EXTENSION
    file = safe_open(filename, " for saving configuration")
    if not file:
        sys.stderr.write("Not saving to file \"%s\"\n" % filename)
        configure_loadsave_error = True
        return
    if file:
        file.write("idnt 0x%x\n"     % IDENTITY)
        file.write("vers %d %d %d\n" % VERSION )
        configs_print([    key
                       for key     in SETTINGS.keys()
                       if  key not in ('confige' ,
                                       'help'    ,
                                       'warranty',
                                       'quit'    )],
                        file                         )
        file.close()
        sys.stdout.write("Saved confguration to file %s\n" % filename)



def configure_load():
    global configure_loadsave_error
    if configure_config['file'].val is None:
        sys.stderr.write("Set \"file=\" parameter\n")
        configure_loadsave_error = True
    else:
        filename = configure_config['file'].val
        if load_config_file(filename):
            sys.stdout.write("Loaded from file %s\n" % filename)
        else:
            configure_loadsave_error = True




### classes/types
#
#

# helpers
#

def disambiguate(choice, choices, case_sensitive=False):
    if case_sensitive:
        matches = [    check
                   for check
                   in  choices
                   if  check.startswith(choice)]
    else:
        choice_lower  = choice.lower()
        matches = [    check
                   for check
                   in  choices
                   if  check.lower().startswith(choice_lower)]
    # matches.sort()
    number_of_matches = len(matches)
    if number_of_matches == 0:
        good_match  = None
        exact_match = None
        # check_match = None
        error       =   "\"%s\" is not prefix of any: %s" \
                      % (choice, comma_or_concat(tuple(choices), 'or'))
                                               # tuple in case called with
                                               # dict_keys
    elif number_of_matches == 1:
        good_match = matches[0]
        if case_sensitive:
            if matches[0] == choice:
                exact_match = matches[0]
            else:
                exact_match = None
        else:
            if matches[0].lower() == choice_lower:
                exact_match = matches[0]
            else:
                exact_match = None
        if exact_match:
            error = None
        if not exact_match:
            error = "\"%s\" does not exactly match \"%s\"" % (good_match,
                                                              matches[0])
    else:
        if case_sensitive:
            if choice in matches:
                exact_match = choice
            else:
                exact_match = None
        else:
            matches_lower = [check.lower() for check in matches]
            if choice_lower in matches_lower:
                exact_match = matches[matches_lower.index(choice_lower)]
            else:
                exact_match = None
        if exact_match:
            good_match = exact_match
        else:
            matches.sort()
            good_match = matches[0]
        error =   "\"%s\" is ambiguous prefix of: %s" \
                % (choice, comma_or_concat(matches, 'and'))
    return collections.namedtuple("disambig"                      ,
                                  "good exact count matches error") \
                                 (good_match       ,
                                  exact_match      ,
                                  number_of_matches,
                                  matches          ,
                                  error            )



def float_completer(text, units):
    if not text:
        return [char for char in '+-0123456789.']
    alloweds = '0123456789'
    if '.' not in text:         alloweds += '.'
    if 'e' not in text.lower(): alloweds += 'eE'
    if text[-1].lower() == 'e': alloweds += '+-'
    if text[-1] in '0123456789.eE+-':
        # will cause problems if any in units starts with 'e' or 'E'
        completes = [text + char for char in alloweds]
        if not text[-1] in 'eE+-':
            completes += [text + unit for unit in units]
    else:
        found = re.search('[^.0123456789eE+-]', text)
        # will cause problems if any in units starts with 'e' or 'E'
        if found:
            pos       = found.span()[0]
            letters   = text[pos:]
            # can't do case-insensitive, either manually here or
            #   with disambiguate() because GNUreadline won't allow
            #   replacing e.g. "1m" with "1MHz" (wipes out the "m"
            #   leaving "1" for second completion, doesn't show choices)
            completes = [    text[:pos] + unit + " "
                         for unit
                         in  units
                         if  unit.startswith(letters)]
        else:
            completes = []
    return completes




### base classes
#

class RangeInt(object):
    __slots__ = ['_val', '_min', '_max', '_help']
    def __init__(self, value=0, min=0, max=0):
        self._min = min
        self._max = max
        self.val = value
    def __str__(self):
        return '%d' % self._val
    @property
    def val(self):
        return self._val
    def set_value(self, value):
        try:
            if type(value) is str:
                value = int(value, 16 if 'x' in value.lower() else 10)
            else:
                value = int(value)
            assert(value >= self._min and value <= self._max)
            self._val = value
        except Exception as error:
            raise ValueError(  "\"%s\" is not %s" % (value, self.suitable()))
    @val.setter
    def val(self, value):
        self.set_value(value)
    def suitable(self):
        return "decimal or hex integer in range [%d ... %d]" % (self._min,
                                                                self._max)
    def complete(self, text):
        return [text + digit for digit in 'x0123456789abcde']




class SpecialInt(RangeInt):
    UNITS = ('unlimited', '<number>')
    _slots_ = ['_val', '_special', '_min', '_max', '_specval']
    def __init__(self, value=0, special='unlimited', min=0,max=0, specval=None):
        self._min     = min
        self._max     = max
        self._special = special
        self._specval = max if specval is None else specval
        self.val      = value
    def __str__(self):
        if self._val >= self._max:
            return self._special
        else:
            return '%d' % self._val
    def is_special(self):
        return self._val >= self._specval
    def special_str(self):
        return self._special
    @property
    def val(self):
        return self._val
    @val.setter
    def val(self, value):
        if value.lower()[0] == self._special[0]:
            self._val = self._specval
            return
        try:
            RangeInt.set_value(self, value)
            # value = int(value)
            # assert(value >= self._min and value <= self._max)
            # self._val = value
        except Exception as error:
            raise ValueError("\"%s\" is not %s" % (value, self.suitable()))
    def suitable(self):
        return   "\"%s\" or integer " "in range [%d ... %d]" \
               % (self._special, self._min, self._max)
    def complete(self, text):
        completed = []
        if text and text[0].lower() == self._special[0]:
            return ["%s " % self._special]
        if not text:
            completed = ["%s " % self._special]
        return completed + RangeInt.complete(self, text)



class StringsAndValues(object):
    __slots__ = ['__val', '__str', '__case', '_help']
    def __init__(self, init='', case=False):
        self.__case = case
        self.val = init
    def str(self):
        return self.__str
    def __str__(self):
        return self.str()
    @classmethod
    def __getitem__(self, key):
        return self.strings_and_values[key]
    @classmethod
    def keys(self):
        return self.strings_and_values.keys()
    @classmethod
    def choices(self):
        return '|'.join(self.strings_and_values.keys())
    @property
    def val(self):
        return self.__val
    @val.setter
    def val(self, value):
        if type(value) is not str:
            self.set_by_val(value)
            return
        if value == '':
            raise ValueError(  "Empty value not allowed -- "
                               "not changing \"%s\""
                             % self.str()                   )
        disambig = disambiguate(value                                ,
                                tuple(self.strings_and_values.keys()),
                                self.__case                          )
        if disambig.good:
            self.__str = disambig.good
            self.__val = self.strings_and_values[disambig.good]
        else:  # can only be non-zero with poorly-constructed self.s_a_v
            raise ValueError(disambig.error)
    def set_by_val(self, value):
        try:
            string = dict(map(reversed, self.strings_and_values.items()))[value]
            self.__str = string
            self.__val = value
        except Exception as error:
            raise ValueError("%s is not valid %s value" % (value, type(self)))
    def suitable(self):
        return comma_or_concat(tuple(self.keys()))
    def complete(self, text):
        return [key + " " for key in self.keys() if key.startswith(text)]



class TimeVal(object):
                                       # derived can't access if '__min','__max'
    __slots__ = ['__value', '_error', '_min', '_max', '_help']
    UNITS = tuple("y d our min s ms us μs ns".split())
    def __init__(self, init, min=0.0, max=sys.float_info.max):
        self._min   = min
        self._max   = max
        self._error = None
        self.val    = init
    @staticmethod
    def suffixed(value, signed='%g'):
        if abs(value) < 1e-9:
            return (signed + 'ps' )  % (value * 1e12)
        if abs(value) < 1e-6:
            return (signed + 'ns' )  % (value * 1e9)
        if abs(value) < 1e-3:
            return (signed + 'μs' )  % (value * 1e6)
        if abs(value) < 1.0:
            return (signed + 'ms' )  % (value * 1e3)
        if abs(value) < 60:
            return (signed + 's'  )  % value
        if abs(value) < 60 * 60:
            return (signed + 'min')  % (value / 60)
        if abs(value) < 60 * 60 * 24:
            return (signed + 'our' )      % (value / (60 * 60))
        if abs(value) < 60 * 60 * 24 * 365:
            return (signed + 'd'  )   % (value / (60 * 60 * 24))
        return     (signed + 'y'  )   % (value / (60 * 60 * 24 * 365))
    @staticmethod
    def str(value, error=None):
        if error:
            return "%s(error:%s)" % (TimeVal.suffixed(value       ),
                                     TimeVal.suffixed(error, '%+g'))
        else:
            return TimeVal.suffixed(value)
    def __str__(self):
        return self.str(self.__value, self._error)
    def parse(self, text):
        found = re.search('[^.0123456789eE+-]', text)
        if not found:
            raise ValueError("\"%s\" is not %s" % (text, self.suitable()))
        value = text[:found.span()[0] ]
        units = text[ found.span()[0]:]
        if units != text:  # number-less input
            disambig = disambiguate(units, self.UNITS)
            if disambig.count != 1:
                raise ValueError(disambig.error)
            units = disambig.good
        if   units == 'ms':         scale = 1e-3
        elif units in ('us', 'μs'): scale = 1e-6
        elif units == 'ns':         scale = 1e-9
        elif units == 's':          scale = 1.0
        elif units == 'min':        scale = 60.0
        elif units == 'our':        scale = 60.0 * 60.0
        elif units == 'd':          scale = 60.0 * 60.0 * 24.0
        elif units == 'y':          scale = 60.0 * 60.0 * 24.0 * 365.0
        else:                       scale = 1.0
        try:
            scaled = scale * float(value)
            assert(scaled >= self._min and scaled <= self._max)
            return scaled
        except Exception as error:
            raise ValueError("\"%s\" is not %s" % (text, self.suitable()))
    @property
    def val(self):
        return self.__value
    @val.setter
    def val(self, value):
        self.__value = self.parse(value)
    def suitable(self):
        return   "floating point number with %s suffix in range [%s ... %s]" \
               % (comma_or_concat(self.UNITS),
                  self.str(self._min)        ,
                  self.str(self._max)        )
    def complete(self, text):
        return float_completer(text, self.UNITS)



class TimeFreqVal(TimeVal):
    __slots__ = ['__value']
    UNITS = tuple("y d our min s ms us μs ns Hz kHz MHz GHz".split())
    def __init__(self, init, min= 0, max=sys.float_info.max):
        super().__init__(init, min, max)
    @staticmethod
    def suffixed(value, signed='%g'):
        if value == 0.0:
            return '0Hz(approx)'
        if abs(value) <= 1e-12:
            return (signed + 'THz'    ) % (1e-12 / value)
        if abs(value) <= 1e-9:
            return (signed + 'GHz'    ) % (1e-9 / value)
        if abs(value) <= 1e-6:
            return (signed + 'MHz'    ) % (1e-6 / value)
        if abs(value) <= 1e-3:
            return (signed + 'kHz'    ) % (1e-3 / value)
        if abs(value) > 1e10:
            return (signed + 'pHz'    ) % (1e12 / value)
        if abs(value) > 1e7:
            return (signed + 'nHz'    ) % (1e9  / value)
        if abs(value) > 1e4:
            return (signed + 'μHz'    ) % (1e6  / value)
        if abs(value) > 1e1:
            return (signed + 'milliHz') % (1e3  / value)
        return     (signed + 'Hz'     ) % (1.0  / value)
    @staticmethod
    def str(value, error=None):
        if error:
            if abs(error) > 1e-18:  # check for underflow, causes divide by zero
                hz_error = 1.0 / (1.0 / value - 1.0 / (value - error))
            else:
                hz_error = 0.0
            return "%s=%s(error:%s,%s)"                   \
                   % (TimeFreqVal.suffixed(   value       ),
                      TimeVal    .suffixed(   value       ),
                      TimeFreqVal.suffixed(hz_error, '%+g'),
                      TimeVal    .suffixed(   error, '%+g'))
        else:
            return '%s=%s' % (TimeFreqVal.suffixed(value),
                              TimeVal    .suffixed(value))
    def __str__(self):
        return self.str(self.__value, self._error)
    def parse(self, text):
        found = re.search('[^.0123456789eE+-]', text)
        if not found:
            raise ValueError("\"%s\" is not %s" % (text, self.suitable()))
        value = text[:found.span()[0] ]
        units = text[ found.span()[0]:]
        if units != text:  # number-less input
            disambig = disambiguate(units, self.UNITS)
            if disambig.count != 1:
                raise ValueError(disambig.error)
            units = disambig.good
        if   units == 'kHz': scale = 1e-3
        elif units == 'MHz': scale = 1e-6
        elif units == 'GHz': scale = 1e-9
        elif units == 'Hz' : scale = 1.0
        else:                scale = None
        try:
            if scale:
                scaled = scale / float(value)
                assert(scaled >= self._min and scaled <= self._max)
                return scaled
            else:
                return TimeVal.parse(self, text)
        except Exception as error:
            raise ValueError("\"%s\" is not %s" % (text, self.suitable()))
    @property
    def val(self):
        return self.__value
    @val.setter
    def val(self, value=''):
        self.__value = self.parse(value)
    def suitable(self):
        return   "floating point number with %s suffix in range [%s ... %s]" \
               % (comma_or_concat(self.UNITS),
                  self.str(self._min)        ,
                  self.str(self._max)        )
    def complete(self, text):
        return float_completer(text, self.UNITS)



class TimePcntVal(TimeVal):
    __slots__ = ['__value', '__parnt', '__prcnt', '__time', '_help']
    UNITS = tuple("% s ms us μs ns Hz kHz MHz GHz".split())
    def __init__(self, init='50%', parnt=None):
        self.__parnt = parnt
        super().__init__(init, 0, 0xffff)
        if parnt: parnt.dependent(self)
        self.val = init
    @staticmethod
    def str(value, prcnt=None, error=None):
        if prcnt:
            return '%g%%=%s' % (prcnt * 100.0, TimeVal.str(value, error))
        else:
            return TimeVal.str(value, error)
    def __str__(self):
        return self.str(self.as_time(self.__value), self.__prcnt, self._error)
    def as_time(self, value):
        return (value + 1) * (self.__parnt.psc() + 1) / CPU_HZ
    def check_set_val(self, time):
        value = int(round(time * CPU_HZ / (self.__parnt.psc() + 1))) - 1
        if not 0 <= value <= self.__parnt.arr():
            sys.stderr.write(  "%s not in range [%s ... %s] possible with "
                               "current timer prescaler\n"
                             % (TimeVal.str(time                           ),
                                TimeVal.str(self.as_time(0)                ),
                                TimeVal.str(self.as_time(self.__parnt.arr()))))
            self.__value = 0 if value < 0 else self.__parnt.arr()
            sys.stderr.write(  "Setting to %s (was %s)\n"
                             % (self, TimeVal.str(self.__time)))
            self.__time = self.as_time(self.__value)
        else:
            self.__value = value
    def compute_error(self):
        if self.__prcnt:
            self._error =   self.as_time(self.__parnt.arr() * self.__prcnt) \
                          - self.as_time(self.__value)
        else:
            self._error = self.__time - self.as_time(self.__value)
    def update(self, time):
        if self.__prcnt:
            self.__value = int(round(self.__prcnt * self.__parnt.arr()))
        else:
            self.check_set_val(self.__time)
        self.compute_error()
    @property
    def val(self):
        return self.__value
    @val.setter
    def val(self, input):
        if type(input) is str and input.endswith('%'):
            percent = 0.01 * float(input[:-1])
            if not 0.0 <= percent <=1.0:
                raise ValueError("\"%s\" not in range [0%% ... 100%%]" % input)
            self.__prcnt = percent
            self.__value = int(round(self.__prcnt * self.__parnt.arr()))
            self.__time  = self.as_time(self.__value)
        else:
            time = TimeVal.parse(self, input)  # can raise ValueError
            self.__prcnt = None
            self.__time  = time
            self.check_set_val(time)     # can clamp
        self.compute_error()
    def suitable(self):
        return   TimeFreqVal.suitable(self) \
               + " or <xxx>% (xxx floating point number in range [0 ... 100])"



class TimeSpecialVal(TimeVal):
    __slots__ = ['__value', '_help', '_special', '_help']
    def __init__(self, init, min = 1, max = 0xffffffff, special='unlimited'):
        self._special = special
        super().__init__(init, min / CPU_HZ , max / CPU_HZ)
        self.val      = init
    def __str__(self):
        if self.__value == 0: return self._special
        else:                 return self.str(self.as_float(), self._error)
    def as_float(self):
        return self.__value / CPU_HZ
    @property
    def val(self):
        return self.__value
    @val.setter
    def val(self, value):
        if not value:
            raise ValueError("Bad time//special value (blank/empty)")
        if value.lower()[0] == self._special.lower()[0]:
            self.__value = 0
            self._error  = None
        else:
            parsed = self.parse(value)  # can raise exception
            self.__value = int(round(parsed * CPU_HZ))  # & 0xffffffff
            self._error  = self.as_float() - parsed
    def suitable(self):
        return "%s, or \"%s\"" %  (TimeVal.suitable(self),
                                   self._special             )
    def complete(self, text):
        special = []
        if text and text[0].lower() == self._special.lower()[0]:
            return [self._special + " "]
        if not text:
            special = [self._special + " "]
        return special + TimeVal.complete(self, text)



class TimeFreqSpecialVal(TimeFreqVal):
    __slots__ = ['__value', '_help', '_special', '_help']
    def __init__(self, init, min = 1, max = 0xffffffff, special='unlimited'):
        self._special = special
        super().__init__(init, min / CPU_HZ , max / CPU_HZ)
        self.val      = init
    def __str__(self):
        if self.__value == 0: return self._special
        else:                 return self.str(self.as_float(), self._error)
    def as_float(self):
        return self.__value / CPU_HZ
    @property
    def val(self):
        return self.__value
    @val.setter
    def val(self, value):
        if not value:
            raise ValueError("Bad time/freq/special value (blank/empty)")
        if value.lower()[0] == self._special.lower()[0]:
            self.__value = 0
            self._error  = None
        else:
            time = self.parse(value)  # can raise exception
            self.__value = int(round(time * CPU_HZ))  # & 0xffffffff
            self._error  = self.as_float() - time
    def suitable(self):
        return "%s, or \"%s\"" %  (TimeFreqVal.suitable(self),
                                   self._special             )
    def complete(self, text):
        special = []
        if text and text[0].lower() == self._special.lower()[0]:
            return [self._special + " "]
        if not text:
            special = [self._special + " "]
        return special + TimeFreqVal.complete(self, text)



class TimeFreqPscArrVal(TimeFreqVal):
    __slots__ = ['__psc', '__arr', '__dependents', '_help']
    def __init__(self, init, min = 1.0 / CPU_HZ, max = 0xffffffff / CPU_HZ):
        self.__dependents = [] # must be first for super()->update_dependents()
        super().__init__(init, min, max)
        self.val = init
    def __str__(self):
        return self.str(self.as_float(), self._error)
    def as_float(self):
        return (self.__psc + 1) * (self.__arr + 1) / CPU_HZ
    def psc(self):
        return self.__psc
    def arr(self): return self.__arr
    @property
    def val(self):
        return (self.__psc, self.__arr)
    @val.setter
    def val(self, value=''):
        time         = TimeFreqVal.parse(self, value)
        psc          = int(math.ceil(time * CPU_HZ / (1<<16)))
        self.__arr   = int(round    (time * CPU_HZ / psc    )) - 1
        self.__psc   = psc - 1
        self._error  = self.as_float() - time
        self.update_dependents(time)
    def dependent(self, dependent):
        self.__dependents.append(dependent)
    def update_dependents(self, value):
        for dependent in self.__dependents:
                dependent.update(value)



class Duration(TimeVal):
    UNITS = tuple("y d our min s ms us μs ns".split())
    __slots__ = ['__value', '__tick', '__arr', '_help']
    def __init__(self, init, tick=(1<<16)/CPU_HZ, min=1, max=0xffff, arr=0):
        self.__tick    = tick
        self.__arr     = arr
        super().__init__(init, min * tick, max * tick)
        self.val       = init
    def __str__(self):
        if self.enabled():
            return self.str(self.as_float(), self._error)
        else:
            return 'infinite'
    def as_float(self):
        if self.enabled():
            return (self.val + self.__arr) * self.__tick
        else:
            return 0.0
    def enabled(self):
        return self.__value is not None
    @property
    def val(self):
        return self.__value
    @val.setter
    def val(self, value):
        if not value:
            raise ValueError("Bad duration value (blank/empty instead of "
                             "\"infinite\" or time value)")
        if value.lower()[0] == 'i':
            self.__value   = None
            self._error    = None
        else:
            seconds = TimeVal.parse(self, value)  # can raise exception
            self.__value = int(round(seconds / self.__tick)) - self.__arr
            self._error  = self.as_float() - seconds
    def suitable(self):
        return TimeVal.suitable(self) + ", or \"infinite\""
    def complete(self, text):
        infinite = []
        if text and text[0].lower() == 'i':
            return ["infinite "]
        if not text:
            infinite = ["infinite "]
        return infinite + TimeVal.complete(self, text)




### standalone classes
#

class Pager(object):
    __slots__ = ['__stream'  ,
                 '__lines'   ,
                 '__tab_w'   ,
                 '__indent'  ,
                 '__initial' ,
                 '__final'   ,
                 '__one_line',
                 '__term_w'  ,
                 '__term_h' ]
    def __init__(self,
                 stream=sys.stdout,
                 text=''          ,
                 tab_w=4          ,
                 initial=0        ,
                 indent=0         ,
                 final=True       ,
                 one_line=False   ):
        self.__stream   = stream
        self.__tab_w    = tab_w
        self.__indent   = indent
        self.__initial  = initial
        self.__final    = final
        self.__one_line = one_line
        term_size     = shutil.get_terminal_size()
        self.__term_h = term_size.lines
        self.__term_w = term_size.columns
        self.__lines    = []
        if text:
            self.__call__(text)
        if one_line:
            self.write()
    def write(self         ,
              text         ,
              indent=None  ,
              immed=False  ,
              final=None   ,
              one_line=None):
        if indent   is not None: self.__indent   = indent
        if final    is not None: self.__final    = final
        if one_line is not None: self.__one_line = one_line
        for line in text.split('\n'):
            if not line:
                continue
            line = line.expandtabs(self.__tab_w)
            pos       = 0
            begs_ends = []
            for word in line.split():
                beg = line.find(word, pos)
                end = beg + len(word)
                begs_ends.append((beg, end))
                pos = end
            begs_ends.append((int(1e9), int(1e9)))
            beg = 0
            ind = 0
            one = begs_ends[0][0]
            two = begs_ends[0][0] if len(begs_ends) == 1 else begs_ends[1][0]
            for ndx in range(len(begs_ends) - 1):
                if begs_ends[ndx + 1][1] + ind - beg > self.__term_w:
                    self.__lines.append(  "%s%s"
                                        % (" " * ind                     ,
                                           line[beg:begs_ends[ndx][1]]))
                    beg = begs_ends[ndx + 1][0]
                    if   self.__indent == -1: ind = one
                    elif self.__indent == -2: ind = two
                    else:                     ind = self.__indent
        if immed:
            self.flush()
    def __call__(self, *args, **kwargs):
        self.write(*args, **kwargs)
    def flush(self, initial=None, final=None, one_line=None):
        if final    is not None: self.__final    = final
        if one_line is not None: self.__one_line = one_line
        if initial  is not None: self.__initial  = initial
        line_num  = self.__initial + 1  # hack
        num_lines = len(self.__lines)
        for (ndx, line) in enumerate(self.__lines):
            if self.__one_line and ndx == num_lines - 1: newline = " "
            else:                                        newline = '\n'
            self.__stream.write("%s%s" % (line, newline))
            line_num += 1
            if line_num >= self.__term_h:
                cooked = termios.tcgetattr(sys.stdin)
                raw    = termios.tcgetattr(sys.stdin)
                raw[3] = raw[3] & ~(    termios.ECHO
                                    | termios.ECHONL
                                    | termios.ICANON)
                termios.tcsetattr(sys.stdin, termios.TCSANOW, raw)
                self.__stream.write("%s%s%s" % ('\b' * self.__term_w,
                                                " "  * self.__term_w,
                                                '\b' * self.__term_w))
                self.__stream.write("<SPACE> to page, <ENTER> for "
                                    "line, any other to abort ... ")
                self.__stream.flush()
                command = sys.stdin.read(1)
                termios.tcsetattr(sys.stdin, termios.TCSANOW, cooked)
                self.__stream.write("%s%s%s" % ('\b' * self.__term_w,
                                                " "  * self.__term_w,
                                                '\b' * self.__term_w))
                self.__stream.flush()
                if command[0] == " ":
                    line_num = 1
                elif command[0] != '\n':
                    break
        self.__lines = []
        if self.__final:
            self.__stream.write('\n')


class Action(object):
    __slots__ = ['__action', '__help']
    def __init__(self, action, help=''):
        self.__action = action
        self.__help = help
    def action(self): return self.__action
    def help(self)  : return self.__help


class Trigger(object):
    __slots__ = ['__mask', '__bits', '__pass', '__fail', '__cmnt']
    IGNORE_CHARS = 'xXpPUu.*'
    ONES_CHARS   = '1iI'
    ZEROS_CHARS  = '0oO'
    def __init__(self, encoded, comment=''):
        self.val = (encoded, comment)
    def mask_bits(self):
        mask_bits = ""
        for bitnum in range(7, -1, -1):
            bit = 1 << bitnum
            if self.__mask & bit:
                if self.__bits & bit: mask_bits += '1'
                else:                 mask_bits += '0'
            else:
                mask_bits += 'x'
        return mask_bits
    def __str__(self):
        return "%s-%d-%d" % (self.mask_bits(), self.__pass, self.__fail)
    def fullstr(self):
        return "%s %s" % (self, self.__cmnt)
    def mach(self): return self.__pass
    def fail(self): return self.__fail
    def mask(self): return self.__mask
    def bits(self): return self.__bits
    def bytes(self):
        # little-endian, seen as FFPPBBMM lsb-to-msb on hardware
        # order important for optimized asm code
        return struct.pack('BBBB',
                           self.__mask,
                           self.__pass,
                           self.__fail,
                           self.__bits)
    @property
    def val(self):
        # REMOVE (change to bytes()
        return collections.namedtuple('TriggerTuple', "mask bits pass fail") \
                                     (self.__mask,
                                      self.__pass,
                                      self.__fail,
                                      self.__bits)
    @val.setter
    def val(self, encoded_and_comment):
        (encoded, comment) = encoded_and_comment
        try:
            (mask_bits, mach, fail) = encoded.split('-')
            assert(len(mask_bits) == 8)
            mach = int(mach)
            fail = int(fail)
        except Exception:
            raise ValueError(  "Bad encoded trigger \"%s\", "
                               "should be \"<ndx>=<8 mask/bits>-<pass>-<fail>\""
                             % encoded                                         )
        assert(mach in range(0, 256) and fail in range(0, 256)), \
              "pass=%d or fail=%d out of range [0 ... 255]" % (mach, fail)
        mask = 0x00
        bits = 0x00
        for (bit, char) in enumerate(mask_bits):
            if char not in   Trigger.IGNORE_CHARS \
                           + Trigger.ONES_CHARS   \
                           + Trigger.ZEROS_CHARS:
                raise ValueError(  "Trigger code char '%s' not in %s, %s, or %s"
                                 % (char                ,
                                    Trigger.IGNORE_CHARS,
                                    Trigger.ONES_CHARS  ,
                                    Trigger.ZEROS_CHARS ))
            if char not in Trigger.IGNORE_CHARS: mask |= (1 << (7 - bit))
            if char     in Trigger.  ONES_CHARS: bits |= (1 << (7 - bit))
        (self.__mask,
         self.__bits,
         self.__pass,
         self.__fail,
         self.__cmnt) = (mask, bits, mach, fail, comment)
    def complete(self, text):
        return [self.fullstr()]



class UsartBaud(TimeFreqVal):
    __slots__ = ['__value', '_help']
    def __init__(self, init, min = 16, max = 0xffff):
        super().__init__(init, min / CPU_HZ , max / CPU_HZ)
        self.val = init
    def __str__(self):
        return self.str(self.as_float(), self._error)
    def as_float(self):
        return self.__value / CPU_HZ
    @property
    def val(self):
        return self.__value
    @val.setter
    def val(self, value):
        time         = self.parse(value)
        self.__value = int(round(time * CPU_HZ)) # & 0xffff
        self._error  = self.as_float() - time



class Text(object):
    __slots__ = ['__value', '__max_len', '_help']
    def __init__(self, init, max_len=1):
        self.__value   = init
        self.__max_len = max_len
    def __str__(self):
        return str(self.__value)
    @property
    def val(self):
        return self.__value
    @val.setter
    def val(self, value):
        if type(value) is str:
            value = value.encode('latin-1', 'backslashreplace') \
                          .decode('unicode-escape')
            if len(value) > self.__max_len:
                Pager(stream=sys.stderr)(
                        "\"%s\" longer than %d chars, truncating to \"%s\""
                      % (value, self.__max_len, value[:self.__max_len])    ,
                      immed=True, one_line=True                            )
                value = value[:self.__max_len]
            self.__value = value
        elif type(value) is bytes:
            if len(value) > self.__max_len:
                Pager(stream=sys.stderr)(
                        "%s longer than %d chars, truncating to %s"
                      % (value, self.__max_len, value[:self.__max_len]),
                      immed=True, one_line=True                        )
            self.__value = value[:self.__max_len]
        else:
            Pager(stream=sys.stderr)(
                    "\"%s\" is type \"%s\", not str or bytes: ignoring"
                  % (value, type(value))                               ,
                  immed=True, one_line=True                            )
    def suitable(self):
        return   "<any string <= %d chars including "   \
                 "optional \C character, " \
                 "\\xMN hexadecimal, \\ABC octal, "     \
                 "and/ or \\uABCD unicode escapes) "    \
                 "(use \"\\x20\" or \"\\040\" for "     \
                 "space)>"                              \
               % self.__max_len
    def complete(self, text):
        return string.ascii_letters + string.digits + string.punctuation



class DataBytes(object):
    __slots__ = ['__value', '__min_len', '__max_len', '_help']
    def __init__(self, init, min_len=0, max_len=1):
        self.__value   = init
        self.__min_len = min_len
        self.__max_len = max_len
    @staticmethod
    def str(val):
        return '.'.join(['%02x' % byt for byt in val])
    def __str__(self):
        return self.str(self.__value)
    def len(self):
        return len(self.__value)
    @staticmethod
    def parse(text, splitter=None, minlen=0, maxlen=1<<32):
        if type(text) not in (str, list):
            raise ValueError(  "\"%s\" is type \"%s\", not str: ignoring"
                             % (text, type(text))                        )
        try:
            maxlen = int(maxlen)
        except ValueError:
            raise ValueError(  "Internal error: DataBytes.parse() called with "
                               "non-int \"maxlen\" argument (\"%s\" of "
                               "type \"%s\""
                             % (maxlen, type(maxlen))                          )
        try:
            minlen = int(minlen)
        except ValueError:
            raise ValueError(  "Internal error: DataBytes.parse() called with "
                               "non-int \"minlen\" argument (\"%s\" of "
                               "type \"%s\""
                             % (minlen, type(minlen))                          )
        parsed  = b''
        pending = False
        for field in text if isinstance(text, list) else text.split(splitter):
            try:
                if   len(field) == 2: byte = int(field, 16)
                elif len(field) == 3: byte = int(field, 10)
                elif len(field) == 1: byte = ord(field    )
                elif len(field) == 8: byte = int(field,  2)
                elif len(field) == 0:
                    if pending:
                        byte    = ord('.')
                        pending = False
                    else:
                        pending = True
                        continue
                else:
                    raise ValueError
                if byte not in range(0, 256):
                    raise ValueError
            except:
                raise ValueError(  "\"%s\" is not 2 hex, 3 decimal, "
                                   "8 binary, or 1 ascii chars with "
                                   "value in range [0 ... 255]"
                                 % field                              )
            parsed += bytes([byte])
            pending = False
        if len(parsed) not in range(minlen, maxlen + 1):
            raise ValueError(  "Number of databytes (%d) not "
                               "in range %d ... %d"
                             % (len(parsed), minlen, maxlen + 1))
        return parsed
    @property
    def val(self):
        return self.__value
    @val.setter
    def val(self, value):
        self.__value = self.parse(value, '.', self.__min_len, self.__max_len)
    def suitable(self):
        return   "%d to %d \".\"-separated values, each 2 "         \
                 "hexadecimal, 3 decimal, or 1 ascii character(s)"  \
               % (self.__min_len, self.__max_len)
    def complete(self, text):
        return string.hexdigits


class FileName(object):
    __slots__ = ['__value', '__suffix', '_help']
    def __init__(self, init, suffix=None):
        self.__value  = init
        self.__suffix = suffix
    def __str__(self):
        return '' if self.__value is None else str(self.__value)
    @property
    def val(self):
        return self.__value
    @val.setter
    def val(self, value):
        if value:
            if re.search('[^.0123456789a-zA-Z~/_-]', value):
                raise ValueError(  "\"%s\" contains char(s) other "
                                   "than \"-.0123456789~/_"
                                 % value                           )
            self.__value = value
        else:
            self.__value = None
    def complete(self, text):
        (dir, file) = os.path.split(text)
        if self.__suffix and file.endswith(self.__suffix):
            file = file[:-len(self.__suffix)]
        names  = os.listdir(None if dir == '' else dir)
        names.append('..')
        if dir != '' and not dir.endswith('/'):
            dir = dir + '/'
        dirs   = [    '%s%s/' % (dir, name)
                  for name
                  in  names
                  if  os.path.isdir(dir + name) and name.startswith(file)]
        files  = [    name
                  for name
                  in  names
                  if  name.startswith(file) and not os.path.isdir(dir + name)]
                      # not .isdir() simpler test, plus
                      # .isfile() or (.islink and not .isdir())
                      # fails on /dev/null (and other special files?)
                      # because .isfile() returns False (why?) and no
                      # os.path.isspecial(), etc
        if len(files) == 1:
            dirs = []
        if self.__suffix:
            files = [name for name in files if name.endswith(self.__suffix)]
        files = ["%s%s " % (dir, name) for name in files]
        return dirs + files
    def suitable(self):
        return "<any valid file or path name> (no overwrite warning " \
               "if filename contains \"tmp\", \"temp\", or \"scratch\")"



class Printf(object):
    __slots__ = ['__value', '__max_len', '_help']
    def __init__(self, init, max_len=1):
        self.__value   = init
        self.__max_len = max_len
    def __str__(self):
        return self.__value
    @property
    def val(self):
        return self.__value
    @val.setter
    def val(self, value):
        if type(value) is str:
            if len(value) > self.__max_len:
                raise ValueError(  "\"%s\" longer than max %d chars"
                                 % (value, self.__max_len)          )
        else:
            raise ValueError("\"%s\" is not string" % value)
        try:
            value % 3.1419
        except:
            raise ValueError(  "\"%s\" is not valid printf format string "
                               "for use with single float argument"
                             % value                                      )
        self.__value = value.encode('latin-1', 'backslashreplace') \
                            .decode('unicode-escape')
    def suitable(self):
        return   "<\"printf\"-style format string with <= %d "      \
                 "total characters for use with single float "      \
                 "argument. Must have single \"%%\" followed "      \
                 "by valid printf format character. May contain "   \
                 "arbitrary additional text before and/or after "   \
                 "printf format, including optional escape "        \
                 "sequences in the form \C character, " \
                 "\\xMN hexadecimal, \\ABC octal, "     \
                 "and/ or \\uABCD unicode escapes) "    \
                 "(use \"\\x20\" or \"\\040\" for "     \
                 "space) (use \"%%%%\" for single "     \
                 "literal non-format \"%%\" "           \
                 "character). Examples: \"%%.2f\", "    \
                 "\"%%10.4e\", \"%%g seconds\", "       \
                 "\"control: %%6.4f \\x03bcV\", "       \
                 "\"%%8.3f%%%%\""           \
               % self.__max_len
    def complete(self, text):
        return string.ascii_letters + string.digits + string.punctuation



class ScaleHyst(object):
    __slots__ = ['__lo', '__hi', '__hyst', '_help']
    def __init__(self, lo=0, hi=4095, hyst=0):
        self.__lo   = lo
        self.__hi   = hi
        self.__hyst = hyst
    def __str__(self):
        return '%g:%g:%g' % (self.__lo, self.__hi, self.__hyst)
    def ranged(self, value):
        return    self.__lo                     \
               + (float(value) / float(0xfff))  \
               * (self.__hi - self.__lo)
    def scaled_hyst(self, high=0xfff, hyst=None, lo=None, hi=None):
        if hyst is None: hyst = self.__hyst
        if lo   is None: lo   = self.__lo
        if hi   is None: hi   = self.__hi
        return int(round(high * hyst / (hi - lo)))
    def ranged_hyst(self, high=0xfff, hyst=None, lo=None, hi=None):
        if hyst is None: hyst = self.__hyst
        if lo   is None: lo   = self.__lo
        if hi   is None: hi   = self.__hi
        return lo + (float(hyst) / float(high)) * (hi - lo)
    @property
    def val(self):
        return (self.__lo, self.__hi, self.__hyst)
    @val.setter
    def val(self, value):
        try:
            lhh      = value.split(':')
            lo_val   = float(lhh[0])
            hi_val   = float(lhh[1])
            hyst_val = float(lhh[2])
        except:
            raise ValueError(  "Can't parse \"%s\" as three "
                               "\":\"-separated float values "
                               "(<low>:<high>:<hysteresis>)"
                             % value                          )
        if lo_val >= hi_val:
            raise ValueError("%g low is not < %g high" % (lo_val, hi_val))
        if hyst_val < 0.0:
            raise ValueError("hysteresis value %g must be >= 0.0" % hyst_val)
        if self.scaled_hyst(0xfff, hyst_val, lo_val, hi_val) > 0xff:
            raise ValueError(  "hysteresis %g of %s (low:high:hysteresis) "
                               "is greater than %g max (1/16 "
                               "of high-low=%g range)"
                             % (hyst_val                ,
                                value                   ,
                                self.ranged_hyst(0xfff,
                                                 0xff,
                                                 lo_val,
                                                 hi_val),
                                hi_val - lo_val         )                  )
        self.__lo   =  lo_val
        self.__hi   =  hi_val
        self.__hyst = hyst_val
    def suitable(self):
        return "<three \":\"-separated floating point values> " \
               "(<low>:<high>:<hysteresis>)"
    def complete(self, text):
        return [text + char for char in ':0123456789.+-eE']



class SclLvlHyst(object):
    __slots__ = ['__lo', '__hi', '__level', '__hyst', '_help']
    def __init__(self, lo=0, hi=4095, level=2047, hyst=0):
        self.__lo    = lo
        self.__hi    = hi
        self.__level = level
        self.__hyst  = hyst
    def __str__(self):
        return '%g:%g:%g:%g' % (self.__lo, self.__hi, self.__level, self.__hyst)
    def level(self): return self.__level
    def hyst (self): return self.__hyst
    def scaled_hyst(self, high=0xfff, hyst=None, lo=None, hi=None):
        if hyst  is None: hyst  = self.__hyst
        if lo    is None: lo    = self.__lo
        if hi    is None: hi    = self.__hi
        return int(round(high * hyst / (hi - lo)))
    def scaled_level(self, high=0xfff, level=None, lo=None, hi=None):
        if level is None: level = self.__level
        if lo    is None: lo    = self.__lo
        if hi    is None: hi    = self.__hi
        return int(round(high * (level - lo) / (hi - lo)))
    def ranged_level(self, high=0xfff, level=None, lo=None, hi=None):
        if level is None: level = self.__level
        if lo    is None: lo    = self.__lo
        if hi    is None: hi    = self.__hi
        return lo + (float(level) / float(high)) * (hi - lo)
    def ranged_hyst(self, hyst=None, high=0xfff, lo=None, hi=None):
        if hyst  is None: hyst  = self.__hyst
        if lo    is None: lo    = self.__lo
        if hi    is None: hi    = self.__hi
        return (float(hyst) / float(high)) * (hi - lo)
    @property
    def val(self):
        return (self.__lo, self.__hi, self.__hyst)
    @val.setter
    def val(self, value):
        try:
            lhlh      = value.split(':')
            lo_val    = float(lhlh[0])
            hi_val    = float(lhlh[1])
            level_val = float(lhlh[2])
            hyst_val  = float(lhlh[3])
        except:
            raise ValueError(  "Can't parse \"%s\" as four "
                               "\":\"-separated float values"
                               "(<low>:<high>:<level>:<hysteresis>)"
                             % value                                )
        if lo_val >= hi_val:
            raise ValueError("%g low is not < %g high" % (lo_val, hi_val))
        if level_val < lo_val or level_val > hi_val:
            raise ValueError(  "level value %g must be in "
                               "low...high range %g...%g"
                             % (level_val, lo_val, hi_val) )
        if hyst_val < 0.0:
            raise ValueError("hysteresis value %g must be >= 0.0" % hyst_val)
        if hyst_val > hi_val - lo_val:
            raise ValueError(  "hysteresis value  %g must be less than "
                               "or equal to high-low ((%g)-(%g)=(%g)) span"
                             % (hyst_val, hi_val, lo_val, hi_val - lo_val))

        self.__lo    =  lo_val
        self.__hi    =  hi_val
        self.__level = level_val
        self.__hyst  = hyst_val
    def suitable(self):
        return "<four \":\"-separated floating point values> " \
               "(<low>:<high>:<level>:<hysteresis>)"
    def complete(self, text):
        return [text + char for char in ':0123456789.+-eE']



class ActivePorts:
    __slots__ = ['_val', '_help']
    def __init__(self, init='4.5.6.7.8.9.10.11'):
        self.val = init
    def val(self):
        return self._val
    def __str__(self):
        return '.'.join([str(port) for port in self._val])
    @property
    def val(self):
        return self._val
    @val.setter
    def val(self, value):
        try:
            parsed = ([int(port) for port in value.split('.')])
        except Exception as error:
            raise ValueError(  "\"%s\" is not %s (%s)"
                             % (value, self.suitable(), error))
        if len(parsed) not in range(1,9):
            raise ValueError(  "\"%s\" is not %s -- not between "
                               "1 and 8 ports"
                             % (value, self.suitable())                 )
        if not all([port in range(4,12) for port in parsed]):
            raise ValueError(  "\"%s\" is not %s -- one or more ports "
                               "not in range 4...11"
                             % (value, self.suitable())                 )
        self._val = parsed
    def suitable(self):
        return "1 to 8 \".\"-separated integers in range 4...11"
    def complete(self, text):
        return [text + digit for digit in '.45678910']



class Trim(object):
    __slots__ = ['__value', '__observed', '__desired', '_help']
    def __init__(self, init, observed=1.0, desired=1.0):
        self.__value   = init
        self.__observed = observed
        self.__desired  = desired
    def __str__(self):
        return str("%#g/%#g" % (self.__observed, self.__desired))
    def as_float(self):
        return
    @property
    def val(self):
        return self.__observed / self.__desired
    @val.setter
    def val(self, value):
        try:
            obs_des = value.split('/')
            assert(len(obs_des) == 2)
            self.__observed = float(obs_des[0])
            self.__desired  = float(obs_des[1])
        except:
            raise ValueError("\"%s\" is not two floating point "
                             "numbers separated by \"/\""       )
    def suitable(self):
        return   "<X/Y with X and Y floating point numbers>"
    def complete(self, text):
        return list('0123456789./') + [None]



class HelpFloat(float):   # just to allow adding '__help' attribute
    def suitable(self):
        return   "<any floating point number>"
    def complete(self, text):
        return float_completer(text, [])




### derived classes
#

class I2cSpeed(TimeFreqVal):
    __slots__ = ['__value', '_help', '_min', '_max', '_error']
    def __init__(self, init, min=1/1e6, max=1/5e3):
        self._min = min
        self._max = max
        self.val = init
        self._error = 0
    def __str__(self):
        return self.str(self.as_float(), self._error)
    def ccr(self):
        if i2c_config['flavor'].str() == 'standard':
            return int(round(36e6 * self.__value /  2.0))
        if i2c_config['fast-duty'].str() == '2:1':
            return int(round(36e6 * self.__value /  3.0))
        else:
            return int(round(36e6 * self.__value / 25.0))
    def as_float(self):
        return self.__value
    @property
    def val(self):
        return self.__value
    @val.setter
    def val(self, value):
        self.__value = self.parse(value)



class SamplingMode(StringsAndValues):
    MHZ_6_26  =  0
    IRREGULAR =  1
    UNIFORM   =  2
    MHZ_4     =  3
    ANALOG    = 15  # only for upld_cmd, not valid for `logic mode=`
    strings_and_values = {
        '6.26MHz'   :  MHZ_6_26 ,
        'irregular' :  IRREGULAR,
        'uniform'   :  UNIFORM  ,
        '4MHz'      :  MHZ_4    ,
    }
    def __init__(self, init='4MHz-avg'): super().__init__(init)
SAMPLING_MODE_INVERSE = {     val:key
                      for (key, val)
                      in  SamplingMode.strings_and_values.items()}
def sampling_mode_inverse(code):
    return      SAMPLING_MODE_INVERSE[code]     \
           if   code in SAMPLING_MODE_INVERSE   \
           else "<none>"

class Able(StringsAndValues):
    strings_and_values = {
         'enabled' : 1,
        'disabled' : 0,
    }
    def __init__(self, init='disabled'): super().__init__(init)


class PulseMode(StringsAndValues):
    strings_and_values = {
        'frozen' : 0,
        'up'     : 1,
        'down'   : 2,
        'toggle' : 3,
        'low'    : 4,
        'high'   : 5,
        'fall'   : 6,
        'rise'   : 7,
    }
    def __init__(self, init='rise'): super().__init__(init)


class UsartPorts(StringsAndValues):
    PA_8_10 = 0
    PA_0_3  = 1
    strings_and_values = {
        'pa8-10' : PA_8_10,
        'pa0-3'  : PA_0_3 ,
    }
    def __init__(self, init='8bits'): super().__init__(init)


class UsartDataBits(StringsAndValues):
    BITS_7 = -1
    BITS_8 =  0
    BITS_9 =  1
    strings_and_values = {
        '7bits' : BITS_7,
        '8bits' : BITS_8,
        '9bits' : BITS_9,
    }
    def __init__(self, init='8bits'): super().__init__(init)


class UsartStopBits(StringsAndValues):
    strings_and_values = {
        '1bit'    : 0b00,
        '2bits'   : 0b10,
        '0.5bits' : 0b01,
        '1.5bits' : 0b11,
    }
    def __init__(self, init='1bit'): super().__init__(init)


class UsartParity(StringsAndValues):
    NONE = 0b00
    EVEN = 0b10
    ODD  = 0b11
    strings_and_values = {
        'none' : NONE,
        'even' : EVEN,
        'odd'  : ODD ,
    }
    def __init__(self, init='none'): super().__init__(init)


class SpiI2cMode(StringsAndValues):
    strings_and_values = {
        'disabled' : (0, 0),  # off, slave
        'master'   : (1, 1),  # on , master
        'slave'    : (1, 0),  # on , slave
    }
    def __init__(self, init='disabled'): super().__init__(init)


class SpiSelect(StringsAndValues):
    SOFTWARE = 0
    HARDWARE = 1
    strings_and_values = {
        'software' : SOFTWARE,
        'hardware' : HARDWARE,
    }
    def __init__(self, init='software'): super().__init__(init)


class SpiBaud(StringsAndValues):
    strings_and_values = {
        '36MHz'     : 0,
        '18MHz'     : 1,
        '9MHz'      : 2,
        '4.5MHz'    : 3,
        '2.25MHz'   : 4,
        '1.125MHz'  : 5,
        '562.5KHz'  : 6,
        '281.25KHz' : 7,
    }
    def __init__(self, init='281.25KHz'): super().__init__(init)


class SpiEndian(StringsAndValues):
    strings_and_values = {
        'msb' : 0,
        'lsb' : 1,
    }
    def __init__(self, init='msb'): super().__init__(init)


class SerialIdle(StringsAndValues):
    strings_and_values = {
        'low' : 0,
        'high' : 1,
    }
    def __init__(self, init='low'): super().__init__(init)


class SerialPhase(StringsAndValues):
    strings_and_values = {
        '1st' : 0,
        '2nd' : 1,
    }
    def __init__(self, init='1st'): super().__init__(init)


class GpioSpeed(StringsAndValues):
    strings_and_values = {
        '2MHz'  : 0b10,
        '10MHz' : 0b01,
        '50MHz' : 0b11,
    }
    def __init__(self, init='2MHz'): super().__init__(init)


class AdcSampHold(StringsAndValues):
    T_1_5   =  0
    T_7_5   =  1
    T_13_5  =  2
    T_28_5  =  3
    T_41_5  =  4
    T_55_5  =  5
    T_71_5  =  6
    T_239_5 =  7
    strings_and_values = {  # 14 MHz max ADC clock, using 12MHz == APB2 / 6
        '1.5+12.5@12MHz->857kHz'    : T_1_5  ,
        '7.5+12.5@12MHz->600kHz'    : T_7_5  ,
        '13.5+12.5@12MHz->462kHz'   : T_13_5 ,
        '28.5+12.5@12MHz->293kHz'   : T_28_5 ,
        '41.5+12.5@12MHz->222kHz'   : T_41_5 ,
        '55.5+12.5@12MHz->176kHz'   : T_55_5 ,
        '71.5+12.5@12MHz->143kHz'   : T_71_5 ,
        '239.5+12.5@12MHz->47.6kHz' : T_239_5,
    }
    def __init__(self, init=T_239_5): super().__init__(init)


class OpenDrain(StringsAndValues):
    strings_and_values = {
        'push-pull'  : 0,
        'open-drain' : 1,
    }
    def __init__(self, init='open-drain'): super().__init__(init)


class OpenDrainHiZ(StringsAndValues):
    strings_and_values = {
        'push-pull'  : 0,
        'open-drain' : 1,
        'hi-z'       : 2,
    }
    def __init__(self, init='open-drain'): super().__init__(init)


class Pull(StringsAndValues):
    strings_and_values = {
        'floating' : 0,
        'up'       : 1,
        'down'     : 2,
    }
    def __init__(self, init='floating'): super().__init__(init)


class DigitalFormat(StringsAndValues):
    CSV = 1
    VCD = 2
    strings_and_values = {
        'csv' : CSV,
        'vcd' : VCD,
    }
    def __init__(self, init='vcd'): super().__init__(init)

class AnalogFormat(StringsAndValues):
    CSV = 1
    strings_and_values = {
        'csv' : CSV,
    }
    def __init__(self, init='vcd'): super().__init__(init)


class TermOrFile(StringsAndValues):
    TERM = 1
    FILE = 2
    strings_and_values = {
        'terminal' : TERM,
        'file'     : FILE,
    }
    def __init__(self, init='terminal'): super().__init__(init)

class TermFileBoth(StringsAndValues):
    TERM = 0x1
    FILE = 0x2
    BOTH = 0x3
    strings_and_values = {
        'terminal' : TERM,
        'file'     : FILE,
        'both'     : BOTH,
    }
    def __init__(self, init='both'): super().__init__(init)


class TermPtySock(StringsAndValues):
    strings_and_values = {
        'terminal' : False,
        'external' : True,
    }
    def __init__(self, init='terminal'): super().__init__(init)


class BinaryGray(StringsAndValues):
    strings_and_values = {
        'binary' : 0,
        'gray'   : 1,
    }
    def __init__(self, init='binary'): super().__init__(init)


class Nss(StringsAndValues):
    FLOATING = 0
    LOW      = 1
    ACTIVE   = 2
    strings_and_values = {
        'floating' : FLOATING,
        'low'      : LOW     ,
        'active'   : ACTIVE  ,
    }
    def __init__(self, init='floating'): super().__init__(init)


class Slope(StringsAndValues):
    NONE     = 0
    POSITIVE = 1
    NEGATIVE = 2
    strings_and_values = {
        'disabled' : NONE    ,
        'positive' : POSITIVE,
        'negative' : NEGATIVE,
    }
    def __init__(self, init='positive'): super().__init__(init)


class CodeMem(StringsAndValues):
    strings_and_values = {
        'ram'   : 0,
        'flash' : 1,
    }
    def __init__(self, init='flash'): super().__init__(init)


class AsciiNumeric(StringsAndValues):
    ASCII   = 0
    NUMERIC = 1
    strings_and_values = {
        'ascii'   : ASCII  ,
        'numeric' : NUMERIC,
    }
    def __init__(self, init='ascii'): super().__init__(init)
AsciiNumeric._help = "Interactive text input/output mode. See input format instructions printed at command start."


class Snoop(StringsAndValues):
    DISABLED    = 0
    HEXADECIMAL = 1
    ASCII       = 2
    strings_and_values = {
        'disabled'    : DISABLED   ,
        'hexadecimal' : HEXADECIMAL,
        'ascii'       : ASCII      ,
    }
    def __init__(self, init='disabled'): super().__init__(init)



class TickUnits(StringsAndValues):
    strings_and_values = {
        's'  : 1e0,
        'ms' : 1e3,  # inverse for use as multiplier
        'us' : 1e6,  #              "
        'ns' : 1e9,  #              "
        'ps' : 1e12, #              "
        'fs' : 1e15, #              "
        'as' : 1e18, #              "
        'zs' : 1e21, #              "
        'ys' : 1e24, #              "
    }
    def __init__(self, init='us'): super().__init__(init)


class PointType(StringsAndValues):
    strings_and_values = {
        'none'           :  0,
        'plus'           :  1,
        'x'              :  2,
        'asterisk'       :  3,
        'square-open'    :  4,
        'square-filled'  :  5,
        'circle-open'    :  6,
        'circle-filled'  :  7,
        'delta-open'     :  8,
        'delta-filled'   :  9,
        'nabla-open'     : 10,
        'nabla-filled'   : 11,
        'diamond-open'   : 12,
        'diamond-filled' : 13,
        'arrow-open'     : 12,
        'arrow-filled'   : 13,
    }
    def __init__(self, init='none'): super().__init__(init)



class Viewer(StringsAndValues):
    GNUPLOT   = 1
    PULSEVIEW = 2
    GTKWAVE   = 3
    OTHER     = 4
    strings_and_values = {
        'gnuplot'   : GNUPLOT  ,
        'pulseview' : PULSEVIEW,
        'gtkwave'   : GTKWAVE  ,
        'other'     : OTHER    ,
    }
    def __init__(self, init='gnuplot'): super().__init__(init)



class I2cStdFast(StringsAndValues):
    strings_and_values = {
        'standard' : 0,
        'fast'     : 1,
    }
    def __init__(self, init='standard'): super().__init__(init)
I2cStdFast._help = "I2C speed, \"standard\" (up to 100 KHz) or "    \
                   "\"fast\" (up to 400 KHz)"




class I2cDuty(StringsAndValues):
    strings_and_values = {
        '2:1'  : 0,
        '16:9' : 1,
    }
    def __init__(self, init='2:1'): super().__init__(init)
I2cDuty._help = "Duty cycle for I2C \"fast\" flavor (see "  \
                "\"help i2c flavor=\")"




### constants
#
#

IDENTITY = 0xea017af5

DEV_ACM = '/dev/ttyACM0'
TTY_MAX = 1024

MAX_USB_BRIDGE = 62
MAX_FULL_USB   = 64
MAX_I2C_BRIDGE = 60

MAX_END_STR = 16

HALT_MEMORY = 1
HALT_TIME   = 2
HALT_USB    = 3
HALT_NAMES = {
    HALT_MEMORY : 'number of samples',
    HALT_TIME   : 'time elapsed'     ,
    HALT_USB    : 'user interrupt'   ,
}
def halt_name(code):
    return      HALT_NAMES[code]                    \
           if   code in HALT_NAMES                  \
           else "<unknown halt code %d>" % code


TRGR_NOT = 0x0100
TRGR_EXT = 0x0200
TRGR_ADC = 0x0400
TRGR_NRM = 0x0800
def triggered_at(where):
    if where & TRGR_EXT:
        if where & TRGR_ADC:
            return "Triggered via external sync while waiting " \
                   "for analog slope/level/hysteresis"
        else:
            return "Triggered via external sync at state #%d" % (where & 0xff)
    elif where & TRGR_ADC:
        if where & TRGR_NOT:
            return "Not triggered, interrupted while waiting " \
                   "for analog slope/level/hystersis"
        else:
            return "Triggered on analog level/slope/hysteresis"
    elif where & TRGR_NRM:
        return "Triggered at state #%d" % (where & 0xff)
    elif where & TRGR_NOT:
        return "Not triggered, interrupted at state #%d" % (where & 0xff)
    else:
        return "Unknown sampling trigger condition 0x%x" % where

PERIPH_STATUS = {
     0 : " OK"    ,  # first char blank for live_cmd single char error display
     1 : 'EMPTY'  ,
     2 : 'HALTED' ,
     3 : 'TIMEOUT',
     4 : 'BUSY'   ,
     5 : 'OVERRUN',
     6 : 'FAIL'   ,
     7 : 'LINE'   ,   # really BREAK, but conflict with BUSY
     8 : 'NOISE'  ,   # conflict with NACK okay, context dependent
     9 : 'PARITY' ,   # conflict with POTS okay, context dependent
    10 : 'USART'  ,   # really FRAMING, but conflict with FAIL
    11 : 'START'  ,
    12 : 'ADDR'   ,
    13 : 'NACK'   ,   # conflict with NOISE okay, context dependent
    14 : 'XFER'   ,
    15 : 'RXNE'   ,
    16 : 'POTS'   ,   # conflict with PARITY okay, context dependent
}
PERIPH_STATUS_INVERSE = {val:key for (key, val) in PERIPH_STATUS.items()}
def periph_status(code):
    return PERIPH_STATUS[code] if code in PERIPH_STATUS else "UNKNOWN"

# command codes
#
# placeholders, not sent to stm32f103
QUIT_CMD = -1
HELP_CMD = -2
USNG_CMD = -3
WRTY_CMD = -4
CONF_CMD = -11
# sent to stm32f103
IDNT_CMD =  1
VERS_CMD =  2
RSET_CMD =  3
LIVE_CMD =  4
HALT_CMD =  5
DGTL_CMD =  6
ANLG_CMD  = 7
UPLD_CMD =  8
WAVE_CMD =  9
PARL_CMD = 10
USRT_CMD = 11
SPIB_CMD = 12
CNTR_CMD = 13
FLSH_CMD = 14
I2C_CMD  = 15
SRNO_CMD = 16
BLNK_CMD = 17
SIGN_CMD = 0xf2


SIGNATURE = [SIGN_CMD,
             0x9e, 0xc4, 0xaa, 0xdf,
             0xd8, 0xca, 0x8f, 0xbd,
             0xbe, 0xa9, 0xfe, 0x83,
             0x99, 0xd1, 0xae, 0xeb,
             0   , 0   , 0          ]  # pad to mod 4

CONFIG_FILE_EXTENSION = '.b50'

MIN_PULS = 2.0 / CPU_HZ
MAX_PULS = 0x10000 * 0x10000 / CPU_HZ

GNUPLOT_COLORS = [
    'dark-goldenrod',  # 'brown'
    'red'           ,
    'orange'        ,
    'yellow'        ,
    'green'         ,
    'blue'          ,
    'violet'        ,
    'gray80'        ,
]

SAFE_ASCII = [chr(n) if n in range(32,128) else '.' for n in range(256)]



# globals
usb_fd            = -1
firmware_version  = None



# configurations/settings
#
#

class HelpDict(dict):   # just to allow adding '__help' attribute
    pass

channels_config = HelpDict({
     '4' : Text('brn', 16),
     '5' : Text('red', 16),
     '6' : Text('org', 16),
     '7' : Text('yel', 16),
     '8' : Text('grn', 16),
     '9' : Text('blu', 16),
    '10' : Text('vio', 16),
    '11' : Text('gry', 16),
})
channels_config.__help = """
Names for digital input ports PB4 through PB11.
"""
channels_config[ '4']._help = "Symbolic name for input pin PB4"
channels_config[ '5']._help = "Symbolic name for input pin PB5"
channels_config[ '6']._help = "Symbolic name for input pin PB6"
channels_config[ '7']._help = "Symbolic name for input pin PB7"
channels_config[ '8']._help = "Symbolic name for input pin PB8"
channels_config[ '9']._help = "Symbolic name for input pin PB9"
channels_config['10']._help = "Symbolic name for input pin PB10"
channels_config['11']._help = "Symbolic name for input pin PB11"

def channel_names():
    return [text.val for text in channels_config.values()]


configure_loadsave_error     = False
configuration_modified       = False
configure_config = HelpDict({
    'file' : FileName(None, CONFIG_FILE_EXTENSION),
    'trim' :     Trim(1.0 , 1.0                  ),
})
configure_config._actions = {
    'save'   : Action(configure_save, "Save all configurations/parameters "
                                      "to file, name specified with"
                                      "\"file=\" parameter"),
    'load'   : Action(configure_load, "Load all configurations/parameters "
                                      "to file, name specified with"
                                      "\"file=\" parameter"),
    'adjust' : Action(adjust_cpu_hz , "Adjust reported times by \"trim\""
                                     " parameter value"                  ),
}
configure_config.__help = """
Load and/or save all configuration parameters to file
  - Will append ".b50" to file name if missing
  - Supports <TAB> completion of filenames
"""
configure_config['file']._help = "Name of \".b50\" file to load or "        \
                                 "save configurations and parameters. "     \
                                 "\".b50\" suffix will be appended if "     \
                                 "missing. See \"load\" and \"save\" "      \
                                 "actions."
configure_config['trim']._help = "Trim all reported times by factor of "    \
                                 "X/Y where X is currently reported time "  \
                                 "and Y is desired. Use \"adjust\" action " \
                                 "to effect."


digital_config = HelpDict({
    'mode'      : SamplingMode('6.26MHz'                                  ),
    'duration'  : Duration    ('infinite' , (1<<16) / CPU_HZ, 1, 0xffff, 1),
    'edges'     : SpecialInt  ('unlimited', 'unlimited'     , 4, 0xffff   ),
    'code-mem'  : CodeMem     ('flash'                                    ),
    'autodump'  : Able        ('disabled'                                 ),
})
digital_config.__help = """
Digital (logic analyzer) capture parameters.
"""
digital_config['mode']._help = """
Sampling mode
  - digital digital ports PB4(lsb)...PB11(msb)
  - all clock counts @ 72MHz main CPU)
  - see https://github.com/thanks4opensource/buck50/#why_so_slow
  - if "code-mem=flash":
    6.26MHz     : sequence    5@11+1@14 CPU clocks/sample (72*6/69 = 6.26MHz)
    irregular   : alternating   11 + 14  "    "      "    (72*2/25 = 5.76MHz)
    uniform     : constant           15  "    "      "    (72  /15 = 4.80MHz)
    4MHz        : constant           18  "    "      "    (72  /18 = 4.00MHz)
  - if "code-mem=ram":
    6.26MHz     : sequence    5@13+1@17 CPU clocks/sample (72*6/69 = 5.27MHz)
    irregular   : alternating   13 + 17  "    "      "    (72*2/30 = 4.80MHz)
    uniform     : constant           17  "    "      "    (72  /17 = 4.24MHz)
    4MHz        : constant           18  "    "      "    (72  /18 = 3.43MHz)
"""
digital_config['duration']._help = "Sampling time limit"
digital_config['edges'   ]._help = "Maximum number of digital samples "     \
                                    "(including extra timing samples at "   \
                                    "c. 4Hz)"
digital_config['code-mem']._help = "Sampling code memory bank. "            \
                                    "See \"help logic logic\" and "         \
                                    "https://github.com/thanks4opensource/" \
                                    "buck50/#flash_vs_ram"
digital_config['autodump']._help = "Automatically (CAUTION!) do \"dump\" "  \
                                   "command after completion. See "         \
                                   "\"CAUTION!\" in \"help dump "           \
                                   "auto-digital.\""


analog_config = HelpDict({
    'samples'    : SpecialInt ('unlimited', 'unlimited'      , 2, 0xffff   ),
    'trgr-chnl'  : RangeInt   (0          , 0                , 7           ),
    'scnd-chnl'  : SpecialInt ('none'     , 'none'           , 0   , 0xf   ),
    'trigger'    : SclLvlHyst (0.0        , 3.3              , 1.65, 0.05  ),
    'slope'      : Slope      ('positive'                                  ),
    's/h+adc'    : AdcSampHold(AdcSampHold.T_239_5                         ),
    'time-scale' : HelpFloat  (1.0e6                                       ),
    'printf'     : Printf     ("%10.2f μs", 64                             ),
    'autodump'   : Able       ('disabled'                                  ),
})
analog_config.__help = """
Analog DSO (digital storage oscilloscope) parameters
"""
analog_config['samples'   ]._help = "Maximum number of single or dual "     \
                                    "analog samples"
analog_config['trgr-chnl' ]._help = "Port number PA<x> (x=0...7) of "       \
                                    "trigger/first/single channel"
analog_config['scnd-chnl' ]._help = "Port number PA<x> (x=0...7) of "       \
                                    "second channel, or \"none\" for "      \
                                    "single channel sampling"
analog_config['trigger'   ]._help = "Triggering scaling, level, and "       \
                                    "hysteresis. Level and hysteresis "     \
                                    "in scaling (low-to-high) units/range."
analog_config['slope'     ]._help = "Analog triggering slope (or "          \
                                    "\"disabled\" for immediate start "     \
                                    "of sampling)"

analog_config['s/h+adc'   ]._help = "Sampling rate, sample and hold time "  \
                                    "plus analog conversion time. "         \
                                    "Limited set of values supported "      \
                                    "by hardware."
analog_config['time-scale']._help = "Scale factor for time values in "      \
                                    "\"dump\" command output"
analog_config['printf'    ]._help = "C format string for "  \
                                    "time values in \"dump\" command "
analog_config['autodump'  ]._help = "Automatically do \"dump\" "    \
                                    "command after completion"



upload_config = HelpDict({
    'begin'          : RangeInt     (0          , 0          ,    0xffff   ),
    'count'          : SpecialInt   ('unlimited', 'unlimited', 1, 0xffff   ),
    'file'           : FileName     ('/tmp/temp'                           ),
    'digital-frmt'   : DigitalFormat('vcd'                                 ),
    'analog-frmt'    : AnalogFormat ('csv'                                 ),
    'actives'        : ActivePorts  ('4.5.6.7.8.9.10.11'                   ),
    'tick-units'     : TickUnits    ('ns'                                  ),
    'per-tick'       : RangeInt     (125        , 1          ,    int(1e15)),
    'output'         : TermOrFile   ('terminal'                            ),
    'viewer-csv'     : Viewer       ('other'                               ),
    'viewer-vcd'     : Viewer       ('other'                               ),
    'other-csv'      : Text         (''          , 1024                    ),
    'other-vcd'      : Text         (''          , 1024                    ),
    'auto-digital'   : Able         ('disabled'                            ),
    'auto-analog'    : Able         ('disabled'                            ),
    'warn-pulseview' : HelpFloat    ('100e6'                               ),
    'linewidth'      : HelpFloat    (0.0                                   ),
    'pointtype'      : PointType    ('none'                                ),
    'pointsize'      : HelpFloat    (0.0                                   ),
})
upload_config.__help = """
Dump samples to terminal and/or file.
"""
upload_config['begin'         ]._help = "First sample number"
upload_config['count'         ]._help = "Number of samples to upload, "       \
                                        "starting with \"begin=\""
upload_config['file'          ]._help = "File to write samples to"
upload_config['digital-frmt'  ]._help = "Digital dump file format"
upload_config['analog-frmt'   ]._help = "Analog dump file format"
upload_config['actives'       ]._help = "List of \"logic\" ports to dump "    \
                                        "to file (does not affect "           \
                                        "terminal or \"oscope\" output)"
upload_config['tick-units'    ]._help = "Time units per tick for digital "    \
                                        "\"vcd\" format. Note only \"us\", "  \
                                        "\"ns\", and \"fs\" valid for "       \
                                        "`pulseview`. See \"help dump "       \
                                        "per-tick\""
upload_config['per-tick'      ]._help = "Counts per tick for digital "        \
                                        "\"vcd\" format. If using "           \
                                        "\"pulseview\", set this parameter "  \
                                        "and \"tick-units\" as large as "     \
                                        "possible without exceeding the "     \
                                        "shortest anticipated time period "   \
                                        "between samples. See "               \
                                        "https://github.com/thanks"           \
                                        "4opensource/buck50/#pulseview"
upload_config['output'        ]._help = "Dump output destination"
upload_config['viewer-csv'    ]._help = "External viewer program for CSV "    \
                                        "files"
upload_config['viewer-vcd'    ]._help = "External viewer program for VCD "    \
                                        "files"
upload_config['other-csv'     ]._help = "External viewer program for CSV "    \
                                        "files when \"viewer-vcd=other\". "   \
                                        "May contain \"%s\" character for "   \
                                        "filename. Use \\040 for spaces "     \
                                        "between commandline arguments"
upload_config['other-vcd'     ]._help = "External viewer program for VCD "    \
                                        "files when \"viewer-vcd=other\". "   \
                                        "May contain \"%s\" character for "   \
                                        "filename. Use \\040 for spaces "     \
                                        "between commandline arguments"
upload_config['auto-digital'  ]._help = "Automatically (CAUTION!) run "       \
                                        "external viewer program. See "       \
                                        "\"viewer_vcd=\" and "                \
                                        "\"viewer_csv=\" parameters. "        \
                                        "CAUTION! If \"viewer_vcd="           \
                                        "pulseview\", pathological "          \
                                        "\"logic\" sampling captures may "    \
                                        "cause almost unlimited host memory " \
                                        "consumption and possible system "    \
                                        "lockup. See "                        \
                                        "https://github.com/thanks"           \
                                        "4opensource/buck50/#pulseview"
upload_config['auto-analog'   ]._help = "Automatically run external "         \
                                        "viewer program. See "                \
                                        "\"viewer_vcd=\" and "                \
                                        "\"viewer_csv=\" parameters."
upload_config['warn-pulseview']._help = "Warn if estimated \"digital-frmt="   \
                                        "vcd\" memory usage exceeds value "   \
                                        "and might trigger "                  \
                                        "host system slowdown or lockup "     \
                                        "when file loaded into `pulseview`. " \
                                        "See \"dump auto-digital=\" and "     \
                                        "https://github."                     \
                                        "com/thanks4opensource/buck50"        \
                                        "/#pulseview"
upload_config['linewidth'     ]._help = "analog gnuplot line width"
upload_config['pointtype'     ]._help = "analog gnuplot point type"
upload_config['pointsize'     ]._help = "analog gnuplot point size "          \
                                        "(0 for no points)"


triggers_config = HelpDict({
    0 : Trigger('xxxxxxxx-0-0', "immediate trigger (default)")
})
triggers_config._actions = {
    'check'  : Action(check_triggers                                 ,
                     "Check triggers for conflicts"                  ),
    'delete' : Action(lambda : None                                  ,
                      "Delete one or more triggers: n[-m] [p[-q] ..."),
    'undo'   : Action(triggers_undo                                  ,
                      "Undo last trigger command change or delete"   ),
}
triggers_config.__help = """
Command usage:
    trigger <state number> <abcdefgh>-<pass state>-<fail state> [text]
where:
    state number      0...255
    a,b,c,d,e,f,g,h   X = don't care   alternate chars: %s
                      0 = port low     alternate chars: %s
                      1 = port high    alternate chars: %s
    pass state        0...255
    fail state        0...255
    text              optional comments
Type "help trigger"
"""  % (Trigger.IGNORE_CHARS[2:],
        Trigger. ZEROS_CHARS[1:],
        Trigger.  ONES_CHARS[1:])
triggers_backup = HelpDict({
    0 : Trigger('xxxxxxxx-0-0', "immediate trigger (default)")
})



live_config = HelpDict({
    'rate'     : TimeFreqSpecialVal('4Hz'     , 1,          MAX_DURATION      ),
    'duration' : Duration          ('infinite', 1.0/CPU_HZ, 1, MAX_DURATION, 0),
    'pb4-11'   : Able              ('enabled'                                 ),
    'file'     : FileName          (None                                      ),
    'output'   : TermFileBoth      ('both'                                    ),
    'printf'   : Printf            ('%7.3f'   , 64                            ),
})
live_config['rate'    ]._help = "Max update frequency. "                 \
                                "See \"Time/Frequency Errors\" section " \
                                "in \"help\"."
live_config['duration']._help = "Halt after time elapsed"
live_config['pb4-11'  ]._help = "Monitor/log digital inputs"
live_config['file'    ]._help = "Log file name"
live_config['output'  ]._help = "Log to file (\"file\" or \"both\") only if " \
                                "\"file=\" is also set)"
live_config['printf'  ]._help = "C format string for time (seconds)."
live_config.__help = """
"""



pulse_config_freq = TimeFreqPscArrVal('1kHz', MIN_PULS, MAX_PULS)
pulse_config = HelpDict({
    'active' : Able        ('disabled'              ),
    'time'   :              pulse_config_freq        ,
    'time1'  : TimePcntVal ('25%', pulse_config_freq),
    'time2'  : TimePcntVal ('50%', pulse_config_freq),
    'time3'  : TimePcntVal ('75%', pulse_config_freq),
    'mode1'  : PulseMode   ('rise'                  ),
    'mode2'  : PulseMode   ('rise'                  ),
    'mode3'  : PulseMode   ('rise'                  ),
    'gpio1'  : OpenDrainHiZ('push-pull'             ),
    'gpio2'  : OpenDrainHiZ('push-pull'             ),
    'gpio3'  : OpenDrainHiZ('push-pull'             ),
    'speed1' : GpioSpeed   ('2MHz'                  ),
    'speed2' : GpioSpeed   ('2MHz'                  ),
    'speed3' : GpioSpeed   ('2MHz'                  ),
})
pulse_config._actions = {
    'enable'  : Action(lambda : config_set(pulse_config, 'active', 'enabled'),
                       "Set parameter \"active=enabled\""                    ),
    'disable' : Action(lambda : config_set(pulse_config, 'active', 'disabled'),
                       "Set parameter \"active=disabled\""                   ),
}
pulse_config.__help = """
Parameters "mode1", "mode2", "mode3" control output levels on PA1/PA2/PA3 ports at "time1", "time2", "time3", all within master "time" period.
"""
pulse_config['active']._help = "If \"enabled\" runs (even after "           \
                               " program exit) until halted with "          \
                               "\"disabled\" or by \"gpio\", \"numbers\" "  \
                               "or \"usart ports=pa0-3\" command."
pulse_config['time'  ]._help = "Master period/frequency for time1, "        \
                               "time2, and time3. "                         \
                               "See \"Time/Frequency Errors\" section "     \
                               "in \"help\"."
pulse_config['time1' ]._help = "Time for \"mode1\" event type. If  "        \
                               "\"%\"  suffix is percent of master "        \
                               "\"time\" parameter. If explicit "           \
                               "time/freq must be less than or "            \
                               "equal to \"time\" (greater than or "        \
                               "equal to frequency), and will be "          \
                               "clamped if \"time\" changes. "              \
                               "See \"Time/Frequency Errors\" section "     \
                               "in \"help\"."
pulse_config['time2' ]._help = "See \"help pulse time1\""
pulse_config['time3' ]._help = "See \"help pulse time1\""
pulse_config['mode1' ]._help = "Port PA1 change at \"time1\":\n"            \
                                "  frozen : no change\n"                    \
                                "      up : set high\n"                     \
                                "    down : set low\n"                      \
                                "  toggle : set high if low, or set "       \
                                "low if high (no change at "                \
                                "master \"time\")\n"                        \
                                "    high : continuously high\n"            \
                                "     low : continuously low\n"             \
                                "    fall : high-to-low (return high "      \
                                "at master \"time\")\n"                     \
                                "    rise : low-to-high (return low "       \
                                "at master \"time\")\n"
pulse_config['mode2' ]._help = "See \"help pulse mode1\""
pulse_config['mode3' ]._help = "See \"help pulse mode1\""
pulse_config['gpio1' ]._help = "Disable (\"hi-z\") or enable "              \
                                "\"push-pull\" or \"open-drain\") "         \
                                "on port PA1"
pulse_config['gpio2' ]._help = "See \"help pulse gpio1\""
pulse_config['gpio3' ]._help = "See \"help pulse gpio1\""
pulse_config['speed1']._help = "Port PA1 slew rate"
pulse_config['speed2']._help = "See \"help pulse speed1\""
pulse_config['speed3']._help = "See \"help pulse speed1\""



parallel_config = HelpDict({
    'speed'     : GpioSpeed         ('2MHz'               ),
    'open-pull' : OpenDrain         ('push-pull'          ),
    'rate'      : TimeFreqSpecialVal('10Hz', 1, 0xffffffff),
    'ascii-num' : AsciiNumeric      ('numeric'            ),
    'end'       : Text              ('END' , MAX_END_STR  ),
})
parallel_config.__help = """
Configuration for Interactive 8-bit parallel data (ports PA4(lsb) through PA11(msb).
"""
parallel_config['speed'    ]._help = "Output port slew rate"
parallel_config['open-pull']._help = "Output port mode"
parallel_config['rate'     ]._help = "Byte output data rate. "              \
                                     "See \"Time/Frequency Errors\" "       \
                                     "section in \"help\"."
parallel_config['ascii-num']._help = "Terminal input/output mode"
parallel_config['end'      ]._help = "Character sequence to exit command "  \
                                     "in \"ascii-num=ascii\" mode"


usart_config = HelpDict({
    'active'    : Able              ('disabled'                            ),
    'xmit'      : Able              ('enabled'                             ),
    'recv'      : Able              ('enabled'                             ),
    'ports'     : UsartPorts        ('pa8-10'                              ),
    'baud'      : UsartBaud         ('9600Hz'      , 16, 0xffff            ),
    'datalen'   : UsartDataBits     ('8bits'                               ),
    'stoplen'   : UsartStopBits     ('1bit'                                ),
    'parity'    : UsartParity       ('none'                                ),
    'synchro'   : Able              ('disabled'                            ),
    'idle'      : SerialIdle        ('low'                                 ),
    'phase'     : SerialPhase       ('1st'                                 ),
    'lastclok'  : Able              ('enabled'                             ),
    'gpio'      : GpioSpeed         ('2MHz'                                ),
    'rate'      : TimeFreqSpecialVal('unlimited'   , 1 , 0xffffffff        ),
    'tx-timeout': TimeSpecialVal    ('10ms'        , 1 , 0xffffffff, 'none'),
    'rx-wait'   : TimeSpecialVal    ('10ms'        , 1 , 0xffffffff, 'none'),
    'tx-data'   : RangeInt          (0             , 0 , 0xff              ),
    'rx-len'    : RangeInt          (MAX_USB_BRIDGE, 1 , MAX_USB_BRIDGE    ),
    'cts'       : Able              ('disabled'                            ),
    'rts'       : Able              ('disabled'                            ),
    'ascii-num' : AsciiNumeric      ('ascii'                               ),
    'end'       : Text              ('END'         , MAX_END_STR           ),
    'snoop'     : Snoop             ('disabled'                            ),
})
usart_config._actions = {
    'enable'  : Action(lambda : config_set(usart_config, 'active', 'enabled'),
                       "Set parameter \"active=enabled\""                    ),
    'disable' : Action(lambda : config_set(usart_config, 'active', 'disabled'),
                       "Set parameter \"active=disabled\""                   ),
}
usart_config.__help = """
Configuration for async/sync (UART/USART) serial peripheral on ports PA9(TX), PA10(RX) and PA8(clock) (if "ports=pa8-10") or ports PA2(TX), PA3(RX), PA1(RTS) and PA0(CTS) (if "ports=pa0-3")
  - Note "datalen=7bits" with "parity=none", or "datalen=9bits" with other than "parity=none," not supported.

"""
usart_config['active'    ]._help = "Used by monitor command"
usart_config['xmit'      ]._help = "Transmit RX data on port PA9"
usart_config['recv'      ]._help = "Receive RX data on port PA10"
usart_config['ports'     ]._help = "Select one of two hardware USARTs "     \
                                   "(by PAx-y port numbers)"
usart_config['snoop'     ]._help = "Print socket traffic to terminal when " \
                                   "\"socket_config[active]=enabled\" "     \
                                   "and/or \"pty_config[active]=enabled\""
usart_config['baud'      ]._help = "Baud rate for both xmit and recv if "   \
                                   "\"synchro=enabled\", only xmit if "     \
                                   "\"disabled\". "                         \
                                   "See \"Time/Frequency Errors\" section " \
                                   "in \"help\"."
usart_config['datalen'   ]._help = "Number of data bits. See \"help "       \
                                   "usart usart\" for restrictions with "   \
                                   "\"parity=\"."
usart_config['stoplen'   ]._help = "Number of stop bits"
usart_config['parity'    ]._help = "Parity. See \"help usart usart\" "      \
                                   "for restrictions with \"datalen=\"."
usart_config['synchro'   ]._help = "Async(UART)(no clock) or "              \
                                   "synchronous(USART) mode"
usart_config['idle'      ]._help = "Clock output level at idle in "         \
                                   "\"synchro=enabled\" mode"
usart_config['phase'     ]._help = "Clock edge to sample bit at in "        \
                                   "\"synchro=enabled\" mode"
usart_config['lastclok'  ]._help = "Last clock pulse in "                   \
                                   "\"synchro=enabled\" mode"
usart_config['gpio'      ]._help = "Ports output slew rate "
usart_config['rate'      ]._help = "Byte output data rate. "                \
                                   "See \"Time/Frequency Errors\" "         \
                                   "section in \"help\"."
usart_config['tx-timeout']._help = "Wait for hardware TX ready. "           \
                                   "See \"Time/Frequency Errors\" section " \
                                   "in \"help\"."
usart_config['rx-wait'   ]._help = "Wait for hardware RX ready. "           \
                                   "See \"Time/Frequency Errors\" section " \
                                   "in \"help\"."
usart_config['tx-data'   ]._help = "Byte to xmit in \"monitor\" command "   \
                                   "if \"synchro=enabled\""
usart_config['cts'       ]._help = "CTS handshaking on port PA0 (only "     \
                                   "supported for \"usart=pa0-3\""
usart_config['rts'       ]._help = "RTS handshaking on port PA1 (only "     \
                                   "supported for \"usart=pa0-3\""
usart_config['end'       ]._help = "Character sequence to exit command "    \
                                   "in \"ascii-num=ascii\" mode"


spi_config = HelpDict({
    'mode'      : SpiI2cMode        ('disabled'                             ),
    'xmit-only' : Able              ('disabled'                             ),
    'snoop'     : Snoop             ('disabled'                             ),
    'select'    : SpiSelect         ('software'                             ),
    'baud'      : SpiBaud           ('281.25KHz'                            ),
    'endian'    : SpiEndian         ('msb'                                  ),
    'idle'      : SerialIdle        ('low'                                  ),
    'phase'     : SerialPhase       ('1st'                                  ),
    'miso'      : OpenDrain         ('open-drain'                           ),
    'pull'      : Pull              ('floating'                             ),
    'speed'     : GpioSpeed         ('2MHz'                                 ),
    'nss'       : Nss               ('floating'                             ),
    'tx-data'   : DataBytes         (b'\x00'    , 1,   32                   ),
    'rate'      : TimeFreqSpecialVal('unlimited', 1  , 0xffffffff           ),
    'busy-wait' : TimeSpecialVal    ('1ms'      , 1  , 0xffffffff,'infinite'),
    'rx-wait'   : TimeSpecialVal    ('10ms'     , 1  , 0xffffffff, 'none'   ),
    'nss-time'  : TimeSpecialVal    ('zero'     , 1  , 0xffffffff, 'zero'   ),
    'ascii-num' : AsciiNumeric      ('numeric'                              ),
    'end'       : Text              ('END'      , MAX_END_STR               ),
})
spi_config._actions = {
    'disable' : Action(lambda : config_set(spi_config, 'mode', 'disabled'),
                       "Set parameter \"active=disabled\""               ),
    'master'  : Action(lambda : config_set(spi_config, 'mode', 'master'  ),
                       "Set parameter \"active=master\""                 ),
    'slave'   : Action(lambda : config_set(spi_config, 'mode', 'slave'   ),
                       "Set parameter \"active=slave\""                  ),
}
spi_config.__help = """
Configuration for SPI serial peripheral on ports PA4(NSS), PA5(clock/SCK), PA6(MISO) and PA7(MOSI)
Slave select:
  Slave ("mode=slave"):
    - "select=software"  slave enabled/active
    - "select=hardware"  slave controlled by level on NSS port (port PA4)
  Master ("mode=master") NSS pin (port PA4):
    - "nss=floating"     inactive/hi-Z
    - "nss=low"          set low (any connected slave selected)
    - "nss-active"       set high at idle, goes low "nss-time=<value>" before
                         first clock (port PA5) edge and stays low until
                         goes high "nss-time=<value>" after MOSI data finished
"""
spi_config['mode'     ]._help = "SPI mode, or \"disabled\""
spi_config['xmit-only']._help = "Ignore MISO data in \"spi\" command, "     \
                                "\"mode=master\""
spi_config['snoop'    ]._help = "Print socket traffic to terminal when "    \
                                "\"socket_config[active]=enabled\" "        \
                                "and/or \"pty_config[active]=enabled\""
spi_config['select'   ]._help = "See \"help spi spi\""
spi_config['baud'     ]._help = "One of fixed set of supported baud rates"
spi_config['endian'   ]._help = "Bit order for MOSI and MISO data"
spi_config['idle'     ]._help = "Clock port PA5 level at idle, "            \
                                "\"mode=master\""
spi_config['phase'    ]._help = "Clock edge at which data bit sampled"
spi_config['miso'     ]._help = "Slave (\"mode=slave\"): MISO port PA6 "    \
                                "hardware mode. Set \"open-drain\" with "   \
                                "external pull-up if multiple hardware "    \
                                "slaves, else \"push-pull\"."
spi_config['pull'     ]._help = "Master (\"mode=master\"): "                \
                                "MISO (port PA6). Use \"pull=floating\" "   \
                                "if single connected slave MISO is "        \
                                "push-pull or external pull-up on MISO "    \
                                "line, else \"pull=up\". Do not use "       \
                                "\"pull=down\"."
spi_config['speed'    ]._help = "Output ports (PA7=MOSI, PA5=clock) slew rate"
spi_config['nss'      ]._help = "See \"help spi spi\""
spi_config['tx-data'  ]._help = "NEED UPDATE: \"mode=slave\" MISO data byte for "       \
                                "\"monitor\" command, and for \"spi\" "     \
                                "command if interactive input "             \
                                "empty/exhausted"
spi_config['rate'     ]._help = "MOSI data output rate. "                   \
                                "See \"Time/Frequency Errors\" "            \
                                "section in \"help\"."
spi_config['busy-wait']._help = "Wait time for hardware TX not ready. "     \
                                "See \"Time/Frequency Errors\" section "    \
                                "in \"help\"."
spi_config['rx-wait'  ]._help = "Delay for concatenating MISO data into "   \
                                "single message to host, \"spi\" command. " \
                                "See \"Time/Frequency Errors\" section "    \
                                "in \"help\"."
spi_config['nss-time' ]._help = "See \"help spi spi\". "                    \
                                "See \"Time/Frequency Errors\" section "    \
                                "in \"help\"."
spi_config['end'      ]._help = "Character sequence to exit command "       \
                                   "in \"ascii-num=ascii\" mode"



i2c_config = HelpDict({
    'mode'      : SpiI2cMode    ('disabled'                            ),
    'flavor'    : I2cStdFast    ('standard'                            ),
    'fast-duty' : I2cDuty       ('2:1'                                 ),
    'freq'      : I2cSpeed      ('100KHz'                              ),
    'addr'      : RangeInt      (0                        , 0, 127     ),
    'gen-call'  : Able          ('disabled'                            ),
    'oar1'      : RangeInt      (8                        , 8, 119     ),
    'oar2'      : SpecialInt    ('disabled', 'disabled'   , 8, 119, 255),
    'tx-data'   : DataBytes     (b'\x00'   , 1            , 16         ),
    'rx-size'   : RangeInt      (0         , 0            , 60         ),
    'timeout'   : TimeSpecialVal('100ms'   , 1, 0xfffffffd, 'infinite' ),
    'gpio'      : GpioSpeed     ('2MHz'                                ),
})
i2c_config._actions = {
    'disable' : Action(lambda : config_set(i2c_config, 'mode', 'disabled'),
                       "Set parameter \"active=disabled\""               ),
    'master'  : Action(lambda : config_set(i2c_config, 'mode', 'master'  ),
                       "Set parameter \"active=master\""                 ),
    'slave'   : Action(lambda : config_set(i2c_config, 'mode', 'slave'   ),
                       "Set parameter \"active=slave\""                  ),
}
i2c_config.__help = """
Configuration for I2C serial peripheral on ports PB10(clock/SCL) and PB11(data/SDA)
"""
i2c_config['mode'     ]._help = "I2C mode, or \"disabled\""
i2c_config['flavor'   ]._help = "I2C protocol mode"
i2c_config['fast-duty']._help = "Duty cycle for I2C clock in "          \
                                "\"flavor=fast\" mode"
i2c_config['freq'     ]._help = "I2C clock frequency (only used for "   \
                                "\"mode=master\". "                     \
                                "See \"Time/Frequency Errors\" "        \
                                "section in \"help\"."
i2c_config['addr'     ]._help = "Master TX address for \"monitor\" "    \
                                "command"
i2c_config['gen-call' ]._help = "Slave accepts \"general call\" "       \
                                "address (0) in addition to \"oar1=\" " \
                                "and \"oar2=\""
i2c_config['oar1'     ]._help = "Slave address"
i2c_config['oar2'     ]._help = "Optional second slave address, or "    \
                                "\"disabled\""
i2c_config['tx-data'  ]._help = "Data for master TX in \"monitor\" "    \
                                "command, or slave TX in \"i2c\" "      \
                                "command if no data enqueued "          \
                                "interactively in \"i2c\" command"
i2c_config['rx-size'  ]._help = "Maximum size of slave RX data (any "   \
                                "additonal discarded) in \"monitor\" "  \
                                "command"
i2c_config['timeout'  ]._help = "Timeout value to abort firmware-to-"   \
                                "hardware commands if no response. "    \
                                "See \"Time/Frequency Errors\" section "\
                                "in \"help\"."
i2c_config['gpio'     ]._help = "I/O port output slew rate, PB11(SDA) " \
                                "(master or slave) and PB10(SCL) for "  \
                                "master"



adc_0_config = HelpDict({
    'active'     : Able       ('disabled'                    ),
    's/h+adc'    : AdcSampHold(AdcSampHold.T_239_5           ),
    'scale-hyst' : ScaleHyst  (0.0                , 3.3 , 0.0),
    'exponent'   : RangeInt   (1                  , 0   , 8  ),
    'weight'     : RangeInt   (1                  , 0   , 255),
    'name'       : Text       ('PA0'              , 16       ),
    'printf'     : Printf     ('%5.3fV'           , 32       ),
})
adc_0_config._actions = {
    'enable'  : Action(lambda : config_set(adc_0_config, 'active', 'enabled' ),
                       "Set parameter \"active=enabled\""                     ),
    'disable' : Action(lambda : config_set(adc_0_config, 'active', 'disabled'),
                       "Set parameter \"active=disabled\""                    ),
}
adc_0_config.__help = """
Analog-to-digital input on port PA0.
Filtering/scaling:
  - Filtering for "monitor" command (computed by firmware on STM32F103):
      new_filtered_value =   prev_filtered_value * (pow(2,exponent) - weight)
                           + current_adc_value   * (                  weight)
    but then not reported unless:
      abs(new_filtered_value - prev_reported_value) > hysteresis
  - Scaling for "monitor" and "dump" commands (computed by software on host):
      normalized = value / float(max_12_bit_adc_value)
      scaled     = (1.0 - normalized) * lowscale + normalized * highscale
"""
adc_0_config['active'    ]._help = "For \"monitor\" command"
adc_0_config['s/h+adc'   ]._help = "Sampling rate, sample and hold time "   \
                                   "plus analog conversion time. "          \
                                   "Limited set of values supported "       \
                                   "by STM32F103."
adc_0_config['scale-hyst']._help = "Scaling units/range for \"monitor\" "   \
                                   "and \"dump\" commands, and hysteresis " \
                                   "value for \"monitor\""
adc_0_config['exponent'  ]._help = "Filter width. See \"analog\" section "  \
                                   "in \"help monitor\"."
adc_0_config['weight'    ]._help = "Filter width. See \"analog\" section "  \
                                   "in \"help monitor\"."
adc_0_config['name'      ]._help = "Port/channel name for \"monitor\", "    \
                                   "\"oscope\", and \"dump\" commands' "    \
                                   "output."
adc_0_config['printf'    ]._help = "C format string to print scaled "       \
                                   "value. For \"monitor\" and \"oscope\" " \
                                   "commands"
adc_1_config = copy.deepcopy(adc_0_config)
adc_2_config = copy.deepcopy(adc_0_config)
adc_3_config = copy.deepcopy(adc_0_config)
adc_4_config = copy.deepcopy(adc_0_config)
adc_5_config = copy.deepcopy(adc_0_config)
adc_6_config = copy.deepcopy(adc_0_config)
adc_7_config = copy.deepcopy(adc_0_config)

adc_1_config['name'] = Text('PA1', 16)
adc_2_config['name'] = Text('PA2', 16)
adc_3_config['name'] = Text('PA3', 16)
adc_4_config['name'] = Text('PA4', 16)
adc_5_config['name'] = Text('PA5', 16)
adc_6_config['name'] = Text('PA6', 16)
adc_7_config['name'] = Text('PA7', 16)

adc_1_config._actions = {
    'enable'  : Action(lambda : config_set(adc_1_config, 'active', 'enabled' ),
                       "Set parameter \"active=enabled\""                     ),
    'disable' : Action(lambda : config_set(adc_1_config, 'active', 'disabled'),
                       "Set parameter \"active=disabled\""                    ),
}
adc_2_config._actions = {
    'enable'  : Action(lambda : config_set(adc_2_config, 'active', 'enabled' ),
                       "Set parameter \"active=enabled\""                     ),
    'disable' : Action(lambda : config_set(adc_2_config, 'active', 'disabled'),
                       "Set parameter \"active=disabled\""                    ),
}
adc_3_config._actions = {
    'enable'  : Action(lambda : config_set(adc_3_config, 'active', 'enabled' ),
                       "Set parameter \"active=enabled\""                     ),
    'disable' : Action(lambda : config_set(adc_3_config, 'active', 'disabled'),
                       "Set parameter \"active=disabled\""                    ),
}
adc_4_config._actions = {
    'enable'  : Action(lambda : config_set(adc_4_config, 'active', 'enabled' ),
                       "Set parameter \"active=enabled\""                     ),
    'disable' : Action(lambda : config_set(adc_4_config, 'active', 'disabled'),
                       "Set parameter \"active=disabled\""                    ),
}
adc_5_config._actions = {
    'enable'  : Action(lambda : config_set(adc_5_config, 'active', 'enabled' ),
                       "Set parameter \"active=enabled\""                     ),
    'disable' : Action(lambda : config_set(adc_5_config, 'active', 'disabled'),
                       "Set parameter \"active=disabled\""                    ),
}
adc_6_config._actions = {
    'enable'  : Action(lambda : config_set(adc_6_config, 'active', 'enabled' ),
                       "Set parameter \"active=enabled\""                     ),
    'disable' : Action(lambda : config_set(adc_6_config, 'active', 'disabled'),
                       "Set parameter \"active=disabled\""                    ),
}
adc_7_config._actions = {
    'enable'  : Action(lambda : config_set(adc_7_config, 'active', 'enabled' ),
                       "Set parameter \"active=enabled\""                     ),
    'disable' : Action(lambda : config_set(adc_7_config, 'active', 'disabled'),
                       "Set parameter \"active=disabled\""                    ),
}

adc_configs = (
    adc_0_config,
    adc_1_config,
    adc_2_config,
    adc_3_config,
    adc_4_config,
    adc_5_config,
    adc_6_config,
    adc_7_config,
)



counter_config = HelpDict({
    'mode'       : BinaryGray        ('binary'               ),
    'increment'  : RangeInt          (1    , -127, 127       ),
    'low'        : RangeInt          (0    , 0,    255       ),
    'high'       : RangeInt          (255   ,0,    255       ),
    'rate'       : TimeFreqSpecialVal('4Hz', 1   , 0xffffffff),
    'gpio-speed' : GpioSpeed         ('2MHz'                 ),
    'open-pull'  : OpenDrain         ('push-pull'            ),
    'limited'    : Duration          ('infinite',
                                      1.0 / CPU_HZ, 1         ,
                                      (1 << 64) - 1           ,
                                      0                      ),
    'unlimited'  : Duration          ('infinite'              ,
                                      (1<<16) / CPU_HZ        ,
                                      1                       ,
                                      0xffff                  ,
                                      1                      ),
})
counter_config.__help = """
"limited="   duration used if "rate=unlimited"
"unlimited=" duration otherwise
"""
counter_config['mode'      ]._help = "Numeric sequence encoding"
counter_config['increment' ]._help = "Change between successive values"
counter_config['low'       ]._help = "Low value of low-to-high values range"
counter_config['high'      ]._help = "High value of low-to-high values range"
counter_config['rate'      ]._help = "Frequency/speed if not \"rate= "      \
                                     "unlimited\". "                        \
                                     "See \"Time/Frequency Errors\" "       \
                                     "section in \"help\". "            \
                                     "Additional rate inaccuracies due to " \
                                     "firmware speed limitations at rates " \
                                     "greater than approx 100kHz."
counter_config['gpio-speed']._help = "Hardware slew rate"
counter_config['open-pull' ]._help = "Hardware port mode"
counter_config['limited'   ]._help = "Command duration if \"rate=limited\""
counter_config['unlimited' ]._help = "Command duration if \"rate=unlimited\""



reset_config = HelpDict({
    'ganged'  : Able    ('disabled'               ),
    'wait'    : RangeInt(2,    0,  2              ),
    'pre'     : Able    ('enabled'                ),
    'usb'     : FileName('/dev/tty/ACM0'          ),
    'timeout' : Duration('1s', 1 , 0, MAX_DURATION),
})
reset_config._actions = {
    'ext-trig' : Action( lambda
                        : os.write(usb_fd,
                                   struct.pack('4B'                      ,
                                               RSET_CMD                  ,
                                               reset_config['ganged'].val,
                                               0, 0                      )),
                        "Activates or de-activates external trigger sync "
                        "after setting \"ganged=\" parameter (\"enabled\" "
                        "or \"disabled\"). See \"help reset ganged=\". "
                        "Also resets all PAx and PBx ports to default."       ),
    'flash'    : Action(lambda :
                        os.write(usb_fd,
                                 struct.pack('4B',
                                             FLSH_CMD                ,
                                             reset_config['wait'].val,
                                             reset_config['pre' ].val,
                                             0                       )),
                        "Set flash memory wait states and prefetch buffering."
                        " Must be after \"wait=\" and \"pre=\" parameters."   ),
    'halt'      : Action(lambda : cmnd_cmd(HALT_CMD)                          ,
                         "Send halt command to firmware"                      ),
    'flush'     : Action(lambda : flush_read( reset_config['timeout']
                                             .as_float()             )        ,
                           "Flush USB data pipe (%s)"
                         % reset_config['usb']                                ),
    'init'      : Action(firmware_connect,
                         "Send initialization commands to firmware"           ),
    'connect'   : Action(reset_connect                                        ,
                         "Disconnect/re-connect to same or different USB port "
                         "/ device driver (first set \"usb\" parameter "
                         "if changing)"                                       ),
    'serialnum' : Action(serial_number                                        ,
                         "Get device serial number"                           ),
    'blink'     : Action(blink_user_led                                       ,
                         "Blink user LED to identify device"                  ),
}
reset_config.__help = """
- Set any/all of "halt=", "flush=", "reconnect=", and/or "init="  to "enabled"
- Will execute in above order
- All reset to "disabled" after command completion.
"""
reset_config['ganged']._help = "Synchronized digital (\"logic\") and/or "   \
                               "analog (\"oscope\") command triggering "    \
                               " across multiple devices. Requires two "    \
                               "open-drain bus lines, each with pull-up "   \
                               "resistor to Vdd. Connect all devices' "     \
                               "PB14 ports to one open-drain bus, and all " \
                               "PB15 ports to other. After enabling with "  \
                               "\"ganged=enabled\" must be activated via "  \
                               "\"reset ext-trig\" action before \"logic\" "\
                               "or \"oscope\" commands. Triggering on any " \
                               "device will trigger all."
reset_config['usb'    ]._help = "USB port and device driver"
reset_config['timeout']._help = "Timeout value for reset USB operations "   \
                                "(can interrupt by <ENTER> key"
reset_config['wait'  ]._help =  "Flash memory wait states. Stability "      \
                                "unlikely if less than default "            \
                                "value of 2. Use \"flash\" action "         \
                                "to set."
reset_config['pre'   ]._help =  "Flash memory prefetch buffer. "            \
                                "Performance impacted if not enabled. "     \
                                "Use \"flash\" action to set"



ipc_config = HelpDict({
    'i/o'   : TermPtySock('terminal'                  ),
    'flush' : TimeVal    ('1s'     , 1.0        , 60.0),
})
ipc_config._actions = {
    'terminal' : Action(lambda : config_set(ipc_config, 'i/o', 'terminal'),
                          "Set parameter \"i/o=terminal\""                ),
    'external' : Action(lambda : config_set(ipc_config, 'i/o', 'external'),
                          "Set parameter \"i/o=external\""                 ),
}
ipc_config.__help = """
Enable/disable IPC (pty and/or socket) I/O
  - Also see "help <command> pty" and "help <command> socket"
"""
ipc_config['flush']._help = "Delay time to flush pending received "     \
                            "device-to-host data at command exit"
ipc_config['i/o'  ]._help = "I/O to terminal or to pty and/or socket "  \
                            "(set \"pty enable\" and/or "               \
                            "\"socket enable\")"


socket_config = HelpDict({
    'active' : Able    ('disabled'        ),
    'host'   : Text    ('localhost', 63   ),
    'port'   : RangeInt(1024, 1024 , 65535),
    'snoop'  : Snoop   ('disabled'        ),
})
socket_config._actions = {
    'enable'  : Action(lambda : config_set(socket_config, 'active', 'enabled' ),
                       "Set parameter \"active=enabled\""                     ),
    'disable' : Action(lambda : config_set(socket_config, 'active', 'disabled'),
                       "Set parameter \"active=disabled\""                    ),
}
socket_config.__help = """
Connect USB I/O to Unix socket.
  - Socket is server, supports connections from multiple external clients
  - Set "host" and "port" parameters
  - Connect via external client program, e.g. `telnet %s %d`
  - Must also set "ipc external"
  - Set "snoop" parameter to view xmit/recv data
""" % (socket_config['host'].val, socket_config['port'].val)
socket_config['active']._help = "Enable/disable I/O to socket"
socket_config['host'  ]._help = "Server socket hostname"
socket_config['port'  ]._help = "Server socket port number"
socket_config['snoop' ]._help = "Print socket traffic to terminal"



pty_config = HelpDict({
    'active' : Able ('disabled'),
    'snoop'  : Snoop('disabled'),
})
pty_config._actions = {
    'enable'  : Action(lambda : config_set(pty_config, 'active', 'enabled'),
                       "Set parameter \"active=enabled\""                   ),
    'disable' : Action(lambda : config_set(pty_config, 'active', 'disabled'),
                       "Set parameter \"active=disabled\""                  ),
}
pty_config.__help = """
Connect USB I/O to /dev/pty/... special device file.
  - Connect external program to /dev/pts/N ("N" printed to terminal at startup)
  - Example: `miniterm /dev/pts/32`
  - Pty stty/termios values not passed through to usart -- use usart config parameters (see "help usart usart") to set baud, parity, etc.
  - Must also set "ipc external"
  - Set "snoop=enabled" parameter to view xmit/recv data
"""
pty_config['active']._help = "Create, and I/O to, /dev/pts/N"
pty_config['snoop' ]._help = "Print socket traffic to terminal"




SETTINGS = {
    'configure' : configure_config,
    'monitor'   :      live_config,
    'pulse'     :     pulse_config,
    'numbers'   :   counter_config,
    'gpio'      :  parallel_config,
    'usart'     :     usart_config,
    'spi'       :       spi_config,
    'i2c'       :       i2c_config,
    'adc0'      :     adc_0_config,
    'adc1'      :     adc_1_config,
    'adc2'      :     adc_2_config,
    'adc3'      :     adc_3_config,
    'adc4'      :     adc_4_config,
    'adc5'      :     adc_5_config,
    'adc6'      :     adc_6_config,
    'adc7'      :     adc_7_config,
    'logic'     :   digital_config,
    'oscope'    :    analog_config,
    'dump'      :    upload_config,
    'lines'     :  channels_config,
    'triggers'  :  triggers_config,
    'reset'     :     reset_config,
    'socket'    :    socket_config,
    'pty'       :       pty_config,
    'ipc'       :       ipc_config,
}




### utils
#
#

def pack_bits(*values):
    return functools.reduce(lambda bits, value: (bits << 1) | bool(value),
                            values                                       ,
                            0                                            )


def cur_chans():
    return  "names  " + " ".join([   "%s=%s" % key_val
                                 for key_val
                                 in  channels_config.items()])
def chan_usage():
    return CHAN_USAGE % cur_chans()


def encode_escape(text):
    return text.encode('utf-8').decode('unicode_escape').encode('latin-1')


def decode_escape(byts):
    # return byts.decode('utf-8', 'backslashreplace')   # < 32 prints as glyphs
    # return str(byts)[2:-1]   # all <32 or >127 print as
                               #   \xNM (including newline, etc)
    return ''.join([         chr(byt)
                        if   byt in range(32, 127)
                        else ("\\x%02x" % byt)
                    for byt in byts                   ])



def r_u_sure(question):
    Pager()("%s (Y/n): " % question, immed=True, final=False, one_line=True)
    sys.stdout.flush()
    try:
        answer = sys.stdin.readline()
    except KeyboardInterrupt:
        return False
    return answer[0] == 'Y'



def check_data_socket_pty():
    if            ipc_config['i/o'   ].str() == 'external'  \
       and not    pty_config['active'].val                  \
       and not socket_config['active'].val                :
        Pager(stream=sys.stderr)("Must set \"socket enable\" "
                                 "and/or \"pty enable\" if "
                                 "\"ipc i/o=external\"",
                                 immed=True, one_line=True    )
        return False
    return True



def check_spi():
    if spi_config['mode'].str() != 'master':
        return True
    abort = False
    if spi_config['pull'].str() == 'down':
        if not r_u_sure("SPI master mode MISO pin \"pull=\" is \"down\" "
                        "(should be \"floating\" or \"up\"). Continue?"):
            abort = True
    if     spi_config['nss-time'].val   != 0         \
       and spi_config['nss'     ].str() != 'active':
        if not r_u_sure("SPI master \"nss-time=\" is not \"unlimited\" and "
                        "\"nss=\" is not \"active\". Continue?"             ):
            abort = True

    if     spi_config['select'].val == SpiSelect.HARDWARE \
       and spi_config['nss'   ].val ==       Nss.FLOATING:
        if not r_u_sure("SPI master \"select=hardware\" and "
                        "\"nss=floating\". Continue?"        ):
            abort = True
    if abort:
        sys.stdout.write("Aborting command\n")
        return False
    else:
        return True


def st_usart_settings():
    if     usart_config['parity' ].val == UsartParity  .NONE  \
       and usart_config['datalen'].val == UsartDataBits.BITS_7:
        Pager(stream=sys.stderr)("Error: Unsupported USART configuration "
                                 "\"datalen=7bits\" with \"parity=none\".",
                                 immed=True, one_line=True, indent=7      )
        return None
    if     usart_config['parity' ].val != UsartParity  .NONE  \
       and usart_config['datalen'].val == UsartDataBits.BITS_9:
        Pager(stream=sys.stderr)("Error: Unsupported USART configuration "
                                 "\"datalen=9bits\" with \"parity=even\" "
                                 "or \"parity=odd\"."                     ,
                                 immed=True, one_line=True, indent=7      )
        return None
    if   usart_config['datalen'].val == UsartDataBits.BITS_7:
        return (UsartDataBits.BITS_8, usart_config['parity'].val)
    elif usart_config['datalen'].val == UsartDataBits.BITS_9:
        return (UsartDataBits.BITS_9, UsartParity.NONE)
    else:
        if usart_config['parity'].val == UsartParity.NONE:
            return (UsartDataBits.BITS_8, UsartParity.NONE)
        else:
            return (UsartDataBits.BITS_9, usart_config['parity'].val)



def check_usart():
    if      usart_config['ports'].val == UsartPorts.PA_8_10          \
       and (usart_config['cts'  ].val or usart_config['cts'].val):
        Pager(stream=sys.stderr)("Warning: Ignoring usart \"cts=enabled\" "
                                 "and/or \"rts=enabled\" with \"usart="
                                 "pa8-10\" because CTS/RTS only available "
                                 "for \"usart=pa0-3\""                     ,
                                 immed=True, one_line=True, indent=9       )
    if     usart_config['ports'  ].val == UsartPorts.PA_0_3 \
       and usart_config['synchro'].val:
        Pager(stream=sys.stderr)("Warning: Ignoring usart \"synchro=enabled\" "
                                 "with \"usart=pa8-10\" because synchro "
                                 "only available for\"usart=pa0-3\"",
                                 immed=True, one_line=True, indent=9       )
    if not st_usart_settings():
        return False
    if usart_config['synchro'].val and not usart_config['xmit'].val:
        return r_u_sure("usart \"synchro=enabled\" but \"xmit=disabled\". "
                        "Continue?"                                        )
    else:
        return True



def channel_bits(bits):
    names = format(bits, '08b') + "   "
    for bit in range(7, -1, -1):
        if bits & (1 << bit):
            names = names + channel_names()[bit] + " "
        else:
            names = names + '.' * len(channel_names()[bit]) + " "
    return names


def size_read(size, error=True):
    data = os.read(usb_fd, size)
    if len(data) == size or not error:
        return data
    else:
        sys.stderr.write(  "Short read from USB CDC-ACM pipe, "
                           "expected %d bytes, got %d\n" \
                         % (size, len(data))                    )
        return None


WAIT_READ_STDIN    = '<stdin>'
def wait_read(size, timeout=None, error=True):
    try:
        (readers, writers, exceptors) = select.select((usb_fd, sys.stdin),
                                                      (                 ),
                                                      (                 ),
                                                      timeout            )
        if usb_fd in readers:
            return size_read(size, error)
        elif sys.stdin in readers:
            sys.stdin.readline()  # throw away input
            return WAIT_READ_STDIN
        else:
            if error:
                sys.stderr.write(  "Timeout waiting %s second%s for %d bytes "
                                   "from USB CDC-ACM pipe\n"
                                 % (timeout                    ,
                                    '' if timeout == 1 else 's',
                                    size                       )               )
            return None
    except KeyboardInterrupt:
        return WAIT_READ_STDIN



def flush_read(wait_time):
    sys.stdout.write(  "flushing USB CDC-ACM pipe for %g second%s "
                       "(<ENTER> to abort) ...\n"
                     % (wait_time, '' if wait_time == 1.0 else 's'))
    begin_time = time.time()
    while time.time() - begin_time < wait_time:
        try:
            (readers, writers, exceptors) = select.select((usb_fd, sys.stdin),
                                                          (                 ),
                                                          (usb_fd,          ),
                                                          wait_time)
            if usb_fd in exceptors:
                sys.stderr.write("... error reading from USB-ACM pipe\n")
                return
            elif sys.stdin in readers:
                sys.stdin.readline()
                sys.stderr.write("... aborting\n")
                break
            elif usb_fd in readers:
                flushed = os.read(usb_fd, TTY_MAX)
                sys.stdout.write(  "... flushed %d bytes: " % len(flushed))
                for byt in flushed:
                    sys.stdout.write(" %02x" % byt)
                sys.stdout.write('\n')
        except KeyboardInterrupt:
            sys.stderr.write("... aborting\n")



def cmnd_cmd(cmd, input=None, fields=None):
    os.write(usb_fd, struct.pack('4B', cmd, 0, 0, 0))



def cmd_rsp(cmd, size, name, timeout=2.0):
    cmnd_cmd(cmd)
    response = wait_read(size, timeout)
    if response is not None and response is not WAIT_READ_STDIN:
        return response
    else:
        sys.stderr.write(  "Timeout or interrupt waiting "
                           "for response to command %s\n"
                         % name                           )
        return None



def safe_input(prompt, where, warning=None):
    while True:
        try:
            text = input(prompt)
        except (EOFError, KeyboardInterrupt):
            if warning: sys.stderr.write('\n%s\n' % warning)
            else:       sys.stderr.write('\n'              )
            return None
        except Exception as error:
            sys.stderr.write(  "\nUnhandled input exception in %s(): %s\n"
                             % (where, error)                             )
            return None
        else:
            return text



def safe_open(filename, message, mode='w'):
    if os.path.exists(filename):
        if any(    name
               in  os.path.basename(filename)
               for name in ('tmp', 'temp', 'scratch')):
            answer = 'Y'
        else:
            answer = safe_input(  "File \"%s\" exists: Overwrite (Y/n)? "
                                % filename                               ,
                                "safe_open"                              )
        if answer[0] == 'Y':
            try:
                os.unlink(filename)
            except Exception as error:
                sys.stderr.write(  "Can't remove file \"%s\": %s\n"
                                 % (filename, error)           )
                return None
        else:
            sys.stderr.write(  "Not overwriting file \"%s\"%s\n"
                             % (filename, message)              )
            return None
    try:
        file = open(filename, mode)
    except Exception as error:
        sys.stderr.write(  "Can't open file %s with mode \"%s\": %s\n"
                         % (filename, mode, error)                    )
        return None
    return file



def set_viewers(quiet):
    if  not hasattr(shutil, 'which'):
        if not quiet:
            Pager(stream=sys.stderr)(
            "Python \"shutil\" module doesn't have \"which\" function. "
            "Not doing automatic search for waveform viewers and default "
            "parameter setting. Run `buck50.py --help`, and/or see "
            "\"help logic autodump=\", \"help oscope autodump=\", and "
            "\"help dump\"."                                              ,
            immed=True, one_line=True                                     )
        return
    digital_config['autodump'].val = 'enabled'
    analog_config ['autodump'].val = 'enabled'
    gnuplot   = shutil.which('gnuplot'  )
    pulseview = shutil.which('pulseview')
    gtkwave   = shutil.which('gtkwave'  )
    report    = ''
    if pulseview:
        upload_config['output'      ].val = 'file'
        upload_config['digital-frmt'].val = 'vcd'
        upload_config['viewer-vcd'  ].val = 'pulseview'
        upload_config['auto-digital'].val = 'enabled'
        report += "Found %s" % pulseview
    elif gtkwave:
        upload_config['output'      ].val = 'file'
        upload_config['digital-frmt'].val = 'vcd'
        upload_config['viewer-vcd'  ].val = 'gtkwave'
        upload_config['auto-digital'].val = 'enabled'
        report += "Found %s" % gtkwave
    if gnuplot:  # override VCD viewer because gnuplot faster and better exit
        upload_config['output'      ].val = 'file'
        upload_config['digital-frmt'].val = 'csv'
        upload_config['viewer-csv'  ].val = 'gnuplot'
        upload_config['auto-digital'].val = 'enabled'
        upload_config['auto-analog' ].val = 'enabled'
        if pulseview or gtkwave: report +=  " and %s. " % gnuplot
        else:                    report += "Found %s. " % gnuplot
    elif pulseview or gtkwave:
        report += ". "
    if not pulseview and not gtkwave and not gnuplot:
        report += "Found no waveform viewing programs. "
        upload_config['viewer-vcd'  ].val = 'other'
        upload_config['viewer-csv'  ].val = 'other'
        upload_config['output'      ].val = 'terminal'
        upload_config['digital-frmt'].val = 'vcd'
    if not quiet:
        report += "Have now set:"
        Pager()(report, indent=4, immed=True, one_line=True)
        sys.stdout.write(  "configure logic autodump=%s"
                         % digital_config['autodump'])
        sys.stdout.write(  " oscope autodump=%s"
                         % analog_config ['autodump'])
        sys.stdout.write(  " dump output=%s"
                           " auto-digital=%s"
                           " auto-analog=%s"
                           " digital-frmt=%s"
                           " viewer-csv=%s"
                           " viewer-vcd=%s\n"
                         % (upload_config['output'      ],
                            upload_config['auto-digital'],
                            upload_config['auto-analog' ],
                            upload_config['digital-frmt'],
                            upload_config['viewer-csv'  ],
                            upload_config['viewer-vcd'  ]))



### help system utils
#
#

def using_cmd(cmd, input, fields):
    Pager()(using_cmd.__help, indent=-1, immed=True)
using_cmd.__help = """
Input:
  - Type "<command> ..." followed by ENTER key
  - Abbreviate any command, configuration, parameter, or value to its unique prefix
    Example:
        m d=i r=4k a w=2 spe=6
      same as:
        monitor duration=infinite rate=4kHz adc weight=2 speed=600kHz=1.66667μs
  - "#" in line: rest of line ignored
Completion:
  key(s)        result:
  ------        -------
  <TAB>         Completes partially typed command, configuration, parameter,
                or value (if unique prefix), else ...
  <TAB><TAB>    ... sounds terminal bell and second <TAB> prints list
                of possible completions
  Example:
      n<TAB> i<TAB>3 o<TAB>p<TAB> m<TAB><TAB>g<TAB> l<TAB><TAB>o<TAB>16
    completes:
      numbers increment=3 open-pull=push-pull mode=gray low=16
    (note could use abbreviations instead):
      n i=3 o=p m=g lo=16
  Example:
      usart p<TAB><TAB>
    prints list:
      parity=  phase=   ports=   pty
    then:
      usart pa<TAB>
    completes:
      usart parity=
  Example (note space after "usart", before double <TAB> press):
      spi <TAB><TAB>
    prints list:
      ascii-num=  endian=     mode=       pull=       snoop=      xmit-only=
      baud=       idle=       nss-time=   rate=       socket
      busy-wait=  ipc         nss=        rx-wait=    speed=
      disable     master      phase=      select=     spi
      end=        miso=       pty         slave       tx-data=
    then:
      spi x<TAB>
    completes:
      spi xmit-only=
    and:
      spi xmit-only=<TAB><TAB>
    prints list:
      disabled   enabled
    and finally:
      spi xmit-only=e<TAB>
    results in:
      spi xmit-only=enabled
Editing, history:
  (only basics listed, web search "GNU readline" for more)
    key                         result:
    -------------------------   -------
    <LEFT-ARROW>  or <CTRL+b>   cursor left
    <RIGHT-ARROW> or <CTRL+f>   cursor right
    <BACKSPACE>   or <DELETE>   delete character
    <HOME>        or <CTRL+a>   cursor to start of line
    <END>         or <CTRL+f>   cursor to end of line
    <UP-ARROW>    or <CTRL+p>   previous input line
    <DOWN-ARROW>  or <CTRL+n>   next input line
                     <CTRL+r>   search input history
                     <CTRL+g>   clear input line
Alternate "?" help syntax:
  "?" anywhere in line same as "help <line>" (line truncated at "?")
  "?" can immediately follow command/configuration/parameter/value or be separated by space
  Examples ("?", " ?", and " ? " always equivalent):
    "?" syntax                                  same as:
    ----------                                  --------
    usart?                                      help usart
    usart ?                                     help usart
    numbers rate?                               help numbers rate
    numbers rate=?                              help numbers rate=
    numbers rate=2Hz?                           help numbers rate=2Hz
    monitor adc3?                               help monitor adc3
    configure usart baud?                       help configure usart baud
    configure pulse time=1kHz spi?              help configure spi
    spi mode=? endian=lsb                       help spi mode=
    monitor duration=? adc weight=3             help monitor duration
    configure gpio end=XX rate=? pulse time=1s  help gpio rate
More help:
  Type "help" for general help on commands, configurations, actions, and parameters
"""



def param_help(command, config, param):
    if command in ('help', 'quit', 'warranty'):
        command_help(command)
        return
    pager = Pager(initial=1)
    pager(  "Help for parameter \"%s=\" (e.g. \"%s %s %s=\"):"
          % (param, command, config, param)             )
    if hasattr(SETTINGS[config][param], '_help'):
        pager("- %s\n" % SETTINGS[config][param]._help, indent=2)
    elif hasattr(SETTINGS[config][param].__class__, 'help'):
        pager("- %s\n" % SETTINGS[config][param].__class__._help, indent=2)
    else:
        pager(  "- (no help available for \"%s %s %s\")\n"
              % (command, config, param)                 ,
              indent=2                                   )
    pager("Current value: %s" % SETTINGS[config][param]           , indent=15)
    pager("Valid values:  %s" % SETTINGS[config][param].suitable(), indent=15)
    pager(  "Type \"help %s %s\" for list of %s configuration parameters"
          % (command, config, config)                                        ,
          indent=4                                                           )
    pager("Type \"help %s\" for command description" % command               )
    pager.flush()



def action_help(command, config, action):
    if command in ('help', 'quit', 'warranty'):
        command_help(command)
        return
    if not hasattr(SETTINGS[config], '_actions'):
        return
    pager = Pager(initial=1)
    pager(  "Help for action \"%s\" (e.g. \"%s%s %s\"):"
          % (action                                                 ,
             command                                                ,
             " " + config if len(COMMANDS[command].sets) > 1 else '',
             action                                                 )   )
    pager("- %s\n" % SETTINGS[config]._actions[action].help(), indent=2 )
    pager(  "Type \"help %s %s\" for list of %s configuration actions\n"
          % (command, config, config)                                   )
    pager("Type \"help %s\" for command description" % command)
    pager.flush()



def param_action_help(pager, name, help):
    if len(name) > 16:
        name = name[:16]
    spaces = " " * (16 - len(name))
    if len(help) > 48  or '\n' in help:
        if '\n' in help:
            help = help[1:help[1:].find('\n')+1]
        help = help[:48]
        pager("    %s%s%s..." % (name, spaces, help), indent=21)
    else:
        pager("    %s%s%s"    % (name, spaces, help), indent=21)



def config_help(command, config):
    if command in ('help', 'quit', 'warranty'):
        command_help(command)
        return
    pager = Pager(initial=1, indent=4)
    pager(  "Help for configuration \"%s\" (e.g. \"%s %s\"):"
          % (config, command, config))
    if hasattr(SETTINGS[config], '__help'):
        pager(SETTINGS[config].__help)
    else:
        pager("No help available for \"%s %s\"" % (command, config))
    pager(  "Command/configuration usage:\n   "
            "%s%s [<parameter>=<value> ...]%s"
          % (command                                            ,
             ''            if   config == command
                           else " " + config,
             " [<action>]" if   hasattr(SETTINGS[config], '_actions')
                           else ''                                   ))
    if hasattr(SETTINGS[config], '_actions'):
        pager('Actions:')
        for action in SETTINGS[config]._actions:
            param_action_help(pager                                   ,
                              action                                  ,
                              SETTINGS[config]._actions[action].help())
    if command == 'trigger':
        pager("Type \"help %s\" for command description" % command)
        pager.flush()
        return
    pager("Parameters:")
    for param in SETTINGS[config]:
        if hasattr(SETTINGS[config][param], '_help'):
            param_action_help(pager                       ,
                              param + '='                 ,
                              SETTINGS[config][param]._help)
        else:
            pager("    %s" % (param + '='))
    pager("Configuration \"%s\" used by commands:" % config)
    for cmnd in COMMANDS:
        if COMMANDS[cmnd].sets and config in COMMANDS[cmnd].sets:
            pager("    %s" % cmnd)
    pager("Type \"help %s\" for command description" % command)
    config_name =      " " + config                     \
                  if   len(COMMANDS[command].sets) > 1  \
                  else ''
    pager(  "Type \"help %s%s <parameter>\" for parameter description"
          % (command, config_name)                                    )
    if hasattr(SETTINGS[config], '_actions'):
        pager(  "Type \"help %s%s <action>\" for action description"
              % (command, config_name)                              )
    pager.flush()



def command_help(command):
    command_func   = COMMANDS[command].func
    configurations = COMMANDS[command].sets
    pager = Pager(indent=4, initial=1)
    pager("Help for command \"%s\":" % command)
    if hasattr(command_func, '__help'):
        pager(command_func.__help)
    else:
        pager("No help available for \"%s\"\n" % command)
    if not configurations:
        pager.flush()
        return
    pager(  "Command usage:\n     "
            "%s [<parameter>=<value> ...] [<action>]%s"
          % (command                                    ,
                  " [<configuration> "
                  "<parameter>=<value ...] "
                  "[<action>]"
             if   len(configurations) > 1
             else ''                                    ),
          indent=7                                       )
    if len(COMMANDS[command].sets) == 1:
        pager("Configuration: %s" % configurations[0])
        pager(  "Type \"help %s %s\" for "
                "configuration description, parameters, "
                "and actions"
              % (command, configurations[0])              ,
              indent=4                                    )
        pager(  "Type \"help %s <parameter>\" for parameter description"
              % command                                                 )
        if hasattr(SETTINGS[COMMANDS[command].sets[0]], '_actions'):
            pager(  "Type \"help %s <action>\" for action description"
                  % command                                           )
    else:
        pager(  "Configurations (primary=\"%s\"):\n    %s"
              % (configurations[0], ", ".join(configurations)),
              indent=4                                        )
        pager(   "Type \"help %s <configuration>\" for "
                 "configuration description,  parameters, "
                 "and actions"
              % command                                 )
        pager(  "Type \"help %s <configuration> <parameter>\" "
                "for parameter description"
              % command                                        )
    pager.flush()



def main_help():
    pager = Pager(initial=1, indent=4)
    pager("Help:")
    pager(help_cmd.__help, indent=-2)
    pager("Commands:\n    %s" % ", ".join(COMMANDS), indent=4)
    pager(  "Configurations:\n    %s"
          % ", ".join([setting
                       for setting
                       in  SETTINGS
                       if  setting not in ('triggers', 'warranty', 'help')]))
    pager(help_cmd.__help2)
    pager.flush()




### config utils
#
#

def config_setting(settings_name, parm_val):
    disambig = disambiguate(settings_name, SETTINGS.keys())
    if disambig.count != 1 and not disambig.exact:
        # can't happen unless bad configuration name in COMMANDS.sets
        raise ValueError(  "Unknown or bad configuration \"%s\": %s"
                         % (settings_name, disambig.error))
    settings     = SETTINGS[disambig.good]
    (param, val) = parm_val.split('=')
    disambig     = disambiguate(param, settings)
    if disambig.count != 1 and not disambig.exact:
        raise ValueError(  "Unknown or bad  configuration \"%s\" "
                           "parameter \"%s\": %s"
                         % (settings_name, param, disambig.error))
    param = disambig.good
    if hasattr(settings[param], 'val'):
        try:
            settings[param].val = val
        except ValueError as error:
            raise ValueError(  "Bad configuration \"%s\" setting \"%s\": %s"
                             % (settings_name, parm_val, error)             )
    elif isinstance(settings[param], float):
        settings[param] = float(val)
    else:
        settings[param] = val
    return param



def config(command, config, fields, no_report=False):
    if not config in SETTINGS:  # must be called with primary configuration
        Pager(stream=sys.stderr)(  "Internal program error: \"config()\" "
                                   "called with bad primary configuration "
                                   "\"%s\" for command \"%s\"\n"
                                 % (config, command)                       ,
                                 immed=True, one_line=True                 )
        return False
    global configuration_modified
    changes = []
    actions = {}
    pager = Pager(stream=sys.stderr, final=False)
    for field in fields:
        is_action = False
        is_param  = False
        if '=' in field:
            is_param = True
            if '==' in field:
                pager("Bad setting \"%s\" (has \"==\")" % field)
                changes.append(None)
                continue
            elif field.count('=') > 2:
                pager("Bad setting \"%s\" (too many \"=\"s)" % field)
                changes.append(None)
                continue
            elif field.count('=') == 2:  # frequency: strip redundant time
                field = field[:field.rfind('=')]
            try:
                param = config_setting(config, field)
                changes.append((config, param))
                if command != 'configure' and param != 'file':
                    configuration_modified = True
            except Exception as error:
                pager(str(error))
                changes.append(None)
        else:
            choices = set(COMMANDS[command].sets)
            if command != 'trigger':
                choices.update(set(SETTINGS[config].keys()))
            if hasattr(SETTINGS[config], '_actions'):
                choices.update(SETTINGS[config]._actions.keys())
            disambig = disambiguate(field, choices)
            if disambig.count != 1 and not disambig.exact:
                pager(  "Ambiguous or unknown command, configuration, "
                        "or action: \"%s\": %s"
                      % (field, comma_or_concat(disambig.matches)))
                changes.append(None)
                continue
        if hasattr(SETTINGS[config], '_actions'):
            disambig = disambiguate(field, list(SETTINGS[config]._actions))
            if disambig.count == 1:
                # don't do immediately because only one action per configuration
                actions.setdefault(config, []).append(disambig.good)
                is_action = True
        if not (is_param or is_action):
            disambig = disambiguate(field, COMMANDS[command].sets) # case insen
            if disambig.count == 1:
                config = disambig.good
            else:
                pager(  "Bad configuration name \"%s\": %s\n"
                      % (field, disambig.error)              )
                changes.append(None)
    for (cnf, acts) in actions.items():
        if len(acts) > 1:
            Pager(stream=sys.stderr)(  "Only one \"%s\" action allowed "
                                       "per command: Skipping all of %s"
                                     % (cnf, comma_or_concat(acts))     ,
                                     immed=True, one_line=True          )
        else:
            if not no_report:
                sys.stderr.write("%s: %s\n" % (cnf, acts[0]))
            SETTINGS[cnf]._actions[acts[0]].action()()
    if None in changes:
        pager.flush()
    if not no_report:
        for change in changes:
            if change is not None:
                sys.stdout.write(  "%s: %s=%s\n"
                                 % (         change[0]            ,
                                                        change[1] ,
                                    SETTINGS[change[0]][change[1]]))
    return not None in changes



def load_config_file(filename):
    global configuration_modified
    if     configuration_modified \
       and not r_u_sure(  "Loading file %s will overwrite changes. Continue?"
                        % filename                                           ):
        return False
    if not filename.endswith(CONFIG_FILE_EXTENSION):
        filename += CONFIG_FILE_EXTENSION
    try:
        with open(filename, 'r') as file:
            line = file.readline().strip()
            try:
                identity = int(line.split()[1], 16)
            except:
                raise SyntaxError(  "Bad identity line \"%s\" in file \"%s\""
                                  % (line, filename)                         )
            assert(identity ==  IDENTITY),                          \
                    "File \"%s\" identity \"%s\" != program 0x%x"   \
                  % (filename, line, IDENTITY)

            line = file.readline().strip()
            try:
                version = [int(number) for number in line.split()[1:]]
            except:
                raise SyntaxError(  "Bad version line \"%s\" in file \"%s\""
                                  % (line, filename)                         )
            assert(version[0] == VERSION[0]), \
                     "File \"%s\" major version %d != "         \
                     "program major version %d -- not loading"  \
                   % (filename, version[0], VERSION[0])
            assert(version[1] <= VERSION[1]), \
                     "File \"%s\" minor version %d > "          \
                     "program major version %d -- not loading " \
                   % (filename, version[1], VERSION[1])
            lineno = 2
            for line in file:
                fields = line.split()
                disambig = disambiguate(fields[0], COMMANDS.keys())
                if disambig.count != 1:
                    sys.stderr.write(  "File \"%s\", line %d: "
                                       "Bad command \"%s\": %s \n"
                                     % (filename      ,
                                        lineno        ,
                                        fields[0]     ,
                                        disambig.error)           )
                    lineno += 1
                    continue
                command = disambig.good
                if command not in ('configure', 'trigger'):
                    sys.stderr.write(  "File \"%s\", line %d: command \"%s\" "
                                       "not allowed (only \"configure\" "
                                       "or \"trigger\")"
                                     % (filename, lineno, command)            )
                    lineno += 1
                    continue
                if command == 'trigger':
                    trig_cmd(command, line, fields[1:])
                elif not config(command                  ,
                                COMMANDS[command].sets[0],
                                fields[1:]               ,
                                no_report=True           ):
                    sys.stderr.write(  "... on line %d, file \"%s\"\n"
                                     % (lineno, filename)            )
                lineno += 1
    except FileNotFoundError as error:
        sys.stderr.write(  "Can't open configuration file \"%s\": %s\n"
                         % (filename, error)                           )
        return False
    except (ValueError, AssertionError) as error:
        sys.stderr.write('%s\n' % error)
    adjust_cpu_hz()  # in case configure_config['trim'] is not default "1/1"
    configuration_modified = False
    return True



def trigs_print(file, cut_paste="    "):
    keys = list(triggers_config.keys())
    keys.sort()
    for ndx in keys:
        trig = triggers_config[ndx]
        if isinstance(file, Pager):
            file.write("%s%3d=%s\n" % (cut_paste, ndx, trig.fullstr()))
        else:
            file.write("trigger %d=%s\n" % (ndx, trig.fullstr()))



def config_print(caption, config, file):
    if isinstance(file, Pager):
        file.write(  "%s:\n%s"
                   % (caption                                          ,
                      "\n".join([      "%14s : %s"
                                     % (key,
                                        str(val).replace(" ", '\\040'))
                                 for (key, val)
                                 in  config.items()               ])   ))
        file.write('\n')
    else:
        for (key, val) in config.items():
            file.write(  "configure %s %s=%s\n"
                       % (caption, key, str(val).replace(" ", '\\040')))



def configs_print(configs, file):
    if file is sys.stdout:
        file = Pager()
    for config in configs:
        if config == 'triggers':
            if isinstance(file, Pager):
                file.write("triggers:\n")
            trigs_print(file)
        else:
            config_print(config, SETTINGS[config], file)
    # needed if Pager(), doesn't hurt if disk file
    file.flush()




### upload (dump) utils
#
#

def upload_digital_header(first, count, total, max_memory, mode, legend=True):
    sys.stdout.write(  "logic samples: %d...%d of %d (max %d) @ %s"
                     % (first                      ,
                        first + count - 1          ,
                        total                      ,
                        max_memory                 ,
                        sampling_mode_inverse(mode))      )
    if legend:
        names_width = len(channel_bits(0)) -1    # has trailing space
        sys.stdout.write(  "\nindex                PB11 ... PB4  %s"
                           "   ticks     time\n"
                           "-----  ----%s    -------  ---------\n"
                         % (" " * (names_width - 22),
                            '-' *  names_width      )               )



def upload_digital(samples   ,
                   first     ,
                   count     ,
                   total     ,
                   max_memory,
                   mode      ,
                   save_as   ,
                   file      ,
                   filename  ):
    if file:
        active_chans = upload_config['actives'].val
        active_ndxs  = [chan - 4 for chan in active_chans]  # chans are 4...11
        if save_as == DigitalFormat.VCD:
            file.write(  "$timescale %d %s $end\n"
                       % (upload_config['per-tick'  ].val  ,
                          upload_config['tick-units'].str()))
            file.write("$scope module top $end\n")
            file.write("$var wire 1 z blk $end\n")
            for (ndx, chan) in zip(active_ndxs, active_chans):
                file.write(  "$var wire 1 %d %s $end\n"
                           % (ndx                           ,
                              channels_config[str(chan)].val))
            file.write("$upscope $end\n"       )
            file.write("$enddefinitions $end\n")
            pulseview_warning = False
        if save_as == DigitalFormat.CSV:
            file.write('time')
            for chan in active_chans:
                file.write(",%s" % channels_config[str(chan)].val)
            file.write('\n')
        upload_digital_header(first       ,
                              count       ,
                              total       ,
                              max_memory  ,
                              mode        ,
                              legend=False)
        sys.stdout.write(" ... saving to file %s\n" % filename)
    else:
        term_size  = shutil.get_terminal_size().lines
        linenumber  = 0
        upload_digital_header(first, count, total, max_memory, mode)

    prev = None
    tick = 0
    tcks = 0
    last = 0x00
    for ndx in range(count):
        data = samples[ndx]
        tick = data  & 0xffffff
        bits = data >> 24
        if prev is None:
            prev = tick
        delt = prev - tick
        if tick > prev:  # rolled over to negative 24-bit twos complement
            delt += 0x1000000
        tcks += delt
        prev  = tick

        if file:
            if save_as == DigitalFormat.VCD:
                chng = bits ^ last
                if chng:
                    timval =    tcks   * upload_config['tick-units'].val \
                             / (CPU_HZ * upload_config['per-tick'  ].val)
                    file.write('#%d\n' % int(round(timval)))
                    for ndx in active_ndxs:
                        bit = 1 << ndx
                        if chng & bit:
                            if bits & bit: file.write("1%d\n" % ndx)
                            else:          file.write("0%d\n" % ndx)
                last = bits
            elif save_as == DigitalFormat.CSV:
                file.write('%g' % (tcks / CPU_HZ))
                for ndx in active_ndxs:
                    if bits & (1 << ndx):
                        file.write(",1")
                    else:
                        file.write(",0")
                file.write('\n')
        else:
            sys.stdout.write(  "%5d  %02x  %s  %8d  %s\n"
                             % (ndx + first               ,
                                bits                      ,
                                channel_bits(bits)        ,
                                delt                      ,
                                TimeVal.str(delt / CPU_HZ))  )
            linenumber += 1
            if linenumber >= term_size - 4:
                sys.stdout.write("<ENTER> to continue, "
                                 "any letter+<ENTER> to abort):  ")
                sys.stdout.flush()
                if sys.stdin.readline()[0] not in " \n":
                    break
                upload_digital_header(first, count, total, max_memory, mode)
                linenumber = 0

    if file:
        if     save_as == DigitalFormat.VCD                       \
           and upload_config['viewer-vcd'].val == Viewer.PULSEVIEW:
            # PulseView doesn't show last edge transition, so
            #   generate fake duplicate of last bits
            timval += 2.0   # make sure doesn't round to same as previous
            file.write('#%d\n' % int(round(timval)))
            for ndx in active_ndxs:
                bit = 1 << ndx
                if bits & bit: file.write("1%d\n" % ndx)
                else:          file.write("0%d\n" % ndx)
        file.close()

        if not upload_config['auto-digital'].val:
            return

        if upload_config['digital-frmt'].val == DigitalFormat.CSV:
            if upload_config['viewer-csv'].val == Viewer.GNUPLOT:
                commandline  = "gnuplot -e 'set object 1 rectangle from "
                commandline += "screen 0,0 to screen 1,1 fillcolor rgb "
                commandline += "\"black\" behind ; "
                commandline += "set border linecolor \"gray60\" ; "
                commandline += "set title \"%s\" textcolor \"gray60\" ; "
                commandline += "set key textcolor variable ; "
                commandline += "set key autotitle columnheader ; "
                commandline += "unset ytics ; "
                commandline += "set datafile separator \",\" ; "
                commandline +=   "plot [:][-1:%d] \"%s\" "                 \
                               % (1 + 2 * len(active_chans),
                                  filename                                 )
                commandline +=   "using 1:2 with steps linetype rgb \"%s\" " \
                               % GNUPLOT_COLORS[active_ndxs[0]]
                for (pos, (ndx, chan)) in enumerate(zip(active_ndxs [1:],
                                                        active_chans[1:])):
                    commandline +=   ", \"\" using 1:(2*%d+$%d) " \
                                     "with steps "                  \
                                     "linetype rgb \"%s\""          \
                                   % (pos + 1                 ,
                                      pos + 3                 ,
                                      GNUPLOT_COLORS[chan - 4])
                for ndx in range(len(active_chans) + 1):
                    commandline +=   ", %f linetype rgb \"gray20\"" \
                                   % (-0.5 + ndx * 2.0)
                commandline += " ; pause -1 \"<ENTER> here to exit, "       \
                               "<h> in graphic window for help ...  \"'"
            elif upload_config['viewer-csv' ].val in (Viewer.PULSEVIEW,
                                                      Viewer.GTKWAVE  ):
                Pager(stream=sys.stderr) \
                     ("Neither \"pulseview\" nor \"gtkwave\" "
                      "can succesfully load CSV files. Aborting "
                      "and setting \"dump auto-digital=disabled\".",
                      immed=True,one_line=True                     )
                upload_config['auto-digital'].val = 'disabled'
                return
            elif     upload_config['viewer-csv'].val == Viewer.OTHER \
                 and upload_config[ 'other-csv'].val                :
                commandline = upload_config['other-csv'].val
            else:
                return
        # else format is VCD
        elif upload_config['viewer-vcd'].val == Viewer.PULSEVIEW:
            # pulseview allocates 1 byte per tick per channel (??)
            pulseview_memory =   int(timval)                    \
                               * len(active_chans)              \
                               / upload_config['per-tick'].val
            if pulseview_memory > upload_config['warn-pulseview']:
                if not r_u_sure(  "Warning: Estimated `pulseview` memory "
                                  "requirement of %d bytes greater than \"dump "
                                  "warn-pulseview=%g\". May cause system "
                                  "slowdown or lockup if VCD file loaded "
                                  "into `pulseview` without appropriate "
                                  "\"compress idle periods\" setting. (Not "
                                  "settable from here. Run `pulseview` "
                                  "separately, do \"Import Value Change "
                                  "Dump Data...\" and set in popup dialog.) "
                                  "See https://github.com/"
                                  "thanks4opensource/buck50/#pulseview  "
                                  "Continue anyway?"
                                % (pulseview_memory               ,
                                   upload_config['warn-pulseview'])           ):
                    return
            commandline = "pulseview %s"
        elif upload_config['viewer-vcd'].val == Viewer.GTKWAVE:
            commandline = "gtkwave %s"
        elif     upload_config['viewer-vcd'].val == Viewer.OTHER \
             and upload_config[ 'other-vcd'].val                :
                commandline = upload_config['other-vcd'].val
        else:
            return

        if '%s' in commandline:
            commandline = commandline % filename
        sys.stdout.write("Viewer commandline:\n%s\n" % commandline)
        sys.stdout.write("Running above commandline ...\n")
        try:
            subprocess.call(shlex.split(commandline))
        except Exception as error:
            Pager(stream=sys.stderr)("Viewer command failed: %s" % error,
                                     immed=True,one_line=True           )


def upload_analog(samples     ,
                  first       ,
                  count       ,
                  max_memory  ,
                  num_channels,
                  channel_ndxs,
                  sample_rate,
                  num_samples ,
                  save_as     ,
                  file        ,
                  filename    ):
    # recompute each time because MCU_HZ might
    #   have been changet by configure['adjust'] action
    # firmware uses ADCPRE_DIV_6
    ADCCLK_HZ = CPU_HZ / 6.0
    SAMPLE_MICROSECONDS = {
        AdcSampHold.T_1_5   : (1.5   + 12.5) / ADCCLK_HZ,
        AdcSampHold.T_7_5   : (7.5   + 12.5) / ADCCLK_HZ,
        AdcSampHold.T_13_5  : (13.5  + 12.5) / ADCCLK_HZ,
        AdcSampHold.T_28_5  : (28.5  + 12.5) / ADCCLK_HZ,
        AdcSampHold.T_41_5  : (41.5  + 12.5) / ADCCLK_HZ,
        AdcSampHold.T_55_5  : (55.5  + 12.5) / ADCCLK_HZ,
        AdcSampHold.T_71_5  : (71.5  + 12.5) / ADCCLK_HZ,
        AdcSampHold.T_239_5 : (239.5 + 12.5) / ADCCLK_HZ,
    }

    trig_chan_name = adc_configs[channel_ndxs & 0x0f]['name'].val
    if num_channels == 1:
        scnd_chan_name = ''
    else:
        scnd_chan_name = adc_configs[channel_ndxs >> 4]['name'].val

    Pager()(   "Samples %d...%d of %d (max %d)  "
              "%d channel%s (%s%s) per sample "
              "at %s%s\n"
            % ( first          * (3 - num_channels)            ,  # (1,2)->(2,1)
               (first + count) * (3 - num_channels) - 1        ,  # (1,2)->(2,1)
                    num_samples
               if   num_channels == 2
               else num_samples * 2                            ,
                    max_memory
               if   num_channels == 2
               else max_memory * 2                             ,
               num_channels                                    ,
               '' if num_channels == 1 else 's'                ,
               trig_chan_name                                  ,
               ',%s' % scnd_chan_name if scnd_chan_name else '',
                    AdcSampHold(sample_rate)
               if   sample_rate <= AdcSampHold.T_239_5
               else '???'                                      ,
                    " ... saving to file %s\n" % filename
               if   file
               else ''                                         ),
            immed=True, one_line=True                            )

    if sample_rate  > AdcSampHold.T_239_5:  # safety check
        sample_rate = AdcSampHold.T_239_5
    tick = SAMPLE_MICROSECONDS[sample_rate] * analog_config['time-scale']

    trig_chan   = channel_ndxs  & 0xf
    scnd_chan   = channel_ndxs >>   4 if (channel_ndxs >> 4) < 8 else 0
    time_printf = analog_config ['printf'    ].val
    trgr_adc    =    adc_configs[trig_chan   ]
    scnd_adc    =    adc_configs[scnd_chan   ]
    trgr_printf =       trgr_adc['printf'    ].val
    scnd_printf =       scnd_adc['printf'    ].val
    trgr_ranger =       trgr_adc['scale-hyst'].ranged
    scnd_ranger =       scnd_adc['scale-hyst'].ranged

    if num_channels == 1:
        printf =   "%%4d   %s    %s %s\n%%4d   %s    %s %s\n"   \
                 % (time_printf   ,
                    trig_chan_name,
                    trgr_printf   ,
                    time_printf   ,
                    trig_chan_name,
                    trgr_printf   )
        if file:
            file.write("time,%s\n" % trig_chan_name)
    else: # two channels
        printf =   "%%4d   %s    %s %s   %s %s\n"   \
                 % (time_printf   ,
                    trig_chan_name,
                    trgr_printf   ,
                    scnd_chan_name,
                    scnd_printf   )
        if file:
            file.write("time,%s,%s\n" % (trig_chan_name, scnd_chan_name))

    if not file:
        term_size   = shutil.get_terminal_size().lines
        extra_lines = 2
        linenumber  = 0

    for ndx in range(count):
        sample_1 = samples[ndx]  & 0xffff
        sample_2 = samples[ndx] >>     16
        ranged_1 = trgr_ranger(sample_1)
        if num_channels == 1:
            num      = (first + ndx) * 2
            ranged_2 = trgr_ranger(sample_2)
            text     = printf % ( num            ,
                                  num * tick     ,
                                  ranged_1       ,
                                  num + 1        ,
                                 (num + 1) * tick,
                                  ranged_2       )
        else:
            num      = first + ndx
            ranged_2 = scnd_ranger(sample_2)
            text     = printf % (num      ,
                                 num * tick,
                                 ranged_1  ,
                                 ranged_2  )
        if file:
            if num_channels == 1:
                file.write("%g,%g\n%g,%g\n" % ( num * tick     ,
                                                ranged_1       ,
                                               (num + 1) * tick,
                                                ranged_2)      )
            else:
                file.write("%g,%g,%g\n" % (num * tick, ranged_1, ranged_2))
        else:
            sys.stdout.write(text)
            linenumber += 1
            if linenumber + extra_lines >= term_size - 1:
                sys.stdout.write("<ENTER> to continue, any letter to abort):  ")
                sys.stdout.flush()
                if sys.stdin.readline()[0] not in " \n":
                    break
                linenumber  = 0
                extra_lines = 0

    if file:
        if upload_config['viewer-csv'].val == Viewer.GNUPLOT:
            linestyle =   "linewidth %g pointtype %d pointsize %g" \
                        % (upload_config['linewidth']    ,
                           upload_config['pointtype'].val,
                           upload_config['pointsize']    )
            commandline  = "gnuplot -e 'set object 1 rectangle from "
            commandline += "screen 0,0 to screen 1,1 fillcolor rgb "
            commandline += "\"black\" behind ; "
            commandline += "set border linecolor \"gray60\" ; "
            commandline += "set title \"%s\" textcolor \"gray60\" ; " % filename
            commandline += "set label textcolor \"gray60\" ; "
            commandline += "set key textcolor variable ; "
            commandline += "set key autotitle columnheader ; "
            commandline += "set datafile separator \",\" ; "
            commandline += "plot \"%s\" " % filename
            commandline +=   "using 1:2 with linespoints "  \
                             "linetype rgb \"yellow\" %s, " \
                           % linestyle
            if num_channels == 2:
                commandline +=   "\"\" using 1:3 with linespoints "     \
                                 "linetype rgb \"orange\" %s ; "        \
                               % linestyle
            commandline += " ; pause -1 \"<ENTER> here to exit, "       \
                           "<h> in graphic window for help ...  \"'"
        elif     upload_config['viewer-csv'].val == Viewer.OTHER \
             and upload_config[ 'other-csv'].val                :
            commandline = upload_config['other-csv'].val
        else:
            commandline = ''

        file.close()
        if '%s' in commandline:
            commandline = commandline % filename

        if upload_config['auto-analog'].val:
            if upload_config['viewer-csv'].val in (Viewer.PULSEVIEW,
                                                   Viewer.GTKWAVE  ):
                Pager(stream=sys.stderr) \
                     ("Neither \"pulseview\" nor \"gtkwave\" "
                      "can succesfully load CSV files. Aborting "
                      "\"auto-analog=enabled\"."                   ,
                      immed=True,one_line=True                   )
                return
            sys.stdout.write("Viewer commandline:\n%s\n" % commandline)
            sys.stdout.write("Running above command "
                             "(\"auto-analog=enabled\") ...\n")
            try:
                subprocess.call(shlex.split(commandline))
            except Exception as error:
                Pager(stream=sys.stderr)("Viewer command failed: %s" % error,
                                         immed=True,one_line=True           )




### usb bridge utils
#
#

def usb_bridge_instructions(ascii_numeric, synchro):
    if ascii_numeric == AsciiNumeric.ASCII:
        Pager()(  "%s ascii text with optional \"\\n\", \"\\t\", "
                  "and \"\\x<two hex digits>\" escape sequences ..."
                % ("<ENTER> to input, then input" if synchro else "Input"),
                immed=True, one_line=True                                 )
    else:
        Pager()(  "%s space-separated bytes (2 digit hex, "
                  "3 digit decimal, or 8 digit binary numbers, "
                  "or one character ascii letters) ..."
                % ("<ENTER> to input, then input" if synchro else "Input"),
                immed=True, one_line=True                                 )



def usb_bridge_stdin(ascii_numeric, end_bytes):
    has_end = False
    encoded = None
    while encoded is None:
        if ascii_numeric == AsciiNumeric.ASCII:
            message = safe_input(  "\"%s\" to finish: "
                                 % decode_escape(end_bytes),
                                 "usb_bridge_stdin"        )
            if not message:
                sys.stdout.write("continuing ...\n")
                return True
            try:
                encoded = encode_escape(message)
            except Exception as error:
                Pager(stream=sys.stderr)("Internal error: %s" % error,
                                         immed=True, one_line=True   )
                encoded = None
            else:
                if encoded.find(end_bytes) != -1:
                    has_end = True
        else:  # AsciiNumeric.NUMERIC:
            message = safe_input("\"999\" to finish: ", "usb_bridge_stdin")
            if not message:
                sys.stdout.write("continuing ...\n")
                return True
            if message.startswith('999'):
                encoded = b'\xff'  # to exit loop, can't be empty or b'\0'
                has_end = True
            else:
                try:
                    encoded = DataBytes.parse(message.split())
                except Exception as error:
                    Pager(stream=sys.stderr)(str(error)   ,
                                             immed=True   ,
                                             one_line=True)
                    encoded = None
    if has_end:
        os.write(usb_fd, b'\0\0\0\0')
        return False
    length = len(encoded)
    if  length > MAX_USB_BRIDGE:
        Pager(stream=sys.stderr)(  "Input %d bytes exceeds max "
                                    "%d -- truncating"
                                 % (length, MAX_USB_BRIDGE)      ,
                                 immed=True, one_line=True       )
        encoded = encoded[:MAX_USB_BRIDGE]
        length  = MAX_USB_BRIDGE
    try:
        os.write(usb_fd, bytes((length,)) + encoded)
    except Exception as error:
        Pager(stream=sys.stderr)(  "Error sending to USB data "
                                   "to buck50 device: %s"
                                 % error                        ,
                                 immed=True, one_line=True      )
    return True



def usb_bridge_usb(ascii_numeric):
    length_byte = wait_read(1)
    if length_byte in (WAIT_READ_STDIN, None):
        Pager(stream=sys.stderr)("Interrupted while waiting "
                                 "for USB, text ignored")
        return False
    status_data = wait_read(int(length_byte[0]) + 1)   # +1 for status byte
    if status_data in (WAIT_READ_STDIN, None):
        Pager(stream=sys.stderr)("Interrupted while waiting "
                                 "for USB, text ignored")
        return False
    status = int(status_data[0])
    if ascii_numeric == AsciiNumeric.NUMERIC:
        text = " ".join([      '%03d-%02x-%c'
                             % (byte,
                                byte,
                                chr(byte) if byte in range(32, 128) else '.')
                         for byte
                         in   status_data[1:]                                ])
    else:
        text = decode_escape(status_data[1:])
    Pager()(  text
            + (    ''
               if   status == 0
               else " (%s)" % periph_status(status)),
            immed=True, one_line=True               )
    return True



def usb_bridge_synchro(ascii_numeric, end_string, xmit_only):
    end_bytes = encode_escape(end_string)
    usb_bridge_instructions(ascii_numeric, False)
    while True:
        if not usb_bridge_stdin(ascii_numeric, end_bytes):
            return   # user commanded exit
        if not xmit_only:
            usb_bridge_usb(ascii_numeric)


def usb_bridge_async(ascii_numeric, end_string):
    end_bytes = encode_escape(end_string)
    usb_bridge_instructions(ascii_numeric, True)
    while True:
        if not usb_bridge_usb(ascii_numeric):
            if not usb_bridge_stdin(ascii_numeric, end_bytes):
                return



def usb_bridge_print(name, data, snoop, periph_status=''):
    if snoop == Snoop.DISABLED:
        return
    elif snoop == Snoop.ASCII:
        Pager()(  "%d bytes%s from %s: %s"
                % (len(data)              ,
                   periph_status          ,
                   name                   ,
                   decode_escape(data)    ),
                immed=True,one_line=True   )
    else:
        sys.stdout.write(  "%d bytes from %s%s:\n"
                         % (len(data), name, periph_status))
        term_width = shutil.get_terminal_size().columns
        # 4+3(index) + 2+1(hex) + 1(ascii) + 4(spaces) + 2(extra, for 80->16)
        per_line   = (term_width - 13) // 4
        if per_line < 1: per_line = 1
        length = len(data)
        for first in range(0, length, per_line):
            sys.stdout.write(  "%4d:  %s%s%s\n"
                             % (first                                         ,
                                " ".join([     "%02x" % byte
                                          for byte
                                          in  data[first:first+per_line]])    ,
                                  " "
                                * (4 + 3 * (     0
                                            if   length - first >= per_line
                                            else per_line - (length - first))),
                                '' .join([         chr(byte)
                                              if   byte in range(32, 128)
                                              else '.'
                                          for byte
                                          in  data[first:first+per_line]])    ))



def usb_bridge_sock_pty(snoop):
    global usb_fd
    file_objs = [sys.stdin, usb_fd]
    MAX_RW_SIZE = 1024

    if pty_config['active'].val:
        try:
            (ptym, pts) = os.openpty()
        except Exception as error:
            Pager(stream=sys.stderr)("Failure opening pty: %s\n" % error,
                                     immed=True, one_line=True           )
            (ptym, pts) = (None, None)
        else:
            tty.setraw(ptym)
            file_objs.append(ptym)
            sys.stdout.write("pty %s ready\n" % os.ttyname(pts))
    else:
        (ptym, pts) = (None, None)

    if socket_config['active'].val:
        try:
            server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            server.bind((socket_config['host'].val, socket_config['port'].val))
            server.listen()
        except Exception as error:
            Pager(stream=sys.stderr)("Failure creating socket: %s\n" % error,
                                     immed=True, one_line=True               )
            server  = None
            clients = {}
        else:
            clients = {}
            file_objs.append(server)
            sys.stdout.write(  "server socket %s:%d ready\n"
                             % (socket_config['host'].val,
                                socket_config['port'].val   ))
    else:
        server  = None
        clients = {}

    if not ptym and not server:
        Pager(stream=sys.stderr)("Socket and/or pty failures -- can "
                                 "use terminal I/O by settting "
                                 "\"ipc term-ptysock=terminal\""     ,
                                 immed=True, one_line=True           )
        return

    sys.stdout.write("I/O active, press <ENTER> to exit ...\n")
    exiting = False
    while True:
        try:
            (reads, writes, errors) = select.select(file_objs, [], [])
        except (EOFError, KeyboardInterrupt):
            sys.stderr.write("Use <ENTER> to exit\n")   # ;)
            continue
        except Exception as error:
            sys.stderr.write("Unexpected exception: %s\n" % error)
            continue
        send_data = b''
        if sys.stdin in reads:
            sys.stdin.readline()  # flush
            reads.remove(sys.stdin)
            os.write(usb_fd, b'\0\0\0\0')  # end sentinel for firmware
            break
        if server in reads:
            (client_sock, client_addr) = server.accept()
            sys.stdout.write(  "New client socket connection:  %s:%d\n"
                             % client_sock.getpeername()               )
            file_objs.append(client_sock)
            clients[client_sock.fileno()] = client_sock
            reads.remove(server)
        if ptym in reads:
            data       = os.read(ptym, MAX_RW_SIZE)
            send_data += data
            reads.remove(ptym)
            usb_bridge_print('pty', data, pty_config['snoop'].val)
        if usb_fd in reads:
            data = os.read(usb_fd, MAX_FULL_USB)
            size = data[0]
            if size < 1 or len(data) == 1 or len(data) != size + 2:
                sys.stderr.write(  "Incorrect or zero  USB data "
                                   "USB data size: %d vs %d\n"
                                 % (len(data), size)              )
                continue
            if ptym:
                os.write(ptym, data[2:])
            for client in clients.values():
                client.send(data[2:])
            status = periph_status(data[1])
            if status[0] == " ": status = status[1:]   # special case " OK"
            status = " (%s)" % status
            usb_bridge_print('usb', data[2:], snoop, status)
            reads.remove(usb_fd)
        # remaining must be sockets
        for file_obj in reads:
            data = file_obj.recv(MAX_RW_SIZE)
            if len(data) == 0:
                sys.stdout.write(  "Got disconnect from client socket: %s:%d\n"
                                 % file_obj.getpeername()                      )
                file_objs.remove(file_obj)
                del clients[file_obj.fileno()]
                file_obj.close()
            else:
                send_data += data
                usb_bridge_print( "socket %s:%d" % file_obj.getpeername(),
                                 data                                    ,
                                 socket_config['snoop'].val              )
        while send_data:
            size = len(send_data)
            if size > MAX_USB_BRIDGE:
                size = MAX_USB_BRIDGE
            try:
                os.write(usb_fd, bytes((size,)) + send_data[:size])
            except Exception as error:
                Pager(stream=sys.stderr)(  "Error sending to USB data "
                                           "to buck50 device: %s"
                                         % error                        ,
                                         immed=True, one_line=True      )
            send_data = send_data[size:]

    if ptym: os.close(ptym)
    if pts:  os.close(pts )
    for client in clients.values():
        client.close()
    if server:
        server.close()




### i2c utils
#
#

def i2c_print(status, num_tx, num_rx, oar_gc, rx):
    info =   "stat=%s tx=%-2d rx=%-2d%s : "                         \
           % (status                                                ,
              num_tx                                                ,
              num_rx                                                ,
                   ''
              if   oar_gc not in range(3)
              else '@%s' % ('GC','A1','A2','M')[oar_gc]             )
    if num_rx == 0:
        sys.stdout.write('%s\n' % info)
        return
    term_width = shutil.get_terminal_size().columns
    per_line = (term_width - len(info)) // 9
    if per_line < 1: per_line = 1
    for first in range(0, num_rx, per_line):
        sys.stdout.write('%s%s\n'
                         % (info if first == 0 else " " * len(info)     ,
                            " ".join([  "%03d-%02x-%s"
                                      % (byt,
                                         byt,
                                              chr(byt)
                                         if    byt in range(32, 128)
                                         else '.'                   )
                                      for byt
                                      in  rx[first:first+per_line]   ])))



def parse_i2c(text, with_addr, error_msg):
    min_words = 2 if with_addr else 1
    fields = text.split()
    assert len(fields) >= min_words, error_msg
    if with_addr:
        if re.search('[A-FXa-fx]', fields[0]): address = int(fields[0], 16)
        else:                                  address = int(fields[0]    )
    if min_words == 2: ndx = 1
    else:              ndx = 0
    if re.search('[A-FXa-fx]', fields[ndx]): rx_size = int(fields[ndx], 16)
    else:                                    rx_size = int(fields[ndx]    )
    tx_data = DataBytes.parse(fields[min_words:])
    if len(tx_data) > MAX_I2C_BRIDGE:
        Pager(stream=sys.stderr)(  "Truncating at max %d bytes "
                                   "(last byte: \"%s\")"
                                 % (MAX_I2C_BRIDGE,
                                    tx_data[MAX_I2C_BRIDGE])    ,
                                 immed=True, one_line=True      )
    if with_addr:
        return (address, rx_size, tx_data)
    else:
        return (rx_size, tx_data)



def i2c_master():
    Pager()("Input format: <slave address> <rx size> "
            "<tx data <2hex|3dec|1ascii>) ...> [; ...]",
            immed=True, one_line=True                  )
    while True:
        text = safe_input("\"0 0\" to end: " ,"i2c_master")
        if not text:
            continue
        for command in text.split(';'):
            try:
                (addr, rx_size, tx_data) \
                = parse_i2c(command                      ,
                            True                         ,
                            "Must have at least address "
                            "and rx size"                )
            except Exception as error:
                Pager(stream=sys.stderr)(str(error), immed=True, one_line=True)
                continue
            if addr != 0 and rx_size == 0 and len(tx_data) == 0:
                Pager(stream=sys.stderr)("Must set rx size and/or tx data",
                                         immed=True, one_line=True)
                continue
            if rx_size > MAX_I2C_BRIDGE:
                Pager(stream=sys.stderr)(  "rx size > %d max -- ignoring"
                                         % MAX_I2C_BRIDGE,
                                         immed=True, one_line=True      )
                continue
            length  = len(tx_data)
            os.write(usb_fd,   struct.pack('BBB'  ,
                                           addr   ,
                                           rx_size,
                                           length )
                             + tx_data             )
            if addr == 0 and rx_size == 0 and length == 0:
                return
            response = wait_read(MAX_FULL_USB, None, False)
            if response is WAIT_READ_STDIN:
                Pager(stream=sys.stderr)("No I2C response, will halt and  "
                                         "flush USB pipe for 2 seconds ...\n",
                                         immed=True, one_line=True           )
                cmnd_cmd(HALT_CMD)
                flush_read(2.0)
                return
            if len(response) < 3:
                sys.stderr.write(  "I2C response error (short read "
                                   "of %d bytes: %s)\n"
                                 % (len(response), response)        )
                continue
            i2c_print(periph_status(response[0]),
                      response[2]               ,
                      response[3]               ,
                      -1                        ,
                      response[4:]              )



def i2c_slave():
    Pager()("<enter> to queue tx_data, and expected rx size ...",
            immed=True, one_line=True                           )

    # set default, rx anything up to max
    os.write(usb_fd, struct.pack('BBBB', 0, MAX_I2C_BRIDGE, 0, 0))

    while True:
        try:
            (readers, writers, exceptors) = select.select((usb_fd, sys.stdin),
                                                          (                 ),
                                                          (                 ))
            if sys.stdin in readers:
                sys.stdin.readline()  # throw away input
                while True:
                    text = safe_input("<rx_max> <tx queue 2hex|3dec|"
                                     "1ascii ...> (\"0\" to end): "   ,
                                     "i2c_slave"                      )
                    if not text:
                        break
                    try:
                        (rx_size, tx_data) \
                        = parse_i2c(text                       ,
                                    False                      ,
                                    "Must have at least rx size")
                        break
                    except Exception as error:
                        Pager(stream=sys.stderr)(str(error)   ,
                                                 immed=True   ,
                                                 one_line=True)
                        continue
                if len(tx_data) > MAX_I2C_BRIDGE:
                    Pager(stream=sys.stderr)(  "tx data > %d max -- truncating"
                                             % MAX_I2C_BRIDGE,
                                             immed=True, one_line=True       )
                    tx_data = tx_data[:MAX_I2C_BRIDGE]
                if rx_size> MAX_I2C_BRIDGE:
                    Pager(stream=sys.stderr)(  "rx size > %d max -- setting "
                                               "to %d"
                                             % (MAX_I2C_BRIDGE, MAX_I2C_BRIDGE),
                                             immed=True, one_line=True         )
                    rx_size = MAX_I2C_BRIDGE
                length  = len(tx_data)
                os.write(usb_fd,   struct.pack('BBB'  ,
                                               0      ,   # addr not used
                                               rx_size,
                                               length )
                                 + tx_data             )
                if rx_size == 0 and len(tx_data) == 0:
                    return
                Pager()(    "Queued %d tx bytes, max %d rx bytes"
                        % (len(tx_data), rx_size)                ,
                        immed=True, one_line=True                )
                Pager()("<enter> to queue tx_data, and expected rx size ...",
                        immed=True, one_line=True                           )
            if usb_fd in readers:
                response = wait_read(MAX_FULL_USB, None, False)
                if len(response) < 4:
                    sys.stderr.write(  "I2C response error (short read "
                                       "of %d bytes: %s)\n"
                                     % (len(response), response)        )
                    continue
                (status, oar_gc, num_tx, num_rx) = struct.unpack('BBBB'      ,
                                                                 response[:4])
                if oar_gc not in range(3): oar_gc = -1  # sanity check
                i2c_print(periph_status(status)     ,
                                        num_tx      ,
                                        num_rx      ,
                                        oar_gc      ,
                                        response[4:])
        except:
            sys.stderr.write("Use <ENTER>, then \"0\" to end\n")




### commands
#
#

def reset_cmd(cmd, input, fields):
    if not config('reset', 'reset', fields):
        return
    # no code: all done in actions
reset_cmd.__help = """
Reset various host, device, and USB subsystems:
  - Enabling or disabling multiple connected devices external triggering and sampling synchronization ("ganged=", "ext-trig")
  - USB disconnect, (re-)connect, or port/driver change ("connect", "usb=")
  - Stalled USB communications recovery ("flush")
  - Firmware halt and/or reset ("halt", "init")
  - Change flash memory (firmware code) wait states and/or enable/disable prefetch queue ("flash" with "wait=" and/or "pre=")
Performing "halt", "flush", and/or "init" required only in case of firmware/software bug or USB communication failure.
Similarly, "connect" with or without new "usb=/dev/<...>" for error or to connect to different port/device.
"""



def quit_cmd(cmd, input, fields):
    "Exit program"
    sys.exit(0)
quit_cmd.__help = """
Exit program.
"""


def wrty_cmd(cmd, input, fields):
    sys.stdout.write(WARRANTY)
wrty_cmd.__help = WARRANTY



def help_cmd(cmd, input, fields):
    command = None
    config  = None
    param   = None
    if not fields:
        main_help()
        return
    pager = Pager(stream=sys.stderr, indent=4, final=False)
    for field in fields:
        new_config  = None
        new_param   = None
        new_action  = None
        no_params   = False
        no_actions  = False
        if command:
            # can't do immediately w/ command, need config==None for plain cmd
            if not config and COMMANDS[command].sets:
                config = COMMANDS[command].sets[0]
        else:
            disambig = disambiguate(field, COMMANDS.keys())
            if disambig.count == 1 or disambig.exact:
                command = disambig.good
            else:
                pager(  "Unknown or ambiguous command \"%s\": %s"
                      %  (field, disambig.error)                 )
                pager("Type \"help <command>\"")
                pager.flush()
                return
            # if command == 'help':  # pathological "help ... ?"
            #   command = None
            continue
        cfg_eq_cmd  = '' if config == command or not config else " " + config
        cfg_prm_act = field.split('=')[0]
        if config and config != 'triggers':
            disambig = disambiguate(cfg_prm_act, SETTINGS[config])
            if disambig.count == 0:
                no_params = True
            elif disambig.count == 1 or disambig.exact:
                new_param = disambig.good
            else:
                pager(  "Ambiguous \"%s%s\" parameter: %s\n"
                      % (command                          ,
                         cfg_eq_cmd                   ,
                         comma_or_concat(disambig.matches)) )
                pager(  "Type \"help %s%s <parameter>"
                      % (command, cfg_eq_cmd)               )
                pager.flush()
                return
            if hasattr(SETTINGS[config], '_actions'):
                disambig = disambiguate(cfg_prm_act, SETTINGS[config]._actions)
                if disambig.count == 0:
                    no_actions = True
                elif disambig.count == 1 or disambig.exact:
                    new_action = disambig.good
                else:
                    pager(   "Ambiguous \"%s%s\" action: %s\n"
                          % (command                          ,
                             cfg_eq_cmd                       ,
                             comma_or_concat(disambig.matches)))
                    pager(  "Type \"help %s%s <action>\"\n"
                          % (command, cfg_eq_cmd)          )
                    pager.flush()
                    return
        if COMMANDS[command].sets and not ('=' in field or new_action):
            disambig = disambiguate(cfg_prm_act, COMMANDS[command].sets)
            if disambig.count == 1 or disambig.exact:
                new_config = disambig.good
        if bool(new_config) + bool(new_param) + bool(new_action) > 1:
            pager(  "Ambiguous config, param, or action: \"%s\" "
                    "(try \"%s<TAB><TAB>\")"
                  % (cfg_prm_act, cfg_prm_act)                   ,
                  immed=True                                     )
            return
        if new_config:
            config = new_config
        elif config and no_params and no_actions:
            if config == COMMANDS[command].sets[0]:
                pager(   "Unknown \"%s\" parameter \"%s=\"\n"
                      % (command, cfg_prm_act)               )
                pager(  "Type \"help %s <configuration> <parameter\"\n"
                      % command                                       )
            else:
                pager(  "Unknown \"%s %s\" parameter \"%s\"\n"
                      % (command, config, cfg_prm_act)        )
                pager(  "Type \"help %s %s\" or \"help %s %s <parameter\"\n"
                      % (command, config, command, config)                  )
            pager.flush()
            return
    if new_param:
        param_help(command, config, new_param)
    elif new_action:
        action_help(command, config, new_action)
    elif config:
        config_help(command, config)
    elif command:
        command_help(command)
    else:
        main_help()
help_cmd.__help = """
Commands, Configurations, Actions, Parameters, Values:
    <command> [[<action>] [<parameter>=<value>] ...] [<configuration> [<action>] [<parameter>=<value>] ...] ... [# comment ...]
Syntax:
  symbol                explanation
  ------                -----------
  <command>             command name
  [...]                 optional
  [<action>]            optional action name
  [<parameter>=<value>] optional parameter=value names
  <configuration>       configuration name
  ...                   optional repeats
  [# comment ...]       text from "#" to end of line ignored
Verbose explanation:
  - Program executes Commands
  - Commands use Parameter=Value settings
  - Parameter=Value settings are grouped into Configurations
  - Configurations may have optional Actions
  - Parameters have default Values
  - Parameters retain their Values until changed by new Parameter=Value setting, "configure load", or program exit/restart.
  - Commands have a primary Configuration, with same name as the Command
  - Commands may have one or more secondary Configurations
  - Primary configuration is initial current Configuration
  - Configuration remains current until changed by explicit new Configuration
  - Only current Configuration's Parameters=Values can be set
  - Configurations may be used by multiple Commands
  - Only one Action allowed per Configuration per Command
Examples:
  logic
    - Command with no Parameter=Value settings, Actions, or secondary Configurations
  usart baud=19.2kHz
    - Command with Parameter=Value setting in primary Configuration
  pulse time=125ms mode1=rise time1=15%
    - Command with multiple Parameters=Values settings in primary Configuration
  monitor lines 6=clock 7=data
    - Command with multiple Parameter=Value settings in secondary Configuration
  reset halt
    - Command with Action in primary Configuration
  monitor spi master
    - Command with Action in secondary Configuration
  monitor rate=10Hz pb4-11=disabled adc0 enable s/h+adc=239.5+12.5@12MHz->47.6kHz spi slave miso=push-pull monitor file=monitored.txt output=both
    - Command with multiple Parameters=Values settings in primary Configuration, two secondary Configurations each with a Parameter=Value setting and an Action, then explicit primary Configuration with two more Parameter=Value settings
  monitor
    - Command with no Parameters=Values settings, Actions, or Configurations. All current Parameter=Value settings in all Configurations remain in effect.
"""
help_cmd.__help2 = """
Time/Frequency Errors:
  - Many time and/or frequency parameters have limited precision due to their STM32F103 hardware implementations (32 bit register values, etc). Arbitrary Parameter=Values are rounded to the nearest achievable value and the error difference reported. Examples:
  31.125μs(error:-5.5ns)
    - time is 5.5ns shorter than requested
  3.13043MHz=319.444ns(error:-28.8452kHz,+2.91663ns)
    - frequency is 28.8452kHz lower, time 2.91663ns longer, than requested
More help:
    help using
    help <command>
    help <command> <parameter>
    help <command> <configuration>
    help <command> <configuration> <parameter>
"""



def conf_cmd(cmd, input, fields):
    global configure_loadsave_error

    if not config('configure', 'configure', fields):  # , no_report=True)
        return

    if configure_loadsave_error:
        configure_loadsave_error = False
        return

    settings = []
    for field in fields:
        disambig = disambiguate(field, COMMANDS['configure'].sets)
        if disambig.good:  # ambiguous would have been caught by config() above
            settings.append(disambig.good)

    if not settings and not fields:
        settings = [    key
                    for key     in SETTINGS.keys()
                    if  key not in ('help'         ,
                                    'warranty'     ,
                                    'quit'         )]
    elif not settings:
        settings = ['configure']

    configs_print(settings, sys.stdout)

conf_cmd.__help = """
View, change, load, and/or save settings
  - View any or all configurations' parameters:
  - Save or load all configurations' parameters to/from disk file
  - Adjust all time output values (trim hardware CPU clock)
"""



def trig_cmd(cmd, input, fields):
    if     fields                               \
       and not '=' in fields[0]                 \
       and not fields[0].lower().startswith('d'):
        config('trigger', 'triggers', fields)
        return
    global triggers_config, triggers_backup
    if fields:
        if fields[0][0].lower() == 'd':
            # can't be done as action because needs access to fields
            triggers_backup = copy.deepcopy(triggers_config)
            deletes = []
            for field in fields[1:]:
                if re.match('[0-9]+-[0-9]+$', field):
                    (first, last) = list(map(int, field.split('-')))
                    deletes += range(first, last + 1)
                elif all([char in string.digits for char in field]):
                    deletes.append(int(field))
                else:
                    sys.stderr.write(  "Bad delete \"%s\" (not <#>|<#-#>...)\n"
                                     % field)
            if not deletes:
                sys.stderr.write("Specify one or more triggers "
                                 "to delete: [-m] [p[-q] ...\n" )
                return
            for delete in deletes:
                if delete == 0:
                    sys.stderr.write("Deleting trigger #0 not allowed\n")
                elif delete in triggers_config:
                    del triggers_config[delete]
                # elif delete < 256:
                #   sys.stderr.write("No trigger #%d to delete\n" % delete)
        else:
            text = fields[0]
            if '=' not in text:
                sys.stderr.write("Bad trigger \"%s\" (no \"=\")\n" % text)
                return
            elif '==' in text:
                sys.stderr.write("Bad trigger \"%s\" (has \"==\")\n" % text)
                return
            elif text.count('=') > 1:
                sys.stderr.write(  "Bad trigger \"%s\" (too many \"=\"s)\n"
                                 % text)
                return
            (num, trig) = text.split('=')
            try:
                num = int(num)
            except ValueError:
                sys.stderr.write(  "Trigger number \"%s\ not an integer\n"
                                 % num)
                return
            if num not in range(256):
                 sys.stderr.write(  "Trigger number %d not in "
                                    "range [0 ... 255]\n"
                                  % num)
                 return
            triggers_backup = copy.deepcopy(triggers_config)
            comment = input[input.find(text)+len(text)+1:].strip()
            try:
                if num in triggers_config:
                    triggers_config[num].val = (trig, comment)
                else:
                    triggers_config[num] = Trigger(trig, comment)
            except ValueError as error:
                sys.stderr.write("Bad %d trigger setting: %s\n" % (num, error))
                return
            global configuration_modified
            configuration_modified = True
    else:
        trigs_print(sys.stdout, "trigger ")
trig_cmd.__help = """
Digital capture/sampling triggering.
Usage:
    trigger                                         list trigger states
    trigger <n>=<abcdefgh>-<pass>-<fail> [text]     define trigger state
    trigger check                                   check trigger states
    trigger delete n[-m] [p[-q] ...                 delete trigger state(s)
    trigger undo                                    undo last define or delete
  where:
    state number        0...255
    a,b,c,d,e,f,g,h     state bits:
                          X = don't care     alternate chars: %s
                          0 = port low       alternate chars: %s
                          1 = port high      alternate chars: %s
    pass                state number, 0...255
    fail                state number, 0...255
    text                optional comments
  Examples:
    trigger  0=xxxx101x-1-0
    trigger 17=11001111-23-15
Trigger state machine pseudocode:
  set state to 0
  loop while not triggered:
      if connected to other units and PB14 external trigger is low:
          triggering complete, exit loop, start sample capture
      else:
          set input to ports PB4...PB11 (lsb...msb)
          set begin_state to state
      loop while checking input:
          if input matches state's bits (masked by "don't care" bits):
              if state's "pass" state is 0:
                  triggering complete, exit both loops, start sample capture
              else:
                  set state to state's "pass" state
                  exit inner "checking input" loop, continue outer loop
          else:
              set state to state's "fail" state
              if state is same as begin_state:
                  exit inner "checking trigger" loop, continue outer loop
              else:
                  continue inner "checking trigger" loop
Triggers:
  - at least one trigger state, state #0, required
  - default state machine: one trigger, "0=xxxxxxxx-0-0
  - trigger state numbers arbitrary in range [0 ... 255]
  - state numbers need not be contiguous (slight download speed advantage if so)
  - "pass" and "fail" states must point to existing/defined trigger states
  - chain of "pass" states back to state 0 (triggering complete) required
  - logical "OR" implemented by chains of "pass" states looping back first in chain or ending with "xxxxxxxx-X-Y" all "don't care" state
  - warnings for above and other requirements reported by "trigger check" and at start of "logic" command
  - cannot trigger on implicitly clocked asynchronous (UART) serial data, only synchronous/clocked (USART, SPI, etc)
  - see "help logic mode" for sampling speed after triggering
  - note triggering speed is slower than sampling speed, especially if multiple logical "OR" states
  - note minor additional slowdown if multiple units connected (see "help reset ganged=")
Triggers examples:
  Trigger immediately (default):
    t 0=xxxxxxxx-0-0 any input matches, go to state 0, triggered
  Trigger on bit pattern:
    t 0=x1x101x0-0-0 trigger if pattern matches, else continue checking input
  Trigger on rising edge on port 9
    t 0=xx0xxxxx-1-0 wait until port PA9 is low
    t 1=xx1xxxxx-0-1 trigger when PA9 goes high
  Trigger on one bit pattern followed by second bit pattern
    t 0=xxxx01xx-1-0 go to state 1 if pattern matches, else recheck
    t 1=111xxxxx-0-1 trigger if new pattern matches, else recheck
  Trigger on one bit pattern followed immediately by second bit pattern
    t 0=xxxxxxxx-1-0 go to wait for state 1 match
    t 1=xxxx01xx-2-1 go to state 2 if pattern matches, else recheck
    t 2=111xxxxx-0-3 trigger if new pattern matches, else ...
    t 3=xxxxxxxx-1-1 ... recheck first pattern
  Trigger on serial 1-0-1 bitstream on PA8, clocked on rising edge on PA6
    t 0=xxxxxxxx-1-0 start state machine
    t 1=xxx0xxxx-2-1 wait until clock line PA8 low
    t 2=xxx1xxxx-3-2 wait until clock line PA8 high
    t 3=xxx1x1xx-4-0 continue if clocked PA6 bit is 1, else restart
    t 4=xxx0xxxx-5-4 wait until clock line PA8 low
    t 5=xxx1xxxx-6-5 wait until clock line PA8 high
    t 6=xxx1x0xx-7-0 continue if clocked PA6 bit is 0, else restart
    t 7=xxx0xxxx-8-7 wait until clock line PA8 low
    t 8=xxx1xxxx-9-8 wait until clock line PA8 high
    t 9=xxx1x1xx-0-0 trigger if clocked PA6 bit is 1, else restart
  Trigger on binary 3, 4, or 10 on ports PA4(lsb) through PA7(msb)
    t 0=xxxxxxxx-1-0 go to check for 3, 4, or 10
    t 1=xxxx0011-0-2 trigger if binary 3,  else check if binary 4
    t 2=xxxx0100-0-3 trigger if binary 4,  else check if binary 10
    t 3=xxxx1010-0-1 trigger if binary 10, else end 3, 4, or 10 check
  Trigger on 7-bit ascii "u", "$", or 'Z", followed immediately by "~", on PA4(lsb) through PA10, all data clocked on falling edge on PA11
    t 0=xxxxxxxx-1-0 start state machine
    t 1=1xxxxxxx-2-1 wait for clock high
    t 2=0xxxxxxx-3-2 wait for clock low
    t 3=01110101-7-4 check if "u", else check if "$"
    t 4=00100100-7-5 check if "$", else check if "Z"
    t 5=01011010-7-6 check if "Z", else end "u" or "$" or "Z" check
    t 6=xxxxxxxx-1-7 end  "u" or "$" or "Z" check
    t 7=1xxxxxxx-8-7 wait for clock high
    t 8=0xxxxxxx-9-8 wait for clock low
    t 9=01111110-0-0 trigger if "~", else restart from beginning
  Never trigger (useful if only external triggering desired, see below):
    t 0=xxxxxxxx-1-0 start state machine
    t 1=00000000-2-1 go to state 2 on success
    t 2=11111111-1-2 go to state 1 on success
External/sync'd triggering:
  - See "help reset ganged=" and "help reset ext-trig"
"""  % (Trigger.IGNORE_CHARS[2:],
        Trigger. ZEROS_CHARS[1:],
        Trigger.  ONES_CHARS[1:])



def dgtl_cmd(cmd, input, fields):
    if not config('logic', 'logic', fields):
        return

    if not check_triggers():
        if not r_u_sure("Trigger errors may prevent triggering "
                        " and/or cause infinite firmware loop "
                        "while ignoring PB4-PB11 input lines "
                        "(<ENTER> will still abort). "
                        "Continue anyway?"                   ):
            return
    max_trig = max([ndx for ndx in triggers_config])

    if digital_config['duration'].enabled():
        duration    = digital_config['duration'].val
        dur_enabled = True
    else:
        duration    = 0
        dur_enabled = False

    buffer = struct.pack("< 6B 3H"                     ,
                         DGTL_CMD                      ,
                         digital_config['mode'    ].val,
                         max_trig + 1                  ,
                           reset_config['ganged'  ].val,
                         dur_enabled                   ,
                         digital_config['code-mem'].val,
                         duration                      ,
                         digital_config['edges'   ].val,
                         0                             )  # 32-bit alignment

    bufndx  = len(buffer)

    default = Trigger('xxxxxxxx-0-0').bytes()
    for ndx in range(max_trig + 1):
        if ndx in triggers_config:
            buffer += triggers_config[ndx].bytes()
        else:
            buffer += default
        bufndx += 4
        if bufndx == MAX_FULL_USB or ndx == max_trig:
            os.write(usb_fd, buffer)
            buffer = b''
            bufndx = 0

    sys.stdout.write("Waiting for sampling finish (<ENTER> to abort) ...  ")
    sys.stdout.flush()
    begin_time = time.time()
    finish = wait_read(6)
    if finish is WAIT_READ_STDIN:
        cmnd_cmd(HALT_CMD)
        finish = wait_read(6)
        if finish is WAIT_READ_STDIN:
            return  # second attempt at user halt, firmware is hung
    else:
        sys.stdout.write('\n')
    if finish is None:
        return  # wait_read(), via size_read(), printed error message
    (mode, halt_code, triggered, num_samples) = struct.unpack('<BBHH', finish)
    Pager()(  "%s: %d samples (%s) in %.2f seconds. Stopped by %s.\n"
             % (triggered_at(triggered)    ,
                num_samples                ,
                sampling_mode_inverse(mode),
                time.time() - begin_time   ,
                halt_name(halt_code)       )                        ,
              immed=True, one_line=True                             )

    if digital_config['autodump'].val:
        upld_cmd(UPLD_CMD, '', [])

dgtl_cmd.__help = """
Digital logic analyzer.
- capture samples of logic levels on ports PB4(lsb) through PB11(msb)
- samples recorded only if level changes on any port
- samples also recorded every 0.233016889 seconds regardless if any change
- sampling starts when trigger condition met (see "help trigger")
- sampling continues until first of:
  . user interrupt (<ENTER> key)
  . duration elapsed ("logic duration=")
  . number of samples ("logic edges=") (incl. extra samples @ 233ms)
  . memory full
- see "help logic mode" for sampling speed
- see "help trigger" for triggering
- see "help reset ganged=" and "help reset ext-trig" for external triggering and multiple device synchronization
- on-board user LED off at start of triggering, back on when sampling finished
- port PB13 toggles at c. 4.29 Hz during sampling, can monitored by external device, drive LED at 3mA max, and/or aid post-processing of multiple device captures
- Use "dump" command to upload, print, save, and/or view samples.
"""



def anlg_cmd(cmd, input, fields):
    if not config('oscope', 'oscope', fields):
        return

    trgr_chnl  = analog_config['trgr-chnl' ].val
    scnd_chnl  = analog_config['scnd-chnl' ].val
    scaling =    analog_config['trigger'   ]
    slope      = analog_config['slope'     ].val

    if slope == Slope.NONE:
        level    = 0x7ff   # needed for level_lo,_hi check but not meaningful
        hyster   = 0       #   "     "     "    , "    "    "   "      "
        level_lo = 0       # sent     but not used
        level_hi = 0xfff   #  "       "   "   "
    else:
        level  = scaling.scaled_level()
        hyster = scaling.scaled_hyst ()

    if   slope == Slope.POSITIVE:
        level_hi = level
        level_lo = level - hyster
    elif slope == Slope.NEGATIVE:
        level_lo = level
        level_hi = level + hyster


    if level_lo < 0:
        hyster = level
        ranged = scaling.ranged_hyst(hyster)
        if r_u_sure(  "Level %g minus hysteresis %g less "
                      "than minimum %g (\"scaling=%s\"). "
                      "Use hysteresis=%g instead ?"
                    % (analog_config['trigger'].level(),
                       analog_config['trigger'].hyst (),
                       scaling.val  [0        ]       ,
                       scaling                        ,
                       ranged                         )   ):
            level_lo = 0
            level_hi = level_lo + hyster
        else:
            return

    if level_hi > 0xfff:  # 12 bit ADC
        hyster = 0xfff - level
        ranged = scaling.ranged_hyst(hyster)
        if r_u_sure(  "Level %g plus hysteresis %g greater "
                      "than minimum %g (\"scaling=%s\"). "
                      "Use hysteresis=%g instead?"
                    % (analog_config['trigger'].level(),
                       analog_config['trigger'].hyst (),
                       scaling.val[1             ],
                       scaling                    ,
                       ranged                     )         ):
            level_hi = 0xfff
            level_lo = level_hi - hyster
        else:
            return

    num_samples = analog_config['samples'].val
    if analog_config['scnd-chnl'].is_special():
        # no second channel, firmware stores two samples in one word
        num_samples //= 2

    buffer = struct.pack("< 6B 3H"                   ,
                         ANLG_CMD                    ,
                         trgr_chnl                   ,
                         scnd_chnl                   ,
                         analog_config['slope'  ].val,
                         analog_config['s/h+adc'].val,
                          reset_config['ganged' ].val, # 16 bit alignment
                         num_samples                 ,
                         level_lo                    ,
                         level_hi                    )
    os.write(usb_fd, buffer)

    sys.stdout.write("Waiting for sampling finish (<ENTER> to abort) ...  ")
    sys.stdout.flush()
    begin_time = time.time()
    finish = wait_read(8)
    if finish is WAIT_READ_STDIN:
        cmnd_cmd(HALT_CMD)
        finish = wait_read(8)
        if finish is WAIT_READ_STDIN:
            return  # second attempt at user halt, firmware is hung
    else:
        sys.stdout.write('\n')
    if finish is None:
        return  # wait_read(), via size_read(), printed error message

    (halt_code    ,
     num_channels,
     channel_ndxs,
     sample_rate ,
     num_samples ,
     triggered   ) = struct.unpack('<4B2H', finish)

    trig_chan_name = adc_configs[channel_ndxs & 0x0f]['name'].val
    if num_channels == 2:
        scnd_chan_name = ',' + adc_configs[channel_ndxs >>    4]['name'].val
        multiplier     = 1
    else:
        scnd_chan_name = ''
        multiplier     = 2

    Pager()(  "%d samples, %d channel%s (%s%s) "
              "at %s in %.2fs wall clock time. "
              "%s. Stopped by %s.\n"
            % (num_samples * multiplier               ,
               num_channels                           ,
               '' if num_channels == 1 else 's'       ,
               trig_chan_name                         ,
               scnd_chan_name                         ,
                    AdcSampHold(sample_rate)
               if   sample_rate <= AdcSampHold.T_239_5
               else '???'                             ,
               time.time() - begin_time               ,
               triggered_at(triggered)                ,
               halt_name(halt_code)                   ),
            immed=True, one_line=True                  )

    if analog_config['autodump'].val:
        upld_cmd(UPLD_CMD, '', [])

anlg_cmd.__help = """
Primitive DSO (digital storage oscilloscope)
- samples one or two analog channels chosen from ports PB0...PB7
- primary/trigger channel: "trgr-chnl="
- second channel: "scnd-chnl=", (set to "none" for single channel operation)
- triggered at specified level (with hysteresis) and slope
  . see "help oscope trigger=" and "help oscope slope="
  . hysteresis implemented due to ADC conversion noise (otherwise slowly rising or falling signals will trigger near level regardless of "slope=" setting) (as noise dithers around level)
  . trigger when signal rises above level after first falling below level minus hysteresis if "slope=positive"
  . trigger when signal falls below level after first rising above level plus hysteresis if "slope=negative"
  . use "slope=disabled" to trigger immediately without regard to level
- samples recorded continuously at rate specified by "s/h+adc="
- sampling continues number specified by "samples=" parameter
  . triggering and/or sampling interruptible via <ENTER> key
- on-board user LED off, and output port PB13 toggles at c. 4 Hz, during triggering and sampling
- Use "dump" command to upload, print, save, and/or view samples.
- See "help reset ganged=" and "help reset ext-trig" for external/sync'd triggering.
"""


def upld_cmd(cmd, input, fields):
    if not config('dump', 'dump', fields):
        return
    first = upload_config['begin'].val
    count = upload_config['count'].val
    os.write(usb_fd, struct.pack('<2B2H2B', UPLD_CMD, 0, first, count, 0, 0))

    samples_header = wait_read(14)
    if samples_header is None or samples_header is WAIT_READ_STDIN:
        return  # wait_read() or size_read() printed error
    (first        ,
     count        ,
     num_samples  ,
     max_memory   ,
     sampling_mode,
     num_adc_chans,
     adc_chan_ndxs,
     adc_samp_rate,
     num_adc_smpls) = struct.unpack("< 4H 4B H", samples_header)

    if     count == 0                                                  \
       or (sampling_mode == SamplingMode.ANALOG and num_adc_smpls == 0):
        Pager()(  "Zero samples uploaded (of max memory capacity %d samples)"
                % max_memory                                                 ,
                immed=True, one_line=True                                    )
        return

    samples = [0] * count
    for ndx in range(count):
        sample = wait_read(4, 2.0)
        if sample in (WAIT_READ_STDIN, None):
            Pager(stream=sys.stderr) \
                 (  "Upload of %d samples interrupted/failed @ #%d\n"
                  % (count, ndx + 1)                                 ,
                  immed=True, one_line=True                          )
            cmnd_cmd(HALT_CMD)
            flush_read(2.0)
            return
        samples[ndx] = struct.unpack('I', sample)[0]

    if sampling_mode == SamplingMode.ANALOG:
        save_as = upload_config['analog-frmt' ].val
        suffix  = upload_config['analog-frmt' ].str()
    else:
        save_as = upload_config['digital-frmt'].val
        suffix  = upload_config['digital-frmt'].str()

    if     upload_config['file'  ].val is not None           \
       and upload_config['output'].val ==     TermOrFile.FILE:
        filename = upload_config['file'].val
        if not filename.endswith('.%s' % suffix):
            filename += '.%s' % suffix
        file = safe_open(filename, " for saving captured data", 'w')
    else:
        filename = None
        file     = None

    if sampling_mode == SamplingMode.ANALOG:
        upload_analog (samples      ,
                       first        ,
                       count        ,
                       max_memory   ,
                       num_adc_chans,
                       adc_chan_ndxs,
                       adc_samp_rate,
                       num_adc_smpls,
                       save_as      ,
                       file         ,
                       filename     )
    else:
        upload_digital(samples      ,
                       first        ,
                       count        ,
                       num_samples  ,
                       max_memory   ,
                       sampling_mode,
                       save_as      ,
                       file         ,
                       filename     )

upld_cmd.__help = """
Upload most recent captured samples (from "logic" or "oscope" commands) to terminal and/or file.
Digital ("logic") samples:
- Initial line, range of samples. Example:
  logic samples: 0...31 @ of 32 (max 4842)
- One line per sample. Example:
    23  0xd3  11010011   gry vio ... grn ... ... red brn        622  8.63889μs
  \--/  \--/  \------/   \-----------------------------/   \------/  \-------/
   |     |       |           |                                |          |
   |     |       |           +-- port name ("configure        |          |
   |     |       |               names <bit>=<name>") if      |          |
   |     |       |               port high else "..."         |          |
   |     |       +-- state of ports PB11(msb) to PB4(lsb)     |          |
   |     +-- state of ports PB11(msb) to PB4(lsb)             |          |
   +-- sample number                                          |          |
                                number of 72MHz ticks since --+          |
                                previous sample                          |
                                                    elapsed time since --+
                                                    previous sample
Analog ("oscope") samples:
- Initial line, range of samples. Example:
  Samples 0...59 of 60 (max 4842)   2 channels (PA0,PA1) per sample at
  239.5+12.5@12MHz->47.6kHz
- One line per one- or two-channel  samples. Example:
  37      1554.00 μs    PA0 1.631V   PA1 77.9°F
  \--/    \----------/  \--------/   \--------/
   |           |             |            |
   |           |             |            +-- second channel value
   |           |             +-- trigger channel value
   |           +-- sample time
   +-- sample number
"""



def live_cmd(cmd, input, fields):
    if not config('monitor', 'monitor', fields):
        return

    if     live_config['output'].val & TermFileBoth.FILE \
       and live_config['file'  ].val is not None:
        filename = live_config['file'].val
        file = safe_open(filename, " for saving monitored data")
    else:
        file = None

    if live_config['output'].val & TermFileBoth.TERM:
        term_size  = shutil.get_terminal_size().lines
        linenumber = 0

    # must call check_usart() before st_usart_settings(), below
    if   spi_config['mode'  ].str() != 'disabled' and not check_spi  (): return
    if usart_config['active'].val                 and not check_usart(): return

    if live_config['duration'].enabled():
        duration = live_config['duration'].val
    else:
        duration = MAX_DURATION

    num_adcs    = 0
    active_adcs = 0
    for (ndx, adc) in enumerate(adc_configs):
        if adc['active'].val:
            active_adcs |= 1 << ndx
            num_adcs    += 1

    os.write(usb_fd,
             struct.pack('<8BQQ'                          ,
                         LIVE_CMD                         ,
                          live_config['pb4-11'    ].val   ,
                         usart_config['active'    ].val   ,
                           spi_config['mode'      ].val[0],   # on/off
                           i2c_config['mode'      ].val[0],   # on/off
                           active_adcs                    ,   # bit flags
                         0, 0                             ,   # align 64 bits
                          duration                        ,   # 64 bits
                          live_config['rate'      ].val   ))  # 64 bits

    if usart_config['active'].val:
        (datalen, parity) = st_usart_settings()  # never None, did check_usart()
        os.write(usb_fd, struct.pack("<10B HI 2I"                    ,
                                     USRT_CMD                          ,
                                     pack_bits(usart_config['xmit'    ].val,
                                               usart_config['recv'    ].val,
                                               usart_config['synchro' ].val,
                                                             datalen       ,
                                               usart_config['idle'    ].val,
                                               usart_config['phase'   ].val,
                                               usart_config['lastclok'].val),
                                     pack_bits(usart_config['rts'     ].val,
                                               usart_config['cts'     ].val,
                                               usart_config['ports'   ].val),
                                                   parity          ,
                                     usart_config['stoplen'   ].val,
                                     usart_config['gpio'      ].val,
                                     usart_config['tx-data'   ].val,
                                     usart_config['rx-len'    ].val,
                                     0, 0                          ,  # align
                                     usart_config['baud'      ].val,  # 16 bits
                                     usart_config['tx-timeout'].val,  # align32
                                     usart_config['rx-wait'   ].val,  # 32 bit
                                     usart_config['rate'      ].val)) # 32 bit

    if spi_config['mode'].val[0]:
        os.write(usb_fd,
                   struct.pack("<8B 4I",
                               SPIB_CMD                           ,
                                     spi_config['mode'].val[1]      ,
                                     pack_bits(spi_config['select'].val,
                                               spi_config['endian'].val,
                                               spi_config['idle'  ].val,
                                               spi_config['phase' ].val,
                                               spi_config['miso'  ].val),
                                     spi_config['baud'     ].val,
                                     spi_config['pull'     ].val,
                                     spi_config['speed'    ].val,
                                 len(spi_config['tx-data'  ].val),
                                     spi_config['nss'      ].val,
                                     spi_config['nss-time' ].val,  # 32-bit
                                     spi_config['busy-wait'].val,
                                     spi_config['rate'     ].val,  # 32-bit
                                     spi_config['rx-wait'  ].val)  # 32-bit
                 +                   spi_config['tx-data'  ].val )

    if i2c_config['mode'].val[0]:
        os.write(usb_fd,
                   struct.pack('<10BHI'                                ,
                               I2C_CMD                                 ,
                               pack_bits(i2c_config['mode'     ].val[1],
                                         i2c_config['flavor'   ].val   ,
                                         i2c_config['fast-duty'].val   ,
                                         i2c_config['gen-call' ].val   ),
                               i2c_config['gpio'   ].val  ,
                               i2c_config['addr'   ].val  ,
                               i2c_config['oar1'   ].val  ,
                               i2c_config['oar2'   ].val  ,
                               i2c_config['tx-data'].len(),
                               i2c_config['rx-size'].val  ,
                               0, 0                       ,
                               i2c_config['freq'   ].ccr(),  # 16-bit
                               i2c_config['timeout'].val  )  # 32-bit
                 +             i2c_config['tx-data'].val   )

    for (channel, adc_conf) in enumerate(adc_configs):
        if adc_conf['active'].val:
            os.write(usb_fd, struct.pack('4B',
                                            channel
                                         | (adc_conf['s/h+adc'].val << 4)      ,
                                         adc_conf  ['scale-hyst'].scaled_hyst(),
                                         adc_conf  ['exponent'].val            ,
                                         adc_conf  ['weight'  ].val           ))

    if live_config['duration'].enabled():
        end_time = time.time() + live_config['duration'].as_float()
    else:
        end_time = MAX_PYTIME

    sys.stdout.write("<ENTER> to end\n")

    error_spi    = False
    error_usart  = False
    error_i2c    = False
    block_spi    = False
    block_usart  = False
    block_i2c    = False
    prev_digital = 0x100  # out of range value

    time_printf = live_config['printf'].val

    while time.time() < end_time:
        # always get 8 bytes timestamp plus one 32-bit word digital value
        monitored = wait_read(12, end_time - time.time())
        if monitored in (WAIT_READ_STDIN, None): break

        timestamp = struct.unpack('<Q', monitored[0:8])[0]
        text      = time_printf % (timestamp / CPU_HZ)

        # always sent, even if not enapled
        digital = struct.unpack('<I', monitored[8:12])[0]

        for ndx in range(num_adcs):
            monitored = wait_read(4, end_time - time.time())
            if monitored in (WAIT_READ_STDIN, None):
                break
            (adc_val, adc_stat, adc_chan) = struct.unpack('<HBB', monitored)
            adc_printf =   "  %%s%s:%s"                     \
                         % (adc_configs[adc_chan]['name'  ].val,
                            adc_configs[adc_chan]['printf'].val)
            text   += adc_printf % (periph_status(adc_stat)[0]          ,
                                     adc_configs[adc_chan]['scale-hyst']
                                    .ranged_hyst(0xfff, adc_val)        )
                                    # not really "hyst", but that's method name

        if live_config['pb4-11'].val:
            text += "  %s" % channel_bits(digital)

        if usart_config['active'].val:
            monitored = wait_read(usart_config['rx-len'].val + 2,
                                  end_time - time.time()        )
            if monitored in (WAIT_READ_STDIN, None):
                break
            (usart_len, usart_stat) = struct.unpack('BB', monitored[:2])
            text += "  %su:" % periph_status(usart_stat)[0]
            if usart_config['ascii-num'].val == AsciiNumeric.ASCII:
                text += ''.join([SAFE_ASCII[byte] for byte in monitored[2:]])
            else:
                text += '.'.join(['%02x' % byte for byte in monitored[2:]])

        if spi_config['mode'].val[0]:
            monitored = wait_read(len(spi_config['tx-data'].val) + 2,
                                  end_time - time.time()            )
            if monitored in (WAIT_READ_STDIN, None):
                break
            (spi_len, spi_stat) = struct.unpack('BB', monitored[:2])
            text += "  %ss:" % periph_status(spi_stat)[0]
            if spi_config['ascii-num'].val == AsciiNumeric.NUMERIC:
                text += '.'.join(['%02x' % byte for byte in monitored[2:]])
            else:
                text += decode_escape(monitored[2:])

        if i2c_config['mode'].val[0]:
            monitored = wait_read(i2c_config['rx-size'].val + 3,
                                  end_time - time.time()       )
            if monitored in (WAIT_READ_STDIN, None):
                break
            (i2c_stat, i2c_addr, i2c_rcvd) = struct.unpack('BBB', monitored[:3])
            if i2c_addr not in range(3): i2c_addr = 3  # sanity check
            text += "  %s%si:" % (periph_status(i2c_stat)[0],
                                  '012m'       [i2c_addr]   )
            text += '.'.join(['%02x' % byte for byte in monitored[3:]])


        if    ( live_config['pb4-11'].val    and digital != prev_digital) \
           or  num_adcs > 0                                               \
           or (usart_config['active'].val    and not block_usart        ) \
           or (  spi_config['mode'  ].val[0] and not block_spi          ) \
           or (  i2c_config['mode'  ].val[0] and not block_i2c          ):

            if file:
                file.write("%s\n" % text)
            if live_config['output'].val & TermFileBoth.TERM:
                sys.stdout.write('%s\n' % text)
                linenumber += 1
                if linenumber >= term_size - 1:
                    sys.stdout.write("(<ENTER> to halt)\n")
                    linenumber = 0

        if error_usart:
            block_usart = True
            error_usart = False
        if error_spi:
            block_spi = True
            error_spi = False
        if error_i2c:
            block_i2c = True
            error_i2c = False
        if live_config['pb4-11'].val:
            prev_digital = digital

    # on timeout or user halt
    if time.time() >= end_time: sys.stdout.write('\n')
    cmnd_cmd(HALT_CMD)
    flush_read(1.0)
    if file:
        file.close()
    return

live_cmd.__help = """
Monitor and/or log digital, analog, spi, and/or usart inputs
Continue until "duration=<time>" (if not "infinite") elapsed, or user interrupt via <ENTER> key.
Prints text line with monitored data to terminal ("output=terminal"), file ("file=<filename> output=file) or both ("file=<filename> output=both")
Lines printed if rate limiting ("rate=<time>") time elapsed and any of the following conditions met:
  digital:("pb4-11=enabled")
    - level change on any digital port PB4 through PB11
  analog: ("adcN enable")
    - analog voltage change on enabled analog configs "adc<N>" for N in 0...7
    - values filtered by "adcN exponent=", "weight=", and "hysteresis=" parameters
      via formula:
        filtered_voltage =   filtered_voltage * (pow(2,exponent) - weight)
                           + current_voltage  * (                  weight)
      but only if:
        abs(filtered_voltage - previous_printed_voltage) > hysteresis
    - values printed using "adcN scale-hyst=" scaling, with "name=" name, and "printf=" printf formula including optional additional text
    - Example:
      Port PA4 connected to analog temperature sensor with 0 to 3.3V output corresponding to -20 to 160 degrees Fahrenheit:
      monitor adc4 enable scale-hyst=-20:160:0.5 exponent=3 weight=5 name=outdoors printf=%5.1f\\x20F
      If scaled current value filtered with previous value using ratio of 5/8 to 3/8 is 82 and previous printed value was less than 81.5 or greater than 82.5 will print:
      Xoutdoors: 82.0 F
      (see below for "x" error status character, normally blank space)
  usart: ("usart enable")
    - "usart synchro=disabled": if RX data received (note also "usart tx-data=<byte>" sent on TX port each "rate=<time>" period)
    - "usart synchro=enabled": if read RX data read on port different than previous
  spi:
    "mode=master": Send "tx-data=" on MOSI once per "rate=" period. Print received MISO data if different than previous printed
    "mode=slave": Print received MOSI data (having sent "tx-data=" on MISO one-to-one with MOSI bytes)
  i2c:
    "mode=master": Send "tx-data=" on per "rate=" period. Print received slave TX  data if different than previous printed
    "mode=slave": Print received master data. Also sends "tx-data=" (if any), as slave-tx-to-master.
Prints one or more fields depending on enabled configurations/parameters:
  all:
      004.491
      \-----/  seconds since command start ("monitor printf=<format code>")
  digital: ("pb4-11=enabled") state of digital ports PB4 to PB11. Example:
      10111011   gry ... blu grn yel ... red brn
      \------/   \-----------------------------/
         |           |
         |           +-- port name ("configure names <bit>=<name>")
         |               if port high else "..."
         +-- state of ports, PB11(msb) to PB4(lsb)
  analog: ("adcN enable") voltage on analog port PA<N>. Example:
    (see above for more details)
       PA2:1.963V
      |||||||||||
      |||||++++++-- "scale-hyst=" scaled value, "printf=" formula
      ||||+-------- ascii ":" character
      |+++--------  "name=" name
      +-----------  error code, normally blank (see below)
  usart: ("usart enable=enabled"), RX data on port PA10
       u:pr.rs...
      |||||||||||
      |||++++++++-- RX data bytes, length of "spi tx-data=",
      |||             two hexadecimal digits per byte, separated by "."
      ||+---------- ascii ":" character
      |+----------- ascii "u" character
      +------------ error code, normally blank (see below)
  spi:
       s:pr.rs...
      |||||||||||
      |||++++++++-- MISO or MOSI data bytes, length of "spi tx-data=",
      |||             two hexadecimal digits per byte, separated by "."
      ||+---------- ascii ":" character
      |+----------- ascii "s" character
      +------------ error code, normally blank (see below)
  i2c:
       mi:pr.rs...
      ||||||||||||
      ||||++++++++-- received data bytes (master or slave),
      ||||             two hexadecimal digits per byte, separated by "."
      |||+---------- ascii ":" character
      ||+----------- ascii "i" character
      |+------------ master:ascii "m"  slave:"0"/"1"/"2"==gen-call/oar1/oar2
      +------------- error code, normally blank (see below)
  all:
      (<ENTER> to abort) ...
  error codes:
      " "   no error
      "E"   receive data register empty (usart, spi)
      "B"   peripheral busy
      "O"   receive data overrun
      "T"   send or receive data hardware timeout
      "F"   generic error
      "L"   line break (usart)
      "N"   noise (usart)
      "P"   parity (usart)
      "U"   framing (usart)
      "S"   start (i2c)
      "A"   address (i2c)
      "N"   nack (i2c)
      "X"   missing BTF ("byte transfer finished") flag (i2c)
      "R"   missing RXNE ("receive register not empty") flag (i2c)
      "P"   missing STOP (i2c)
"""




def wave_cmd(cmd, input, fields):
    if not config('pulse', 'pulse', fields):
        return
    os.write(usb_fd, struct.pack('<12B5H2B'                    ,
                                 WAVE_CMD                      ,
                                 pulse_config['active'].val    ,
                                 pulse_config['mode1' ].val    ,
                                 pulse_config['mode2' ].val    ,
                                 pulse_config['mode3' ].val    ,
                                 pulse_config['gpio1' ].val    ,
                                 pulse_config['gpio2' ].val    ,
                                 pulse_config['gpio3' ].val    ,
                                 pulse_config['speed1'].val    ,
                                 pulse_config['speed2'].val    ,
                                 pulse_config['speed3'].val    ,
                                 0                             ,
                                 pulse_config['time'   ].val[0],
                                 pulse_config['time'   ].val[1],
                                 pulse_config['time1'  ].val   ,
                                 pulse_config['time2'  ].val   ,
                                 pulse_config['time3'  ].val   ,
                                 0, 0                          ))
wave_cmd.__help="""
Output digital pulse waveforms on ports PA1, PA2, and/or PA3.
- Waveform output continues, even after program exit, until explicitly halted with "pulse enable=disabled", or "gpio", "numbers", or \"usart ports=pa0-3\" commands (which use same ports/pins).
- "time1", "time2", and "time3" specify event times less than or equal to master "time" period
- At event times "time1"/"time2"/"time3" ports PA1/PA2/PA3 respectively change level in various ways controlled by "mode1", "mode2", and "mode3"
- At end of master "time" period ports PA1/PA2/PA3 return to default states
"""



def parallel_cmd(cmd, input, fields):
    if not config('parallel', 'gpio', fields):
        return
    if         parallel_config['ascii-num'].val == AsciiNumeric.ASCII \
       and not parallel_config['end'].val:
        sys.stderr.write("Must set non-null \"end\" parameter\n")
        return
    os.write(usb_fd, struct.pack('<4BI',
                                 PARL_CMD,
                                 parallel_config['speed'    ].val,
                                 parallel_config['open-pull'].val,
                                 0                               ,
                                 parallel_config['rate'     ].val))
    usb_bridge_synchro(parallel_config['ascii-num'].val,
                       parallel_config['end'      ].val,
                       xmit_only=True                  )
parallel_cmd.__help = """
Output 8-bit parallel data on port PA4(lsb) through PA11(msb).
- Input line of ascii or numeric data (\"gpio ascii-num=") then <ENTER> key to send. See data entry instructions at command start.
- Line editing and history supported, type "help help" for more info.
- Enter \"999\" if \"ascii-num=numeric\" or end string (\"end=\" parameter) if \"ascii-num=ascii\", followed by <ENTER> key to exit command.
"""



def usart_cmd(cmd, input, fields):
    if not config('usart', 'usart', fields):
        return
    if not usart_config['xmit'].val and not usart_config['recv'].val:
        sys.stderr.write("Must set \"usart\" \"xmit=enabled\" "
                         "or/and \"recv=enabled\"\n"           )
        return
    if         usart_config['ascii-num'].val == AsciiNumeric.ASCII \
       and not usart_config['end'].val:
        sys.stderr.write("Must set non-null \"end\" parameter\n")
        return

    # must call check_usart() before st_usart_settings()
    if not check_usart() or not check_data_socket_pty():
        return
    (datalen, parity) = st_usart_settings()  # never None, did check_usart()
    os.write(usb_fd, struct.pack("<10B HI 2I"                    ,
                                 USRT_CMD                          ,
                                 pack_bits(usart_config['xmit'    ].val,
                                           usart_config['recv'    ].val,
                                           usart_config['synchro' ].val,
                                                         datalen       ,
                                           usart_config['idle'    ].val,
                                           usart_config['phase'   ].val,
                                           usart_config['lastclok'].val),
                                 pack_bits(usart_config['rts'     ].val,
                                           usart_config['cts'     ].val,
                                           usart_config['ports'   ].val),
                                              parity           ,
                                 usart_config['stoplen'   ].val,
                                 usart_config['gpio'      ].val,
                                 usart_config['tx-data'   ].val,
                                 usart_config['rx-len'    ].val,
                                 0, 0                          ,  # align
                                 usart_config['baud'      ].val,  # 16 bits
                                 usart_config['tx-timeout'].val,  # align32
                                 usart_config['rx-wait'   ].val,  # 32 bit
                                 usart_config['rate'      ].val)) # 32 bit

    if ipc_config['i/o'].str() == 'terminal':
        if usart_config['recv'].val:
            usb_bridge_async  (usart_config['ascii-num'].val,
                               usart_config['end'      ].val)
        else:
            usb_bridge_synchro(usart_config['ascii-num'].val,
                               usart_config['end'      ].val,
                                xmit_only=True              )
    else:
        usb_bridge_sock_pty(usart_config['snoop'].val)

    if usart_config['recv'].val and ipc_config['flush'].val != 0.0:
        flush_read(ipc_config['flush'].val)

usart_cmd.__help = """
Transmit and/or receive UART/USART serial data.
  - Asynchronous/UART ("synchro=disabled") or synchronous/USART ("synchro=disabled" modes
  - Uses one of two USART peripherals and sets of I/O ports:
    "usart ports=pa8-10": TX=PA9  RX=PA10, CLK=PA8
    "usart ports=0-30pa": TX=PA2  RX=PA3   CTS=PA0  RTS=PA1


  - Synchronous clock output on port PA8 ("master" mode)
  - Hardware does not support synchronous "slave" mode (clock input)
  - Synchronous "master" mode with "datalen=8bits", "parity=none" can compatibly connect to SPI slave device
  - TX and RX data to and from terminal, or socket and/or pty (see "help usart ipc term-ptysock=", "help usart socket", and "help usart pty")
  - Usart xmit and/or recv data to socket and/or pty if either active, else to terminal
Terminal I/O, asynchronous mode:
  - If "recv=enabled" will print asynchronous recv data to terminal.
  - <ENTER> key to pause recv data and enter line of ascii data and/or <\\xXY\> hexadecimal values, <ENTER> key to xmit.
Terminal I/O, synchronous mode:
  - "master" mode: enter line of ascii data and/or <\\xXY\> hexadecimal values, <ENTER> key to xmit
  - Will display recv data bytes (same number as xmitted) and prompt for next line of xmit data
Terminal I/O, both modes:
  - Input line of ascii or numeric data (\"gpio ascii-num=") then <ENTER> key to send. See data entry instructions at command start.
  - Line editing and history supported, type "help help" for more info.
  - Enter \"999\" if \"ascii-num=numeric\" or end string (\"end=\" parameter) if \"ascii-num=ascii\", followed by <ENTER> key to exit command.
"""




def spi_cmd(cmd, input, fields):
    if not config('spi', 'spi', fields):
        return
    if not spi_config['mode'].val[0]:
        sys.stderr.write("spi disabled\n")
        return
    if         spi_config['ascii-num'].val == AsciiNumeric.ASCII \
         and not spi_config['end'].val:
        sys.stderr.write("Must set non-null \"end\" parameter\n")
        return
    if not check_spi() or not check_data_socket_pty():
        return
    if          str(spi_config['rate'     ])            \
           != 'unlimited'                               \
       and          spi_config['busy-wait'].as_float()  \
                  + spi_config['rx-wait'  ].as_float()  \
           >= 0.9 * spi_config['rate'     ].as_float():
        sys.stdout.write(  "Warning: \"busy-wait=%s\" + \"rxwait=%s\" "
                           "close to or greater than \"rate=%s\" "
                           " -- may cause \"rate=\" inaccuracies\n"
                         % (spi_config['busy-wait'],
                            spi_config[  'rx-wait'],
                            spi_config[     'rate'])                    )

    os.write(usb_fd,
               struct.pack("<8B 4I",
                           SPIB_CMD                                   ,
                                 spi_config['mode'].val[1]            ,
                                 pack_bits(spi_config['xmit-only'].val,
                                           spi_config['select'   ].val,
                                           spi_config['endian'   ].val,
                                           spi_config['idle'     ].val,
                                           spi_config['phase'    ].val,
                                           spi_config['miso'     ].val),
                                 spi_config['baud'     ].val,
                                 spi_config['pull'     ].val,
                                 spi_config['speed'    ].val,
                             len(spi_config['tx-data'  ].val),
                                 spi_config['nss'      ].val,
                                 spi_config['nss-time' ].val,  # 32-bit
                                 spi_config['busy-wait'].val,
                                 spi_config['rate'     ].val,  # 32-bit
                                 spi_config['rx-wait'  ].val)  # 32-bit
             +                   spi_config['tx-data'  ].val )
    if ipc_config['i/o'].str() == 'terminal':
        if spi_config['mode'].val[1]:  # master
            usb_bridge_synchro(spi_config['ascii-num'].val,
                               spi_config['end'      ].val,
                               spi_config['xmit-only'].val)
        else:  # slave
            usb_bridge_async  (spi_config['ascii-num'].val,
                               spi_config['end'      ].val)
    else:
        usb_bridge_sock_pty(spi_config['snoop'].val)

    if not spi_config['xmit-only'].val and ipc_config['flush'].val != 0.0:
        flush_read(ipc_config['flush'].val)

spi_cmd.__help = """
Send SPI MOSI and/or receive MISO data on ports PA7 and PA6.
  - SPI master or slave mode
  - SPI clock on port PA5 ( output if master, input if slave)
  - SPI select ("NSS") on port PA4 (see "help spi select=" and "help spi nss=")
  - MOSI and MISO data to and from terminal, or socket and/or pty (see "help spi ipc term-ptysock=", "help spi socket", and "help spi pty")
Terminal I/O, master mode:
  - Enter line of numeric or ascii data (format as per instructions printed at command start, mode set with "ascii-num=" parameter), then <ENTER> key to send on MOSI.
  - Will send data on MOSI and print received MISO data (same number of bytes as sent) to terminal (unless "xmit-only=enabled") and prompt for next line of MOSI data
  - As per SPI protocol, will receive MISO data regardless of whether slave device connected (defaults to zero)
Terminal I/O, slave mode:
  - Prints received MOSI data, format as per "ascii-num=" parameter.
  - <ENTER> key to pause, then enter MISO data (format as per instructions printed at command start, mode set with "ascii-num=" parameter), then<ENTER> to queue MISO data and return to MOSI data printing
  - MISO data will be sent one-to-one with subsequent received MOSI bytes
  - If MISO data queue empty (e.g. at command start) default "tx-data=" (number of bytes and byte values) sent on MISO
  - MOSI bytes printed in groups, size determined by lesser of number of queued or default MISO bytes and MOSI bytes received (unless "rx-wait=" time elapsed)
Terminal I/O, both modes:
  - Line editing and history supported, type "help help" for more info.
  - Enter \"999\" if \"ascii-num=numeric\" or end string (\"end=\" parameter) if \"ascii-num=ascii\", followed by <ENTER> key to exit command.
pty/socket I/O:
  - See "help spi ipc term-ptysock=", "help spi pty" and "help spi socket"
pty/socket I/O, master mode:
  - Data received from pty and/or socket sent via MOSI. Same number of bytes received on MISO sent to pty and/or socket(s)
pty/socket I/O, slave mode:
  - Data received from pty and/or socket enqueued for sent via MISO, one-to-one with received MOSI data
  - Data received from MOSI sent to pty and/or socket(s)
"""



def i2c_cmd(cmd, unused, fields):
    if not config('i2c', 'i2c', fields):
        return
    if not i2c_config['mode'].val[0]:
        return
    os.write(usb_fd,   struct.pack("<10BHI"                                 ,
                                   I2C_CMD                                  ,
                                       pack_bits(i2c_config['mode'     ].val[1],
                                                 i2c_config['flavor'   ].val,
                                                 i2c_config['fast-duty'].val,
                                                 i2c_config['gen-call' ].val),
                                       i2c_config['gpio'      ].val   ,
                                       i2c_config['addr'      ].val   ,
                                       i2c_config['oar1'      ].val   ,
                                       i2c_config['oar2'      ].val   ,
                                       i2c_config['tx-data'   ].len() ,
                                       i2c_config['rx-size'   ].val   ,
                                       0, 0                           ,
                                       i2c_config['freq'      ].ccr() , # 16-bit
                                       i2c_config['timeout'   ].val   ) # 32-bit
                     + i2c_config['tx-data'   ].val                    )

    if i2c_config['mode'].val[1]:
        i2c_master()
    else:
        i2c_slave()
i2c_cmd.__help = """
Send and/or receive I2C data
  - I2C master or slave mode
  - I2C clock (SCL) on port PB10, data (SDA) on port PB11
  - Master send to, and optionally receives data from, I2C address
  - Slave receive data from master at one or more I2C addresses, and optionally send TX data to master.
  - I2C data to and from terminal (only), input and output in hexadecimal, decimal, and/or ascii values
Master ("i2c mode=master")
  - Enter lines with slave address, size of RX data to receive from slave, and optional data bytes to TX to slave, finished with <ENTER> key
  - Data bytes formatted as space separated entries, each as either 2 hexadecimal digits, 3 decimal digits, or one ascii character
  - Multiple "<address> <rx-size> [data ...]" commands separated by ";" allowed
  - Prints peripheral status, number of bytes sent and received, and received byte values (if any) in "ddd-hh-a" format ("ddd" 3 decimal digits, "hh" two hexadecimal digits, "a" one ascii character or "." if unprintable)
  - Example:
    "0 0" to end: 8 2 097 61 a
    stat= OK tx=3  rx=2  : 077-4d-M 078-4e-N
  - Enter "0 0" to exit command
  - See help text printed at command start
Slave ("i2c mode=slave")
  - Prints data received from master with status, size of data, which address was matched ("oar1=", "oar2=", "gen-call=") and size of TX data sent to master (if any requested by master)
  - Received data bytes printed in "ddd-hh-a" format ("ddd" 3 decimal digits, "hh" two hexadecimal digits, "a" one ascii character or "." if unprintable)
  - Examples:
    stat= OK tx=2  rx=3 @A1 : 097-61-a 097-61-a 097-61-a
    stat= OK tx=1  rx=4 @A2 : 100-64-d 101-65-e 102-66-f 103-67-g
    stat= OK tx=0  rx=1 @GC : 120-78-x
  - Note: TX slave-to-master not allowed for "gen-call" address 0
  - Press <ENTER> key to set maximum number of data bytes to receive from master (any additional will be discarded), and optional data bytes to queue to be sent to master (format: space-separated triple decimal or double hexadecimal digits, or single ascii characters), finished with <ENTER> key
  - If no slave-to-master data enqueued, will cyclically send "i2c tx-data=" bytes for any number of slave TX bytes requested by master
  - Enter "0" to exit command
  - See help texts printed at command start and at <ENTER> key
Both modes:
  - Line editing and history supported, type "help help" for more info.
  - Note I2C multi-master, 10-bit addresses, SMBus mode, etc. not supported.
"""



def counter_cmd(cmd, input, fields):
    if not config('numbers', 'numbers', fields):
        return
    if counter_config['rate'].val == 0:
        if    counter_config['gpio-speed'].val     \
           == GpioSpeed.strings_and_values['2MHz']:
            sys.stdout.write("WARNING: Use 10MHz or 50MHz gpio "
                             "with unlimited rate\n")
        use = 'unlimited'
    else:
        use = 'limited'
    if counter_config[use].enabled():
        timed         = 1
        duration_text = " (or wait %s)" % counter_config[use]
        duration      = counter_config[use].val
    else:
        timed         = 0
        duration_text = ''
        duration      = (1 << 64) - 1  # used by rate limited, centuries
    os.write(usb_fd,
             struct.pack('<5B B 2B IQ'                       ,
                         CNTR_CMD                        ,
                         counter_config['gpio-speed'].val,
                         counter_config['open-pull' ].val,
                         timed                           ,
                         counter_config['mode'      ].val,
                         counter_config['increment' ].val,
                         counter_config['low'       ].val,
                         counter_config['high'      ].val,
                         counter_config['rate'      ].val,   # aligned 32 bits
                         duration                        ))  # aligned 64 bits
    sys.stdout.write("<ENTER> to halt%s ...   " % duration_text)
    sys.stdout.flush()
    finish = wait_read(1)
    if finish is WAIT_READ_STDIN:
        cmnd_cmd(HALT_CMD)
        finish = wait_read(1)
        if finish is WAIT_READ_STDIN:
            return  # second attempt at user halt, firmware is hung
    if finish is not None:
        sys.stdout.write("halted by %s\n" % halt_name(finish[0]))
    # else wait_read() printed error message
counter_cmd.__help = """
Output binary values on ports PB-4 (lsb) to PB-11 (msb).
- Configurable rate, or unlimited (maximum firmware speed)
- Incrementing/decrementing values
- High/low limits
- Binary or gray code
"""



### command/config mappings (must be after commands)
#
#

Cmd = collections.namedtuple('Cmd', "code func sets")
COMMANDS = {
    'configure'     : Cmd(CONF_CMD,     conf_cmd, ('configure',
                                                   'lines'    ,
                                                   'numbers'  ,
                                                   'gpio'     ,
                                                   'usart'    ,
                                                   'adc0'     ,
                                                   'adc1'     ,
                                                   'adc2'     ,
                                                   'adc3'     ,
                                                   'adc4'     ,
                                                   'adc5'     ,
                                                   'adc6'     ,
                                                   'adc7'     ,
                                                   'spi'      ,
                                                   'i2c'      ,
                                                   'pulse'    ,
                                                   'monitor'  ,
                                                   'logic'    ,
                                                   'oscope'   ,
                                                   'dump'     ,
                                                   'reset'    ,
                                                   'socket'   ,
                                                   'pty'      ,
                                                   'ipc'      )),
    'trigger'       : Cmd(''       ,     trig_cmd, ('triggers',
                                                              )),
    'logic'         : Cmd(DGTL_CMD,     dgtl_cmd, ('logic'    ,
                                                              )),
    'oscope'        : Cmd(ANLG_CMD,     anlg_cmd, ('oscope'   ,
                                                   'adc0'     ,
                                                   'adc1'     ,
                                                   'adc2'     ,
                                                   'adc3'     ,
                                                   'adc4'     ,
                                                   'adc5'     ,
                                                   'adc6'     ,
                                                   'adc7'     )),
    'dump'          : Cmd(UPLD_CMD,     upld_cmd, ('dump'     ,
                                                   'lines'    ,
                                                   'oscope'   ,
                                                   'adc0'     ,
                                                   'adc1'     ,
                                                   'adc2'     ,
                                                   'adc3'     ,
                                                   'adc4'     ,
                                                   'adc5'     ,
                                                   'adc6'     ,
                                                   'adc7'     )),
    'monitor'       : Cmd(LIVE_CMD,     live_cmd, ('monitor'  ,
                                                   'lines'    ,
                                                   'usart'    ,
                                                   'spi'      ,
                                                   'i2c'      ,
                                                   'adc0'     ,
                                                   'adc1'     ,
                                                   'adc2'     ,
                                                   'adc3'     ,
                                                   'adc4'     ,
                                                   'adc5'     ,
                                                   'adc6'     ,
                                                   'adc7'     )),
    'pulse'         : Cmd(WAVE_CMD,     wave_cmd, ('pulse'    ,
                                                              )),
    'gpio'          : Cmd(PARL_CMD, parallel_cmd, ('gpio'     ,
                                                   'ipc'      ,
                                                              )),
    'usart'         : Cmd(USRT_CMD,    usart_cmd, ('usart'    ,
                                                   'ipc'      ,
                                                   'pty'      ,
                                                   'socket'   ,)),
    'spi'           : Cmd(SPIB_CMD,      spi_cmd, ('spi'      ,
                                                   'ipc'      ,
                                                   'pty'      ,
                                                   'socket'   )),
    'i2c'           : Cmd(I2C_CMD,       i2c_cmd, ('i2c'      ,
                                                              )),
    'numbers'       : Cmd(CNTR_CMD,  counter_cmd, ('numbers'  ,
                                                              )),
    'reset'         : Cmd(RSET_CMD,    reset_cmd, ('reset'    ,
                                                              )),
    'warranty'      : Cmd(WRTY_CMD,      wrty_cmd, None        ),
    'help'          : Cmd(HELP_CMD,      help_cmd, None        ),
    'using'         : Cmd(USNG_CMD,     using_cmd, None        ),
    'quit'          : Cmd(QUIT_CMD,      quit_cmd, None        ),
}




### readline completer
#
#

class Completer:
    __slots__ = ['__completions', '__previous']
    def __init__(self):
        self.flush_cache()

    def flush_cache(self):
        self.__completions = None
        self.__previous    = ()

    def complete(self, text, state):
        """Context sensitive completion of text (possibly empty string)"""
        line_buf = readline.get_line_buffer()
        text_span = (readline.get_begidx(), readline.get_endidx())

        # only complete full words, otherwise completion replaces
        #   substring in existing word (wrong)
        if     text_span[1] < len(line_buf)         \
           and line_buf[text_span[1]] not in (" \t"):
            return None

        current = (text_span, line_buf)

        if     self.__completions                \
           and current == self.__previous        \
           and state   <  len(self.__completions):  # safety check
            return self.__completions[state]
        else:
            self.__completions = None
            self.__previous    = current

        # delete past completion text
        line_buf = line_buf[:text_span[1]]
        tokens = line_buf.replace('\t', " ").split()   # in case pathological \t
        if text == '' and (line_buf == '' or tokens[-1][-1] != '='):
            tokens.append(text)

        command  = None
        config   = None
        commands = []
        configs  = []
        settings = []
        actions  = []
        params   = []

        disambig = disambiguate(tokens[0], COMMANDS.keys())
        if (disambig.count == 1 or disambig.exact) and disambig.good == 'help':
            del(tokens[0])
            help = True
        else:
            help = False

        if not tokens:
            self.__completions = ["help ", None]
            return self.__completions[state]

        if help:
            keys = COMMANDS.keys()
        else:
            keys = [key for key in COMMANDS.keys() if key != 'using']
        disambig = disambiguate(tokens[0], keys)
        del(tokens[0])
        commands = disambig.matches
        if disambig.count == 1 or disambig.exact:
            command = disambig.good
            configs = COMMANDS[command].sets
            if configs:
                config   = configs[0]
                params   = [    str(key) + '='   # triggers keys are ints
                            for key
                            in  SETTINGS[config].keys()]
            elif len(tokens) == 0:
                self.__completions = [command + " ", None]
                return self.__completions[state]
            else:
                self.__completions = [None]
                return None

        if not tokens:
            self.__completions = [cmd + " " for cmd in commands] + [None]
            return self.__completions[state]

        for (ndx, token) in enumerate(tokens):
            if not '=' in token:
                is_action = False
                # preference given to actions
                if config and hasattr(SETTINGS[config], '_actions'):
                    actions   = list(SETTINGS[config]._actions)
                    disambig = disambiguate(token, actions)
                    if disambig.count == 1:
                        actions   = [disambig.good]
                        is_action = True
                if not is_action:
                    disambig = disambiguate(token, configs)
                    if disambig.count == 1:  # and disambig.exact
                        config = disambig.good
                        commands = []
                        if ndx != len(tokens) - 1:
                            params   = [    key + '='
                                        for key
                                        in  SETTINGS[config].keys()]

        if '=' in tokens[-1]:
            (param, value) = tokens[-1].split('=')
            disambig = disambiguate(param, params)
            param_choices = disambig.matches
            found = False
            if len(param_choices) == 1:
                param = param_choices[0][:-1]
                found = True
            if config == 'triggers':
                param = int(param)
            if config and param in SETTINGS[config].keys():
                found = True
            if not found:
                self.__completions = None
                self.__previous    = []
                return None
            instance = SETTINGS[config][param]
            if hasattr(instance, 'complete'):
                self.__completions = instance.complete(value)
                self.__completions.append(None)
                return self.__completions[state]
            else:
                self.__completions = None
                self.__previous    = []
                return None

        possibles = params + actions
        if not command:
            possibles += commands
        if len(configs) > 1 or help:
            possibles += configs

        choices  = disambiguate(tokens[-1], possibles).matches

        self.__completions = [    choice + ('' if choice in params else " ")
                              for choice
                              in  choices                                   ]
        self.__completions.append(None)

        return self.__completions[state]



'''
# For displaying input back in terminal after "?" help.
# Doesn't work, never called despite readline.set_pre_input_hook(pre_input_hook)
# Same with readline.set_startup_hook()
# User has to use up-arrow to restore and re-edit line.
__repeat_user_input = ''
def pre_input_hook():
    sys.stderr.write("pre_inut_hook: %d\n" % len(__repeat_user_input))  # DEBUG
    if __repeat_user_input:
        readline.insert_text(__repeat_user_input)
        __repeat_user_input = ''
        readline.redisplay
'''




### main
#
#

def main(quiet, acm_name, no_auto_viewer, load_file, halt):
    if not quiet:
        sys.stdout.write(BANNER)

    global dev_acm, usb_fd

    if acm_name:
        if   acm_name.startswith('/dev'): dev_acm =              acm_name
        elif acm_name.startswith( 'tty'): dev_acm = '/dev/'    + acm_name
        elif acm_name.startswith(   '.'): dev_acm =              acm_name
        else:                             dev_acm = '/dev/tty' + acm_name

    usb_connect()

    if halt and halt != 'none':
        if halt[0] in ('usart', 'spi', 'i2c'): cmnd_cmd(0)  # zero-length data
        else:                                  cmnd_cmd(HALT_CMD)
    else:
        firmware_connect(quiet)

    if not no_auto_viewer:
        set_viewers(quiet)

    if load_file:
        load_config_file(load_file)

    readline.parse_and_bind('tab: complete')
    completer = Completer()
    readline.set_completer       (completer.complete)
    readline.set_completer_delims("= \t"            )
    # readline.set_pre_input_hook(pre_input_hook    ) # see pre_input_hook above

    cmnd_history = []
    data_history = []

    empty_line_helped = False
    while True:
        user_input = safe_input("$1.50: "                        ,
                                "main"                           ,
                                "Use \"quit\" (or \"q\") to exit")
        completer.flush_cache()  # so doesn't complete with old if same input

        if not user_input:
            if not empty_line_helped:
                sys.stdout.write("Type \"help\" or other command "
                                 "(press <TAB> key twice for list)\n")
                empty_line_helped = True
            continue
        else:
            empty_line_helped = False

        cmnd_history.append(user_input)
        user_input = user_input.strip()

        if '#' in user_input:
            user_input = user_input[:user_input.find('#')]
        if not user_input: continue

        fields = user_input.split()
        try:
            pos = next(    ndx
                       for (ndx, field)
                       in  enumerate(fields)
                       if  field.endswith('?'))
        except StopIteration:
            pass
        else:
            final = fields[pos]
            fields = ['help'] + fields[:pos]
            if final != '?':
                fields.append(final[:-1])
            user_input = " ".join(fields)
            # __repeat_user_input = user_input.replace('?', '')  # see above

        command = fields[0]
        disambig = disambiguate(command, COMMANDS.keys())
        if disambig.count == 1:
            cmd = disambig.good

            readline.clear_history()
            for data in data_history:
                readline.add_history(data)

            COMMANDS[cmd].func(COMMANDS[cmd].code, user_input, fields[1:])

            data_hist_len = readline.get_current_history_length()
            for ndx in range(len(data_history) + 1, data_hist_len + 1):
                data_history.append(readline.get_history_item(ndx))
            readline.clear_history()
            for cmnd in cmnd_history:
                readline.add_history(cmnd)
        else:
            Pager(stream=sys.stderr)('%s\n' % disambig.error,
                                     immed=True, final=False)



def parse_commandline():
    parser = argparse.ArgumentParser(
                formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument('-v', '--version',
                        action="store_true",
                        help="Print program/protocol version and exit")

    parser.add_argument('-q', '--quiet',
                        action="store_true",
                        help="Don't print startup info")

    parser.add_argument('-f', '--file',
                        nargs='?',
                        default=None,
                        help="Config file to load at startup (see "
                             "\"help configure load\"")

    parser.add_argument('-x', '--no-auto-viewer',
                        action="store_true",
                        help="Disable automatic, pre-\"--file\" search "
                             "for waveform viewing software and setting "
                             "of \"logic autodump=\", \"oscope autodump=\", "
                             "and \"dump auto-digital= auto-analog= "
                             "viewer-csv= viewer-vcd= digital-frmt= "
                             "analog-frmt=\" parameters. See "
                             "\"help xxx ...\" for those commands/"
                             "configurations/parameters.")

    parser.add_argument('--halt',
                        default='none',
                        nargs=1,
                        choices=('monitor',
                                 'gpio',
                                 'usart',
                                 'spi',
                                 'i2c',
                                 'numbers'),
                        help="Experimental. Reset firmware if buck50.py exit "
                             "while command in progress. Must be done before "
                             "buck50.py without \"--halt\", and must specify "
                             "correct command name, else firmware/hardware "
                             "reset and/or power cycle, and USB re-enumeration "
                             "required."                                      )

    parser.add_argument("acm",
                        nargs='?',
                        default=DEV_ACM,
                        help="CDC/ACM device (with or without \"/dev\" or "
                             "\"/dev/tty\" prefix)")

    return parser.parse_args()



if __name__ == '__main__':
    args = parse_commandline()

    if args.version:
        sys.stdout.write("%s\n" % COPYRIGHT)
        sys.exit(0)

    main(args.quiet, args.acm, args.no_auto_viewer, args.file, args.halt)
