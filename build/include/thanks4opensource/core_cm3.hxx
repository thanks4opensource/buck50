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


#ifndef CORE_CM3_HXX
#define CORE_CM3_HXX

#define CORE_CMX_HXX_INCLUDED

#include <stdint.h>

#include <regbits.hxx>


#if REGBITS_MAJOR_VERSION == 1
#if REGBITS_MINOR_VERSION  < 0
#warning REGBITS_MINOR_VERSION < 0 with required REGBITS_MAJOR_VERSION == 1
#endif
#else
#error REGBITS_MAJOR_VERSION != 1
#endif

#define ARM_CORE_CM3_MAJOR_VERSION  1
#define ARM_CORE_CM3_MINOR_VERSION  0
#define ARM_CORE_CM3_MICRO_VERSION  1


using namespace regbits;

namespace arm {

struct Xpsr {
    using              pos_t = Pos<uint32_t, Xpsr>;
    static constexpr   pos_t
           N_POS = pos_t(31),
           Z_POS = pos_t(30),
           C_POS = pos_t(29),
           V_POS = pos_t(28),
           Q_POS = pos_t(27),
      IT_1_0_POS = pos_t(25),
           T_POS = pos_t(24),
          GE_POS = pos_t(16),
      IT_7_2_POS = pos_t(10),
           A_POS = pos_t( 9),
         ICI_POS = pos_t( 2),
         ISR_POS = pos_t( 0);

    using            bits_t = regbits::Bits<uint32_t, Xpsr>;
    static constexpr bits_t
    N = bits_t(1, N_POS),
    Z = bits_t(1, Z_POS),
    C = bits_t(1, C_POS),
    V = bits_t(1, V_POS),
    Q = bits_t(1, Q_POS),
    T = bits_t(1, T_POS),
    A = bits_t(1, A_POS);  // normally 0, "stack align" for exception handling

    static const uint32_t
          GE_MASK = 0xF  ,
         ISR_MASK = 0x1FF,
      IT_1_0_MASK = 0x3  ,
      IT_7_2_MASK = 0x3F ,
         ICI_MASK = 0xF  ;

    using   mskd_t = Mskd<uint32_t, Xpsr>;
    using   shft_t = Shft<uint32_t, Xpsr>;

    REGBITS_MSKD_RANGE("Xpsr::Ge",
                       GE,
                       ge,
                       GE_MASK,
                       GE_POS,
                       GE_MASK);
    REGBITS_MSKD_RANGE("Xpsr::Ici",
                       ICI,
                       ici,
                       ICI_MASK,
                       ICI_POS,
                       ICI_MASK);
    REGBITS_MSKD_RANGE("Xpsr::Isr",
                       ISR,
                       isr,
                       ISR_MASK,
                       ISR_POS,
                       ISR_MASK);


#if 0
    uint8_t it() const { return   ((this->_bits & 0x0000fc00) >>  8)
                                | ((this->_bits & 0x06000000) >> 25); }

    void get() { __asm__ volatile("mrs %0, xpsr" : "=r" (*this) :            ); }
    void set() { __asm__ volatile("msr %0, xpsr" :              : "r" (*this)); }
#endif
};
using xpsr_t = regbits::Reg<uint32_t, Xpsr>;



struct Primask {
    static uint32_t get()
    {
        uint32_t    value;
        __asm__ volatile ("mrs %0, primask" : "=r" (value) );
        return(value);
    }
    static void set(
    const uint32_t  value)
    {
      __asm__ volatile ("msr primask, %0" : : "r" (value) : );
    }
};

struct Faultmask {
    static uint32_t get()
    {
        uint32_t    value;
        __asm__ volatile ("mrs %0, faultmask" : "=r" (value) );
        return(value);
    }
    static void set(
    const uint32_t  value)
    {
      __asm__ volatile ("msr faultmask, %0" : : "r" (value) : );
    }
};

struct Basepri {
    static uint32_t get()
    {
        uint32_t    value;
        __asm__ volatile ("mrs %0, basepri" : "=r" (value) );
        return(value);
    }
    static void set(
    const uint32_t  value)
    {
      __asm__ volatile ("msr basepri, %0" : : "r" (value) : );
    }
};



#ifdef CORE_CM3_DEBUG_OBJECTS
xpsr_t      xpsr;
Primask     primask;
Faultmask   faultmask;
Basepri     basepri;
#endif



struct ScnScb {
    uint32_t    reserved0,
                ictr     ,
                actlr    ;
};  // struct ScnScb
static_assert(sizeof(ScnScb) == 12, "sizeof(ScnScb) != 12");



struct SysTick {
    struct Ctrl {
        using            pos_t = regbits::Pos<uint32_t, Ctrl>;
        static constexpr pos_t
           COUNTFLAG_POS = pos_t(16),
           CLKSOURCE_POS = pos_t( 2),
             TICKINT_POS = pos_t( 1),
              ENABLE_POS = pos_t( 0);

