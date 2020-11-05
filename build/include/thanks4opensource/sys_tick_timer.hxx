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


#ifndef SYS_TICK_TIMER_HXX
#define SYS_TICK_TIMER_HXX

#define ARM_SYS_TICK_TIMER_MAJOR_VERSION    1
#define ARM_SYS_TICK_TIMER_MINOR_VERSION    1
#define ARM_SYS_TICK_TIMER_MICRO_VERSION    1

#include <stdint.h>

#ifndef CORE_CMX_HXX_INCLUDED
#error #include core_cmNxxx.h before __FILE__
#endif

namespace arm {

class SysTickTimer {
  public:
    constexpr
    SysTickTimer()
    :   _start_tick(0),
        _elapsed32 (0),
        _elapsed64 (0)
    {}

    SysTickTimer(
    unsigned    begin)
    {
        switch (begin) {
            case 32: begin32(); break;
            case 64: begin64(); break;
            default:            break;
        }
    }

    static void on () { arm::sys_tick->ctrl |= arm::SysTick::Ctrl::ENABLE; }
    static void off() { arm::sys_tick->ctrl -= arm::SysTick::Ctrl::ENABLE; }

    void begin32() { _start_tick = arm::sys_tick->val; _elapsed32 = 0; }
    void begin64() { _start_tick = arm::sys_tick->val; _elapsed64 = 0; }

    void resume()        { _start_tick = arm::sys_tick->val; }


    // must be called at least once every (1<<24) ticks
    //
    uint32_t elapsed32() { return _elapsed32 += elapsed(); }
    uint64_t elapsed64() { return _elapsed64 += elapsed(); }
    void      update32() {        _elapsed32 += elapsed(); }
    void      update64() {        _elapsed64 += elapsed(); }

    void delay32(
    const uint32_t  ticks)
    {
        begin32();
        while (elapsed32() < ticks);
    }

    void delay64(
    const uint64_t  ticks)
    {
        begin64();
        while (elapsed64() < ticks);
    }

    uint32_t    current() const volatile { return arm::sys_tick->val; }


    static void init(
    const arm::SysTick::Ctrl::bits_t    clock_source =   arm
                                                       ::SysTick
                                                       ::Ctrl
                                                       ::CLKSOURCE,  // or 0
    const arm::SysTick::Ctrl::bits_t    start_on     =   arm
                                                       ::SysTick
                                                        ::Ctrl
                                                        ::ENABLE   )  // or 0
    {
        arm::sys_tick->ctrl = 0;     // halt
        arm::sys_tick->val  = 0;     // ensure will start from LOAD value
        arm::sys_tick->load = arm::SysTick::Load::RELOAD_MAX;
        arm::sys_tick->ctrl = clock_source | start_on;
    }


  protected:
    uint32_t elapsed()
    {
        uint32_t    current = arm::sys_tick->val,
                    elapsed =   (_start_tick - current)
                              & arm::SysTick::Load::RELOAD_MAX;

        _start_tick = current;

        return elapsed;
    }

    volatile uint32_t   _start_tick,
                        _elapsed32 ;
    volatile uint64_t   _elapsed64 ;

};  // class SysTickTimer

}  // namespace arm

#endif  // #ifndef SYS_TICK_TIMER_HXX
