// buck50: Test and measurement firmware for “Blue Pill” STM32F103 development board
// Copyright (C) 2019,2020 Mark R. Rubin aka "thanks4opensource"
//
// This file is part of buck50.
//
// The buck50 program is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// The buck50 program is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// (LICENSE.txt) along with the buck50 program.  If not, see
// <https://www.gnu.org/licenses/gpl.html>


#ifndef BIN_TO_HEX_HXX
#define BIN_TO_HEX_HXX

#include <stdint.h>

namespace bitops {

class BinToHex {
  public:
    const char* hex() const { return _hex; }

    const char* uint4(
    const uint8_t   bin)
    {
        _hex[0] = hex(bin);
        _hex[1] = '\0';
        return _hex;
    }

    static const char* uint4(
    const uint8_t    bin,
          char      *hex)
    {
        *hex = BinToHex::hex(bin);
        return hex;
    }

    const char* uint8(
    const uint8_t   bin)
    {
        byte(bin, 0, _hex);
        _hex[2] = '\0';
        return _hex;
    }

    static const char* uint8(
    const uint8_t    bin,
          char      *hex)
    {
        byte(bin, 0, hex);
        return hex;
    }

    const char* uint16(
    const uint16_t  bin)
    {
        byte(bin >>    8, 0, _hex);
        byte(bin  & 0xff, 2, _hex);
        _hex[4] = '\0';
        return _hex;
    }

    static const char* uint16(
    const uint16_t   bin,
          char      *hex)
    {
        byte(bin >>    8, 0, hex);
        byte(bin  & 0xff, 2, hex);
        return hex;
    }

    const char* uint32(
    const uint32_t  bin)
    {
        byte( bin >> 24        , 0, _hex);
        byte((bin >> 16) & 0xff, 2, _hex);
        byte((bin >>  8) & 0xff, 4, _hex);
        byte( bin        & 0xff, 6, _hex);
        _hex[8] = '\0';
        return _hex;
    }

    static const char* uint32(
    const uint32_t   bin,
          char      *hex)
    {
        byte( bin >> 24        , 0, hex);
        byte((bin >> 16) & 0xff, 2, hex);
        byte((bin >>  8) & 0xff, 4, hex);
        byte( bin        & 0xff, 6, hex);
        return hex;
    }

    static uint32_t hex_to_bin(
    const char* const   hex,
    const uint8_t       maxlen = 8)
    {
        uint32_t    bin = 0;

        for (uint8_t ndx = 0 ; hex[ndx] != '\0' && ndx < maxlen ; ++ndx) {
            uint8_t     hexit = hex[ndx] - '0';

            if (hexit > 9) hexit -= 'a' - ':';

            bin = (bin << 4) | hexit;
        }

        return bin;
    }


  protected:
    static void byte(
    const uint8_t    byte     ,
    const uint8_t    position ,
          char      *hex_chars)
    {
        hex_chars[position    ] = hex(byte >> 4 );
        hex_chars[position + 1] = hex(byte  & 0x0f);
    }

    static char hex(
    const uint8_t   nibble)
    {
        return '0' + nibble + (nibble > 9 ? 'a' - ':' : 0);
    }

    char    _hex[8 + 1];
};

}  // namespace bitops

#endif  // #ifndef BIN_TO_HEX_HXX