        using            bits_t = regbits::Bits<uint32_t, Ctrl>;
        static constexpr bits_t
            COUNTFLAG        = bits_t(1,    COUNTFLAG_POS),
            CLKSOURCE        = bits_t(1,    CLKSOURCE_POS),
            CLK_SRC_CPU      = bits_t(1,    CLKSOURCE_POS),  // main clock
            CLK_SRC_EXTERNAL = bits_t(0,    CLKSOURCE_POS),  // main/8 for STM
            TICKINT          = bits_t(1,      TICKINT_POS),
            ENABLE           = bits_t(1,       ENABLE_POS);
    };  // struct Ctrl
    using ctrl_t = regbits::Reg<uint32_t, Ctrl>;
          ctrl_t   ctrl;


    struct Load {
        static const uint32_t
                 RELOAD_MAX  = 0xFFFFFFUL;
        // just use operator=(uint32) to write and word() to read
        // bits 31:24 are guaranteed RAZ/WI
        // only implemented as Reg for RELOAD_MAX const
    };  // struct Load
    using load_t = regbits::Reg<uint32_t, Load>;
          load_t   load;


    // any MSBs > Load::RELOAD_MAX are RAZ
    // any write to register clears all bits
                 uint32_t   val;
    static const uint32_t   VAL_MAX = 0xffffff;


    struct Calib {
        using            pos_t = regbits::Pos<uint32_t, Calib>;
        static constexpr pos_t
               NOREF_POS = pos_t(31),
                SKEW_POS = pos_t(30),
               TENMS_POS = pos_t( 0);

        using            bits_t = regbits::Bits<uint32_t, Calib>;
        static constexpr bits_t
            NOREF            = bits_t(1,        NOREF_POS),
            SKEW             = bits_t(1,         SKEW_POS);

        static const uint32_t
                  TENMS_MASK = 0xFFFFFFUL;
    };  // struct Calib
    using calib_t = regbits::Reg<uint32_t, Calib>;
          calib_t   calib;

};  // struct SysTick
static_assert(sizeof(SysTick) == 16, "sizeof(SysTick) != 16");



enum class NvicIrqn;

struct Nvic {
    static const uint8_t    NUM_INTERRUPT_REGS =     8,
                            NUM_PRIORITY_REGS    = 240;

    struct IntrptRegs {
        void set(
        NvicIrqn    irqn)
        volatile
        {
              _interrupts[static_cast<unsigned>(irqn) >> 5]
            = (1 << (static_cast<unsigned>(irqn) & 0x1f)  );
        }

        bool get(
        NvicIrqn    irqn)
        volatile
        {
            return   _interrupts[static_cast<unsigned>(irqn) >> 5]
                   & (1 << (static_cast<unsigned>(irqn) & 0x1f)  );
        }

        uint32_t bits(
        NvicIrqn    irqn)
        volatile
        {
            return _interrupts[static_cast<unsigned>(irqn) >> 5];
        }

        volatile uint32_t   _interrupts[NUM_INTERRUPT_REGS];
    };  // struct IntrptRegs


    public:  IntrptRegs     iser          ;
    private: uint32_t       _reserved0[24];

    public:  IntrptRegs     icer          ;
    private: uint32_t       _reserved1[24];

    public:  IntrptRegs     ispr          ;
    private: uint32_t       _reserved2[24];

    public:  IntrptRegs     icpr          ;
    private: uint32_t       _reserved3[24];

    public:  IntrptRegs     iabr          ;
    private: uint32_t       _reserved4[56];

    public:
    volatile uint8_t    Ip       [NUM_PRIORITY_REGS];
    volatile uint32_t   _reserved[              644],
                        Stir                        ;

};   // struct Nvic
static_assert(sizeof(Nvic) == 0xE04, "sizeof(Nvic) != 0xE04");



struct Scb {
    struct Cpuid {
        using              pos_t = Pos<uint32_t, Cpuid>;
        static constexpr   pos_t
         IMPLEMENTER_POS = pos_t(24),
             VARIANT_POS = pos_t(20),
        ARCHITECTURE_POS = pos_t(16),
              PARTNO_POS = pos_t( 4),
            REVISION_POS = pos_t( 0);

        static const uint32_t
         IMPLEMENTER_MASK =     0xFFUL,
             VARIANT_MASK =      0xFUL,
        ARCHITECTURE_MASK =      0xFUL,
              PARTNO_MASK =    0xFFFUL,
            REVISION_MASK =      0xFUL;

        using   mskd_t = Mskd<uint32_t, Cpuid>;
        using   shft_t = Shft<uint32_t, Cpuid>;

        REGBITS_MSKD_RANGE("Cpuid::Implementer",
                           IMPLEMENTER,
                           implementer,
                           IMPLEMENTER_MASK,
                           IMPLEMENTER_POS,
                           IMPLEMENTER_MASK);
        REGBITS_MSKD_RANGE("Cpuid::Variant",
                           VARIANT,
                           variant,
                           VARIANT_MASK,
                           VARIANT_POS,
                           VARIANT_MASK);
        REGBITS_MSKD_RANGE("Cpuid::Architecture",
                           ARCHITECTURE,
                           architecture,
                           ARCHITECTURE_MASK,
                           ARCHITECTURE_POS,
                           ARCHITECTURE_MASK);
        REGBITS_MSKD_RANGE("Cpuid::Partno",
                           PARTNO,
                           partno,
                           PARTNO_MASK,
                           PARTNO_POS,
                           PARTNO_MASK);
        REGBITS_MSKD_RANGE("Cpuid::Revision",
                           REVISION,
                           revision,
                           REVISION_MASK,
                           REVISION_POS,
                           REVISION_MASK);
    };  // struct Cpuid
    using cpuid_t = Reg<uint32_t, Cpuid>;
          cpuid_t   cpuid;


