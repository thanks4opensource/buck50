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


// now in Makefile.base
// .cpu cortex-m3
// .arch armv7-m
// .thumb
// but no way to do this on commandline
.syntax unified
// has no effect even without "n" in -a... option
// and .lst output worthless anyway
.psize 0,79

.altmacro   // stays in effect unless/until .noaltmacro (not done in this file)



#if 1  // no C preprocessor, only for section identification and code navigation
//
// constants
//

// IRQ handler return values
.equ    HALT_MEMORY  ,  1
.equ    HALT_DURATION,  2
.equ    HALT_USB     ,  3

// InProgress bits
.equ    IN_PROG_TRIGGERING , 0x0100
.equ    IN_PROG_EXTERN_TRIG, 0x0200
.equ    IN_PROG_ANALOG     , 0x0400
.equ    IN_PROG_TRIGGERED  , 0x0800
.equ    IN_PROG_SAMPLING   , 0x1000
.equ    IN_PROG_COUNTING   , 0x2000

// HardFault_Hander user LED blink
.equ        SYSTICK,        0xE000E010
.equ        VAL,            8               // systick->val
.equ        GPIOC,          0x40011000
.equ        IDR,            8               // gpiob->idr
.equ        BSRR,           16              // gpiob->bsrr
.equ        USER_LED_ON,    1 << (13 + 16)  // Gpio::Bsrr::BR13
.equ        USER_LED_OFF,   1 <<  13        // Gpio::Bsrr::BS13

#endif  // constants



#if 1  // no C preprocessor, only for section identification and code navigation
//
// Interrupt handlers (IRQs)
//

// irq handler registers
//

//  r0
addr        .req    r0
return_code .req    r0  //  uint18_t C++ global &halt_code
systick     .req    r0
//  addr            = irq_handler_enter
//  addr            _ HardFault_Handler
//  addr            _ TIM3_IRQHandler
//  addr            _ USB_LP_CAN1_RX0_IRQHandler:
//  return_code     : irq_handler_exit
//  systick         : HardFault_Handler

//  r1
in_prog     .req    r1  //  uint16_t C++ global &in_progress
gpioc       .req    r1
//  in_prog         = irq_handler_enter
//                    HardFault_Handler
//                    TIM3_IRQHandler
//  in_prog         _ USB_LP_CAN1_RX0_IRQHandler:
//                    irq_handler_exit
//  systick         : HardFault_Handler

//  r2
sample      .req    r2  // address of C++ global uint32_t* samples_end
//  sample          = (sampling)
//  sample          _ irq_handler_enter
//                    HardFault_Handler
//                    TIM3_IRQHandler
//                    USB_LP_CAN1_RX0_IRQHandler:
//                    irq_handler_exit

//  r3
valu        .req    r3
addr2       .req    r3
# crnt      .req r3
//  crnt            = (sampling)
//                    irq_handler_enter
//  addr2           = HardFault_Handler
//                    TIM3_IRQHandler
//                    USB_LP_CAN1_RX0_IRQHandler:
//  addr2           = irq_handler_exit

//  r5
# ram_dest  .req    r5  // .req below
state       .req    r5
# idrtim    .req    r5  // .req belo
//                    trigger_and_sample_{plain,ganged}
//  ram_dest        = timers_mode_codeloc
//                  _ flash_to_ram
//                    set_samples
//                    ganged_ready
//  state           = trigger_head
//                    ganged_check
//                  _ trigger_tail
//                    ganged_sync
//                    sampling_setup
//                    sampling_speed
//                    trigger_and_sample_{plain,ganged}
//  idrtim          : (sampling)
//  state           _ irq_handler_enter
//                    HardFault_Handler
//                    TIM3_IRQHandler
//                    USB_LP_CAN1_RX0_IRQHandler
//                    irq_handler_exit




// if sampling was in progress at IRQ
//   store smples_end register in samples_end variable
//
//  {
//      if (in_progress & InProgress::TRIGGERING)
//          in_progress |= trigger_state
//      if (in_progress & InProgress::SAMPLING)
//          samples_end = samples_end_register;
//  }
.macro irq_handler_enter
// if digital sampling interrupted before triggered, put current trigger
//   state (r5) into correct place in lower 8 bits of C++ in_progress global
// two caveats:
//   1) analog sampling also has IN_PROG_TRIGGERING set, but was in C++
//      when interrupted so r5 is meaningless
//   2) race condition with funcname&_triggered, below. is first setting
//      in_progress bits into state (r5), then clearing bit, then storing
//      in C++ global. interrupt can happen before/after/during (is strh
//      atomic?)
// solution: mask off all but lower 8 bits (state number) of state/r5 before
//   OR-ing into in_progress global. worst case scenario is in_progess has
//   garbage state number (but important upper 8 bits with in_progress codes
//   not harmed) if analog sampling, but buck50.py host code ignores if
//   analog sampling.
movw    addr, :lower16:in_progress  // addr = &in_progress
movt    addr, :upper16:in_progress  //  "    =      "
ldrh    in_prog, [addr]             // in_prog = *addr
tst     in_prog, IN_PROG_TRIGGERING //     in_prog & InProgress::TRIGGERING {
ittt    ne                          // if (in_prog & InProgress::TRIGGERING) {
andne   state, 0xff                 //     state &= 0xff   // just state number
orrne   in_prog, state              //     in_prog |= state
strhne  in_prog, [addr]             //     in_progress = in_prog }
// store samples_end into C++ global
movw    addr, :lower16:samples_end  // addr = &samples_end
movt    addr, :upper16:samples_end  // "     =       "
tst     in_prog, IN_PROG_SAMPLING   //     in_prog & InProgress::SAMPLING
it      ne                          // if (in_prog & InProgress::SAMPLING) {
strne   sample, [addr]              //     *addr = sample; }
.endm


