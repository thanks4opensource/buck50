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

ifndef GCC_ARM_ROOT
$(error set GCC_ARM_ROOT environment variable)
endif

export PATH := $(GCC_ARM_ROOT)/bin:$(PATH)

CC      = arm-none-eabi-gcc
CXX     = arm-none-eabi-g++
AS      = arm-none-eabi-as
LD      = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy

all: buck50.hex buck50.bin

.PHONY: clean
clean:
	rm -f *.hex *.bin *.elf *.o *.map

CXX_SRCS = buck50.cxx			\
	   usb_dev.cxx			\
	   usb_dev_cdc_acm.cxx
ASM_SRCS = buck50_asm.s
INIT_SRC = stm32f103_flash_init.c
LD_SRC   = src/low_stack_flash.ld


OBJS = $(CXX_SRCS:.cxx=.o) $(ASM_SRCS:.s=.o) $(INIT_SRC:.c=.o) 

buck50.elf: $(OBJS) $(LD_SRC)
	$(LD) -o $@ $(LDOPTS)			\
		    $(OBJS)   			\
		    -T $(LD_SRC)		\
	     > $@.map
	@ awk '/^Memory region/ {on = 1} on {print $0}'  $@.map


vpath %.cxx	src util/stm32f10_12357xx
vpath %.c	init
vpath %.s	src
# vpath %.ld	ld

# vestigal development settings to semi-successfully build code into RAM
FLASH        ?=  2
# FLASH	     ?=  0 
ifeq ($(FLASH), 0)
INLINE_DECL  ?= -DINLINE_DECL=
INLINE_ATTR  ?= -DINLINE_ATTR='__attribute__((noinline))'
else
INLINE_DECL  ?= -DINLINE_DECL=inline
INLINE_ATTR  ?= -DINLINE_ATTR='__attribute__((always_inline))'
endif

EXTRA_CXX_FLAGS = -DUSB_DEV_FLASH_WAIT_STATES=$(FLASH)	\
		  $(INLINE_DECL)			\
		  $(INLINE_ATTR)			\
		  -fno-threadsafe-statics

WARNINGS_FLAGS ?= -Wall -Wextra
DEBUG_FLAG     ?= -g3
OPTIMIZE_FLAG  ?= -O2
STD_CXX_FLAG   ?= -std=c++17
INIT_DEFINES   ?= -DWEAK_FUNCTIONS

EXTRA_DEFINES  ?= 

CC_ARM_FLAGS   ?= -mthumb -mcpu=cortex-m3 -fno-exceptions -fno-unwind-tables
AS_ARM_FLAGS   ?= -mthumb -mcpu=cortex-m3 -march=armv7-m 

LINK_FLAGS      = -static -M --print-memory-usage

INCLUDES 	= -I.				\
 	          -I./init                  	\
		  -I./include/arm           	\
		  -I./include/st/32f        	\
		  -I./include/thanks4opensource	\
		  -I./util/stm32f10_12357xx
CCOPTS  	= $(DEPFLAGS)			\
		  $(WARNINGS_FLAGS)		\
		  $(DEBUG_FLAG)			\
		  $(CC_ARM_FLAGS)		\
		  $(INCLUDES)			\
		  $(INIT_DEFINES)		\
		  $(OPTIMIZE_FLAG)
CXXOPTS 	= $(DEPFLAGS)			\
		  $(STD_CXX_FLAG)		\
		  $(WARNINGS_FLAGS)		\
		  $(DEBUG_FLAG)			\
		  $(CC_ARM_FLAGS)		\
		  $(INCLUDES)			\
		  $(OPTIMIZE_FLAG)		\
		  $(EXTRA_CXX_FLAGS)
LDOPTS  	= $(LINK_FLAGS)


%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

%.o: %.cxx
	$(CXX) -c $(CXXOPTS) $< -o $@

%.o: %.c
	$(CC) -c $(CCOPTS) $< -o $@

%.o: %.s
	$(AS) $(DEBUG_FLAG) $(AS_ARM_FLAGS) $< -o $@

buck50.o: 		src/buck50.cxx
buck50_asm.o: 		src/buck50_asm.s
stm32f103_flash_init.o: init/stm32f103_flash_init.c
usb_dev.o: 		util/stm32f10_12357xx/usb_dev.cxx
usb_dev_cdc_acm.o: 	util/stm32f10_12357xx/usb_dev_cdc_acm.cxx