    struct Icsr {
        using              pos_t = Pos<uint32_t, Icsr>;
        static constexpr   pos_t
          NMIPENDSET_POS = pos_t(31),
           PENDSVSET_POS = pos_t(28),
           PENDSVCLR_POS = pos_t(27),
           PENDSTSET_POS = pos_t(26),
           PENDSTCLR_POS = pos_t(25),
          ISRPREEMPT_POS = pos_t(23),
          ISRPENDING_POS = pos_t(22),
         VECTPENDING_POS = pos_t(12),
           RETTOBASE_POS = pos_t(11),
          VECTACTIVE_POS = pos_t( 0);

        using              bits_t = Bits<uint32_t, Icsr>;
        static constexpr   bits_t
        NMIPENDSET       = bits_t(1,   NMIPENDSET_POS),
        PENDSVSET        = bits_t(1,    PENDSVSET_POS),
        PENDSVCLR        = bits_t(1,    PENDSVCLR_POS),
        PENDSTSET        = bits_t(1,    PENDSTSET_POS),
        PENDSTCLR        = bits_t(1,    PENDSTCLR_POS),
        ISRPREEMPT       = bits_t(1,   ISRPREEMPT_POS),
        ISRPENDING       = bits_t(1,   ISRPENDING_POS),
        RETTOBASE        = bits_t(1,    RETTOBASE_POS);

        static const uint32_t
        VECTPENDING_MASK =    0x1FFUL,
         VECTACTIVE_MASK =    0x1FFUL;

        using   mskd_t = Mskd<uint32_t, Icsr>;
        using   shft_t = Shft<uint32_t, Icsr>;

        REGBITS_MSKD_RANGE("Icsr::Vectpending",
                           VECTPENDING,
                           vectpending,
                           VECTPENDING_MASK,
                           VECTPENDING_POS,
                           VECTPENDING_MASK);
        REGBITS_MSKD_RANGE("Icsr::Vectactive",
                           VECTACTIVE,
                           vectactive,
                           VECTACTIVE_MASK,
                           VECTACTIVE_POS,
                           VECTACTIVE_MASK);
    };  // struct Icsr
    using icsr_t = Reg<uint32_t, Icsr>;
          icsr_t   icsr;


    struct Vtor {
        using              pos_t = Pos<uint32_t, Vtor>;
        static constexpr   pos_t
              TBLOFF_POS = pos_t( 7);


        static const uint32_t
             TBLOFF_MASK = 0x1FFFFFFUL;

        using   mskd_t = Mskd<uint32_t, Vtor>;
        using   shft_t = Shft<uint32_t, Vtor>;

        REGBITS_MSKD_RANGE("Vtor::Tbloff",
                           TBLOFF,
                           tbloff,
                           TBLOFF_MASK,
                           TBLOFF_POS,
                           TBLOFF_MASK);
    };  // struct Vtor
    using vtor_t = Reg<uint32_t, Vtor>;
          vtor_t   vtor;


    struct Aircr {
        using              pos_t = Pos<uint32_t, Aircr>;
        static constexpr   pos_t
             VECTKEY_POS = pos_t(16),
         VECTKEYSTAT_POS = pos_t(16),
           ENDIANESS_POS = pos_t(15),
            PRIGROUP_POS = pos_t( 8),
         SYSRESETREQ_POS = pos_t( 2),
        VECTCLRACTIVE_POS = pos_t( 1),
           VECTRESET_POS = pos_t( 0);

        using              bits_t = Bits<uint32_t, Aircr>;
        static constexpr   bits_t
        ENDIANESS        = bits_t(1,    ENDIANESS_POS),
        SYSRESETREQ      = bits_t(1,  SYSRESETREQ_POS),
        VECTCLRACTIVE    = bits_t(1, VECTCLRACTIVE_POS),
        VECTRESET        = bits_t(1,    VECTRESET_POS);

        static const uint32_t
            VECTKEY_MASK =   0xFFFFUL,
        VECTKEYSTAT_MASK =   0xFFFFUL,
           PRIGROUP_MASK =        7UL;

        using   mskd_t = Mskd<uint32_t, Aircr>;
        using   shft_t = Shft<uint32_t, Aircr>;

        REGBITS_MSKD_RANGE("Aircr::Vectkey",
                           VECTKEY,
                           vectkey,
                           VECTKEY_MASK,
                           VECTKEY_POS,
                           VECTKEY_MASK);
        REGBITS_MSKD_RANGE("Aircr::Vectkeystat",
                           VECTKEYSTAT,
                           vectkeystat,
                           VECTKEYSTAT_MASK,
                           VECTKEYSTAT_POS,
                           VECTKEYSTAT_MASK);
        REGBITS_MSKD_RANGE("Aircr::Prigroup",
                           PRIGROUP,
                           prigroup,
                           PRIGROUP_MASK,
                           PRIGROUP_POS,
                           PRIGROUP_MASK);
    };  // struct Aircr
    using aircr_t = Reg<uint32_t, Aircr>;
          aircr_t   aircr;