.macro irq_handler_exit CODE
// special longjmp from ARM IRQ
movw    addr2,       #:lower16:longjump_buf // addr  = &longjump_buf
movt    addr2,       #:upper16:longjump_buf //  "    =      "
ldr     addr2,       [addr2, #36]           // addr2 = longjump_buf[address]
str     addr2,       [sp, #28]              // ISR stack position (plus 1 extra)
movs    return_code, CODE                   // return_code = CODE
str     return_code, [sp, #4 ]              // stack's ISR return val ($sp + 1)
pop     {pc}                                // pc = lr funky value, IRQ return
.endm




// HardFault_Handler
//
// {
//      irq_handler_enter()  ;  // inline, store samples_end
//      halt_timers()        ;  // C++ extern "C" function
//      longjmp(&longjump_buf); // return HALT_MEMORY
//  }
//
.balign 4
.thumb_func
.global HardFault_Handler
HardFault_Handler:
push    {lr}                            // for IRQ handler longjmp
irq_handler_enter                       // irq_handler_enter();
tst     in_prog, IN_PROG_TRIGGERING | IN_PROG_SAMPLING
bne     .L_hfh_in_prog
movw    systick, :lower16:SYSTICK       // &systick
movt    systick, :upper16:SYSTICK       //     "
movw    gpioc,   :lower16:GPIOC         // &gpioc
movt    gpioc,   :upper16:GPIOC         //     "
.L_hang:
ldr     valu, [systick, VAL]        // valu = systick->val
tst     valu, 0x800000
ite     eq
moveq   valu, USER_LED_ON
movne   valu, USER_LED_OFF
str     valu, [GPIOC, BSRR]
b       .L_hang
.L_hfh_in_prog:
bl      halt_timers                     // C++ extern "C" function
irq_handler_exit HALT_MEMORY




// TIM3_IRQHandler
//
//  {
//      irq_handler_enter()  ;  // inline, store samples_end
//      halt_timers()        ;  // C++ extern "C" function
//      longjmp(&longjump_buf); // return HALT_DURATION
//  }
//
.balign 4
.thumb_func
.global TIM3_IRQHandler
TIM3_IRQHandler:
push    {lr}                            // for IRQ handler longjmp
irq_handler_enter                       // irq_handler_enter();
bl      halt_timers                     // C++ extern "C" function
irq_handler_exit HALT_DURATION



// USB_LP_CAN1_RX0_IRQHandler:
//  {
//      in_prog = irq_handler_enter();
//      usb_dev->interrupt_handler();
//      if (in_progress & (  InProgress::TRIGGERING
//                         | InProgress::SAMPLING
//                         | InProgress::COUNTING) {
//          halt_timers();
//          longjmp(&longjump_buf); // return HALT_USB
//      }
//      else
//          return;
//  }
//
.balign 4
.thumb_func
.global USB_LP_CAN1_RX0_IRQHandler
USB_LP_CAN1_RX0_IRQHandler:
push    {lr}                                // for IRQ handler longjmp
// if sampling save sample from interrupted sampling loop
irq_handler_enter
// stm32f10_12357_xx::UsbDev::interrupt_handler();
push    {in_prog}                       // *sp-- = in_prog
movw    addr, #:lower16:usb_dev         // &usb_dev
movt    addr, #:upper16:usb_dev         //     "
bl      _ZN17stm32f10_12357_xx6UsbDev17interrupt_handlerEv
pop     {in_prog}                       // in_prog = *sp++
// return if not doing longjmp
tst     in_prog, IN_PROG_TRIGGERING | IN_PROG_SAMPLING | IN_PROG_COUNTING
it      eq                  // if (!(in_progress & TRIGGERING_etc)
popeq   {pc}                //     return; }
// doing longjump ...
bl      halt_timers                     // C++ extern "C" function
irq_handler_exit HALT_USB



// remove register aliases
.unreq  addr        //  r0
.unreq  return_code //  r0
.unreq  addr2       //  r1
.unreq  in_prog     //  r1      uint16_t* C++ global &in_progress
.unreq  sample      //  r2      uint32_t* C++ global &samples_end
.unreq  systick     //  r0
.unreq  gpioc       //  r1
.unreq  valu        //  r3

#endif  // Interrupt handlers (IRQs)






#if 1  // no C preprocessor, only for section identification and code navigation
//
// .reqs and .equs
//

// registers
//
// legend:
//      ~   implicit (in register)
//      =   alias created/set
//      >   alias used
//      _   last use of alias
//      :   alias created, set, last use

//  r0
flash_or_ram    .req    r0
gpiob           .req    r0
# addr          .req    r0
# return_code   .req    r0
// <flash_or_ram>   ~ .cxx
//  flash_or_ram      trigger_and_sample_{plain,ganged}
//                  _ timers_mode_codeloc
//  gpiob           = trigger_and_sample_{plain,ganged}
//                    flash_to_ram
//                    set_samples
//                    trigger_and_sample_{plain,ganged}
//                  > ganged_ready
//                  > trigger_head
//                  > ganged_check
//                  > trigger_tail
//                  > ganged_sync
//                  > sampling_setup
//                    sampling_speed
//                  _ (sampling)
//  addr            = irq_handler_enter
//  addr            _ HardFault_Handler
//  addr            _ TIM3_IRQHandler
//  addr            _ USB_LP_CAN1_RX0_IRQHandler
//  return_code     : irq_handler_exit

//  r1
num_samples .req    r1
triggers    .req    r1
systick     .req    r1
# in_prog   .req    r1
// <num_samples>    ~ .cxx
//                    trigger_and_sample_{plain,ganged}
//                    timers_mode_codeloc
//                    flash_to_ram
//                  _ set_samples
//                    ganged_ready
//  triggers        = trigger_head
//                    ganged_check
//                  _ trigger_tail
//  systick         = trigger_tail
//                    ganged_sync
//                  > sampling_setup
//                    sampling_speed
//                    trigger_and_sample_{plain,ganged}
//                  _ (sampling)
//  in_prog         = irq_handler_enter
//                    HardFault_Handler
//                    TIM3_IRQHandler
//  in_prog         _ USB_LP_CAN1_RX0_IRQHandler
//                    irq_handler_exit

//  r2
sample      .req    r2
//                    trigger_and_sample_{plain,ganged}
//                    timers_mode_codeloc
//                    flash_to_ram
//  sample          = set_samples
//                    ganged_ready
//                    trigger_head
//                    ganged_check
//                  > trigger_tail
//                    ganged_sync
//                  > sampling_setup
//                    sampling_speed
//                    trigger_and_sample_{plain,ganged}
//                  > (sampling)
//                  _ irq_handler_enter
//                    HardFault_Handler
//                    TIM3_IRQHandler
//                    USB_LP_CAN1_RX0_IRQHandler
//                    irq_handler_exit

//  r3
flash_beg   .req r3
trigger     .req r3
crnt        .req r3
# addr2     .req r3
//  flash_beg       = trigger_and_sample_{plain,ganged}
//                  > timers_mode_codeloc
//                  _ flash_to_ram
//                    set_samples
//                    ganged_ready
//  trigger         = trigger_head
//                    ganged_check
//                  _ trigger_tail
//                    ganged_sync
//                    sampling_setup
//                    sampling_speed
//                    trigger_and_sample_{plain,ganged}
//  crnt            : (sampling)
//                    irq_handler_enter
//                    HardFault_Handler
//                    TIM3_IRQHandler
//                    USB_LP_CAN1_RX0_IRQHandler
//  addr2           = irq_handler_exit

//  r4
flash_end   .req    r4
gpiob_idr   .req    r4
prev        .req    r4
//  flash_end       = trigger_and_sample_{plain,ganged}
//                  > timers_mode_codeloc
//                  _ flash_to_ram
//                    set_samples
//                    ganged_ready
//  gpiob_idr       = trigger_head
//                  > ganged_check
//                  _ trigger_tail
//                    ganged_sync
//  prev            = sampling_setup
//                    sampling_speed
//                    trigger_and_sample_{plain,ganged}
//                  _ (sampling)
//                    irq_handler_enter
//                    HardFault_Handler
//                    TIM3_IRQHandler
//                    USB_LP_CAN1_RX0_IRQHandler
//                    irq_handler_exit

//  r5
ram_dest    .req    r5
# state     .req    r5  // .req above
idrtim      .req    r5
//                    trigger_and_sample_{plain,ganged}
//  ram_dest        = timers_mode_codeloc
//                  _ flash_to_ram
//                    set_samples
//                    ganged_ready
//  state           = trigger_head
//                    ganged_check
//                  _ trigger_tail
//                    ganged_sync
//                    sampling_setup
//                    sampling_speed
//                    trigger_and_sample_{plain,ganged}
//  idrtim          : (sampling)
//  state           _ irq_handler_enter
//                    HardFault_Handler
//                    TIM3_IRQHandler
//                    USB_LP_CAN1_RX0_IRQHandler
//                    irq_handler_exit

//  r6
addr        .req    r6  // scratch register, brief local lifetimes
trigbits    .req    r6  // mask and bits of trigger, no conflict with addr
//                    trigger_and_sample_{plain,ganged}
//  addr            : timers_mode_codeloc
//                    flash_to_ram
//  addr            : set_samples
//                    ganged_ready
//  trigbits        = trigger_head
//                    ganged_check
//  trigbits        _ trigger_tail
//  addr            : trigger_tail
//                    ganged_sync
//  addr            : sampling_setup
//  addr            : sampling_speed
//                    trigger_and_sample_{plain,ganged}
//                    (sampling)
//                    irq_handler_enter
//                    HardFault_Handler
//                    TIM3_IRQHandler
//                    USB_LP_CAN1_RX0_IRQHandler
//                    irq_handler_exit

//  r7
valu        .req    r7      // scratch register, brief local lifetimes
//                    trigger_and_sample_{plain,ganged}
//  valu            : timers_mode_codeloc
//  valu            : flash_to_ram
//  valu            : set_samples
//  valu            : ganged_ready
//                    trigger_head
//                    ganged_check
//  valu            : trigger_tail
//  valu            : ganged_sync
//  valu            : sampling_setup
//  valu            : sampling_speed
//                    trigger_and_sample_{plain,ganged}
//                    (sampling)
//                    irq_handler_enter
//                    HardFault_Handler
//                    TIM3_IRQHandler
//                    USB_LP_CAN1_RX0_IRQHandler
//                    irq_handler_exit

//  r8
fail    .req    r8
lr_save .req    r8
//                    trigger_and_sample_{plain,ganged}
//                  : timers_mode_codeloc
//                    flash_to_ram
//                    set_samples
//                    ganged_ready
//  fail            = trigger_head
//                    ganged_check
//                  _ trigger_tail
//                    ganged_sync
//                    sampling_setup
//                    sampling_speed
//                    (sampling)
//                    trigger_and_sample_{plain,ganged}
//                    irq_handler_enter
//                    HardFault_Handler
//                    TIM3_IRQHandler
//                    USB_LP_CAN1_RX0_IRQHandler
//                    irq_handler_exit

// r9
ram_beg     .req    r9
//                    trigger_and_sample_{plain,ganged}
//                  = timers_mode_codeloc
//                    flash_to_ram
//                    set_samples
//                  _ trigger_and_sample_{plain,ganged}
//                    ganged_ready
//                    trigger_check
//                    ganged_ready
//                    trigger_tail
//                    ganged_sync
//                    sampling_setup
//                    sampling_speed
//                    (sampling)
//                    irq_handler_enter
//                    HardFault_Handler
//                    TIM3_IRQHandler
//                    USB_LP_CAN1_RX0_IRQHandler
//                    irq_handler_exit

//  r10
in_prog_addr    .req    r10
//                    trigger_and_sample_{plain,ganged}
//  in_prog_addr    = timers_mode_codeloc
//                    flash_to_ram
//                    set_samples
//                    ganged_ready
//                    trigger_head
//                    ganged_check
//                  _ trigger_tail
//                    ganged_sync
//                    sampling_setup
//                    sampling_speed
//                    trigger_and_sample_{plain,ganged}
//                    (sampling)
//                    irq_handler_enter
//                    HardFault_Handler
//                    TIM3_IRQHandler
//                    USB_LP_CAN1_RX0_IRQHandler
//                    irq_handler_exit

// r11
tim_1   .req    r11
//                    trigger_and_sample_{plain,ganged}
//                  = timers_mode_codeloc
//                    flash_to_ram
//                    set_samples
//                    ganged_ready
//                    trigger_head
//                    ganged_check
//                    trigger_tail
//                    ganged_sync
//                  _ sampling_setup
//                    sampling_speed
//                    trigger_and_sample_{plain,ganged}
//                    (sampling)
//                    irq_handler_enter
//                    HardFault_Handler
//                    TIM3_IRQHandler
//                    USB_LP_CAN1_RX0_IRQHandler
//                    irq_handler_exit

//  r12
sample_func .req    r12
//                    trigger_and_sample_{plain,ganged}
//  sample_func     = timers_mode_codeloc
//                    flash_to_ram
//                    set_samples
//                    ganged_ready
//                    trigger_head
//                    ganged_check
//                    trigger_tail
//                    ganged_sync
//  sample_func     _ sampling_setup
//                    sampling_speed
//                    trigger_and_sample_{plain,ganged}
//                    (sampling)
//                    irq_handler_enter
//                    HardFault_Handler
//                    TIM3_IRQHandler
//                    USB_LP_CAN1_RX0_IRQHandler
//                    irq_handler_exit



// .equs
//

// addresses
.equ        END_OF_RAM,     0x20005000
.equ        GPIOB,          0x40010C00
.equ        SYSTICK,        0xE000E010

// struct offsets
.equ        VAL,        8       // systick->val
.equ        IDR,        8       // gpiob->idr
.equ        BSRR,       16      // gpiob->bsrr

// gpio bsrr bits
.equ        GANGED_TRIG_SET, 1<<14      // set or test ODR bit 14
.equ        GANGED_SYNC_SET, 1<<15      //  "  "   "    "   "  15
.equ        GANGED_TRIG_CLR, 1<<(14+16) // clear        "   "  14

// flash or ram
.equ    CODE_MEM_RAM  ,     0
.equ    CODE_MEM_FLASH,     1

// sampling speed codes
.equ    SPEED_MHZ_6     ,   0
.equ    SPEED_IRREGULAR ,   1
.equ    SPEED_UNIFORM   ,   2
.equ    SPEED_MHZ_4     ,   3
.equ    SPEED_CODE_LIMIT,   3

#endif  // .reqs and .equs




#if 1  // no C preprocessor, only for section identification and code navigation
//
// setup utils
//


.balign 4
.thumb_func
flash_to_ram:                   // while (flash_beg < flash_end) {
b       flash_to_ram_compare
flash_to_ram_while:
ldr     valu, [flash_beg], #4   // valu = *flash_beg++
str     valu, [ram_dest ], #4   // *ram_dest++ = valu
flash_to_ram_compare:
cmp     flash_beg, flash_end    //
blt     flash_to_ram_while      // }
mov     pc, lr                  // return, leaving ram_dest set to next addr



.thumb_func
set_samples:
movw    addr, :lower16:END_OF_RAM       // addr = &END_OF_RAM
movt    addr, :upper16:END_OF_RAM       //    "    =     "
sub     valu, addr, ram_dest            // available = end_of_ram - ram_dest
lsls    num_samples, #2                 // num_samples *= 4 (words -> bytes)
cmp     num_samples, valu               // if num_samples > available {
it      gt                              //     num_samples =
movgt   num_samples, valu               //                    available
subs    sample, addr, num_samples       // sample = end_of_ram - num_samples
movw    addr,:lower16:samples           // addr = &samples
movt    addr,:upper16:samples           //   "   =     "
str     sample, [addr]                  // samples = sample
# save here so if not triggered will have corrct "0 samples"
movw    addr,:lower16:samples_end       // addr = &samples_end
movt    addr,:upper16:samples_end       //   "   =       "
str     sample, [addr]                  // samples_end = sample
mov     pc, lr                          // return

#endif  // setup utils




#if 1  // no C preprocessor, only for section identification and code navigation
//
// triggering and sampling
//

#if 1  // no C preprocessor, only for section identification and code navigation
//
// macros
//

.macro ganged_ready
// set ganged trigger sync high to tell connected others self is ready
mov     valu, GANGED_TRIG_SET   // bsrr set odr bit 14 high
str     valu, [gpiob, BSRR]     // gpiob->bsrr = set 14 high
// wait for open-drain trigger line to go high indicating all are ready
LOCAL   ganged_ready_loop
ganged_ready_loop:              // while (!(  gpiob->idr & (1<<15)) {
ldr     valu, [gpiob, IDR]      // r1 = gpiob->idr
tst     valu, GANGED_TRIG_SET   // flags = gpiob & G_T_S
beq     ganged_ready_loop       // } // while
.endm


.macro trigger_head     funcname
// triggering
.equ    MAX_NUM_TRIGGERS,   256
.equ    TRIGGERS        ,   END_OF_RAM - 4 * MAX_NUM_TRIGGERS   // uint32_t
movw    triggers, :lower16:TRIGGERS // triggers = TRIGGERS
movt    triggers, :upper16:TRIGGERS //      "   =    "
movs    state, 0                    // state   = 0

funcname&_trigger_loop:             // while (true) {
ldr     trigger, [triggers, state, lsl #2]  // trigger = triggers[state]
                                    // trigger is:  bbffppmm  msb-to-lsb
lsrs    trigbits, trigger, #24      // trigbits = trigger >> 24  i.e. 000000bb
uxtb    fail, trigger, ror #16      // fail = (trigger >> 16) & 0xff (fail bits)

funcname&_loop_gpiob:
ldr     gpiob_idr, [gpiob, IDR]     // gpiob_idr = gpiob->idr
                                    // if no speed penalty for going to 32-bit
                                    //   by making gpio_idr high register, do so
                                    //   because only used with 32-bit ORR
                                    //   with shift
.endm


.macro ganged_check     funcname
// check whether any other ganged unit has triggered
tst     gpiob_idr, GANGED_TRIG_SET      // apsr.z = ((sample & (1<<14)) == 0)
// always have to clear (at least) PB14/GANGED_TRIG
// only need to do if ganged, otherwise 15:12 are all unset
// bit would interfere with "(shifted_sample & trigger) == trgcpy" below
//   because could &-in a bit in 15:12  from trigger mask causing "=="
//   fail because bits were 7:0 extracted (31:8 all zero)
// clear all of 15:12 anyway (no extra cost) even though
//   PB15/GANGED_SYNC should be low until sync, PB12/unused should be
//   permanently low (unused), and PB13/adv_tim_1/systick_overflow
//   should be low because not started yet
// can't immediately clear bit 14 after setting (before loops)
//   because would then be high pulse and others could miss
// trusting that 31:16, gpio->idr "reserved" are read-as-zero
bfc     gpiob_idr, 12, 4                // do regardless
// branch prediction -- could instead be:
//   bne    funcname&_trigger_tail  // if (!apsr.z) (bit 14 clear) goto ttg
//   orr    state, IN_PROG_EXTERN_TRIG  // else { state |=;  (not orrs, always
//   b      funcname&_triggered         //   goto triggered; }          32-bit)
//   beq    funcname&_triggered
// but tests show null itt faster than branch
itt     eq                              // if (!apsr.z) (bit 14 clear) {
orreq   state, IN_PROG_EXTERN_TRIG      //    set reporting bit
beq     funcname&_triggered             //    goto triggered; |
# else fall thru to funcname&_trigger_tail
.endm


.macro trigger_tail     funcname
funcname&_trigger_tail:
// trigger   is:  bbffppmm  msb-to-lsb
// gpiob_idr is:  00000ii0  msb-to-lsb
and     valu, trigger, gpiob_idr, lsr #4    // valu = trigger & (gpiob_idr>>4)
cmp     valu, trigbits                      // if (valu == trigbits) {
beq     funcname&_trigger_tail_pass         //    goto trigger_tail_pass }
                                            // else /* fail (more likely) */ {
// most common case: fail==state
// could drop into fail loop, but much faster to short-circuit
// because no need to fetch same trigger
cmp     fail, state             // if (valu == state)
beq     funcname&_loop_gpiob    //    continue; w/o re-read trigger}
                                // else {
eors    state, fail             //   swap fail ...
eors    fail,  state            //   .. and state
eors    state, fail             //   (fastest way, even if had spare low reg)

LOCAL   fail_loop
fail_loop:                          // while (true) {
ldr     trigger, [triggers, state, lsl #2]  // trigger = triggers[state]
lsrs    trigbits, trigger, #24      // trigbits = trigger >> 24  i.e. 000000bb
        // would have been better to pre-shift gpio_idr and use 16-bit ands
        // but this is unusual case (OR-states)
        // works out same: pre-shift+and is 22 cycles, 16-bit and is 12
        // and with shift is 17, so 2*17==34=22+12
        // will definitely lose if more than two "OR" cases, but again unusual
and     valu, trigger, gpiob_idr, lsr #4    // valu = trigger & (gpiob_idr>>4)
cmp     valu, trigbits              // if (valu == bb) {
beq     funcname&_trigger_tail_pass //    goto trigger_tail_pass }
uxtb    state, trigger, ror #16     // state = (trigger >> 16) & 0xff i.e. "ff"
cmp     state, fail                 // if (state == fail) {
// could be:
//   bne fail loop
//   b   funcname&_trigger_loop
// but assuming two "OR" cases more likely than more than two,
// so below faster (branch prediction)
beq     funcname&_trigger_loop      //    break; // and continue outer loop
b       fail_loop                   // else continue; (fail_loop)

funcname&_trigger_tail_pass:
uxtb    valu, trigger, ror #8       // value = (trigger >> 8) & 0xff ("pp")
cbz     valu, funcname&_triggered   // if (valu == 0) goto triggered
mov     state, valu                 // else { state = valu;
b       funcname&_trigger_loop      //    continue; }

funcname&_triggered:
// state might also have IN_PROG_EXTERN_TRIG
// see irq_handler_enter for how race state of interrupt anywhere in
//   following three instructions is mitigated
orr     state, IN_PROG_TRIGGERED | IN_PROG_SAMPLING
bic     state, IN_PROG_TRIGGERING
// note is 32-bit instruction (no free low registers for early set of in_prog)
strh    state, [in_prog_addr]   // in_progress  = ...

// save "triggered at" sample
// systick->val slightly late, but saves reading systick each time in loop
// shift sample from bits 11:4 to 31:24
movw    systick, :lower16:SYSTICK       // &systick
movt    systick, :upper16:SYSTICK       //     "
ldr     valu, [systick, VAL]            // valu = systick->val
orr     valu, valu, gpiob_idr, lsl #20  // valu |= (gpiob << 20)
str     valu, [sample], 4               // *sample++ = valu
.endm


.macro ganged_sync
// unset trigger pin, regardless of whether another did or not
// and set sync/ack
mov     valu, GANGED_TRIG_CLR
orr     valu, GANGED_SYNC_SET
str     valu, [gpiob, BSRR]
// wait for open-drain trigger line to go high indicating all are ready
LOCAL   wait_ganged_sync
wait_ganged_sync:               // while (!(  gpiob->idr & (1<<15)) {
ldr     valu, [gpiob, IDR]      // r1 = gpiob->idr
tst     valu, GANGED_SYNC_SET   // flags = valu & (1<<15)
beq     wait_ganged_sync        // } // while
.endm


.macro sampling_setup
// current sample, for host analysis sync with ganged others
// gpiob bits 11..4 with systick
ldr     prev, [gpiob, IDR]          // rprev  = gpiob->idr
ldr     valu, [systick, VAL]        // valu   = systick->val
orr     valu, valu, prev, lsl #20   // valu  |= (gpiob << 20)
str     valu, [sample], 4           // sample++ = ...

// start systick overflow timer
// might have to do by setting PB13 to alt func instead, do CEN before
.equ    CR1 , 0             // adv_tim_1
.equ    CEN , 1
mov     valu, CEN           // = AdvTim1::CEN
str     valu, [tim_1, CR1]  // adv_tim_1.cr1 = CEN

// start gen_tim_3 if enabled
# no, already  started in .cxx, now is for both triggering and sampling
# .equ  OPM ,   8
# cmp       dura_enbld, 0               // either 0 or gen_tim_3 address
# itt       ne                          // if (dura_enbld != 0) {
# movne valu,       CEN | OPM           // valu = GenTim_2_3_4::CEN | OPM
# strne valu,       [dura_enbld, CR1]   // gen_tim
.balign 4
.endm


# see "sampling speed codes" .equs, above
.macro sampling_speed   size
movw    addr, :lower16:sampling_mode
movt    addr, :upper16:sampling_mode
ldr     valu, [addr]
# live dangerously: if host sends bad sampling mode, so be it
# cmp       valu, SPEED_CODE_LIMIT
# it        gt
# movgt valu, SPEED_CODE_LIMIT
LOCAL   here
.ifeq size - 16
tbh     [pc, valu]
here:
.short  (mhz_6     - here) >> 1
.short  (irregular - here) >> 1
.short  (uniform   - here) >> 1
.short  (mhz_4     - here) >> 1
.else
tbb     [pc, valu]
here:
.byte   (mhz_6     - here) >> 1
.byte   (irregular - here) >> 1
.byte   (uniform   - here) >> 1
.byte   (mhz_4     - here) >> 1
.endif


.endm

#endif  // macros



#if 1  // no C preprocessor, only for section identification and code navigation
//
// util funcs
//


.thumb_func
timers_mode_codeloc:
.equ    gen_tim_3,  0x40000400
.equ    adv_tim_1,  0x40012C00
movw    tim_1,      :lower16:adv_tim_1      // tim_1 = &adv_tim_1
movt    tim_1,      :upper16:adv_tim_1      //   for later in sampling_setup()
movw    ram_beg,    :lower16:STORAGE        // for jump to start
movt    ram_beg,    :upper16:STORAGE        //  "   "   "    "
mov     ram_dest,   ram_beg                 // for flash_to_ram &  set_samples()
movw    in_prog_addr, :lower16:in_progress  // = &in_progress, for triggered:
movt    in_prog_addr, :upper16:in_progress  //        "
mov     lr_save,    lr                      // save for func return
cmp     flash_or_ram,   CODE_MEM_RAM        // if (flash_or_ram == RAM) {
it      eq
bleq    flash_to_ram                        //   flash_to_ram() }
movw    addr,       :lower16:sampling_mode
movt    addr,       :upper16:sampling_mode
ldr     valu,       [addr]                  // valu = sampling_mode

cmp     valu,       SPEED_MHZ_6             // if (valu == SPEED_MHZ_6) {
itttt   eq
movweq  flash_beg,  :lower16:mhz_6          //    flash_beg = mhz_6
movteq  flash_beg,  :upper16:mhz_6          //        "     =   "
movweq  flash_end,  :lower16:irregular      //    flash_end = irregular
movteq  flash_end,  :upper16:irregular      //        "     =     "       }

cmp     valu,       SPEED_IRREGULAR         // if (valu == IRREGULAR) {
itttt   eq
movweq  flash_beg,  :lower16:irregular      //    flash_beg = irregular
movteq  flash_beg,  :upper16:irregular      //        "     =     "
movweq  flash_end,  :lower16:uniform        //    flash_end = uniform
movteq  flash_end,  :upper16:uniform        //        "     =    "        }

cmp     valu,       SPEED_UNIFORM           // if (valu == SPEED_UNIFORM) {
itttt   eq
movweq  flash_beg,  :lower16:uniform        //    flash_beg = uniform
movteq  flash_beg,  :upper16:uniform        //        "     =    "
movweq  flash_end,  :lower16:mhz_4          //    flash_end = mhz_4
movteq  flash_end,  :upper16:mhz_4          //        "     =   "         }

cmp     valu,       SPEED_MHZ_4             // if (valu == SPEED_MHZ_4) {
itttt   eq
movweq  flash_beg,  :lower16:mhz_4          //    flash_beg = mhz_4
movteq  flash_beg,  :upper16:mhz_4          //        "     =   "
movweq  flash_end,  :lower16:sampling_end   //    flash_end = <end>
movteq  flash_end,  :upper16:sampling_end   //        "     =   "      }

cmp     flash_or_ram,   CODE_MEM_RAM        // if (flash_or_ram == RAM) {
ite     ne
movne   sample_func,    flash_beg           // else { sample_func = flash_beg; }
bleq    flash_to_ram                        //    flash_to_ram()   }

mov     pc,         lr_save                 // return

#endif  // util funcs



#if 1  // no C preprocessor, only for section identification and code navigation
//
// triggering
//

.balign 4
.thumb_func
.global trigger_and_sample_ganged
trigger_and_sample_ganged:
# entry
#   flash_or_ram(r0)    // arg: copy/run in RAM (0) or leave/run in FLASH (1)
#   num_samples (r1)    // arg: will be clamped to available memory size
#   dura_timer  (r2)    // arg: bool, if duration timer needs to be started
#
movw                flash_beg,  :lower16:ganged_trig_beg
movt                flash_beg,  :upper16:ganged_trig_beg
movw                flash_end,  :lower16:ganged_trig_end
movt                flash_end,  :upper16:ganged_trig_end
bl                  timers_mode_codeloc                 // sets ram_dest
bl                  set_samples
cmp                 flash_or_ram,   CODE_MEM_RAM        // if (!= RAM) set flags
movw                gpiob,          :lower16:GPIOB      // &gpiob
movt                gpiob,          :upper16:GPIOB      //     "
it                  eq
moveq               pc,             ram_beg             //    goto code_in_ram;
.balign 4
ganged_trig_beg:
ganged_ready
trigger_head    ganged
ganged_check    ganged
trigger_tail    ganged
ganged_sync
sampling_setup
.balign 4
ganged_trig_end:
# sampling_speed    8
# b sampling
mov     pc, sample_func


.balign 4
.thumb_func
.global trigger_and_sample_plain
trigger_and_sample_plain:
# entry
#   flash_or_ram(r0)    // arg: copy/run in RAM (0) or leave/run in FLASH (1)
#   num_samples (r1)    // arg: will be clamped to available memory size
#   dura_timer  (r2)    // arg: bool, if duration timer needs to be started
movw                flash_beg,  :lower16:plain_trig_beg
movt                flash_beg,  :upper16:plain_trig_beg
movw                flash_end,  :lower16:plain_trig_end
movt                flash_end,  :upper16:plain_trig_end
bl                  timers_mode_codeloc                 // sets ram_dest
bl                  set_samples
cmp                 flash_or_ram,   CODE_MEM_RAM        // if (!= RAM) set flags
movw                gpiob,          :lower16:GPIOB      // &gpiob
movt                gpiob,          :upper16:GPIOB      //     "
it                  eq
moveq               pc,             ram_beg             //    goto code_in_ram;
.balign 4
plain_trig_beg:
trigger_head    plain
trigger_tail    plain
sampling_setup
.balign 4
plain_trig_end:
# sampling_speed    8
mov     pc, sample_func




// remove (triggering-specific) register aliases
// others remain set for use in sampling
// .unreq   dura_enbld      // r0   // passed-in argument
.unreq  addr            // r1
// .unreq   sample          // r2
.unreq  trigger         // r3   // current trigger
.unreq  gpiob_idr       // r4   // sample read from gpiob->idr
//      gpiob           // r5
//      systick         // r6
// .unreq   systick_val     // r7   // systick->val
.unreq  triggers        // r9   // array of triggers
.unreq  state           // r10  // current trigger number
.unreq  trigbits        // r11  //    "       "      "
.unreq  fail


#endif  // triggering



#if 1  // no C preprocessor, only for section identification and code navigation
//
// sampling
//


// macros

.macro sample_crnt
ldr         crnt, [gpiob, IDR]              // crnt = gpiob->idr
ldr         idrtim, [systick, VAL]          // idrtim = systick->val
orr         idrtim, idrtim, crnt, lsl #20   // idrtim |= (crnt << 20);
cmp         crnt, prev                      // if (crnt != prev)
it          ne                              // {
strne       idrtim, [sample], #4            // *sample++ = idrtim; }
.endm

.macro sample_prev
ldr         prev, [gpiob, IDR]              // prev = gpiob->idr
ldr         idrtim, [systick, VAL]          // idrtim = systick->val
orr         idrtim, idrtim, prev, lsl #20   // idrtim |= (crnt << 20);
cmp         prev, crnt                      // if (crnt != prev)
it          ne                              // {
strne       idrtim, [sample], #4            // *sample++ = idrtim; }
.endm



.balign     4
sampling:

// all loop until memory/timer/usb IRQ

mhz_6:          // flash=11,11,11,11,11,14  ram=13,13,13,13,13,17
sample_crnt
sample_prev
sample_crnt
sample_prev
sample_crnt
sample_prev
b   mhz_6

.balign     4
irregular:      // flash=11,14  ram=13,17
sample_crnt
sample_prev
b   irregular

.balign     4
uniform:        // flash=15 ram=17
sample_crnt
mov     prev, crnt
b       uniform

mhz_4:          // flash=18, ram=21
sample_crnt
mov     prev, crnt
nop     // flash=16, ram=19 with just 1 nop
nop     // flash=??, ram=19 with      2 nops
nop     // flash=18, ram=21 with      3 nops
b   mhz_4

.balign 4
sampling_end:


.unreq  valu
.unreq  crnt
.unreq  prev
.unreq  gpiob
.unreq  idrtim
.unreq  in_prog_addr
.unreq  systick

#endif  // sampling

#endif  // triggering and sampling
