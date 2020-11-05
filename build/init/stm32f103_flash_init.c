/*
buck50: Test and measurement firmware for “Blue Pill” STM32F103 development board
Copyright (C) 2019,2020 Mark R. Rubin aka "thanks4opensource"

This file is part of buck50.

The buck50 program is free software: you can redistribute it
and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

The buck50 program is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
(LICENSE.txt) along with the buck50 program.  If not, see
<https://www.gnu.org/licenses/gpl.html>
*/


#include <stdint.h>

#include <stm32f103xb.h>

void    init(void);   // for 2nd entry in vector table
// The following are 'declared' in the linker script
extern uint8_t  INITIALIZED_DATA_ADDR_IN_FLASH;
extern uint8_t  INITIALIZED_DATA_ADDR_IN_RAM_START;
extern uint8_t  INITIALIZED_DATA_ADDR_IN_RAM_END;
extern uint8_t  BSS_START;
extern uint8_t  BSS_END;
extern uint8_t  TOP_OF_STACK;
#define INTERRUPT_VECTORS_CONST const
#include "stm32f103_vectors_handlers.h"



int     main(void);


void __attribute__ ((isr)) init()
{
    uint8_t     *src,
                *dest;
    uint32_t     len;

// do global/static data initialization
    src  = &INITIALIZED_DATA_ADDR_IN_FLASH;
    dest = &INITIALIZED_DATA_ADDR_IN_RAM_START;
    len  =   &INITIALIZED_DATA_ADDR_IN_RAM_END
           - &INITIALIZED_DATA_ADDR_IN_RAM_START;
    while (len--)
        *dest++ = *src++;

    // zero out the uninitialized global/static variables
    dest = &BSS_START;
    len  = &BSS_END - &BSS_START;
    while (len--)
        *dest++=0;

    // set CPU's vector table address
    SCB->VTOR = (uint32_t)INTERRUPT_VECTORS;

    // start program
    main();
}