    struct Scr {
        using              pos_t = Pos<uint32_t, Scr>;
        static constexpr   pos_t
           SEVONPEND_POS = pos_t( 4),
           SLEEPDEEP_POS = pos_t( 2),
         SLEEPONEXIT_POS = pos_t( 1);

        using              bits_t = Bits<uint32_t, Scr>;
        static constexpr   bits_t
        SEVONPEND        = bits_t(1,    SEVONPEND_POS),
        SLEEPDEEP        = bits_t(1,    SLEEPDEEP_POS),
        SLEEPONEXIT      = bits_t(1,  SLEEPONEXIT_POS);
    };  // struct Scr
    using scr_t = Reg<uint32_t, Scr>;
          scr_t   scr;


    struct Ccr {
        using              pos_t = Pos<uint32_t, Ccr>;
        static constexpr   pos_t
            STKALIGN_POS = pos_t( 9),
           BFHFNMIGN_POS = pos_t( 8),
           DIV_0_TRP_POS = pos_t( 4),
         UNALIGN_TRP_POS = pos_t( 3),
        USERSETMPEND_POS = pos_t( 1),
        NONBASETHRDENA_POS = pos_t( 0);

        using              bits_t = Bits<uint32_t, Ccr>;
        static constexpr   bits_t
        STKALIGN         = bits_t(1,     STKALIGN_POS),
        BFHFNMIGN        = bits_t(1,    BFHFNMIGN_POS),
        DIV_0_TRP        = bits_t(1,    DIV_0_TRP_POS),
        UNALIGN_TRP      = bits_t(1,  UNALIGN_TRP_POS),
        USERSETMPEND     = bits_t(1, USERSETMPEND_POS),
        NONBASETHRDENA   = bits_t(1, NONBASETHRDENA_POS);
    };  // struct Ccr
    using ccr_t = Reg<uint32_t, Ccr>;
          ccr_t   ccr;


    uint8_t     shp[12];


    struct Shcsr {
        using              pos_t = Pos<uint32_t, Shcsr>;
        static constexpr   pos_t
         USGFAULTENA_POS   = pos_t(18),
         BUSFAULTENA_POS   = pos_t(17),
         MEMFAULTENA_POS   = pos_t(16),
        SVCALLPENDED_POS   = pos_t(15),
        BUSFAULTPENDED_POS = pos_t(14),
        MEMFAULTPENDED_POS = pos_t(13),
        USGFAULTPENDED_POS = pos_t(12),
          SYSTICKACT_POS   = pos_t(11),
           PENDSVACT_POS   = pos_t(10),
          MONITORACT_POS   = pos_t( 8),
           SVCALLACT_POS   = pos_t( 7),
         USGFAULTACT_POS   = pos_t( 3),
         BUSFAULTACT_POS   = pos_t( 1),
         MEMFAULTACT_POS   = pos_t( 0);

        using              bits_t = Bits<uint32_t, Shcsr>;
        static constexpr   bits_t
        USGFAULTENA      = bits_t(1,  USGFAULTENA_POS),
        BUSFAULTENA      = bits_t(1,  BUSFAULTENA_POS),
        MEMFAULTENA      = bits_t(1,  MEMFAULTENA_POS),
        SVCALLPENDED     = bits_t(1, SVCALLPENDED_POS),
        BUSFAULTPENDED   = bits_t(1, BUSFAULTPENDED_POS),
        MEMFAULTPENDED   = bits_t(1, MEMFAULTPENDED_POS),
        USGFAULTPENDED   = bits_t(1, USGFAULTPENDED_POS),
        SYSTICKACT       = bits_t(1,   SYSTICKACT_POS),
        PENDSVACT        = bits_t(1,    PENDSVACT_POS),
        MONITORACT       = bits_t(1,   MONITORACT_POS),
        SVCALLACT        = bits_t(1,    SVCALLACT_POS),
        USGFAULTACT      = bits_t(1,  USGFAULTACT_POS),
        BUSFAULTACT      = bits_t(1,  BUSFAULTACT_POS),
        MEMFAULTACT      = bits_t(1,  MEMFAULTACT_POS);
    };  // struct Shcsr
    using shcsr_t = Reg<uint32_t, Shcsr>;
          shcsr_t   shcsr;


    struct Cfsr {
        using              pos_t = Pos<uint32_t, Cfsr>;
        static constexpr   pos_t
          USGFAULTSR_POS = pos_t(16),
          BUSFAULTSR_POS = pos_t( 8),
          MEMFAULTSR_POS = pos_t( 0);

        static const uint32_t
         USGFAULTSR_MASK =   0xFFFFUL,
         BUSFAULTSR_MASK =     0xFFUL,
         MEMFAULTSR_MASK =     0xFFUL;

        using   mskd_t = Mskd<uint32_t, Cfsr>;
        using   shft_t = Shft<uint32_t, Cfsr>;

        REGBITS_MSKD_RANGE("Cfsr::Usgfaultsr",
                           USGFAULTSR,
                           usgfaultsr,
                           USGFAULTSR_MASK,
                           USGFAULTSR_POS,
                           USGFAULTSR_MASK);
        REGBITS_MSKD_RANGE("Cfsr::Busfaultsr",
                           BUSFAULTSR,
                           busfaultsr,
                           BUSFAULTSR_MASK,
                           BUSFAULTSR_POS,
                           BUSFAULTSR_MASK);
        REGBITS_MSKD_RANGE("Cfsr::Memfaultsr",
                           MEMFAULTSR,
                           memfaultsr,
                           MEMFAULTSR_MASK,
                           MEMFAULTSR_POS,
                           MEMFAULTSR_MASK);
    };  // struct Cfsr
    using cfsr_t = Reg<uint32_t, Cfsr>;
          cfsr_t   cfsr;


    struct Hfsr {
        using              pos_t = Pos<uint32_t, Hfsr>;
        static constexpr   pos_t
            DEBUGEVT_POS = pos_t(31),
              FORCED_POS = pos_t(30),
             VECTTBL_POS = pos_t( 1);

        using              bits_t = Bits<uint32_t, Hfsr>;
        static constexpr   bits_t
        DEBUGEVT         = bits_t(1,     DEBUGEVT_POS),
        FORCED           = bits_t(1,       FORCED_POS),
        VECTTBL          = bits_t(1,      VECTTBL_POS);
    };  // struct Hfsr
    using hfsr_t = Reg<uint32_t, Hfsr>;
          hfsr_t   hfsr;


    struct Dfsr {
        using              pos_t = Pos<uint32_t, Dfsr>;
        static constexpr   pos_t
            EXTERNAL_POS = pos_t( 4),
              VCATCH_POS = pos_t( 3),
             DWTTRAP_POS = pos_t( 2),
                BKPT_POS = pos_t( 1),
              HALTED_POS = pos_t( 0);

        using              bits_t = Bits<uint32_t, Dfsr>;
        static constexpr   bits_t
        EXTERNAL         = bits_t(1,     EXTERNAL_POS),
        VCATCH           = bits_t(1,       VCATCH_POS),
        DWTTRAP          = bits_t(1,      DWTTRAP_POS),
        BKPT             = bits_t(1,         BKPT_POS),
        HALTED           = bits_t(1,       HALTED_POS);
    };  // struct Dfsr
    using dfsr_t = Reg<uint32_t, Dfsr>;
          dfsr_t   dfsr;


    uint32_t    mmfar,
                bfar,
                afsr;


    struct Pfr0 {
        using              pos_t = Pos<uint32_t, Pfr0>;
        static constexpr   pos_t
        STATE1_POS = pos_t(4),
        STATE0_POS = pos_t(0);

        static const uint32_t
        STATE1_MASK = 0x0f,
        STATE0_MASK = 0x0f;

        using   mskd_t = Mskd<uint32_t, Pfr0>;
        static constexpr   mskd_t
        STATE1_THUMB  = mskd_t(STATE1_MASK, 3, STATE1_POS),
        STATE0_NO_ARM = mskd_t(STATE0_MASK, 0, STATE0_POS),
        STATE0_ARM    = mskd_t(STATE0_MASK, 3, STATE0_POS);
    };  // struct Pfr0
    using pfr0_t = Reg<uint32_t, Pfr0>;
          pfr0_t   pfr0;


    struct Pfr1 {
        using              pos_t = Pos<uint32_t, Pfr1>;
        static constexpr   pos_t
        TWO_STACK_POS = pos_t(8);

        static const uint32_t
        TWO_STACK_MASK = 0xf;

        using   mskd_t = Mskd<uint32_t, Pfr1>;
        static constexpr   mskd_t
        TWO_STACK = mskd_t(TWO_STACK_MASK, 2, TWO_STACK_POS);
    };  // struct Pfr1
    using pfr1_t = Reg<uint32_t, Pfr1>;
          pfr1_t   pfr1;


    struct Dfr {
        using              pos_t = Pos<uint32_t, Dfr>;
        static constexpr   pos_t
        DEBUG_POS = pos_t(20);

        static const uint32_t
        DEBUG_MASK = 0xf;

        using   mskd_t = Mskd<uint32_t, Dfr>;
        static constexpr   mskd_t
        DEBUG = mskd_t(DEBUG_MASK, 1, DEBUG_POS);
    };  // struct Dfr
    using dfr_t = Reg<uint32_t, Dfr>;
          dfr_t   dfr;


    uint32_t    adr;


    struct Mmfr0 {
        using              pos_t = Pos<uint32_t, Mmfr0>;
        static constexpr   pos_t
            AUX_REGS_POS = pos_t(20),
                 TCM_POS = pos_t(16),
        SHAREABILITY_POS = pos_t(12),
           OUTERMOST_POS = pos_t( 8),
                PMSA_POS = pos_t( 4);

        static const uint32_t
            AUX_REGS_MASK = 0xf,
                 TCM_MASK = 0xf,
        SHAREABILITY_MASK = 0xf,
           OUTERMOST_MASK = 0xf,
                PMSA_MASK = 0xf;

        using   mskd_t = Mskd<uint32_t, Mmfr0>;
        static constexpr   mskd_t
        AUX_REGS_CTRL_ONLY  = mskd_t(     AUX_REGS_MASK, 1,     AUX_REGS_POS),
        TCM                 = mskd_t(         TCM_MASK,  3,          TCM_POS),
        SHAREABILITY_1      = mskd_t(SHAREABILITY_MASK,  0, SHAREABILITY_POS),
        SHAREABILITY_NO_1   = mskd_t(SHAREABILITY_MASK,  1, SHAREABILITY_POS),
        OUTERMOST_NON_CACHE = mskd_t(   OUTERMOST_MASK,  1,    OUTERMOST_POS),
        OUTERMOST_IGNORED   = mskd_t(   OUTERMOST_MASK, 15,    OUTERMOST_POS),
        PMSAV7              = mskd_t(        PMSA_MASK,  3,         PMSA_POS);
    };  // struct Mmfr0
    using mmfr0_t = Reg<uint32_t, Mmfr0>;
          mmfr0_t   mmfr0;


    uint32_t    mmfr1;


    struct Mmfr2 {
        using              pos_t = Pos<uint32_t, Mmfr0>;
        static constexpr   pos_t
        WFI_STALL_POS    = pos_t(24);

        static const uint32_t
        WFI_STALL_MASK = 0xf;

        using   mskd_t = Mskd<uint32_t, Mmfr0>;
        static constexpr   mskd_t
        WFI_STALL = mskd_t(WFI_STALL_MASK, 1, WFI_STALL_POS);
    };  // struct Mmfr0
    using mmfr2_t = Reg<uint32_t, Mmfr2>;
          mmfr2_t   mmfr2;


    uint32_t    mmfr3;


    struct Isar0 {
        using              pos_t = Pos<uint32_t, Isar0>;
        static constexpr   pos_t
           DIVIDE_POS = pos_t(24),
            DEBUG_POS = pos_t(20),
           COPROC_POS = pos_t(16),
        CMPBRANCH_POS = pos_t(12),
           BITFLD_POS = pos_t( 8),
         BITCOUNT_POS = pos_t( 4);

        static const uint32_t
           DIVIDE_MASK = 0xf,
            DEBUG_MASK = 0xf,
           COPROC_MASK = 0xf,
        CMPBRANCH_MASK = 0xf,
           BITFLD_MASK = 0xf,
         BITCOUNT_MASK = 0xf;

        using   mskd_t = Mskd<uint32_t, Isar0>;
        static constexpr   mskd_t
           DIVIDE_DIV  = mskd_t(   DIVIDE_MASK,   1,    DIVIDE_POS),
                 DEBUG = mskd_t(    DEBUG_MASK,   1,     DEBUG_POS),
           COPROC_CDP1 = mskd_t(   COPROC_MASK,   1,    COPROC_POS),
           COPROC_CDP2 = mskd_t(   COPROC_MASK,   2,    COPROC_POS),
           COPROC_MCRR = mskd_t(   COPROC_MASK,   3,    COPROC_POS),
          COPROC_MCRR2 = mskd_t(   COPROC_MASK,   4,    COPROC_POS),
        CMPBRANCH_BFC  = mskd_t(CMPBRANCH_MASK,   1, CMPBRANCH_POS),
         BITCOUNT_CLZ  = mskd_t( BITCOUNT_MASK,   1,  BITCOUNT_POS);
    };  // struct Isar0
    using isar0_t = Reg<uint32_t, Isar0>;
          isar0_t   isar0;


    struct Isar1 {
        using              pos_t = Pos<uint32_t, Isar1>;
        static constexpr   pos_t
        INTERWORK_POS = pos_t(24),
        IMMEDIATE_POS = pos_t(20),
           IFTHEN_POS = pos_t(16),
           EXTEND_POS = pos_t(12);

        static const uint32_t
        INTERWORK_MASK = 0xf,
        IMMEDIATE_MASK = 0xf,
           IFTHEN_MASK = 0xf,
           EXTEND_MASK = 0xf;

        using   mskd_t = Mskd<uint32_t, Isar1>;
        static constexpr   mskd_t
          INTERWORK_BX = mskd_t(INTERWORK_MASK,   1, INTERWORK_POS),
         INTERWORK_BLX = mskd_t(INTERWORK_MASK,   2, INTERWORK_POS),
        IMMEDIATE_MOVT = mskd_t(IMMEDIATE_MASK,   1, IMMEDIATE_POS),
             IFTHEN_IT = mskd_t(   IFTHEN_MASK,   1,    IFTHEN_POS),
           EXTEND_SXTB = mskd_t(   EXTEND_MASK,   1,    EXTEND_POS),
          EXTEND_SXTAB = mskd_t(   EXTEND_MASK,   2,    EXTEND_POS);
    };  // struct Isar1
    using isar1_t = Reg<uint32_t, Isar1>;
          isar1_t   isar1;


    struct Isar2 {
        using              pos_t = Pos<uint32_t, Isar2>;
        static constexpr   pos_t
          REVERSAL_POS = pos_t(20),
             MULTU_POS = pos_t(20),
             MULTS_POS = pos_t(16),
              MULT_POS = pos_t(12),
        MULTIACCESS_POS = pos_t( 8),
           MEMHINT_POS = pos_t( 4),
         LOADSTORE_POS = pos_t( 0);

        static const uint32_t
          REVERSAL_MASK = 0xf,
             MULTU_MASK = 0xf,
             MULTS_MASK = 0xf,
              MULT_MASK = 0xf,
        MULTIACCESS_MASK = 0xf,
           MEMHINT_MASK = 0xf,
         LOADSTORE_MASK = 0xf;

        using   mskd_t = Mskd<uint32_t, Isar2>;
        static constexpr   mskd_t
           REVERSAL_REV = mskd_t(    REVERSAL_MASK,    1,    REVERSAL_POS),
          REVERSAL_RBIT = mskd_t(    REVERSAL_MASK,    2,    REVERSAL_POS),
            MULTU_UMULL = mskd_t(       MULTU_MASK,    1,       MULTU_POS),
            MULTU_UMAAL = mskd_t(       MULTU_MASK,    2,       MULTU_POS),
            MULTS_SMULL = mskd_t(       MULTS_MASK,    1,       MULTS_POS),
           MULTS_SMLABB = mskd_t(       MULTS_MASK,    2,       MULTS_POS),
            MULTS_SMLAD = mskd_t(       MULTS_MASK,    3,       MULTS_POS),
               MULT_MLA = mskd_t(        MULT_MASK,    1,        MULT_POS),
               MULT_MLS = mskd_t(        MULT_MASK,    2,        MULT_POS),
        MULTIACCESS_STRT = mskd_t(MULTIACCESS_MASK,    1, MULTIACCESS_POS),
        MULTIACCESS_CONT = mskd_t(MULTIACCESS_MASK,    2, MULTIACCESS_POS),
           MEMHINT_PLD1 = mskd_t(      MEMHINT_MASK,   1,     MEMHINT_POS),
           MEMHINT_PLD2 = mskd_t(      MEMHINT_MASK,   2,     MEMHINT_POS),
           MEMHINT_PLI  = mskd_t(      MEMHINT_MASK,   3,     MEMHINT_POS),
         LOADSTORE_LDRD = mskd_t(    LOADSTORE_MASK,   1,   LOADSTORE_POS);
    };  // struct Isar2
    using isar2_t = Reg<uint32_t, Isar2>;
          isar2_t   isar2;


    struct Isar3 {
        using              pos_t = Pos<uint32_t, Isar3>;
        static constexpr   pos_t
          TRUENOP_POS = pos_t(24),
        THUMBCOPY_POS = pos_t(20),
        TABBRANCH_POS = pos_t(16),
        SYNCHPRIM_POS = pos_t(12),
              SVC_POS = pos_t( 8),
             SIMD_POS = pos_t( 4),
         SATURATE_POS = pos_t( 0);

        static const uint32_t
          TRUENOP_MASK = 0xf,
        THUMBCOPY_MASK = 0xf,
        TABBRANCH_MASK = 0xf,
        SYNCHPRIM_MASK = 0xf,
              SVC_MASK = 0xf,
             SIMD_MASK = 0xf,
         SATURATE_MASK = 0xf;

        using   mskd_t = Mskd<uint32_t, Isar3>;
        static constexpr   mskd_t
           TRUENOP_NOP = mskd_t(  TRUENOP_MASK,   1,   TRUENOP_POS),
        THUMBCOPY_MOV  = mskd_t(THUMBCOPY_MASK,   1, THUMBCOPY_POS),
        TABBRANCH_TBB  = mskd_t(TABBRANCH_MASK,   1, TABBRANCH_POS),
              SVC_SVC  = mskd_t(      SVC_MASK, 0xf,       SVC_POS),
             SIMD_SSAT = mskd_t(     SIMD_MASK,   1,      SIMD_POS),
             SIMD_QADD = mskd_t(     SIMD_MASK,   3,      SIMD_POS),
         SATURATE_QADD = mskd_t( SATURATE_MASK,   1,  SATURATE_POS);
    };  // struct Isar3
    using isar3_t = Reg<uint32_t, Isar3>;
          isar3_t   isar3;


    struct Isar4 {
        using              pos_t = Pos<uint32_t, Isar4>;
        static constexpr   pos_t
                 PSR_M_POS = pos_t(24),
        SYNCHPRIM_FRAC_POS = pos_t(20),
               BARRIER_POS = pos_t(16),
             WRITEBACK_POS = pos_t( 8),
            WITHSHIFTS_POS = pos_t( 4),
                UNPRIV_POS = pos_t( 0);

        static const uint32_t
                 PSR_M_MASK = 0xf,
        SYNCHPRIM_FRAC_MASK = 0xf,
               BARRIER_MASK = 0xf,
             WRITEBACK_MASK = 0xf,
            WITHSHIFTS_MASK = 0xf,
                UNPRIV_MASK = 0xf;

        using   mskd_t = Mskd<uint32_t, Isar4>;
        static constexpr   mskd_t
                 PSR_M_CPS = mskd_t(         PSR_M_MASK,   1,         PSR_M_POS),
                BARRIER_DMB = mskd_t(      BARRIER_MASK,   1,       BARRIER_POS),
             WRITEBACK_WBCK = mskd_t(    WRITEBACK_MASK,   1,     WRITEBACK_POS),
            WITHSHIFTS_LSL  = mskd_t(   WITHSHIFTS_MASK,   1,    WITHSHIFTS_POS),
            WITHSHIFTS_ALL  = mskd_t(   WITHSHIFTS_MASK,   3,    WITHSHIFTS_POS);
    };  // struct Isar4
    using isar4_t = Reg<uint32_t, Isar4>;
          isar4_t   isar4;


    uint32_t    reserved0[5];


    struct Cpacr {
        using              pos_t = Pos<uint32_t, Cpacr>;
        static constexpr   pos_t
        CP11_POS = pos_t(22),
        CP10_POS = pos_t(20),
         CP9_POS = pos_t(18),
         CP8_POS = pos_t(16),
         CP7_POS = pos_t(14),
         CP6_POS = pos_t(12),
         CP5_POS = pos_t(10),
         CP4_POS = pos_t( 8),
         CP3_POS = pos_t( 6),
         CP2_POS = pos_t( 4),
         CP1_POS = pos_t( 2),
         CP0_POS = pos_t( 0);

        static const uint32_t
        CP11_MASK = 0x3,
        CP10_MASK = 0x3,
         CP7_MASK = 0x3,
         CP6_MASK = 0x3,
         CP5_MASK = 0x3,
         CP4_MASK = 0x3,
         CP3_MASK = 0x3,
         CP2_MASK = 0x3,
         CP1_MASK = 0x3,
         CP0_MASK = 0x3;

        using   mskd_t = Mskd<uint32_t, Cpacr>;
        static constexpr   mskd_t
        CP11_NONE = mskd_t(CP11_MASK, 0b00     , CP11_POS),
        CP11_PRIV = mskd_t(CP11_MASK, 0b01     , CP11_POS),
        CP11_FULL = mskd_t(CP11_MASK, 0b11     , CP11_POS),
        CP10_NONE = mskd_t(CP10_MASK, 0b00     , CP10_POS),
        CP10_PRIV = mskd_t(CP10_MASK, 0b01     , CP10_POS),
        CP10_FULL = mskd_t(CP10_MASK, 0b11     , CP10_POS),
         CP7_NONE = mskd_t( CP7_MASK,  0b00    ,  CP7_POS),
         CP7_PRIV = mskd_t( CP7_MASK,  0b01    ,  CP7_POS),
         CP7_FULL = mskd_t( CP7_MASK,  0b11    ,  CP7_POS),
         CP6_NONE = mskd_t( CP6_MASK,  0b00    ,  CP6_POS),
         CP6_PRIV = mskd_t( CP6_MASK,  0b01    ,  CP6_POS),
         CP6_FULL = mskd_t( CP6_MASK,  0b11    ,  CP6_POS),
         CP5_NONE = mskd_t( CP5_MASK,  0b00    ,  CP5_POS),
         CP5_PRIV = mskd_t( CP5_MASK,  0b01    ,  CP5_POS),
         CP5_FULL = mskd_t( CP5_MASK,  0b11    ,  CP5_POS),
         CP4_NONE = mskd_t( CP4_MASK,  0b00    ,  CP4_POS),
         CP4_PRIV = mskd_t( CP4_MASK,  0b01    ,  CP4_POS),
         CP4_FULL = mskd_t( CP4_MASK,  0b11    ,  CP4_POS),
         CP3_NONE = mskd_t( CP3_MASK,  0b00    ,  CP3_POS),
         CP3_PRIV = mskd_t( CP3_MASK,  0b01    ,  CP3_POS),
         CP3_FULL = mskd_t( CP3_MASK,  0b11    ,  CP3_POS),
         CP2_NONE = mskd_t( CP2_MASK,  0b00    ,  CP2_POS),
         CP2_PRIV = mskd_t( CP2_MASK,  0b01    ,  CP2_POS),
         CP2_FULL = mskd_t( CP2_MASK,  0b11    ,  CP2_POS),
         CP1_NONE = mskd_t( CP1_MASK,  0b00    ,  CP1_POS),
         CP1_PRIV = mskd_t( CP1_MASK,  0b01    ,  CP1_POS),
         CP1_FULL = mskd_t( CP1_MASK,  0b11    ,  CP1_POS),
         CP0_NONE = mskd_t( CP0_MASK,  0b00    ,  CP0_POS),
         CP0_PRIV = mskd_t( CP0_MASK,  0b01    ,  CP0_POS),
         CP0_FULL = mskd_t( CP0_MASK,  0b11    ,  CP0_POS);
    };  // struct Cpacr
    using cpacr_t = Reg<uint32_t, Cpacr>;
          cpacr_t   cpacr;

};   // struct Scb
static_assert(sizeof(Scb) == 4*(16+2+4+5+5)+12,
             "sizeof(Scb) != 4*(16+2+4+5+5)+12");




static const uint32_t   SCS_BASE       = 0xE000E000UL,
                        ITM_BASE       = 0xE0000000UL,
                        DWT_BASE       = 0xE0001000UL,
                        TPI_BASE       = 0xE0040000UL,
                        COREDEBUG_BASE = 0xE000EDF0UL;

static const uint32_t   SYSTICK_BASE = SCS_BASE + 0x0010UL,
                        NVIC_BASE    = SCS_BASE + 0x0100UL,
                        SCB_BASE     = SCS_BASE + 0x0D00UL;

#define ARM_REG_GROUP(TYPE, PERIPH, BASE)           \
    static volatile TYPE* const                     \
    PERIPH = reinterpret_cast<volatile TYPE*>(BASE)

ARM_REG_GROUP(ScnScb,   scn_scb,    SCS_BASE    );
ARM_REG_GROUP(SysTick,  sys_tick,   SYSTICK_BASE);
ARM_REG_GROUP(Nvic,     nvic,       NVIC_BASE   );
ARM_REG_GROUP(Scb,      scb,        SCB_BASE    );

#undef ARM_REG_GROUP

}  // namespace arm

#endif  // #ifndef CORE_CM3_HXX
