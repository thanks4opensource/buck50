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


#ifndef STM32F103XB_HXX
#define STM32F103XB_HXX

#include <stdint.h>

#include <regbits.hxx>


#if REGBITS_MAJOR_VERSION != 1
#error REGBITS_MAJOR_VERSION != 1
#endif

#define STM32F103XB_MAJOR_VERSION   1
#define STM32F103XB_MINOR_VERSION   3
#define STM32F103XB_MICRO_VERSION   0



namespace stm32f103xb {

using namespace regbits;

struct Rcc {
    struct Cr {
        using              pos_t = Pos<uint32_t, Cr>;
        static constexpr   pos_t
               HSION_POS = pos_t( 0),
              HSIRDY_POS = pos_t( 1),
             HSITRIM_POS = pos_t( 3),
              HSICAL_POS = pos_t( 8),
               HSEON_POS = pos_t(16),
              HSERDY_POS = pos_t(17),
              HSEBYP_POS = pos_t(18),
               CSSON_POS = pos_t(19),
               PLLON_POS = pos_t(24),
              PLLRDY_POS = pos_t(25);

        using              bits_t = Bits<uint32_t, Cr>;
        static constexpr   bits_t
        HSION            = bits_t(1,        HSION_POS),
        HSIRDY           = bits_t(1,       HSIRDY_POS),
        HSEON            = bits_t(1,        HSEON_POS),
        HSERDY           = bits_t(1,       HSERDY_POS),
        HSEBYP           = bits_t(1,       HSEBYP_POS),
        CSSON            = bits_t(1,        CSSON_POS),
        PLLON            = bits_t(1,        PLLON_POS),
        PLLRDY           = bits_t(1,       PLLRDY_POS);

        static const uint32_t
            HSITRIM_MASK =      0x1FU,
             HSICAL_MASK =      0xFFU;

        using   mskd_t = Mskd<uint32_t, Cr>;
        using   shft_t = Shft<uint32_t, Cr>;

        REGBITS_MSKD_RANGE("Rcc::Cr",
                           HSITRIM,
                           hsitrim,
                           HSITRIM_MASK,
                           HSITRIM_POS,
                           HSITRIM_MASK);
        REGBITS_MSKD_RANGE("Rcc::Cr",
                           HSICAL,
                           hsical,
                           HSICAL_MASK,
                           HSICAL_POS,
                           HSICAL_MASK);
    };  // struct Cr
    using cr_t = Reg<uint32_t, Cr>;
          cr_t   cr;


    struct Cfgr {
        using              pos_t = Pos<uint32_t, Cfgr>;
        static constexpr   pos_t
                  SW_POS = pos_t( 0),
                 SWS_POS = pos_t( 2),
                HPRE_POS = pos_t( 4),
               PPRE1_POS = pos_t( 8),
               PPRE2_POS = pos_t(11),
              ADCPRE_POS = pos_t(14),
              PLLSRC_POS = pos_t(16),
            PLLXTPRE_POS = pos_t(17),
             PLLMULL_POS = pos_t(18),
              USBPRE_POS = pos_t(22),
                 MCO_POS = pos_t(24);

        using              bits_t = Bits<uint32_t, Cfgr>;
        static constexpr   bits_t
        PLLSRC           = bits_t(1,       PLLSRC_POS),
        PLLXTPRE         = bits_t(1,     PLLXTPRE_POS),
        USBPRE           = bits_t(1,       USBPRE_POS);

        static const uint32_t
                 SW_MASK =       0x3,
                SWS_MASK =       0x3,
               HPRE_MASK =       0xF,
              PPRE1_MASK =       0x7,
              PPRE2_MASK =       0x7,
             ADCPRE_MASK =       0x3,
            PLLMULL_MASK =       0xF,
                MCO_MASK =       0x7;

        using              mskd_t = Mskd<uint32_t, Cfgr>;
        static constexpr   mskd_t
        SW_HSI           = mskd_t(         SW_MASK,  0,        SW_POS),
        SW_HSE           = mskd_t(         SW_MASK,  1,        SW_POS),
        SW_PLL           = mskd_t(         SW_MASK,  2,        SW_POS),
        SWS_HSI          = mskd_t(        SWS_MASK,  0,       SWS_POS),
        SWS_HSE          = mskd_t(        SWS_MASK,  1,       SWS_POS),
        SWS_PLL          = mskd_t(        SWS_MASK,  2,       SWS_POS),

        HPRE_DIV_NONE = mskd_t(HPRE_MASK, 0b0000, HPRE_POS),
        HPRE_DIV_1    = mskd_t(HPRE_MASK, 0b0000, HPRE_POS),
        HPRE_DIV_2    = mskd_t(HPRE_MASK, 0b1000, HPRE_POS),
        HPRE_DIV_4    = mskd_t(HPRE_MASK, 0b1001, HPRE_POS),
        HPRE_DIV_8    = mskd_t(HPRE_MASK, 0b1010, HPRE_POS),
        HPRE_DIV_16   = mskd_t(HPRE_MASK, 0b1011, HPRE_POS),
        HPRE_DIV_64   = mskd_t(HPRE_MASK, 0b1100, HPRE_POS),
        HPRE_DIV_128  = mskd_t(HPRE_MASK, 0b1101, HPRE_POS),
        HPRE_DIV_256  = mskd_t(HPRE_MASK, 0b1110, HPRE_POS),
        HPRE_DIV_512  = mskd_t(HPRE_MASK, 0b1111, HPRE_POS),

        PPRE1_DIV_1   = mskd_t(PPRE1_MASK, 0b000, PPRE1_POS),
        PPRE1_DIV_2   = mskd_t(PPRE1_MASK, 0b100, PPRE1_POS),
        PPRE1_DIV_4   = mskd_t(PPRE1_MASK, 0b101, PPRE1_POS),
        PPRE1_DIV_8   = mskd_t(PPRE1_MASK, 0b110, PPRE1_POS),
        PPRE1_DIV_16  = mskd_t(PPRE1_MASK, 0b111, PPRE1_POS),

        PPRE2_DIV_1   = mskd_t(PPRE2_MASK, 0b000, PPRE2_POS),
        PPRE2_DIV_2   = mskd_t(PPRE2_MASK, 0b100, PPRE2_POS),
        PPRE2_DIV_4   = mskd_t(PPRE2_MASK, 0b101, PPRE2_POS),
        PPRE2_DIV_8   = mskd_t(PPRE2_MASK, 0b110, PPRE2_POS),
        PPRE2_DIV_16  = mskd_t(PPRE2_MASK, 0b111, PPRE2_POS),

        ADCPRE_DIV_2  = mskd_t(    ADCPRE_MASK,  0b00,       ADCPRE_POS),
        ADCPRE_DIV_4  = mskd_t(    ADCPRE_MASK,  0b01,       ADCPRE_POS),
        ADCPRE_DIV_6  = mskd_t(    ADCPRE_MASK,  0b10,       ADCPRE_POS),
        ADCPRE_DIV_8  = mskd_t(    ADCPRE_MASK,  0b11,       ADCPRE_POS),

        PLLMULL_2     = mskd_t(   PLLMULL_MASK,  0b0000,        PLLMULL_POS),
        PLLMULL_3     = mskd_t(   PLLMULL_MASK,  0b0001,        PLLMULL_POS),
        PLLMULL_4     = mskd_t(   PLLMULL_MASK,  0b0010,        PLLMULL_POS),
        PLLMULL_5     = mskd_t(   PLLMULL_MASK,  0b0011,        PLLMULL_POS),
        PLLMULL_6     = mskd_t(   PLLMULL_MASK,  0b0100,        PLLMULL_POS),
        PLLMULL_7     = mskd_t(   PLLMULL_MASK,  0b0101,        PLLMULL_POS),
        PLLMULL_8     = mskd_t(   PLLMULL_MASK,  0b0110,        PLLMULL_POS),
        PLLMULL_9     = mskd_t(   PLLMULL_MASK,  0b0111,        PLLMULL_POS),
        PLLMULL_10    = mskd_t(   PLLMULL_MASK,  0b1000,        PLLMULL_POS),
        PLLMULL_11    = mskd_t(   PLLMULL_MASK,  0b1001,        PLLMULL_POS),
        PLLMULL_12    = mskd_t(   PLLMULL_MASK,  0b1010,        PLLMULL_POS),
        PLLMULL_13    = mskd_t(   PLLMULL_MASK,  0b1011,        PLLMULL_POS),
        PLLMULL_14    = mskd_t(   PLLMULL_MASK,  0b1100,        PLLMULL_POS),
        PLLMULL_15    = mskd_t(   PLLMULL_MASK,  0b1101,        PLLMULL_POS),
        PLLMULL_16    = mskd_t(   PLLMULL_MASK,  0b1110,        PLLMULL_POS),
        PLLMULL_16ALT = mskd_t(   PLLMULL_MASK,  0b1111,        PLLMULL_POS),

        MCO_NONE      = mskd_t(       MCO_MASK,  0b000,         MCO_POS),
        MCO_SYSCLK    = mskd_t(       MCO_MASK,  0b100,         MCO_POS),
        MCO_HSI       = mskd_t(       MCO_MASK,  0b101,         MCO_POS),
        MCO_HSE       = mskd_t(       MCO_MASK,  0b110,         MCO_POS),
        MCO_PLL_DIV_2 = mskd_t(       MCO_MASK,  0b111,         MCO_POS);
    };  // struct Cfgr
    using cfgr_t = Reg<uint32_t, Cfgr>;
          cfgr_t   cfgr;


    struct Cir {
        using              pos_t = Pos<uint32_t, Cir>;
        static constexpr   pos_t
             LSIRDYF_POS = pos_t( 0),
             LSERDYF_POS = pos_t( 1),
             HSIRDYF_POS = pos_t( 2),
             HSERDYF_POS = pos_t( 3),
             PLLRDYF_POS = pos_t( 4),
                CSSF_POS = pos_t( 7),
            LSIRDYIE_POS = pos_t( 8),
            LSERDYIE_POS = pos_t( 9),
            HSIRDYIE_POS = pos_t(10),
            HSERDYIE_POS = pos_t(11),
            PLLRDYIE_POS = pos_t(12),
             LSIRDYC_POS = pos_t(16),
             LSERDYC_POS = pos_t(17),
             HSIRDYC_POS = pos_t(18),
             HSERDYC_POS = pos_t(19),
             PLLRDYC_POS = pos_t(20),
                CSSC_POS = pos_t(23);

        using              bits_t = Bits<uint32_t, Cir>;
        static constexpr   bits_t
        LSIRDYF          = bits_t(1,      LSIRDYF_POS),
        LSERDYF          = bits_t(1,      LSERDYF_POS),
        HSIRDYF          = bits_t(1,      HSIRDYF_POS),
        HSERDYF          = bits_t(1,      HSERDYF_POS),
        PLLRDYF          = bits_t(1,      PLLRDYF_POS),
        CSSF             = bits_t(1,         CSSF_POS),
        LSIRDYIE         = bits_t(1,     LSIRDYIE_POS),
        LSERDYIE         = bits_t(1,     LSERDYIE_POS),
        HSIRDYIE         = bits_t(1,     HSIRDYIE_POS),
        HSERDYIE         = bits_t(1,     HSERDYIE_POS),
        PLLRDYIE         = bits_t(1,     PLLRDYIE_POS),
        LSIRDYC          = bits_t(1,      LSIRDYC_POS),
        LSERDYC          = bits_t(1,      LSERDYC_POS),
        HSIRDYC          = bits_t(1,      HSIRDYC_POS),
        HSERDYC          = bits_t(1,      HSERDYC_POS),
        PLLRDYC          = bits_t(1,      PLLRDYC_POS),
        CSSC             = bits_t(1,         CSSC_POS);
    };  // struct Cir
    using cir_t = Reg<uint32_t, Cir>;
          cir_t   cir;


    struct Apb2rstr {
        using              pos_t = Pos<uint32_t, Apb2rstr>;
        static constexpr   pos_t
             AFIORST_POS = pos_t( 0),
             IOPARST_POS = pos_t( 2),
             IOPBRST_POS = pos_t( 3),
             IOPCRST_POS = pos_t( 4),
             IOPDRST_POS = pos_t( 5),
             ADC1RST_POS = pos_t( 9),
             ADC2RST_POS = pos_t(10),
             TIM1RST_POS = pos_t(11),
             SPI1RST_POS = pos_t(12),
           USART1RST_POS = pos_t(14),
             IOPERST_POS = pos_t( 6);

        using              bits_t = Bits<uint32_t, Apb2rstr>;
        static constexpr   bits_t
        AFIORST          = bits_t(1,      AFIORST_POS),
        IOPARST          = bits_t(1,      IOPARST_POS),
        IOPBRST          = bits_t(1,      IOPBRST_POS),
        IOPCRST          = bits_t(1,      IOPCRST_POS),
        IOPDRST          = bits_t(1,      IOPDRST_POS),
        ADC1RST          = bits_t(1,      ADC1RST_POS),
        ADC2RST          = bits_t(1,      ADC2RST_POS),
        TIM1RST          = bits_t(1,      TIM1RST_POS),
        SPI1RST          = bits_t(1,      SPI1RST_POS),
        USART1RST        = bits_t(1,    USART1RST_POS),
        IOPERST          = bits_t(1,      IOPERST_POS);
    };  // struct Apb2rstr
    using apb2rstr_t = Reg<uint32_t, Apb2rstr>;
          apb2rstr_t   apb2rstr;


    struct Apb1rstr {
        using              pos_t = Pos<uint32_t, Apb1rstr>;
        static constexpr   pos_t
             TIM2RST_POS = pos_t( 0),
             TIM3RST_POS = pos_t( 1),
             WWDGRST_POS = pos_t(11),
           USART2RST_POS = pos_t(17),
             I2C1RST_POS = pos_t(21),
             CAN1RST_POS = pos_t(25),
              BKPRST_POS = pos_t(27),
              PWRRST_POS = pos_t(28),
             TIM4RST_POS = pos_t( 2),
             SPI2RST_POS = pos_t(14),
           USART3RST_POS = pos_t(18),
             I2C2RST_POS = pos_t(22),
              USBRST_POS = pos_t(23);

        using              bits_t = Bits<uint32_t, Apb1rstr>;
        static constexpr   bits_t
        TIM2RST          = bits_t(1,      TIM2RST_POS),
        TIM3RST          = bits_t(1,      TIM3RST_POS),
        WWDGRST          = bits_t(1,      WWDGRST_POS),
        USART2RST        = bits_t(1,    USART2RST_POS),
        I2C1RST          = bits_t(1,      I2C1RST_POS),
        CAN1RST          = bits_t(1,      CAN1RST_POS),
        BKPRST           = bits_t(1,       BKPRST_POS),
        PWRRST           = bits_t(1,       PWRRST_POS),
        TIM4RST          = bits_t(1,      TIM4RST_POS),
        SPI2RST          = bits_t(1,      SPI2RST_POS),
        USART3RST        = bits_t(1,    USART3RST_POS),
        I2C2RST          = bits_t(1,      I2C2RST_POS),
        USBRST           = bits_t(1,       USBRST_POS);
    };  // struct Apb1rstr
    using apb1rstr_t = Reg<uint32_t, Apb1rstr>;
          apb1rstr_t   apb1rstr;


    struct Ahbenr {
        using              pos_t = Pos<uint32_t, Ahbenr>;
        static constexpr   pos_t
              DMA1EN_POS = pos_t( 0),
              SRAMEN_POS = pos_t( 2),
             FLITFEN_POS = pos_t( 4),
               CRCEN_POS = pos_t( 6);

        using              bits_t = Bits<uint32_t, Ahbenr>;
        static constexpr   bits_t
        DMA1EN           = bits_t(1,       DMA1EN_POS),
        SRAMEN           = bits_t(1,       SRAMEN_POS),
        FLITFEN          = bits_t(1,      FLITFEN_POS),
        CRCEN            = bits_t(1,        CRCEN_POS);
    };  // struct Ahbenr
    using ahbenr_t = Reg<uint32_t, Ahbenr>;
          ahbenr_t   ahbenr;



    struct Apb2enr {
        using              pos_t = Pos<uint32_t, Apb2enr>;
        static constexpr   pos_t
              AFIOEN_POS = pos_t( 0),
              IOPAEN_POS = pos_t( 2),
              IOPBEN_POS = pos_t( 3),
              IOPCEN_POS = pos_t( 4),
              IOPDEN_POS = pos_t( 5),
              ADC1EN_POS = pos_t( 9),
              ADC2EN_POS = pos_t(10),
              TIM1EN_POS = pos_t(11),
              SPI1EN_POS = pos_t(12),
            USART1EN_POS = pos_t(14),
              IOPEEN_POS = pos_t( 6);

        using              bits_t = Bits<uint32_t, Apb2enr>;
        static constexpr   bits_t
        AFIOEN           = bits_t(1,       AFIOEN_POS),
        IOPAEN           = bits_t(1,       IOPAEN_POS),
        IOPBEN           = bits_t(1,       IOPBEN_POS),
        IOPCEN           = bits_t(1,       IOPCEN_POS),
        IOPDEN           = bits_t(1,       IOPDEN_POS),
        ADC1EN           = bits_t(1,       ADC1EN_POS),
        ADC2EN           = bits_t(1,       ADC2EN_POS),
        TIM1EN           = bits_t(1,       TIM1EN_POS),
        SPI1EN           = bits_t(1,       SPI1EN_POS),
        USART1EN         = bits_t(1,     USART1EN_POS),
        IOPEEN           = bits_t(1,       IOPEEN_POS);
    };  // struct Apb2enr
    using apb2enr_t = Reg<uint32_t, Apb2enr>;
          apb2enr_t   apb2enr;



    struct Apb1enr {
        using              pos_t = Pos<uint32_t, Apb1enr>;
        static constexpr   pos_t
              TIM2EN_POS = pos_t( 0),
              TIM3EN_POS = pos_t( 1),
              WWDGEN_POS = pos_t(11),
            USART2EN_POS = pos_t(17),
              I2C1EN_POS = pos_t(21),
              CAN1EN_POS = pos_t(25),
               BKPEN_POS = pos_t(27),
               PWREN_POS = pos_t(28),
              TIM4EN_POS = pos_t( 2),
              SPI2EN_POS = pos_t(14),
            USART3EN_POS = pos_t(18),
              I2C2EN_POS = pos_t(22),
               USBEN_POS = pos_t(23);

        using              bits_t = Bits<uint32_t, Apb1enr>;
        static constexpr   bits_t
        TIM2EN           = bits_t(1,       TIM2EN_POS),
        TIM3EN           = bits_t(1,       TIM3EN_POS),
        WWDGEN           = bits_t(1,       WWDGEN_POS),
        USART2EN         = bits_t(1,     USART2EN_POS),
        I2C1EN           = bits_t(1,       I2C1EN_POS),
        CAN1EN           = bits_t(1,       CAN1EN_POS),
        BKPEN            = bits_t(1,        BKPEN_POS),
        PWREN            = bits_t(1,        PWREN_POS),
        TIM4EN           = bits_t(1,       TIM4EN_POS),
        SPI2EN           = bits_t(1,       SPI2EN_POS),
        USART3EN         = bits_t(1,     USART3EN_POS),
        I2C2EN           = bits_t(1,       I2C2EN_POS),
        USBEN            = bits_t(1,        USBEN_POS);
    };  // struct Apb1enr
    using apb1enr_t = Reg<uint32_t, Apb1enr>;
          apb1enr_t   apb1enr;



    struct Bdcr {
        using              pos_t = Pos<uint32_t, Bdcr>;
        static constexpr   pos_t
               LSEON_POS = pos_t( 0),
              LSERDY_POS = pos_t( 1),
              LSEBYP_POS = pos_t( 2),
              RTCSEL_POS = pos_t( 8),
               RTCEN_POS = pos_t(15),
               BDRST_POS = pos_t(16);

        using              bits_t = Bits<uint32_t, Bdcr>;
        static constexpr   bits_t
        LSEON            = bits_t(1,        LSEON_POS),
        LSERDY           = bits_t(1,       LSERDY_POS),
        LSEBYP           = bits_t(1,       LSEBYP_POS),
        RTCEN            = bits_t(1,        RTCEN_POS);

        static const uint32_t
             RTCSEL_MASK =       0x3U;

        using              mskd_t = Mskd<uint32_t, Bdcr>;
        static constexpr   mskd_t

        RTCSEL_NOCLOCK   = mskd_t(     RTCSEL_MASK,  0b00,     RTCSEL_POS),
        RTCSEL_LSE       = mskd_t(     RTCSEL_MASK,  0b01,     RTCSEL_POS),
        RTCSEL_LSI       = mskd_t(     RTCSEL_MASK,  0b10,     RTCSEL_POS),
        RTCSEL_HSE       = mskd_t(     RTCSEL_MASK,  0b11,     RTCSEL_POS);
    };  // struct Bdcr
    using bdcr_t = Reg<uint32_t, Bdcr>;
          bdcr_t   bdcr;



    struct Csr {
        using              pos_t = Pos<uint32_t, Csr>;
        static constexpr   pos_t
               LSION_POS = pos_t( 0),
              LSIRDY_POS = pos_t( 1),
                RMVF_POS = pos_t(24),
             PINRSTF_POS = pos_t(26),
             PORRSTF_POS = pos_t(27),
             SFTRSTF_POS = pos_t(28),
            IWDGRSTF_POS = pos_t(29),
            WWDGRSTF_POS = pos_t(30),
            LPWRRSTF_POS = pos_t(31);

        using              bits_t = Bits<uint32_t, Csr>;
        static constexpr   bits_t
        LSION            = bits_t(1,        LSION_POS),
        LSIRDY           = bits_t(1,       LSIRDY_POS),
        RMVF             = bits_t(1,         RMVF_POS),
        PINRSTF          = bits_t(1,      PINRSTF_POS),
        PORRSTF          = bits_t(1,      PORRSTF_POS),
        SFTRSTF          = bits_t(1,      SFTRSTF_POS),
        IWDGRSTF         = bits_t(1,     IWDGRSTF_POS),
        WWDGRSTF         = bits_t(1,     WWDGRSTF_POS),
        LPWRRSTF         = bits_t(1,     LPWRRSTF_POS);
    };  // struct Csr
    using csr_t = Reg<uint32_t, Csr>;
          csr_t   csr;

};  // struct Rcc
static_assert(sizeof(Rcc) == 40, "sizeof(Rcc) != 40");



struct Gpio {
    struct Crl {
        using              pos_t = Pos<uint32_t, Crl>;
        static constexpr   pos_t
               MODE0_POS = pos_t( 0),
               MODE1_POS = pos_t( 4),
               MODE2_POS = pos_t( 8),
               MODE3_POS = pos_t(12),
               MODE4_POS = pos_t(16),
               MODE5_POS = pos_t(20),
               MODE6_POS = pos_t(24),
               MODE7_POS = pos_t(28),
                CNF0_POS = pos_t( 2),
                CNF1_POS = pos_t( 6),
                CNF2_POS = pos_t(10),
                CNF3_POS = pos_t(14),
                CNF4_POS = pos_t(18),
                CNF5_POS = pos_t(22),
                CNF6_POS = pos_t(26),
                CNF7_POS = pos_t(30);

        static const uint32_t       MASK               = 0b11,
                                    INPUT_ANALOG       = 0b00,
                                    INPUT_FLOATING     = 0b01,
                                    INPUT_PULL_UP_DOWN = 0b10,
                                    OUTPUT_PUSH_PULL   = 0b00,
                                    OUTPUT_OPEN_DRAIN  = 0b01,
                                    ALTFUNC_PUSH_PULL  = 0b10,
                                    ALTFUNC_OPEN_DRAIN = 0b11,
                                    INPUT              = 0b00,
                                    OUTPUT_10_MHZ      = 0b01,
                                    OUTPUT_2_MHZ       = 0b10,
                                    OUTPUT_50_MHZ      = 0b11;

        using              mskd_t = Mskd<uint32_t, Crl>;
        static constexpr   mskd_t
        CNF0_INPUT_ANALOG       = mskd_t(MASK, INPUT_ANALOG      , CNF0_POS),
        CNF0_INPUT_FLOATING     = mskd_t(MASK, INPUT_FLOATING    , CNF0_POS),
        CNF0_INPUT_PULL_UP_DOWN = mskd_t(MASK, INPUT_PULL_UP_DOWN, CNF0_POS),
        CNF0_OUTPUT_PUSH_PULL   = mskd_t(MASK, OUTPUT_PUSH_PULL  , CNF0_POS),
        CNF0_OUTPUT_OPEN_DRAIN  = mskd_t(MASK, OUTPUT_OPEN_DRAIN , CNF0_POS),
        CNF0_ALTFUNC_PUSH_PULL  = mskd_t(MASK, ALTFUNC_PUSH_PULL , CNF0_POS),
        CNF0_ALTFUNC_OPEN_DRAIN = mskd_t(MASK, ALTFUNC_OPEN_DRAIN, CNF0_POS),

        CNF1_INPUT_ANALOG       = mskd_t(MASK, INPUT_ANALOG      , CNF1_POS),
        CNF1_INPUT_FLOATING     = mskd_t(MASK, INPUT_FLOATING    , CNF1_POS),
        CNF1_INPUT_PULL_UP_DOWN = mskd_t(MASK, INPUT_PULL_UP_DOWN, CNF1_POS),
        CNF1_OUTPUT_PUSH_PULL   = mskd_t(MASK, OUTPUT_PUSH_PULL  , CNF1_POS),
        CNF1_OUTPUT_OPEN_DRAIN  = mskd_t(MASK, OUTPUT_OPEN_DRAIN , CNF1_POS),
        CNF1_ALTFUNC_PUSH_PULL  = mskd_t(MASK, ALTFUNC_PUSH_PULL , CNF1_POS),
        CNF1_ALTFUNC_OPEN_DRAIN = mskd_t(MASK, ALTFUNC_OPEN_DRAIN, CNF1_POS),

        CNF2_INPUT_ANALOG       = mskd_t(MASK, INPUT_ANALOG      , CNF2_POS),
        CNF2_INPUT_FLOATING     = mskd_t(MASK, INPUT_FLOATING    , CNF2_POS),
        CNF2_INPUT_PULL_UP_DOWN = mskd_t(MASK, INPUT_PULL_UP_DOWN, CNF2_POS),
        CNF2_OUTPUT_PUSH_PULL   = mskd_t(MASK, OUTPUT_PUSH_PULL  , CNF2_POS),
        CNF2_OUTPUT_OPEN_DRAIN  = mskd_t(MASK, OUTPUT_OPEN_DRAIN , CNF2_POS),
        CNF2_ALTFUNC_PUSH_PULL  = mskd_t(MASK, ALTFUNC_PUSH_PULL , CNF2_POS),
        CNF2_ALTFUNC_OPEN_DRAIN = mskd_t(MASK, ALTFUNC_OPEN_DRAIN, CNF2_POS),

        CNF3_INPUT_ANALOG       = mskd_t(MASK, INPUT_ANALOG      , CNF3_POS),
        CNF3_INPUT_FLOATING     = mskd_t(MASK, INPUT_FLOATING    , CNF3_POS),
        CNF3_INPUT_PULL_UP_DOWN = mskd_t(MASK, INPUT_PULL_UP_DOWN, CNF3_POS),
        CNF3_OUTPUT_PUSH_PULL   = mskd_t(MASK, OUTPUT_PUSH_PULL  , CNF3_POS),
        CNF3_OUTPUT_OPEN_DRAIN  = mskd_t(MASK, OUTPUT_OPEN_DRAIN , CNF3_POS),
        CNF3_ALTFUNC_PUSH_PULL  = mskd_t(MASK, ALTFUNC_PUSH_PULL , CNF3_POS),
        CNF3_ALTFUNC_OPEN_DRAIN = mskd_t(MASK, ALTFUNC_OPEN_DRAIN, CNF3_POS),

        CNF4_INPUT_ANALOG       = mskd_t(MASK, INPUT_ANALOG      , CNF4_POS),
        CNF4_INPUT_FLOATING     = mskd_t(MASK, INPUT_FLOATING    , CNF4_POS),
        CNF4_INPUT_PULL_UP_DOWN = mskd_t(MASK, INPUT_PULL_UP_DOWN, CNF4_POS),
        CNF4_OUTPUT_PUSH_PULL   = mskd_t(MASK, OUTPUT_PUSH_PULL  , CNF4_POS),
        CNF4_OUTPUT_OPEN_DRAIN  = mskd_t(MASK, OUTPUT_OPEN_DRAIN , CNF4_POS),
        CNF4_ALTFUNC_PUSH_PULL  = mskd_t(MASK, ALTFUNC_PUSH_PULL , CNF4_POS),
        CNF4_ALTFUNC_OPEN_DRAIN = mskd_t(MASK, ALTFUNC_OPEN_DRAIN, CNF4_POS),

        CNF5_INPUT_ANALOG       = mskd_t(MASK, INPUT_ANALOG      , CNF5_POS),
        CNF5_INPUT_FLOATING     = mskd_t(MASK, INPUT_FLOATING    , CNF5_POS),
        CNF5_INPUT_PULL_UP_DOWN = mskd_t(MASK, INPUT_PULL_UP_DOWN, CNF5_POS),
        CNF5_OUTPUT_PUSH_PULL   = mskd_t(MASK, OUTPUT_PUSH_PULL  , CNF5_POS),
        CNF5_OUTPUT_OPEN_DRAIN  = mskd_t(MASK, OUTPUT_OPEN_DRAIN , CNF5_POS),
        CNF5_ALTFUNC_PUSH_PULL  = mskd_t(MASK, ALTFUNC_PUSH_PULL , CNF5_POS),
        CNF5_ALTFUNC_OPEN_DRAIN = mskd_t(MASK, ALTFUNC_OPEN_DRAIN, CNF5_POS),

        CNF6_INPUT_ANALOG       = mskd_t(MASK, INPUT_ANALOG      , CNF6_POS),
        CNF6_INPUT_FLOATING     = mskd_t(MASK, INPUT_FLOATING    , CNF6_POS),
        CNF6_INPUT_PULL_UP_DOWN = mskd_t(MASK, INPUT_PULL_UP_DOWN, CNF6_POS),
        CNF6_OUTPUT_PUSH_PULL   = mskd_t(MASK, OUTPUT_PUSH_PULL  , CNF6_POS),
        CNF6_OUTPUT_OPEN_DRAIN  = mskd_t(MASK, OUTPUT_OPEN_DRAIN , CNF6_POS),
        CNF6_ALTFUNC_PUSH_PULL  = mskd_t(MASK, ALTFUNC_PUSH_PULL , CNF6_POS),
        CNF6_ALTFUNC_OPEN_DRAIN = mskd_t(MASK, ALTFUNC_OPEN_DRAIN, CNF6_POS),

        CNF7_INPUT_ANALOG       = mskd_t(MASK, INPUT_ANALOG      , CNF7_POS),
        CNF7_INPUT_FLOATING     = mskd_t(MASK, INPUT_FLOATING    , CNF7_POS),
        CNF7_INPUT_PULL_UP_DOWN = mskd_t(MASK, INPUT_PULL_UP_DOWN, CNF7_POS),
        CNF7_OUTPUT_PUSH_PULL   = mskd_t(MASK, OUTPUT_PUSH_PULL  , CNF7_POS),
        CNF7_OUTPUT_OPEN_DRAIN  = mskd_t(MASK, OUTPUT_OPEN_DRAIN , CNF7_POS),
        CNF7_ALTFUNC_PUSH_PULL  = mskd_t(MASK, ALTFUNC_PUSH_PULL , CNF7_POS),
        CNF7_ALTFUNC_OPEN_DRAIN = mskd_t(MASK, ALTFUNC_OPEN_DRAIN, CNF7_POS),

        MODE0_INPUT             = mskd_t(MASK, INPUT             , MODE0_POS),
        MODE0_OUTPUT_10_MHZ     = mskd_t(MASK, OUTPUT_10_MHZ     , MODE0_POS),
        MODE0_OUTPUT_2_MHZ      = mskd_t(MASK, OUTPUT_2_MHZ      , MODE0_POS),
        MODE0_OUTPUT_50_MHZ     = mskd_t(MASK, OUTPUT_50_MHZ     , MODE0_POS),

        MODE1_INPUT             = mskd_t(MASK, INPUT             , MODE1_POS),
        MODE1_OUTPUT_10_MHZ     = mskd_t(MASK, OUTPUT_10_MHZ     , MODE1_POS),
        MODE1_OUTPUT_2_MHZ      = mskd_t(MASK, OUTPUT_2_MHZ      , MODE1_POS),
        MODE1_OUTPUT_50_MHZ     = mskd_t(MASK, OUTPUT_50_MHZ     , MODE1_POS),

        MODE2_INPUT             = mskd_t(MASK, INPUT             , MODE2_POS),
        MODE2_OUTPUT_10_MHZ     = mskd_t(MASK, OUTPUT_10_MHZ     , MODE2_POS),
        MODE2_OUTPUT_2_MHZ      = mskd_t(MASK, OUTPUT_2_MHZ      , MODE2_POS),
        MODE2_OUTPUT_50_MHZ     = mskd_t(MASK, OUTPUT_50_MHZ     , MODE2_POS),

        MODE3_INPUT             = mskd_t(MASK, INPUT             , MODE3_POS),
        MODE3_OUTPUT_10_MHZ     = mskd_t(MASK, OUTPUT_10_MHZ     , MODE3_POS),
        MODE3_OUTPUT_2_MHZ      = mskd_t(MASK, OUTPUT_2_MHZ      , MODE3_POS),
        MODE3_OUTPUT_50_MHZ     = mskd_t(MASK, OUTPUT_50_MHZ     , MODE3_POS),

        MODE4_INPUT             = mskd_t(MASK, INPUT             , MODE4_POS),
        MODE4_OUTPUT_10_MHZ     = mskd_t(MASK, OUTPUT_10_MHZ     , MODE4_POS),
        MODE4_OUTPUT_2_MHZ      = mskd_t(MASK, OUTPUT_2_MHZ      , MODE4_POS),
        MODE4_OUTPUT_50_MHZ     = mskd_t(MASK, OUTPUT_50_MHZ     , MODE4_POS),

        MODE5_INPUT             = mskd_t(MASK, INPUT             , MODE5_POS),
        MODE5_OUTPUT_10_MHZ     = mskd_t(MASK, OUTPUT_10_MHZ     , MODE5_POS),
        MODE5_OUTPUT_2_MHZ      = mskd_t(MASK, OUTPUT_2_MHZ      , MODE5_POS),
        MODE5_OUTPUT_50_MHZ     = mskd_t(MASK, OUTPUT_50_MHZ     , MODE5_POS),

        MODE6_INPUT             = mskd_t(MASK, INPUT             , MODE6_POS),
        MODE6_OUTPUT_10_MHZ     = mskd_t(MASK, OUTPUT_10_MHZ     , MODE6_POS),
        MODE6_OUTPUT_2_MHZ      = mskd_t(MASK, OUTPUT_2_MHZ      , MODE6_POS),
        MODE6_OUTPUT_50_MHZ     = mskd_t(MASK, OUTPUT_50_MHZ     , MODE6_POS),

        MODE7_INPUT             = mskd_t(MASK, INPUT             , MODE7_POS),
        MODE7_OUTPUT_10_MHZ     = mskd_t(MASK, OUTPUT_10_MHZ     , MODE7_POS),
        MODE7_OUTPUT_2_MHZ      = mskd_t(MASK, OUTPUT_2_MHZ      , MODE7_POS),
        MODE7_OUTPUT_50_MHZ     = mskd_t(MASK, OUTPUT_50_MHZ     , MODE7_POS);
    };  // struct Crl
    using crl_t = Reg<uint32_t, Crl>;
          crl_t   crl;


    struct Crh {
        using              pos_t = Pos<uint32_t, Crh>;
        static constexpr   pos_t
                MODE_POS = pos_t( 0),
               MODE8_POS = pos_t( 0),
               MODE9_POS = pos_t( 4),
              MODE10_POS = pos_t( 8),
              MODE11_POS = pos_t(12),
              MODE12_POS = pos_t(16),
              MODE13_POS = pos_t(20),
              MODE14_POS = pos_t(24),
              MODE15_POS = pos_t(28),
                 CNF_POS = pos_t( 2),
                CNF8_POS = pos_t( 2),
                CNF9_POS = pos_t( 6),
               CNF10_POS = pos_t(10),
               CNF11_POS = pos_t(14),
               CNF12_POS = pos_t(18),
               CNF13_POS = pos_t(22),
               CNF14_POS = pos_t(26),
               CNF15_POS = pos_t(30);

        static const uint32_t       MASK               = 0b11,
                                    INPUT_ANALOG       = 0b00,
                                    INPUT_FLOATING     = 0b01,
                                    INPUT_PULL_UP_DOWN = 0b10,
                                    OUTPUT_PUSH_PULL   = 0b00,
                                    OUTPUT_OPEN_DRAIN  = 0b01,
                                    ALTFUNC_PUSH_PULL  = 0b10,
                                    ALTFUNC_OPEN_DRAIN = 0b11,
                                    INPUT              = 0b00,
                                    OUTPUT_10_MHZ      = 0b01,
                                    OUTPUT_2_MHZ       = 0b10,
                                    OUTPUT_50_MHZ      = 0b11;

        using              mskd_t = Mskd<uint32_t, Crh>;
        static constexpr   mskd_t
        CNF8_INPUT_ANALOG        = mskd_t(MASK, INPUT_ANALOG      , CNF8_POS),
        CNF8_INPUT_FLOATING      = mskd_t(MASK, INPUT_FLOATING    , CNF8_POS),
        CNF8_INPUT_PULL_UP_DOWN  = mskd_t(MASK, INPUT_PULL_UP_DOWN, CNF8_POS),
        CNF8_OUTPUT_PUSH_PULL    = mskd_t(MASK, OUTPUT_PUSH_PULL  , CNF8_POS),
        CNF8_OUTPUT_OPEN_DRAIN   = mskd_t(MASK, OUTPUT_OPEN_DRAIN , CNF8_POS),
        CNF8_ALTFUNC_PUSH_PULL   = mskd_t(MASK, ALTFUNC_PUSH_PULL , CNF8_POS),
        CNF8_ALTFUNC_OPEN_DRAIN  = mskd_t(MASK, ALTFUNC_OPEN_DRAIN, CNF8_POS),

        CNF9_INPUT_ANALOG        = mskd_t(MASK, INPUT_ANALOG      , CNF9_POS),
        CNF9_INPUT_FLOATING      = mskd_t(MASK, INPUT_FLOATING    , CNF9_POS),
        CNF9_INPUT_PULL_UP_DOWN  = mskd_t(MASK, INPUT_PULL_UP_DOWN, CNF9_POS),
        CNF9_OUTPUT_PUSH_PULL    = mskd_t(MASK, OUTPUT_PUSH_PULL  , CNF9_POS),
        CNF9_OUTPUT_OPEN_DRAIN   = mskd_t(MASK, OUTPUT_OPEN_DRAIN , CNF9_POS),
        CNF9_ALTFUNC_PUSH_PULL   = mskd_t(MASK, ALTFUNC_PUSH_PULL , CNF9_POS),
        CNF9_ALTFUNC_OPEN_DRAIN  = mskd_t(MASK, ALTFUNC_OPEN_DRAIN, CNF9_POS),

        CNF10_INPUT_ANALOG       = mskd_t(MASK, INPUT_ANALOG      , CNF10_POS),
        CNF10_INPUT_FLOATING     = mskd_t(MASK, INPUT_FLOATING    , CNF10_POS),
        CNF10_INPUT_PULL_UP_DOWN = mskd_t(MASK, INPUT_PULL_UP_DOWN, CNF10_POS),
        CNF10_OUTPUT_PUSH_PULL   = mskd_t(MASK, OUTPUT_PUSH_PULL  , CNF10_POS),
        CNF10_OUTPUT_OPEN_DRAIN  = mskd_t(MASK, OUTPUT_OPEN_DRAIN , CNF10_POS),
        CNF10_ALTFUNC_PUSH_PULL  = mskd_t(MASK, ALTFUNC_PUSH_PULL , CNF10_POS),
        CNF10_ALTFUNC_OPEN_DRAIN = mskd_t(MASK, ALTFUNC_OPEN_DRAIN, CNF10_POS),

        CNF11_INPUT_ANALOG       = mskd_t(MASK, INPUT_ANALOG      , CNF11_POS),
        CNF11_INPUT_FLOATING     = mskd_t(MASK, INPUT_FLOATING    , CNF11_POS),
        CNF11_INPUT_PULL_UP_DOWN = mskd_t(MASK, INPUT_PULL_UP_DOWN, CNF11_POS),
        CNF11_OUTPUT_PUSH_PULL   = mskd_t(MASK, OUTPUT_PUSH_PULL  , CNF11_POS),
        CNF11_OUTPUT_OPEN_DRAIN  = mskd_t(MASK, OUTPUT_OPEN_DRAIN , CNF11_POS),
        CNF11_ALTFUNC_PUSH_PULL  = mskd_t(MASK, ALTFUNC_PUSH_PULL , CNF11_POS),
        CNF11_ALTFUNC_OPEN_DRAIN = mskd_t(MASK, ALTFUNC_OPEN_DRAIN, CNF11_POS),

        CNF12_INPUT_ANALOG       = mskd_t(MASK, INPUT_ANALOG      , CNF12_POS),
        CNF12_INPUT_FLOATING     = mskd_t(MASK, INPUT_FLOATING    , CNF12_POS),
        CNF12_INPUT_PULL_UP_DOWN = mskd_t(MASK, INPUT_PULL_UP_DOWN, CNF12_POS),
        CNF12_OUTPUT_PUSH_PULL   = mskd_t(MASK, OUTPUT_PUSH_PULL  , CNF12_POS),
        CNF12_OUTPUT_OPEN_DRAIN  = mskd_t(MASK, OUTPUT_OPEN_DRAIN , CNF12_POS),
        CNF12_ALTFUNC_PUSH_PULL  = mskd_t(MASK, ALTFUNC_PUSH_PULL , CNF12_POS),
        CNF12_ALTFUNC_OPEN_DRAIN = mskd_t(MASK, ALTFUNC_OPEN_DRAIN, CNF12_POS),

        CNF13_INPUT_ANALOG       = mskd_t(MASK, INPUT_ANALOG      , CNF13_POS),
        CNF13_INPUT_FLOATING     = mskd_t(MASK, INPUT_FLOATING    , CNF13_POS),
        CNF13_INPUT_PULL_UP_DOWN = mskd_t(MASK, INPUT_PULL_UP_DOWN, CNF13_POS),
        CNF13_OUTPUT_PUSH_PULL   = mskd_t(MASK, OUTPUT_PUSH_PULL  , CNF13_POS),
        CNF13_OUTPUT_OPEN_DRAIN  = mskd_t(MASK, OUTPUT_OPEN_DRAIN , CNF13_POS),
        CNF13_ALTFUNC_PUSH_PULL  = mskd_t(MASK, ALTFUNC_PUSH_PULL , CNF13_POS),
        CNF13_ALTFUNC_OPEN_DRAIN = mskd_t(MASK, ALTFUNC_OPEN_DRAIN, CNF13_POS),

        CNF14_INPUT_ANALOG       = mskd_t(MASK, INPUT_ANALOG      , CNF14_POS),
        CNF14_INPUT_FLOATING     = mskd_t(MASK, INPUT_FLOATING    , CNF14_POS),
        CNF14_INPUT_PULL_UP_DOWN = mskd_t(MASK, INPUT_PULL_UP_DOWN, CNF14_POS),
        CNF14_OUTPUT_PUSH_PULL   = mskd_t(MASK, OUTPUT_PUSH_PULL  , CNF14_POS),
        CNF14_OUTPUT_OPEN_DRAIN  = mskd_t(MASK, OUTPUT_OPEN_DRAIN , CNF14_POS),
        CNF14_ALTFUNC_PUSH_PULL  = mskd_t(MASK, ALTFUNC_PUSH_PULL , CNF14_POS),
        CNF14_ALTFUNC_OPEN_DRAIN = mskd_t(MASK, ALTFUNC_OPEN_DRAIN, CNF14_POS),

        CNF15_INPUT_ANALOG       = mskd_t(MASK, INPUT_ANALOG      , CNF15_POS),
        CNF15_INPUT_FLOATING     = mskd_t(MASK, INPUT_FLOATING    , CNF15_POS),
        CNF15_INPUT_PULL_UP_DOWN = mskd_t(MASK, INPUT_PULL_UP_DOWN, CNF15_POS),
        CNF15_OUTPUT_PUSH_PULL   = mskd_t(MASK, OUTPUT_PUSH_PULL  , CNF15_POS),
        CNF15_OUTPUT_OPEN_DRAIN  = mskd_t(MASK, OUTPUT_OPEN_DRAIN , CNF15_POS),
        CNF15_ALTFUNC_PUSH_PULL  = mskd_t(MASK, ALTFUNC_PUSH_PULL , CNF15_POS),
        CNF15_ALTFUNC_OPEN_DRAIN = mskd_t(MASK, ALTFUNC_OPEN_DRAIN, CNF15_POS),

        MODE8_INPUT              = mskd_t(MASK, INPUT             , MODE8_POS),
        MODE8_OUTPUT_10_MHZ      = mskd_t(MASK, OUTPUT_10_MHZ     , MODE8_POS),
        MODE8_OUTPUT_2_MHZ       = mskd_t(MASK, OUTPUT_2_MHZ      , MODE8_POS),
        MODE8_OUTPUT_50_MHZ      = mskd_t(MASK, OUTPUT_50_MHZ     , MODE8_POS),

        MODE9_INPUT              = mskd_t(MASK, INPUT             , MODE9_POS),
        MODE9_OUTPUT_10_MHZ      = mskd_t(MASK, OUTPUT_10_MHZ     , MODE9_POS),
        MODE9_OUTPUT_2_MHZ       = mskd_t(MASK, OUTPUT_2_MHZ      , MODE9_POS),
        MODE9_OUTPUT_50_MHZ      = mskd_t(MASK, OUTPUT_50_MHZ     , MODE9_POS),

        MODE10_INPUT             = mskd_t(MASK, INPUT             , MODE10_POS),
        MODE10_OUTPUT_10_MHZ     = mskd_t(MASK, OUTPUT_10_MHZ     , MODE10_POS),
        MODE10_OUTPUT_2_MHZ      = mskd_t(MASK, OUTPUT_2_MHZ      , MODE10_POS),
        MODE10_OUTPUT_50_MHZ     = mskd_t(MASK, OUTPUT_50_MHZ     , MODE10_POS),

        MODE11_INPUT             = mskd_t(MASK, INPUT             , MODE11_POS),
        MODE11_OUTPUT_10_MHZ     = mskd_t(MASK, OUTPUT_10_MHZ     , MODE11_POS),
        MODE11_OUTPUT_2_MHZ      = mskd_t(MASK, OUTPUT_2_MHZ      , MODE11_POS),
        MODE11_OUTPUT_50_MHZ     = mskd_t(MASK, OUTPUT_50_MHZ     , MODE11_POS),

        MODE12_INPUT             = mskd_t(MASK, INPUT             , MODE12_POS),
        MODE12_OUTPUT_10_MHZ     = mskd_t(MASK, OUTPUT_10_MHZ     , MODE12_POS),
        MODE12_OUTPUT_2_MHZ      = mskd_t(MASK, OUTPUT_2_MHZ      , MODE12_POS),
        MODE12_OUTPUT_50_MHZ     = mskd_t(MASK, OUTPUT_50_MHZ     , MODE12_POS),

        MODE13_INPUT             = mskd_t(MASK, INPUT             , MODE13_POS),
        MODE13_OUTPUT_10_MHZ     = mskd_t(MASK, OUTPUT_10_MHZ     , MODE13_POS),
        MODE13_OUTPUT_2_MHZ      = mskd_t(MASK, OUTPUT_2_MHZ      , MODE13_POS),
        MODE13_OUTPUT_50_MHZ     = mskd_t(MASK, OUTPUT_50_MHZ     , MODE13_POS),

        MODE14_INPUT             = mskd_t(MASK, INPUT             , MODE14_POS),
        MODE14_OUTPUT_10_MHZ     = mskd_t(MASK, OUTPUT_10_MHZ     , MODE14_POS),
        MODE14_OUTPUT_2_MHZ      = mskd_t(MASK, OUTPUT_2_MHZ      , MODE14_POS),
        MODE14_OUTPUT_50_MHZ     = mskd_t(MASK, OUTPUT_50_MHZ     , MODE14_POS),

        MODE15_INPUT             = mskd_t(MASK, INPUT             , MODE15_POS),
        MODE15_OUTPUT_10_MHZ     = mskd_t(MASK, OUTPUT_10_MHZ     , MODE15_POS),
        MODE15_OUTPUT_2_MHZ      = mskd_t(MASK, OUTPUT_2_MHZ      , MODE15_POS),
        MODE15_OUTPUT_50_MHZ     = mskd_t(MASK, OUTPUT_50_MHZ     , MODE15_POS);
    };  // struct Crh
    using crh_t = Reg<uint32_t, Crh>;
          crh_t   crh;


    struct Idr {
        using              pos_t = Pos<uint32_t, Idr>;
        static constexpr   pos_t
                IDR0_POS = pos_t( 0),
                IDR1_POS = pos_t( 1),
                IDR2_POS = pos_t( 2),
                IDR3_POS = pos_t( 3),
                IDR4_POS = pos_t( 4),
                IDR5_POS = pos_t( 5),
                IDR6_POS = pos_t( 6),
                IDR7_POS = pos_t( 7),
                IDR8_POS = pos_t( 8),
                IDR9_POS = pos_t( 9),
               IDR10_POS = pos_t(10),
               IDR11_POS = pos_t(11),
               IDR12_POS = pos_t(12),
               IDR13_POS = pos_t(13),
               IDR14_POS = pos_t(14),
               IDR15_POS = pos_t(15);

        using              bits_t = Bits<uint32_t, Idr>;
        static constexpr   bits_t
        IDR0             = bits_t(1,         IDR0_POS),
        IDR1             = bits_t(1,         IDR1_POS),
        IDR2             = bits_t(1,         IDR2_POS),
        IDR3             = bits_t(1,         IDR3_POS),
        IDR4             = bits_t(1,         IDR4_POS),
        IDR5             = bits_t(1,         IDR5_POS),
        IDR6             = bits_t(1,         IDR6_POS),
        IDR7             = bits_t(1,         IDR7_POS),
        IDR8             = bits_t(1,         IDR8_POS),
        IDR9             = bits_t(1,         IDR9_POS),
        IDR10            = bits_t(1,        IDR10_POS),
        IDR11            = bits_t(1,        IDR11_POS),
        IDR12            = bits_t(1,        IDR12_POS),
        IDR13            = bits_t(1,        IDR13_POS),
        IDR14            = bits_t(1,        IDR14_POS),
        IDR15            = bits_t(1,        IDR15_POS);
    };  // struct Idr
    using idr_t = Reg<uint32_t, Idr>;
          idr_t   idr;


    struct Odr {
        using              pos_t = Pos<uint32_t, Odr>;
        static constexpr   pos_t
                ODR0_POS = pos_t( 0),
                ODR1_POS = pos_t( 1),
                ODR2_POS = pos_t( 2),
                ODR3_POS = pos_t( 3),
                ODR4_POS = pos_t( 4),
                ODR5_POS = pos_t( 5),
                ODR6_POS = pos_t( 6),
                ODR7_POS = pos_t( 7),
                ODR8_POS = pos_t( 8),
                ODR9_POS = pos_t( 9),
               ODR10_POS = pos_t(10),
               ODR11_POS = pos_t(11),
               ODR12_POS = pos_t(12),
               ODR13_POS = pos_t(13),
               ODR14_POS = pos_t(14),
               ODR15_POS = pos_t(15);

        using              bits_t = Bits<uint32_t, Odr>;
        static constexpr   bits_t
        ODR0             = bits_t(1,         ODR0_POS),
        ODR1             = bits_t(1,         ODR1_POS),
        ODR2             = bits_t(1,         ODR2_POS),
        ODR3             = bits_t(1,         ODR3_POS),
        ODR4             = bits_t(1,         ODR4_POS),
        ODR5             = bits_t(1,         ODR5_POS),
        ODR6             = bits_t(1,         ODR6_POS),
        ODR7             = bits_t(1,         ODR7_POS),
        ODR8             = bits_t(1,         ODR8_POS),
        ODR9             = bits_t(1,         ODR9_POS),
        ODR10            = bits_t(1,        ODR10_POS),
        ODR11            = bits_t(1,        ODR11_POS),
        ODR12            = bits_t(1,        ODR12_POS),
        ODR13            = bits_t(1,        ODR13_POS),
        ODR14            = bits_t(1,        ODR14_POS),
        ODR15            = bits_t(1,        ODR15_POS);
    };  // struct Odr
    using odr_t = Reg<uint32_t, Odr>;
          odr_t   odr;


    struct Bsrr {
        using              pos_t = Pos<uint32_t, Bsrr>;
        static constexpr   pos_t
                 BS0_POS = pos_t( 0),
                 BS1_POS = pos_t( 1),
                 BS2_POS = pos_t( 2),
                 BS3_POS = pos_t( 3),
                 BS4_POS = pos_t( 4),
                 BS5_POS = pos_t( 5),
                 BS6_POS = pos_t( 6),
                 BS7_POS = pos_t( 7),
                 BS8_POS = pos_t( 8),
                 BS9_POS = pos_t( 9),
                BS10_POS = pos_t(10),
                BS11_POS = pos_t(11),
                BS12_POS = pos_t(12),
                BS13_POS = pos_t(13),
                BS14_POS = pos_t(14),
                BS15_POS = pos_t(15),
                 BR0_POS = pos_t(16),
                 BR1_POS = pos_t(17),
                 BR2_POS = pos_t(18),
                 BR3_POS = pos_t(19),
                 BR4_POS = pos_t(20),
                 BR5_POS = pos_t(21),
                 BR6_POS = pos_t(22),
                 BR7_POS = pos_t(23),
                 BR8_POS = pos_t(24),
                 BR9_POS = pos_t(25),
                BR10_POS = pos_t(26),
                BR11_POS = pos_t(27),
                BR12_POS = pos_t(28),
                BR13_POS = pos_t(29),
                BR14_POS = pos_t(30),
                BR15_POS = pos_t(31);

        using              bits_t = Bits<uint32_t, Bsrr>;
        static constexpr   bits_t
        BS0              = bits_t(1,          BS0_POS),
        BS1              = bits_t(1,          BS1_POS),
        BS2              = bits_t(1,          BS2_POS),
        BS3              = bits_t(1,          BS3_POS),
        BS4              = bits_t(1,          BS4_POS),
        BS5              = bits_t(1,          BS5_POS),
        BS6              = bits_t(1,          BS6_POS),
        BS7              = bits_t(1,          BS7_POS),
        BS8              = bits_t(1,          BS8_POS),
        BS9              = bits_t(1,          BS9_POS),
        BS10             = bits_t(1,         BS10_POS),
        BS11             = bits_t(1,         BS11_POS),
        BS12             = bits_t(1,         BS12_POS),
        BS13             = bits_t(1,         BS13_POS),
        BS14             = bits_t(1,         BS14_POS),
        BS15             = bits_t(1,         BS15_POS),
        BR0              = bits_t(1,          BR0_POS),
        BR1              = bits_t(1,          BR1_POS),
        BR2              = bits_t(1,          BR2_POS),
        BR3              = bits_t(1,          BR3_POS),
        BR4              = bits_t(1,          BR4_POS),
        BR5              = bits_t(1,          BR5_POS),
        BR6              = bits_t(1,          BR6_POS),
        BR7              = bits_t(1,          BR7_POS),
        BR8              = bits_t(1,          BR8_POS),
        BR9              = bits_t(1,          BR9_POS),
        BR10             = bits_t(1,         BR10_POS),
        BR11             = bits_t(1,         BR11_POS),
        BR12             = bits_t(1,         BR12_POS),
        BR13             = bits_t(1,         BR13_POS),
        BR14             = bits_t(1,         BR14_POS),
        BR15             = bits_t(1,         BR15_POS);
    };  // struct Bsrr
    using bsrr_t = Reg<uint32_t, Bsrr>;
          bsrr_t   bsrr;


    struct Brr {
        using              pos_t = Pos<uint32_t, Brr>;
        static constexpr   pos_t
                 BR0_POS = pos_t( 0),
                 BR1_POS = pos_t( 1),
                 BR2_POS = pos_t( 2),
                 BR3_POS = pos_t( 3),
                 BR4_POS = pos_t( 4),
                 BR5_POS = pos_t( 5),
                 BR6_POS = pos_t( 6),
                 BR7_POS = pos_t( 7),
                 BR8_POS = pos_t( 8),
                 BR9_POS = pos_t( 9),
                BR10_POS = pos_t(10),
                BR11_POS = pos_t(11),
                BR12_POS = pos_t(12),
                BR13_POS = pos_t(13),
                BR14_POS = pos_t(14),
                BR15_POS = pos_t(15);

        using              bits_t = Bits<uint32_t, Brr>;
        static constexpr   bits_t
        BR0              = bits_t(1,          BR0_POS),
        BR1              = bits_t(1,          BR1_POS),
        BR2              = bits_t(1,          BR2_POS),
        BR3              = bits_t(1,          BR3_POS),
        BR4              = bits_t(1,          BR4_POS),
        BR5              = bits_t(1,          BR5_POS),
        BR6              = bits_t(1,          BR6_POS),
        BR7              = bits_t(1,          BR7_POS),
        BR8              = bits_t(1,          BR8_POS),
        BR9              = bits_t(1,          BR9_POS),
        BR10             = bits_t(1,         BR10_POS),
        BR11             = bits_t(1,         BR11_POS),
        BR12             = bits_t(1,         BR12_POS),
        BR13             = bits_t(1,         BR13_POS),
        BR14             = bits_t(1,         BR14_POS),
        BR15             = bits_t(1,         BR15_POS);
    };  // struct Brr
    using brr_t = Reg<uint32_t, Brr>;
          brr_t   brr;



    struct Lckr {
        using              pos_t = Pos<uint32_t, Lckr>;
        static constexpr   pos_t
                LCK0_POS = pos_t( 0),
                LCK1_POS = pos_t( 1),
                LCK2_POS = pos_t( 2),
                LCK3_POS = pos_t( 3),
                LCK4_POS = pos_t( 4),
                LCK5_POS = pos_t( 5),
                LCK6_POS = pos_t( 6),
                LCK7_POS = pos_t( 7),
                LCK8_POS = pos_t( 8),
                LCK9_POS = pos_t( 9),
               LCK10_POS = pos_t(10),
               LCK11_POS = pos_t(11),
               LCK12_POS = pos_t(12),
               LCK13_POS = pos_t(13),
               LCK14_POS = pos_t(14),
               LCK15_POS = pos_t(15),
                LCKK_POS = pos_t(16);

        using              bits_t = Bits<uint32_t, Lckr>;
        static constexpr   bits_t
        LCK0             = bits_t(1,         LCK0_POS),
        LCK1             = bits_t(1,         LCK1_POS),
        LCK2             = bits_t(1,         LCK2_POS),
        LCK3             = bits_t(1,         LCK3_POS),
        LCK4             = bits_t(1,         LCK4_POS),
        LCK5             = bits_t(1,         LCK5_POS),
        LCK6             = bits_t(1,         LCK6_POS),
        LCK7             = bits_t(1,         LCK7_POS),
        LCK8             = bits_t(1,         LCK8_POS),
        LCK9             = bits_t(1,         LCK9_POS),
        LCK10            = bits_t(1,        LCK10_POS),
        LCK11            = bits_t(1,        LCK11_POS),
        LCK12            = bits_t(1,        LCK12_POS),
        LCK13            = bits_t(1,        LCK13_POS),
        LCK14            = bits_t(1,        LCK14_POS),
        LCK15            = bits_t(1,        LCK15_POS),
        LCKK             = bits_t(1,         LCKK_POS);
    };  // struct Lckr
    using lckr_t = Reg<uint32_t, Lckr>;
          lckr_t   lckr;

};  // struct Gpio
static_assert(sizeof(Gpio) == 28, "sizeof(Gpio) != 28");



struct Afio {
    struct Evcr {
        using              pos_t = Pos<uint32_t, Evcr>;
        static constexpr   pos_t
                 PIN_POS = pos_t( 0),
                PORT_POS = pos_t( 4),
                EVOE_POS = pos_t( 7);

        using              bits_t = Bits<uint32_t, Evcr>;
        static constexpr   bits_t
        EVOE             = bits_t(1,         EVOE_POS);

        static const uint32_t
                PIN_MASK =       0xFU,
               PORT_MASK =       0x7U,
            PORT_PD_MASK =       0x3U;

        using              mskd_t = Mskd<uint32_t, Evcr>;
        using              shft_t = Shft<uint32_t, Evcr>;

        static constexpr   mskd_t
        PORT_PA          = mskd_t(PORT_MASK,     0b000,         PORT_POS),
        PORT_PB          = mskd_t(PORT_MASK,     0b001,         PORT_POS),
        PORT_PC          = mskd_t(PORT_MASK,     0b010,         PORT_POS),
        PORT_PD          = mskd_t(PORT_MASK,     0b011,         PORT_POS),
        PORT_PE          = mskd_t(PORT_MASK,     0b100,         PORT_POS);

        REGBITS_MSKD_RANGE("Afio::Evcr",
                           PIN,
                           pin,
                           PIN_MASK,
                           PIN_POS,
                           PIN_MASK);
    };  // struct Evcr
    using evcr_t = Reg<uint32_t, Evcr>;
          evcr_t   evcr;


    struct Mapr {
        using              pos_t = Pos<uint32_t, Mapr>;
        static constexpr   pos_t
          SPI1_REMAP_POS = pos_t( 0),
          I2C1_REMAP_POS = pos_t( 1),
        USART1_REMAP_POS = pos_t( 2),
        USART2_REMAP_POS = pos_t( 3),
        USART3_REMAP_POS = pos_t( 4),
          TIM1_REMAP_POS = pos_t( 6),
          TIM2_REMAP_POS = pos_t( 8),
          TIM3_REMAP_POS = pos_t(10),
          TIM4_REMAP_POS = pos_t(12),
           CAN_REMAP_POS = pos_t(13),
          PD01_REMAP_POS = pos_t(15),
             SWJ_CFG_POS = pos_t(24);

        using              bits_t = Bits<uint32_t, Mapr>;
        static constexpr   bits_t
        SPI1_REMAP       = bits_t(1,   SPI1_REMAP_POS),
        I2C1_REMAP       = bits_t(1,   I2C1_REMAP_POS),
        USART1_REMAP     = bits_t(1, USART1_REMAP_POS),
        USART2_REMAP     = bits_t(1, USART2_REMAP_POS),
        TIM4_REMAP       = bits_t(1,   TIM4_REMAP_POS);

        static const uint32_t
        USART3_REMAP_MASK =       0x3U,
          TIM1_REMAP_MASK =      0x3U,
          TIM2_REMAP_MASK =      0x3U,
          TIM3_REMAP_MASK =      0x3U,
           CAN_REMAP_MASK =      0x3U,
             SWJ_CFG_MASK =      0x7U;

        using              mskd_t = Mskd<uint32_t, Mapr>;
        static constexpr   mskd_t
        USART3_REMAP_NONE  = mskd_t(USART3_REMAP_MASK,   0b00, USART3_REMAP_POS),
        USART3_REMAP_PB    = mskd_t(USART3_REMAP_MASK,   0b00, USART3_REMAP_POS),
        USART3_REMAP_PC    = mskd_t(USART3_REMAP_MASK,   0b01, USART3_REMAP_POS),
        USART3_REMAP_PD    = mskd_t(USART3_REMAP_MASK,   0b11, USART3_REMAP_POS),

        TIM1_REMAP_NONE    = mskd_t(  TIM1_REMAP_MASK,   0b00,   TIM1_REMAP_POS),
        TIM1_REMAP_PARTIAL = mskd_t(  TIM1_REMAP_MASK,   0b01,   TIM1_REMAP_POS),
        TIM1_REMAP_PE      = mskd_t(  TIM1_REMAP_MASK,   0b11,   TIM1_REMAP_POS),

        TIM2_REMAP_NONE    = mskd_t(  TIM2_REMAP_MASK,   0b00,   TIM2_REMAP_POS),
        TIM2_REMAP_PARTL_1 = mskd_t(  TIM2_REMAP_MASK,   0b01,   TIM2_REMAP_POS),
        TIM2_REMAP_PARTL_2 = mskd_t(  TIM2_REMAP_MASK,   0b10,   TIM2_REMAP_POS),
        TIM2_REMAP_FULL    = mskd_t(  TIM2_REMAP_MASK,   0b11,   TIM2_REMAP_POS),

        TIM3_REMAP_NONE    = mskd_t(  TIM3_REMAP_MASK,   0b00,   TIM3_REMAP_POS),
        TIM3_REMAP_PARTIAL = mskd_t(  TIM3_REMAP_MASK,   0b10,   TIM3_REMAP_POS),

        CAN_REMAP_PA       = mskd_t(   CAN_REMAP_MASK,   0b00,    CAN_REMAP_POS),
        CAN_REMAP_PB       = mskd_t(   CAN_REMAP_MASK,   0b10,    CAN_REMAP_POS),
        CAN_REMAP_PD       = mskd_t(   CAN_REMAP_MASK,   0b11,    CAN_REMAP_POS),

        SWJ_CFG_FULL_SWJ   = mskd_t(     SWJ_CFG_MASK,   0b000,     SWJ_CFG_POS),
        SWJ_CFG_NO_NJTRST  = mskd_t(     SWJ_CFG_MASK,   0b001,     SWJ_CFG_POS),
        SWJ_CFG_SW_DP      = mskd_t(     SWJ_CFG_MASK,   0b010,     SWJ_CFG_POS),
        SWJ_CFG_DISABLED   = mskd_t(     SWJ_CFG_MASK,   0b100,     SWJ_CFG_POS);
    };  // struct Mapr
    using mapr_t = Reg<uint32_t, Mapr>;
          mapr_t   mapr;


    static const uint32_t   EXTI_GPIOA  = 0x0,
                            EXTI_GPIOB  = 0x1,
                            EXTI_GPIOC  = 0x2,
                            EXTI_GPIOD  = 0x3,
                            EXTI_GPIOE  = 0x4,
                            EXTI_GPIOF  = 0x5,
                            EXTI_GPIOG  = 0x6,
                            EXTI_GPIOH  = 0x7,
                            EXTI_GPIOI  = 0x8,
                            EXTI_GPIOJ  = 0x9,
                            EXTI_GPIOK  = 0xa;

    struct Exticr1 {
        using            pos_t = Pos<uint32_t, Exticr1>;
        static constexpr pos_t
           EXTI0_POS = pos_t( 0),
           EXTI1_POS = pos_t( 4),
           EXTI2_POS = pos_t( 8),
           EXTI3_POS = pos_t(12);

        static const uint32_t
              EXTI0_MASK =       0xFU,
              EXTI1_MASK =       0xFU,
              EXTI2_MASK =       0xFU,
              EXTI3_MASK =       0xFU;

        using            mskd_t = Mskd<uint32_t, Exticr1>;
        using            shft_t = Shft<uint32_t, Exticr1>;

        REGBITS_MSKD_RANGE("Afio::Exti0",
                           EXTI0,
                           exti0,
                           EXTI0_MASK,
                           EXTI0_POS,
                           EXTI0_MASK);
        REGBITS_MSKD_RANGE("Afio::Exti1",
                           EXTI1,
                           exti1,
                           EXTI1_MASK,
                           EXTI1_POS,
                           EXTI1_MASK);
        REGBITS_MSKD_RANGE("Afio::Exti2",
                           EXTI2,
                           exti2,
                           EXTI2_MASK,
                           EXTI2_POS,
                           EXTI2_MASK);
        REGBITS_MSKD_RANGE("Afio::Exti3",
                           EXTI3,
                           exti3,
                           EXTI3_MASK,
                           EXTI3_POS,
                           EXTI3_MASK);
    };  // struct Exticr1
    using exticr1_t = Reg<uint32_t, Exticr1>;
          exticr1_t   exticr1;


    struct Exticr2 {
        using            pos_t = Pos<uint32_t, Exticr2>;
        static constexpr pos_t
           EXTI4_POS = pos_t( 0),
           EXTI5_POS = pos_t( 4),
           EXTI6_POS = pos_t( 8),
           EXTI7_POS = pos_t(12);

        static const uint32_t
              EXTI4_MASK =       0xFU,
              EXTI5_MASK =       0xFU,
              EXTI6_MASK =       0xFU,
              EXTI7_MASK =       0xFU;

        using            mskd_t = Mskd<uint32_t, Exticr2>;
        using            shft_t = Shft<uint32_t, Exticr2>;

        REGBITS_MSKD_RANGE("Afio::Exti4",
                           EXTI4,
                           exti4,
                           EXTI4_MASK,
                           EXTI4_POS,
                           EXTI4_MASK);
        REGBITS_MSKD_RANGE("Afio::Exti5",
                           EXTI5,
                           exti5,
                           EXTI5_MASK,
                           EXTI5_POS,
                           EXTI5_MASK);
        REGBITS_MSKD_RANGE("Afio::Exti6",
                           EXTI6,
                           exti6,
                           EXTI6_MASK,
                           EXTI6_POS,
                           EXTI6_MASK);
        REGBITS_MSKD_RANGE("Afio::Exti7",
                           EXTI7,
                           exti7,
                           EXTI7_MASK,
                           EXTI7_POS,
                           EXTI7_MASK);
    };  // struct Exticr2
    using exticr2_t = Reg<uint32_t, Exticr2>;
          exticr2_t   exticr2;


    struct Exticr3 {
        using            pos_t = Pos<uint32_t, Exticr3>;
        static constexpr pos_t
            EXTI8_POS = pos_t( 0),
            EXTI9_POS = pos_t( 4),
           EXTI10_POS = pos_t( 8),
           EXTI11_POS = pos_t(12);

        static const uint32_t
               EXTI8_MASK =      0xFU,
               EXTI9_MASK =      0xFU,
              EXTI10_MASK =      0xFU,
              EXTI11_MASK =      0xFU;

        using            mskd_t = Mskd<uint32_t, Exticr3>;
        using            shft_t = Shft<uint32_t, Exticr3>;

        REGBITS_MSKD_RANGE("Afio::Exti8",
                           EXTI8,
                           exti8,
                           EXTI8_MASK,
                           EXTI8_POS,
                           EXTI8_MASK);
        REGBITS_MSKD_RANGE("Afio::Exti9",
                           EXTI9,
                           exti9,
                           EXTI9_MASK,
                           EXTI9_POS,
                           EXTI9_MASK);
        REGBITS_MSKD_RANGE("Afio::Exti10",
                           EXTI10,
                           exti10,
                           EXTI10_MASK,
                           EXTI10_POS,
                           EXTI10_MASK);
        REGBITS_MSKD_RANGE("Afio::Exti11",
                           EXTI11,
                           exti11,
                           EXTI11_MASK,
                           EXTI11_POS,
                           EXTI11_MASK);
    };  // struct Exticr3
    using exticr3_t = Reg<uint32_t, Exticr3>;
          exticr3_t   exticr3;


    struct Exticr4 {
        using            pos_t = Pos<uint32_t, Exticr4>;
        static constexpr pos_t
           EXTI12_POS = pos_t( 0),
           EXTI13_POS = pos_t( 4),
           EXTI14_POS = pos_t( 8),
           EXTI15_POS = pos_t(12);

        static const uint32_t
              EXTI12_MASK =      0xFU,
              EXTI13_MASK =      0xFU,
              EXTI14_MASK =      0xFU,
              EXTI15_MASK =      0xFU;

        using            mskd_t = Mskd<uint32_t, Exticr4>;
        using            shft_t = Shft<uint32_t, Exticr4>;

        REGBITS_MSKD_RANGE("Afio::Exti12",
                           EXTI12,
                           exti12,
                           EXTI12_MASK,
                           EXTI12_POS,
                           EXTI12_MASK);
        REGBITS_MSKD_RANGE("Afio::Exti13",
                           EXTI13,
                           exti13,
                           EXTI13_MASK,
                           EXTI13_POS,
                           EXTI13_MASK);
        REGBITS_MSKD_RANGE("Afio::Exti14",
                           EXTI14,
                           exti14,
                           EXTI14_MASK,
                           EXTI14_POS,
                           EXTI14_MASK);
        REGBITS_MSKD_RANGE("Afio::Exti15",
                           EXTI15,
                           exti15,
                           EXTI15_MASK,
                           EXTI15_POS,
                           EXTI15_MASK);
    };  // struct Exticr4
    using exticr4_t = Reg<uint32_t, Exticr4>;
          exticr4_t   exticr4;

    private:
    uint32_t    _reserved0,
                _undefined_mapr2;

};  // struct Afio
static_assert(sizeof(Afio) == 32, "sizeof(Afio) != 32");



struct Exti {
    struct Imr {
        using              pos_t = Pos<uint32_t, Imr>;
        static constexpr   pos_t
                 MR0_POS = pos_t( 0),
                 MR1_POS = pos_t( 1),
                 MR2_POS = pos_t( 2),
                 MR3_POS = pos_t( 3),
                 MR4_POS = pos_t( 4),
                 MR5_POS = pos_t( 5),
                 MR6_POS = pos_t( 6),
                 MR7_POS = pos_t( 7),
                 MR8_POS = pos_t( 8),
                 MR9_POS = pos_t( 9),
                MR10_POS = pos_t(10),
                MR11_POS = pos_t(11),
                MR12_POS = pos_t(12),
                MR13_POS = pos_t(13),
                MR14_POS = pos_t(14),
                MR15_POS = pos_t(15),
                MR16_POS = pos_t(16),
                MR17_POS = pos_t(17),
                MR18_POS = pos_t(18);

        using              bits_t = Bits<uint32_t, Imr>;
        static constexpr   bits_t
        MR0              = bits_t(1,          MR0_POS),
        MR1              = bits_t(1,          MR1_POS),
        MR2              = bits_t(1,          MR2_POS),
        MR3              = bits_t(1,          MR3_POS),
        MR4              = bits_t(1,          MR4_POS),
        MR5              = bits_t(1,          MR5_POS),
        MR6              = bits_t(1,          MR6_POS),
        MR7              = bits_t(1,          MR7_POS),
        MR8              = bits_t(1,          MR8_POS),
        MR9              = bits_t(1,          MR9_POS),
        MR10             = bits_t(1,         MR10_POS),
        MR11             = bits_t(1,         MR11_POS),
        MR12             = bits_t(1,         MR12_POS),
        MR13             = bits_t(1,         MR13_POS),
        MR14             = bits_t(1,         MR14_POS),
        MR15             = bits_t(1,         MR15_POS),
        MR16             = bits_t(1,         MR16_POS),
        MR17             = bits_t(1,         MR17_POS),
        MR18             = bits_t(1,         MR18_POS);
    };  // struct Imr
    using imr_t = Reg<uint32_t, Imr>;
          imr_t   imr;


    struct Emr {
        using              pos_t = Pos<uint32_t, Emr>;
        static constexpr   pos_t
                 MR0_POS = pos_t( 0),
                 MR1_POS = pos_t( 1),
                 MR2_POS = pos_t( 2),
                 MR3_POS = pos_t( 3),
                 MR4_POS = pos_t( 4),
                 MR5_POS = pos_t( 5),
                 MR6_POS = pos_t( 6),
                 MR7_POS = pos_t( 7),
                 MR8_POS = pos_t( 8),
                 MR9_POS = pos_t( 9),
                MR10_POS = pos_t(10),
                MR11_POS = pos_t(11),
                MR12_POS = pos_t(12),
                MR13_POS = pos_t(13),
                MR14_POS = pos_t(14),
                MR15_POS = pos_t(15),
                MR16_POS = pos_t(16),
                MR17_POS = pos_t(17),
                MR18_POS = pos_t(18);

        using              bits_t = Bits<uint32_t, Emr>;
        static constexpr   bits_t
        MR0              = bits_t(1,          MR0_POS),
        MR1              = bits_t(1,          MR1_POS),
        MR2              = bits_t(1,          MR2_POS),
        MR3              = bits_t(1,          MR3_POS),
        MR4              = bits_t(1,          MR4_POS),
        MR5              = bits_t(1,          MR5_POS),
        MR6              = bits_t(1,          MR6_POS),
        MR7              = bits_t(1,          MR7_POS),
        MR8              = bits_t(1,          MR8_POS),
        MR9              = bits_t(1,          MR9_POS),
        MR10             = bits_t(1,         MR10_POS),
        MR11             = bits_t(1,         MR11_POS),
        MR12             = bits_t(1,         MR12_POS),
        MR13             = bits_t(1,         MR13_POS),
        MR14             = bits_t(1,         MR14_POS),
        MR15             = bits_t(1,         MR15_POS),
        MR16             = bits_t(1,         MR16_POS),
        MR17             = bits_t(1,         MR17_POS),
        MR18             = bits_t(1,         MR18_POS);
    };  // struct Emr
    using emr_t = Reg<uint32_t, Emr>;
          emr_t   emr;


    struct Rtsr {
        using              pos_t = Pos<uint32_t, Rtsr>;
        static constexpr   pos_t
                 TR0_POS = pos_t( 0),
                 TR1_POS = pos_t( 1),
                 TR2_POS = pos_t( 2),
                 TR3_POS = pos_t( 3),
                 TR4_POS = pos_t( 4),
                 TR5_POS = pos_t( 5),
                 TR6_POS = pos_t( 6),
                 TR7_POS = pos_t( 7),
                 TR8_POS = pos_t( 8),
                 TR9_POS = pos_t( 9),
                TR10_POS = pos_t(10),
                TR11_POS = pos_t(11),
                TR12_POS = pos_t(12),
                TR13_POS = pos_t(13),
                TR14_POS = pos_t(14),
                TR15_POS = pos_t(15),
                TR16_POS = pos_t(16),
                TR17_POS = pos_t(17),
                TR18_POS = pos_t(18);

        using              bits_t = Bits<uint32_t, Rtsr>;
        static constexpr   bits_t
        TR0              = bits_t(1,          TR0_POS),
        TR1              = bits_t(1,          TR1_POS),
        TR2              = bits_t(1,          TR2_POS),
        TR3              = bits_t(1,          TR3_POS),
        TR4              = bits_t(1,          TR4_POS),
        TR5              = bits_t(1,          TR5_POS),
        TR6              = bits_t(1,          TR6_POS),
        TR7              = bits_t(1,          TR7_POS),
        TR8              = bits_t(1,          TR8_POS),
        TR9              = bits_t(1,          TR9_POS),
        TR10             = bits_t(1,         TR10_POS),
        TR11             = bits_t(1,         TR11_POS),
        TR12             = bits_t(1,         TR12_POS),
        TR13             = bits_t(1,         TR13_POS),
        TR14             = bits_t(1,         TR14_POS),
        TR15             = bits_t(1,         TR15_POS),
        TR16             = bits_t(1,         TR16_POS),
        TR17             = bits_t(1,         TR17_POS),
        TR18             = bits_t(1,         TR18_POS);
    };  // struct Rtsr
    using rtsr_t = Reg<uint32_t, Rtsr>;
          rtsr_t   rtsr;


    struct Ftsr {
        using              pos_t = Pos<uint32_t, Ftsr>;
        static constexpr   pos_t
                 TR0_POS = pos_t( 0),
                 TR1_POS = pos_t( 1),
                 TR2_POS = pos_t( 2),
                 TR3_POS = pos_t( 3),
                 TR4_POS = pos_t( 4),
                 TR5_POS = pos_t( 5),
                 TR6_POS = pos_t( 6),
                 TR7_POS = pos_t( 7),
                 TR8_POS = pos_t( 8),
                 TR9_POS = pos_t( 9),
                TR10_POS = pos_t(10),
                TR11_POS = pos_t(11),
                TR12_POS = pos_t(12),
                TR13_POS = pos_t(13),
                TR14_POS = pos_t(14),
                TR15_POS = pos_t(15),
                TR16_POS = pos_t(16),
                TR17_POS = pos_t(17),
                TR18_POS = pos_t(18);

        using              bits_t = Bits<uint32_t, Ftsr>;
        static constexpr   bits_t
        TR0              = bits_t(1,          TR0_POS),
        TR1              = bits_t(1,          TR1_POS),
        TR2              = bits_t(1,          TR2_POS),
        TR3              = bits_t(1,          TR3_POS),
        TR4              = bits_t(1,          TR4_POS),
        TR5              = bits_t(1,          TR5_POS),
        TR6              = bits_t(1,          TR6_POS),
        TR7              = bits_t(1,          TR7_POS),
        TR8              = bits_t(1,          TR8_POS),
        TR9              = bits_t(1,          TR9_POS),
        TR10             = bits_t(1,         TR10_POS),
        TR11             = bits_t(1,         TR11_POS),
        TR12             = bits_t(1,         TR12_POS),
        TR13             = bits_t(1,         TR13_POS),
        TR14             = bits_t(1,         TR14_POS),
        TR15             = bits_t(1,         TR15_POS),
        TR16             = bits_t(1,         TR16_POS),
        TR17             = bits_t(1,         TR17_POS),
        TR18             = bits_t(1,         TR18_POS);
    };  // struct Ftsr
    using ftsr_t = Reg<uint32_t, Ftsr>;
          ftsr_t   ftsr;


    struct Swier {
        using              pos_t = Pos<uint32_t, Swier>;
        static constexpr   pos_t
              SWIER0_POS = pos_t( 0),
              SWIER1_POS = pos_t( 1),
              SWIER2_POS = pos_t( 2),
              SWIER3_POS = pos_t( 3),
              SWIER4_POS = pos_t( 4),
              SWIER5_POS = pos_t( 5),
              SWIER6_POS = pos_t( 6),
              SWIER7_POS = pos_t( 7),
              SWIER8_POS = pos_t( 8),
              SWIER9_POS = pos_t( 9),
             SWIER10_POS = pos_t(10),
             SWIER11_POS = pos_t(11),
             SWIER12_POS = pos_t(12),
             SWIER13_POS = pos_t(13),
             SWIER14_POS = pos_t(14),
             SWIER15_POS = pos_t(15),
             SWIER16_POS = pos_t(16),
             SWIER17_POS = pos_t(17),
             SWIER18_POS = pos_t(18);

        using              bits_t = Bits<uint32_t, Swier>;
        static constexpr   bits_t
        SWIER0           = bits_t(1,       SWIER0_POS),
        SWIER1           = bits_t(1,       SWIER1_POS),
        SWIER2           = bits_t(1,       SWIER2_POS),
        SWIER3           = bits_t(1,       SWIER3_POS),
        SWIER4           = bits_t(1,       SWIER4_POS),
        SWIER5           = bits_t(1,       SWIER5_POS),
        SWIER6           = bits_t(1,       SWIER6_POS),
        SWIER7           = bits_t(1,       SWIER7_POS),
        SWIER8           = bits_t(1,       SWIER8_POS),
        SWIER9           = bits_t(1,       SWIER9_POS),
        SWIER10          = bits_t(1,      SWIER10_POS),
        SWIER11          = bits_t(1,      SWIER11_POS),
        SWIER12          = bits_t(1,      SWIER12_POS),
        SWIER13          = bits_t(1,      SWIER13_POS),
        SWIER14          = bits_t(1,      SWIER14_POS),
        SWIER15          = bits_t(1,      SWIER15_POS),
        SWIER16          = bits_t(1,      SWIER16_POS),
        SWIER17          = bits_t(1,      SWIER17_POS),
        SWIER18          = bits_t(1,      SWIER18_POS);
    };  // struct Swier
    using swier_t = Reg<uint32_t, Swier>;
          swier_t   swier;


    struct Pr {
        using              pos_t = Pos<uint32_t, Pr>;
        static constexpr   pos_t
                 PR0_POS = pos_t( 0),
                 PR1_POS = pos_t( 1),
                 PR2_POS = pos_t( 2),
                 PR3_POS = pos_t( 3),
                 PR4_POS = pos_t( 4),
                 PR5_POS = pos_t( 5),
                 PR6_POS = pos_t( 6),
                 PR7_POS = pos_t( 7),
                 PR8_POS = pos_t( 8),
                 PR9_POS = pos_t( 9),
                PR10_POS = pos_t(10),
                PR11_POS = pos_t(11),
                PR12_POS = pos_t(12),
                PR13_POS = pos_t(13),
                PR14_POS = pos_t(14),
                PR15_POS = pos_t(15),
                PR16_POS = pos_t(16),
                PR17_POS = pos_t(17),
                PR18_POS = pos_t(18);

        using              bits_t = Bits<uint32_t, Pr>;
        static constexpr   bits_t
        PR0              = bits_t(1,          PR0_POS),
        PR1              = bits_t(1,          PR1_POS),
        PR2              = bits_t(1,          PR2_POS),
        PR3              = bits_t(1,          PR3_POS),
        PR4              = bits_t(1,          PR4_POS),
        PR5              = bits_t(1,          PR5_POS),
        PR6              = bits_t(1,          PR6_POS),
        PR7              = bits_t(1,          PR7_POS),
        PR8              = bits_t(1,          PR8_POS),
        PR9              = bits_t(1,          PR9_POS),
        PR10             = bits_t(1,         PR10_POS),
        PR11             = bits_t(1,         PR11_POS),
        PR12             = bits_t(1,         PR12_POS),
        PR13             = bits_t(1,         PR13_POS),
        PR14             = bits_t(1,         PR14_POS),
        PR15             = bits_t(1,         PR15_POS),
        PR16             = bits_t(1,         PR16_POS),
        PR17             = bits_t(1,         PR17_POS),
        PR18             = bits_t(1,         PR18_POS);
    };  // struct Pr
    using pr_t = Reg<uint32_t, Pr>;
          pr_t   pr;

};  // struct Exti
static_assert(sizeof(Exti) == 24, "sizeof(Exti) != 24");



struct Adc {
    struct Sr {
        using              pos_t = Pos<uint32_t, Sr>;
        static constexpr   pos_t
                 AWD_POS = pos_t( 0),
                 EOC_POS = pos_t( 1),
                JEOC_POS = pos_t( 2),
               JSTRT_POS = pos_t( 3),
                STRT_POS = pos_t( 4);

        using              bits_t = Bits<uint32_t, Sr>;
        static constexpr   bits_t
        AWD              = bits_t(1,          AWD_POS),
        EOC              = bits_t(1,          EOC_POS),
        JEOC             = bits_t(1,         JEOC_POS),
        JSTRT            = bits_t(1,        JSTRT_POS),
        STRT             = bits_t(1,         STRT_POS);
    };  // struct Sr
    using sr_t = Reg<uint32_t, Sr>;
          sr_t   sr;


    struct Cr1 {
        using              pos_t = Pos<uint32_t, Cr1>;
        static constexpr   pos_t
               AWDCH_POS = pos_t( 0),
               EOCIE_POS = pos_t( 5),
               AWDIE_POS = pos_t( 6),
              JEOCIE_POS = pos_t( 7),
                SCAN_POS = pos_t( 8),
              AWDSGL_POS = pos_t( 9),
               JAUTO_POS = pos_t(10),
              DISCEN_POS = pos_t(11),
             JDISCEN_POS = pos_t(12),
             DISCNUM_POS = pos_t(13),
             DUALMOD_POS = pos_t(16),
              JAWDEN_POS = pos_t(22),
               AWDEN_POS = pos_t(23);

        using              bits_t = Bits<uint32_t, Cr1>;
        static constexpr   bits_t
        EOCIE            = bits_t(1,        EOCIE_POS),
        AWDIE            = bits_t(1,        AWDIE_POS),
        JEOCIE           = bits_t(1,       JEOCIE_POS),
        SCAN             = bits_t(1,         SCAN_POS),
        AWDSGL           = bits_t(1,       AWDSGL_POS),
        JAUTO            = bits_t(1,        JAUTO_POS),
        DISCEN           = bits_t(1,       DISCEN_POS),
        JDISCEN          = bits_t(1,      JDISCEN_POS),
        JAWDEN           = bits_t(1,       JAWDEN_POS),
        AWDEN            = bits_t(1,        AWDEN_POS);

        static const uint32_t
              AWDCH_MASK =      0x1FU,
            DISCNUM_MASK =       0x7U,
            DUALMOD_MASK =       0xFU;

        using              mskd_t = Mskd<uint32_t, Cr1>;
        using              shft_t = Shft<uint32_t, Cr1>;

        static constexpr   mskd_t
        AWDCH_CHNL_0          = mskd_t( AWDCH_MASK,       0,     AWDCH_POS),
        AWDCH_CHNL_1          = mskd_t( AWDCH_MASK,       1,     AWDCH_POS),
        AWDCH_CHNL_2          = mskd_t( AWDCH_MASK,       2,     AWDCH_POS),
        AWDCH_CHNL_3          = mskd_t( AWDCH_MASK,       3,     AWDCH_POS),
        AWDCH_CHNL_4          = mskd_t( AWDCH_MASK,       4,     AWDCH_POS),
        AWDCH_CHNL_5          = mskd_t( AWDCH_MASK,       5,     AWDCH_POS),
        AWDCH_CHNL_6          = mskd_t( AWDCH_MASK,       6,     AWDCH_POS),
        AWDCH_CHNL_7          = mskd_t( AWDCH_MASK,       7,     AWDCH_POS),
        AWDCH_CHNL_8          = mskd_t( AWDCH_MASK,       8,     AWDCH_POS),
        AWDCH_CHNL_9          = mskd_t( AWDCH_MASK,       9,     AWDCH_POS),
        AWDCH_CHNL_10         = mskd_t( AWDCH_MASK,      10,     AWDCH_POS),
        AWDCH_CHNL_11         = mskd_t( AWDCH_MASK,      11,     AWDCH_POS),
        AWDCH_CHNL_12         = mskd_t( AWDCH_MASK,      12,     AWDCH_POS),
        AWDCH_CHNL_13         = mskd_t( AWDCH_MASK,      13,     AWDCH_POS),
        AWDCH_CHNL_14         = mskd_t( AWDCH_MASK,      14,     AWDCH_POS),
        AWDCH_CHNL_15         = mskd_t( AWDCH_MASK,      15,     AWDCH_POS),
        AWDCH_CHNL_16         = mskd_t( AWDCH_MASK,      16,     AWDCH_POS),
        AWDCH_CHNL_17         = mskd_t( AWDCH_MASK,      17,     AWDCH_POS),
#if 0
        DISCNUM_1_CHNL        = mskd_t(DISCNUM_MASK,      0,   DISCNUM_POS),
        DISCNUM_2_CHNLS       = mskd_t(DISCNUM_MASK,      1,   DISCNUM_POS),
        DISCNUM_3_CHNLS       = mskd_t(DISCNUM_MASK,      2,   DISCNUM_POS),
        DISCNUM_4_CHNLS       = mskd_t(DISCNUM_MASK,      3,   DISCNUM_POS),
        DISCNUM_5_CHNLS       = mskd_t(DISCNUM_MASK,      4,   DISCNUM_POS),
        DISCNUM_6_CHNLS       = mskd_t(DISCNUM_MASK,      5,   DISCNUM_POS),
        DISCNUM_7_CHNLS       = mskd_t(DISCNUM_MASK,      6,   DISCNUM_POS),
        DISCNUM_8_CHNLS       = mskd_t(DISCNUM_MASK,      7,   DISCNUM_POS),
#endif
        DUALMOD_INDEPEND      = mskd_t(DUALMOD_MASK, 0b0000,   DUALMOD_POS),
        DUALMOD_SIMUL_INJECT  = mskd_t(DUALMOD_MASK, 0b0001,   DUALMOD_POS),
        DUALMOD_SIMUL_ALTERN  = mskd_t(DUALMOD_MASK, 0b0010,   DUALMOD_POS),
        DUALMOD_INJCT_FAST    = mskd_t(DUALMOD_MASK, 0b0011,   DUALMOD_POS),
        DUALMOD_INJCT_SLOW    = mskd_t(DUALMOD_MASK, 0b0100,   DUALMOD_POS),
        DUALMOD_INJECT_SIMUL  = mskd_t(DUALMOD_MASK, 0b0101,   DUALMOD_POS),
        DUALMOD_REGULAR_SIMUL = mskd_t(DUALMOD_MASK, 0b0110,   DUALMOD_POS),
        DUALMOD_FAST_INTRLV   = mskd_t(DUALMOD_MASK, 0b0111,   DUALMOD_POS),
        DUALMOD_SLOW_INTLV    = mskd_t(DUALMOD_MASK, 0b1000,   DUALMOD_POS),
        DUALMOD_ALTERN_TRIG   = mskd_t(DUALMOD_MASK, 0b1001,   DUALMOD_POS);

        REGBITS_MSKD_RANGE("Adc::Cr1",
                           DISCNUM,
                           discnum,
                           DISCNUM_MASK,
                           DISCNUM_POS,
                           DISCNUM_MASK);
    };  // struct Cr1
    using cr1_t = Reg<uint32_t, Cr1>;
          cr1_t   cr1;


    struct Cr2 {
        using              pos_t = Pos<uint32_t, Cr2>;
        static constexpr   pos_t
                ADON_POS = pos_t( 0),
                CONT_POS = pos_t( 1),
                 CAL_POS = pos_t( 2),
              RSTCAL_POS = pos_t( 3),
                 DMA_POS = pos_t( 8),
               ALIGN_POS = pos_t(11),
             JEXTSEL_POS = pos_t(12),
            JEXTTRIG_POS = pos_t(15),
              EXTSEL_POS = pos_t(17),
             EXTTRIG_POS = pos_t(20),
            JSWSTART_POS = pos_t(21),
             SWSTART_POS = pos_t(22),
             TSVREFE_POS = pos_t(23);

        using              bits_t = Bits<uint32_t, Cr2>;
        static constexpr   bits_t
        ADON             = bits_t(1,         ADON_POS),
        CONT             = bits_t(1,         CONT_POS),
        CAL              = bits_t(1,          CAL_POS),
        RSTCAL           = bits_t(1,       RSTCAL_POS),
        DMA              = bits_t(1,          DMA_POS),
        ALIGN            = bits_t(1,        ALIGN_POS),
        JEXTTRIG         = bits_t(1,     JEXTTRIG_POS),
        EXTTRIG          = bits_t(1,      EXTTRIG_POS),
        JSWSTART         = bits_t(1,     JSWSTART_POS),
        SWSTART          = bits_t(1,      SWSTART_POS),
        TSVREFE          = bits_t(1,      TSVREFE_POS);

        static const uint32_t
            JEXTSEL_MASK =       0x7U,
             EXTSEL_MASK =       0x7U;

        using              mskd_t = Mskd<uint32_t, Cr2>;
        static constexpr   mskd_t

        JEXTSEL_TRGO_1   = mskd_t(JEXTSEL_MASK,  0b000,     JEXTSEL_POS),
        JEXTSEL_CC4_1    = mskd_t(JEXTSEL_MASK,  0b001,     JEXTSEL_POS),
        JEXTSEL_TRGO_2   = mskd_t(JEXTSEL_MASK,  0b010,     JEXTSEL_POS),
        JEXTSEL_CC3_4    = mskd_t(JEXTSEL_MASK,  0b010,     JEXTSEL_POS),
        JEXTSEL_CC1_2    = mskd_t(JEXTSEL_MASK,  0b011,     JEXTSEL_POS),
        JEXTSEL_CC2_8    = mskd_t(JEXTSEL_MASK,  0b011,     JEXTSEL_POS),
        JEXTSEL_CC4_3    = mskd_t(JEXTSEL_MASK,  0b100,     JEXTSEL_POS),
        JEXTSEL_CC4_8    = mskd_t(JEXTSEL_MASK,  0b100,     JEXTSEL_POS),
        JEXTSEL_TRGO_4   = mskd_t(JEXTSEL_MASK,  0b101,     JEXTSEL_POS),
        JEXTSEL_TRGO_5   = mskd_t(JEXTSEL_MASK,  0b101,     JEXTSEL_POS),
        JEXTSEL_EXTI_15  = mskd_t(JEXTSEL_MASK,  0b110,     JEXTSEL_POS),
        JEXTSEL_CC4_5    = mskd_t(JEXTSEL_MASK,  0b110,     JEXTSEL_POS),
        JEXTSEL_JSWSTART = mskd_t(JEXTSEL_MASK,  0b111,     JEXTSEL_POS),
        EXTSEL_TRGO_1    = mskd_t( EXTSEL_MASK,  0b000,      EXTSEL_POS),
        EXTSEL_CC4_1     = mskd_t( EXTSEL_MASK,  0b001,      EXTSEL_POS),
        EXTSEL_TRGO_2    = mskd_t( EXTSEL_MASK,  0b010,      EXTSEL_POS),
        EXTSEL_CC3_4     = mskd_t( EXTSEL_MASK,  0b010,      EXTSEL_POS),
        EXTSEL_CC1_2     = mskd_t( EXTSEL_MASK,  0b011,      EXTSEL_POS),
        EXTSEL_CC2_8     = mskd_t( EXTSEL_MASK,  0b011,      EXTSEL_POS),
        EXTSEL_CC4_3     = mskd_t( EXTSEL_MASK,  0b100,      EXTSEL_POS),
        EXTSEL_CC4_8     = mskd_t( EXTSEL_MASK,  0b100,      EXTSEL_POS),
        EXTSEL_TRGO_4    = mskd_t( EXTSEL_MASK,  0b101,      EXTSEL_POS),
        EXTSEL_TRGO_5    = mskd_t( EXTSEL_MASK,  0b101,      EXTSEL_POS),
        EXTSEL_EXTI_15   = mskd_t( EXTSEL_MASK,  0b110,      EXTSEL_POS),
        EXTSEL_CC4_5     = mskd_t( EXTSEL_MASK,  0b110,      EXTSEL_POS),
        EXTSEL_SWSTART   = mskd_t( EXTSEL_MASK,  0b111,      EXTSEL_POS);
    };  // struct Cr2
    using cr2_t = Reg<uint32_t, Cr2>;
          cr2_t   cr2;


    struct Smpr1 {
        using              pos_t = Pos<uint32_t, Smpr1>;
        static constexpr   pos_t
               SMP10_POS = pos_t( 0),
               SMP11_POS = pos_t( 3),
               SMP12_POS = pos_t( 6),
               SMP13_POS = pos_t( 9),
               SMP14_POS = pos_t(12),
               SMP15_POS = pos_t(15),
               SMP16_POS = pos_t(18),
               SMP17_POS = pos_t(21);

        static const uint32_t
              SMP10_MASK =       0x7U,
              SMP11_MASK =       0x7U,
              SMP12_MASK =       0x7U,
              SMP13_MASK =       0x7U,
              SMP14_MASK =       0x7U,
              SMP15_MASK =       0x7U,
              SMP16_MASK =       0x7U,
              SMP17_MASK =       0x7U;

        using              mskd_t = Mskd<uint32_t, Smpr1>;
        static constexpr   mskd_t
        SMP10_1P5        = mskd_t(      SMP10_MASK,  0b000,       SMP10_POS),
        SMP10_7P5        = mskd_t(      SMP10_MASK,  0b001,       SMP10_POS),
        SMP10_13P5       = mskd_t(      SMP10_MASK,  0b010,       SMP10_POS),
        SMP10_28P5       = mskd_t(      SMP10_MASK,  0b011,       SMP10_POS),
        SMP10_41P5       = mskd_t(      SMP10_MASK,  0b100,       SMP10_POS),
        SMP10_55P5       = mskd_t(      SMP10_MASK,  0b101,       SMP10_POS),
        SMP10_71P5       = mskd_t(      SMP10_MASK,  0b110,       SMP10_POS),
        SMP10_239P5      = mskd_t(      SMP10_MASK,  0b111,       SMP10_POS),
        SMP11_1P5        = mskd_t(      SMP11_MASK,  0b000,       SMP11_POS),
        SMP11_7P5        = mskd_t(      SMP11_MASK,  0b001,       SMP11_POS),
        SMP11_13P5       = mskd_t(      SMP11_MASK,  0b010,       SMP11_POS),
        SMP11_28P5       = mskd_t(      SMP11_MASK,  0b011,       SMP11_POS),
        SMP11_41P5       = mskd_t(      SMP11_MASK,  0b100,       SMP11_POS),
        SMP11_55P5       = mskd_t(      SMP11_MASK,  0b101,       SMP11_POS),
        SMP11_71P5       = mskd_t(      SMP11_MASK,  0b110,       SMP11_POS),
        SMP11_239P5      = mskd_t(      SMP11_MASK,  0b111,       SMP11_POS),
        SMP12_1P5        = mskd_t(      SMP12_MASK,  0b000,       SMP12_POS),
        SMP12_7P5        = mskd_t(      SMP12_MASK,  0b001,       SMP12_POS),
        SMP12_13P5       = mskd_t(      SMP12_MASK,  0b010,       SMP12_POS),
        SMP12_28P5       = mskd_t(      SMP12_MASK,  0b011,       SMP12_POS),
        SMP12_41P5       = mskd_t(      SMP12_MASK,  0b100,       SMP12_POS),
        SMP12_55P5       = mskd_t(      SMP12_MASK,  0b101,       SMP12_POS),
        SMP12_71P5       = mskd_t(      SMP12_MASK,  0b110,       SMP12_POS),
        SMP12_239P5      = mskd_t(      SMP12_MASK,  0b111,       SMP12_POS),
        SMP13_1P5        = mskd_t(      SMP13_MASK,  0b000,       SMP13_POS),
        SMP13_7P5        = mskd_t(      SMP13_MASK,  0b001,       SMP13_POS),
        SMP13_13P5       = mskd_t(      SMP13_MASK,  0b010,       SMP13_POS),
        SMP13_28P5       = mskd_t(      SMP13_MASK,  0b011,       SMP13_POS),
        SMP13_41P5       = mskd_t(      SMP13_MASK,  0b100,       SMP13_POS),
        SMP13_55P5       = mskd_t(      SMP13_MASK,  0b101,       SMP13_POS),
        SMP13_71P5       = mskd_t(      SMP13_MASK,  0b110,       SMP13_POS),
        SMP13_239P5      = mskd_t(      SMP13_MASK,  0b111,       SMP13_POS),
        SMP14_1P5        = mskd_t(      SMP14_MASK,  0b000,       SMP14_POS),
        SMP14_7P5        = mskd_t(      SMP14_MASK,  0b001,       SMP14_POS),
        SMP14_13P5       = mskd_t(      SMP14_MASK,  0b010,       SMP14_POS),
        SMP14_28P5       = mskd_t(      SMP14_MASK,  0b011,       SMP14_POS),
        SMP14_41P5       = mskd_t(      SMP14_MASK,  0b100,       SMP14_POS),
        SMP14_55P5       = mskd_t(      SMP14_MASK,  0b101,       SMP14_POS),
        SMP14_71P5       = mskd_t(      SMP14_MASK,  0b110,       SMP14_POS),
        SMP14_239P5      = mskd_t(      SMP14_MASK,  0b111,       SMP14_POS),
        SMP15_1P5        = mskd_t(      SMP15_MASK,  0b000,       SMP15_POS),
        SMP15_7P5        = mskd_t(      SMP15_MASK,  0b001,       SMP15_POS),
        SMP15_13P5       = mskd_t(      SMP15_MASK,  0b010,       SMP15_POS),
        SMP15_28P5       = mskd_t(      SMP15_MASK,  0b011,       SMP15_POS),
        SMP15_41P5       = mskd_t(      SMP15_MASK,  0b100,       SMP15_POS),
        SMP15_55P5       = mskd_t(      SMP15_MASK,  0b101,       SMP15_POS),
        SMP15_71P5       = mskd_t(      SMP15_MASK,  0b110,       SMP15_POS),
        SMP15_239P5      = mskd_t(      SMP15_MASK,  0b111,       SMP15_POS),
        SMP16_1P5        = mskd_t(      SMP16_MASK,  0b000,       SMP16_POS),
        SMP16_7P5        = mskd_t(      SMP16_MASK,  0b001,       SMP16_POS),
        SMP16_13P5       = mskd_t(      SMP16_MASK,  0b010,       SMP16_POS),
        SMP16_28P5       = mskd_t(      SMP16_MASK,  0b011,       SMP16_POS),
        SMP16_41P5       = mskd_t(      SMP16_MASK,  0b100,       SMP16_POS),
        SMP16_55P5       = mskd_t(      SMP16_MASK,  0b101,       SMP16_POS),
        SMP16_71P5       = mskd_t(      SMP16_MASK,  0b110,       SMP16_POS),
        SMP16_239P5      = mskd_t(      SMP16_MASK,  0b111,       SMP16_POS),
        SMP17_1P5        = mskd_t(      SMP17_MASK,  0b000,       SMP17_POS),
        SMP17_7P5        = mskd_t(      SMP17_MASK,  0b001,       SMP17_POS),
        SMP17_13P5       = mskd_t(      SMP17_MASK,  0b010,       SMP17_POS),
        SMP17_28P5       = mskd_t(      SMP17_MASK,  0b011,       SMP17_POS),
        SMP17_41P5       = mskd_t(      SMP17_MASK,  0b100,       SMP17_POS),
        SMP17_55P5       = mskd_t(      SMP17_MASK,  0b101,       SMP17_POS),
        SMP17_71P5       = mskd_t(      SMP17_MASK,  0b110,       SMP17_POS),
        SMP17_239P5      = mskd_t(      SMP17_MASK,  0b111,       SMP17_POS);
    };  // struct Smpr1
    using smpr1_t = Reg<uint32_t, Smpr1>;
          smpr1_t   smpr1;


    struct Smpr2 {
        using              pos_t = Pos<uint32_t, Smpr2>;
        static constexpr   pos_t
               SMP0_POS = pos_t( 0),
               SMP1_POS = pos_t( 3),
               SMP2_POS = pos_t( 6),
               SMP3_POS = pos_t( 9),
               SMP4_POS = pos_t(12),
               SMP5_POS = pos_t(15),
               SMP6_POS = pos_t(18),
               SMP7_POS = pos_t(21),
               SMP8_POS = pos_t(24),
               SMP9_POS = pos_t(27);

        static const uint32_t
              SMP0_MASK = 0x7U,
              SMP1_MASK = 0x7U,
              SMP2_MASK = 0x7U,
              SMP3_MASK = 0x7U,
              SMP4_MASK = 0x7U,
              SMP5_MASK = 0x7U,
              SMP6_MASK = 0x7U,
              SMP7_MASK = 0x7U,
              SMP8_MASK = 0x7U,
              SMP9_MASK = 0x7U;

        using              mskd_t = Mskd<uint32_t, Smpr2>;
        static constexpr   mskd_t
        SMP0_1P5         = mskd_t(      SMP0_MASK,   0b000,       SMP0_POS),
        SMP0_7P5         = mskd_t(      SMP0_MASK,   0b001,       SMP0_POS),
        SMP0_13P5        = mskd_t(      SMP0_MASK,   0b010,       SMP0_POS),
        SMP0_28P5        = mskd_t(      SMP0_MASK,   0b011,       SMP0_POS),
        SMP0_41P5        = mskd_t(      SMP0_MASK,   0b100,       SMP0_POS),
        SMP0_55P5        = mskd_t(      SMP0_MASK,   0b101,       SMP0_POS),
        SMP0_71P5        = mskd_t(      SMP0_MASK,   0b110,       SMP0_POS),
        SMP0_239P5       = mskd_t(      SMP0_MASK,   0b111,       SMP0_POS),
        SMP1_1P5         = mskd_t(      SMP1_MASK,   0b000,       SMP1_POS),
        SMP1_7P5         = mskd_t(      SMP1_MASK,   0b001,       SMP1_POS),
        SMP1_13P5        = mskd_t(      SMP1_MASK,   0b010,       SMP1_POS),
        SMP1_28P5        = mskd_t(      SMP1_MASK,   0b011,       SMP1_POS),
        SMP1_41P5        = mskd_t(      SMP1_MASK,   0b100,       SMP1_POS),
        SMP1_55P5        = mskd_t(      SMP1_MASK,   0b101,       SMP1_POS),
        SMP1_71P5        = mskd_t(      SMP1_MASK,   0b110,       SMP1_POS),
        SMP1_239P5       = mskd_t(      SMP1_MASK,   0b111,       SMP1_POS),
        SMP2_1P5         = mskd_t(      SMP2_MASK,   0b000,       SMP2_POS),
        SMP2_7P5         = mskd_t(      SMP2_MASK,   0b001,       SMP2_POS),
        SMP2_13P5        = mskd_t(      SMP2_MASK,   0b010,       SMP2_POS),
        SMP2_28P5        = mskd_t(      SMP2_MASK,   0b011,       SMP2_POS),
        SMP2_41P5        = mskd_t(      SMP2_MASK,   0b100,       SMP2_POS),
        SMP2_55P5        = mskd_t(      SMP2_MASK,   0b101,       SMP2_POS),
        SMP2_71P5        = mskd_t(      SMP2_MASK,   0b110,       SMP2_POS),
        SMP2_239P5       = mskd_t(      SMP2_MASK,   0b111,       SMP2_POS),
        SMP3_1P5         = mskd_t(      SMP3_MASK,   0b000,       SMP3_POS),
        SMP3_7P5         = mskd_t(      SMP3_MASK,   0b001,       SMP3_POS),
        SMP3_13P5        = mskd_t(      SMP3_MASK,   0b010,       SMP3_POS),
        SMP3_28P5        = mskd_t(      SMP3_MASK,   0b011,       SMP3_POS),
        SMP3_41P5        = mskd_t(      SMP3_MASK,   0b100,       SMP3_POS),
        SMP3_55P5        = mskd_t(      SMP3_MASK,   0b101,       SMP3_POS),
        SMP3_71P5        = mskd_t(      SMP3_MASK,   0b110,       SMP3_POS),
        SMP3_239P5       = mskd_t(      SMP3_MASK,   0b111,       SMP3_POS),
        SMP4_1P5         = mskd_t(      SMP4_MASK,   0b000,       SMP4_POS),
        SMP4_7P5         = mskd_t(      SMP4_MASK,   0b001,       SMP4_POS),
        SMP4_13P5        = mskd_t(      SMP4_MASK,   0b010,       SMP4_POS),
        SMP4_28P5        = mskd_t(      SMP4_MASK,   0b011,       SMP4_POS),
        SMP4_41P5        = mskd_t(      SMP4_MASK,   0b100,       SMP4_POS),
        SMP4_55P5        = mskd_t(      SMP4_MASK,   0b101,       SMP4_POS),
        SMP4_71P5        = mskd_t(      SMP4_MASK,   0b110,       SMP4_POS),
        SMP4_239P5       = mskd_t(      SMP4_MASK,   0b111,       SMP4_POS),
        SMP5_1P5         = mskd_t(      SMP5_MASK,   0b000,       SMP5_POS),
        SMP5_7P5         = mskd_t(      SMP5_MASK,   0b001,       SMP5_POS),
        SMP5_13P5        = mskd_t(      SMP5_MASK,   0b010,       SMP5_POS),
        SMP5_28P5        = mskd_t(      SMP5_MASK,   0b011,       SMP5_POS),
        SMP5_41P5        = mskd_t(      SMP5_MASK,   0b100,       SMP5_POS),
        SMP5_55P5        = mskd_t(      SMP5_MASK,   0b101,       SMP5_POS),
        SMP5_71P5        = mskd_t(      SMP5_MASK,   0b110,       SMP5_POS),
        SMP5_239P5       = mskd_t(      SMP5_MASK,   0b111,       SMP5_POS),
        SMP6_1P5         = mskd_t(      SMP6_MASK,   0b000,       SMP6_POS),
        SMP6_7P5         = mskd_t(      SMP6_MASK,   0b001,       SMP6_POS),
        SMP6_13P5        = mskd_t(      SMP6_MASK,   0b010,       SMP6_POS),
        SMP6_28P5        = mskd_t(      SMP6_MASK,   0b011,       SMP6_POS),
        SMP6_41P5        = mskd_t(      SMP6_MASK,   0b100,       SMP6_POS),
        SMP6_55P5        = mskd_t(      SMP6_MASK,   0b101,       SMP6_POS),
        SMP6_71P5        = mskd_t(      SMP6_MASK,   0b110,       SMP6_POS),
        SMP6_239P5       = mskd_t(      SMP6_MASK,   0b111,       SMP6_POS),
        SMP7_1P5         = mskd_t(      SMP7_MASK,   0b000,       SMP7_POS),
        SMP7_7P5         = mskd_t(      SMP7_MASK,   0b001,       SMP7_POS),
        SMP7_13P5        = mskd_t(      SMP7_MASK,   0b010,       SMP7_POS),
        SMP7_28P5        = mskd_t(      SMP7_MASK,   0b011,       SMP7_POS),
        SMP7_41P5        = mskd_t(      SMP7_MASK,   0b100,       SMP7_POS),
        SMP7_55P5        = mskd_t(      SMP7_MASK,   0b101,       SMP7_POS),
        SMP7_71P5        = mskd_t(      SMP7_MASK,   0b110,       SMP7_POS),
        SMP7_239P5       = mskd_t(      SMP7_MASK,   0b111,       SMP7_POS),
        SMP8_1P5         = mskd_t(      SMP8_MASK,   0b000,       SMP8_POS),
        SMP8_7P5         = mskd_t(      SMP8_MASK,   0b001,       SMP8_POS),
        SMP8_13P5        = mskd_t(      SMP8_MASK,   0b010,       SMP8_POS),
        SMP8_28P5        = mskd_t(      SMP8_MASK,   0b011,       SMP8_POS),
        SMP8_41P5        = mskd_t(      SMP8_MASK,   0b100,       SMP8_POS),
        SMP8_55P5        = mskd_t(      SMP8_MASK,   0b101,       SMP8_POS),
        SMP8_71P5        = mskd_t(      SMP8_MASK,   0b110,       SMP8_POS),
        SMP8_239P5       = mskd_t(      SMP8_MASK,   0b111,       SMP8_POS),
        SMP9_1P5         = mskd_t(      SMP9_MASK,   0b000,       SMP9_POS),
        SMP9_9P5         = mskd_t(      SMP9_MASK,   0b001,       SMP9_POS),
        SMP9_13P5        = mskd_t(      SMP9_MASK,   0b010,       SMP9_POS),
        SMP9_28P5        = mskd_t(      SMP9_MASK,   0b011,       SMP9_POS),
        SMP9_41P5        = mskd_t(      SMP9_MASK,   0b100,       SMP9_POS),
        SMP9_55P5        = mskd_t(      SMP9_MASK,   0b101,       SMP9_POS),
        SMP9_91P5        = mskd_t(      SMP9_MASK,   0b110,       SMP9_POS),
        SMP9_239P5       = mskd_t(      SMP9_MASK,   0b111,       SMP9_POS);
};  // struct Smpr2
    using smpr2_t = Reg<uint32_t, Smpr2>;
          smpr2_t   smpr2;


    static const uint32_t   JOFR_MASK = 0xfff;
    uint32_t    jofr1;
    uint32_t    jofr2;
    uint32_t    jofr3;
    uint32_t    jofr4;


    static const uint32_t   ADC_HTR_MASK = 0xfff;
    uint32_t    htr;


    static const uint32_t   ADC_LTR_MASK = 0xfff;
    uint32_t    ltr;


    struct Sqr1 {
        using              pos_t = Pos<uint32_t, Sqr1>;
        static constexpr   pos_t
                SQ13_POS = pos_t( 0),
                SQ14_POS = pos_t( 5),
                SQ15_POS = pos_t(10),
                SQ16_POS = pos_t(15),
                   L_POS = pos_t(20);

        static const uint32_t
               SQ13_MASK =      0x1FU,
               SQ14_MASK =      0x1FU,
               SQ15_MASK =      0x1FU,
               SQ16_MASK =      0x1FU,
                  L_MASK =       0xFU;

        using   mskd_t = Mskd<uint32_t, Sqr1>;
        using   shft_t = Shft<uint32_t, Sqr1>;

        REGBITS_MSKD_RANGE("Adc::Sqr1",
                           SQ13,
                           sq13,
                           SQ13_MASK,
                           SQ13_POS,
                           SQ13_MASK);
        REGBITS_MSKD_RANGE("Adc::Sqr1",
                           SQ14,
                           sq14,
                           SQ14_MASK,
                           SQ14_POS,
                           SQ14_MASK);
        REGBITS_MSKD_RANGE("Adc::Sqr1",
                           SQ15,
                           sq15,
                           SQ15_MASK,
                           SQ15_POS,
                           SQ15_MASK);
        REGBITS_MSKD_RANGE("Adc::Sqr1",
                           SQ16,
                           sq16,
                           SQ16_MASK,
                           SQ16_POS,
                           SQ16_MASK);
        REGBITS_MSKD_RANGE("Adc::Sqr1",
                           L,
                           l,
                           L_MASK,
                           L_POS,
                           L_MASK);
    };  // struct Sqr1
    using sqr1_t = Reg<uint32_t, Sqr1>;
          sqr1_t   sqr1;


    struct Sqr2 {
        using              pos_t = Pos<uint32_t, Sqr2>;
        static constexpr   pos_t
                 SQ7_POS = pos_t( 0),
                 SQ8_POS = pos_t( 5),
                 SQ9_POS = pos_t(10),
                SQ10_POS = pos_t(15),
                SQ11_POS = pos_t(20),
                SQ12_POS = pos_t(25);

        static const uint32_t
                SQ7_MASK =      0x1FU,
                SQ8_MASK =      0x1FU,
                SQ9_MASK =      0x1FU,
               SQ10_MASK =      0x1FU,
               SQ11_MASK =      0x1FU,
               SQ12_MASK =      0x1FU;

        using   mskd_t = Mskd<uint32_t, Sqr2>;
        using   shft_t = Shft<uint32_t, Sqr2>;

        REGBITS_MSKD_RANGE("Adc::Sqr2",
                           SQ7,
                           sq7,
                           SQ7_MASK,
                           SQ7_POS,
                           SQ7_MASK);
        REGBITS_MSKD_RANGE("Adc::Sqr2",
                           SQ8,
                           sq8,
                           SQ8_MASK,
                           SQ8_POS,
                           SQ8_MASK);
        REGBITS_MSKD_RANGE("Adc::Sqr2",
                           SQ9,
                           sq9,
                           SQ9_MASK,
                           SQ9_POS,
                           SQ9_MASK);
        REGBITS_MSKD_RANGE("Adc::Sqr2",
                           SQ10,
                           sq10,
                           SQ10_MASK,
                           SQ10_POS,
                           SQ10_MASK);
        REGBITS_MSKD_RANGE("Adc::Sqr2",
                           SQ11,
                           sq11,
                           SQ11_MASK,
                           SQ11_POS,
                           SQ11_MASK);
        REGBITS_MSKD_RANGE("Adc::Sqr2",
                           SQ12,
                           sq12,
                           SQ12_MASK,
                           SQ12_POS,
                           SQ12_MASK);
    };  // struct Sqr2
    using sqr2_t = Reg<uint32_t, Sqr2>;
          sqr2_t   sqr2;


    struct Sqr3 {
        using              pos_t = Pos<uint32_t, Sqr3>;
        static constexpr   pos_t
                SQ1_POS = pos_t( 0),
                SQ2_POS = pos_t( 5),
                SQ3_POS = pos_t(10),
                SQ4_POS = pos_t(15),
                SQ5_POS = pos_t(20),
                SQ6_POS = pos_t(25);

        static const uint32_t
               SQ1_MASK =       0x1FU,
               SQ2_MASK =       0x1FU,
               SQ3_MASK =       0x1FU,
               SQ4_MASK =       0x1FU,
               SQ5_MASK =       0x1FU,
               SQ6_MASK =       0x1FU;

        using   mskd_t = Mskd<uint32_t, Sqr3>;
        using   shft_t = Shft<uint32_t, Sqr3>;

        REGBITS_MSKD_RANGE("Adc::Sqr3",
                           SQ1,
                           sq1,
                           SQ2_MASK,
                           SQ1_POS,
                           SQ1_MASK);
        REGBITS_MSKD_RANGE("Adc::Sqr3",
                           SQ2,
                           sq2,
                           SQ3_MASK,
                           SQ2_POS,
                           SQ2_MASK);
        REGBITS_MSKD_RANGE("Adc::Sqr3",
                           SQ3,
                           sq3,
                           SQ4_MASK,
                           SQ3_POS,
                           SQ3_MASK);
        REGBITS_MSKD_RANGE("Adc::Sqr3",
                           SQ4,
                           sq4,
                           SQ5_MASK,
                           SQ4_POS,
                           SQ4_MASK);
        REGBITS_MSKD_RANGE("Adc::Sqr3",
                           SQ5,
                           sq5,
                           SQ6_MASK,
                           SQ5_POS,
                           SQ5_MASK);
        REGBITS_MSKD_RANGE("Adc::Sqr3",
                           SQ6,
                           sq6,
                           SQ6_MASK,
                           SQ6_POS,
                           SQ6_MASK);
    };  // struct Sqr3
    using sqr3_t = Reg<uint32_t, Sqr3>;
          sqr3_t   sqr3;


    struct Jsqr {
        using              pos_t = Pos<uint32_t, Jsqr>;
        static constexpr   pos_t
                JSQ1_POS = pos_t( 0),
                JSQ2_POS = pos_t( 5),
                JSQ3_POS = pos_t(10),
                JSQ4_POS = pos_t(15),
                  JL_POS = pos_t(20);

        static const uint32_t
               JSQ1_MASK =      0x1FU,
               JSQ2_MASK =      0x1FU,
               JSQ3_MASK =      0x1FU,
               JSQ4_MASK =      0x1FU,
                 JL_MASK =       0x3U;

        using              mskd_t = Mskd<uint32_t, Jsqr>;
        using              shft_t = Shft<uint32_t, Jsqr>;

        static constexpr   mskd_t
        JL_1_CNVRS       = mskd_t(         JL_MASK,  0b00,           JL_POS),
        JL_2_CNVRS       = mskd_t(         JL_MASK,  0b01,           JL_POS),
        JL_3_CNVRS       = mskd_t(         JL_MASK,  0b10,           JL_POS),
        JL_4_CNVRS       = mskd_t(         JL_MASK,  0b11,           JL_POS);

        REGBITS_MSKD_RANGE("Adc::Jsqr",
                           JSQ1,
                           jsq1,
                           JSQ1_MASK,
                           JSQ1_POS,
                           JSQ1_MASK);
        REGBITS_MSKD_RANGE("Adc::Jsqr",
                           JSQ2,
                           jsq2,
                           JSQ2_MASK,
                           JSQ2_POS,
                           JSQ2_MASK);
        REGBITS_MSKD_RANGE("Adc::Jsqr",
                           JSQ3,
                           jsq3,
                           JSQ3_MASK,
                           JSQ3_POS,
                           JSQ3_MASK);
        REGBITS_MSKD_RANGE("Adc::Jsqr",
                           JSQ4,
                           jsq4,
                           JSQ4_MASK,
                           JSQ4_POS,
                           JSQ4_MASK);
        REGBITS_MSKD_RANGE("Adc::Jsqr",
                           JL,
                           Jl,
                           JL_MASK,
                           JL_POS,
                           JL_MASK);
    };  // struct Jsqr
    using jsqr_t = Reg<uint32_t, Jsqr>;
          jsqr_t   jsqr;


    static const uint32_t   JDR_MASK = 0xffff;
    uint32_t    jdr1;
    uint32_t    jdr2;
    uint32_t    jdr3;
    uint32_t    jdr4;


    union {
        struct {    // little-endian
            uint16_t    data    ;
            uint16_t    adc2data;
        }                             ;
        uint32_t                    dr;
    }                               dr;

};  // struct Adc
static_assert(sizeof(Adc) == 80, "sizeof(Adc) != 80");



struct Dma {
    struct Isr {
        using              pos_t = Pos<uint32_t, Isr>;
        static constexpr   pos_t
                GIF1_POS = pos_t( 0),
               TCIF1_POS = pos_t( 1),
               HTIF1_POS = pos_t( 2),
               TEIF1_POS = pos_t( 3),
                GIF2_POS = pos_t( 4),
               TCIF2_POS = pos_t( 5),
               HTIF2_POS = pos_t( 6),
               TEIF2_POS = pos_t( 7),
                GIF3_POS = pos_t( 8),
               TCIF3_POS = pos_t( 9),
               HTIF3_POS = pos_t(10),
               TEIF3_POS = pos_t(11),
                GIF4_POS = pos_t(12),
               TCIF4_POS = pos_t(13),
               HTIF4_POS = pos_t(14),
               TEIF4_POS = pos_t(15),
                GIF5_POS = pos_t(16),
               TCIF5_POS = pos_t(17),
               HTIF5_POS = pos_t(18),
               TEIF5_POS = pos_t(19),
                GIF6_POS = pos_t(20),
               TCIF6_POS = pos_t(21),
               HTIF6_POS = pos_t(22),
               TEIF6_POS = pos_t(23),
                GIF7_POS = pos_t(24),
               TCIF7_POS = pos_t(25),
               HTIF7_POS = pos_t(26),
               TEIF7_POS = pos_t(27);

        using              bits_t = Bits<uint32_t, Isr>;
        static constexpr   bits_t
        GIF1             = bits_t(1,         GIF1_POS),
        TCIF1            = bits_t(1,        TCIF1_POS),
        HTIF1            = bits_t(1,        HTIF1_POS),
        TEIF1            = bits_t(1,        TEIF1_POS),
        GIF2             = bits_t(1,         GIF2_POS),
        TCIF2            = bits_t(1,        TCIF2_POS),
        HTIF2            = bits_t(1,        HTIF2_POS),
        TEIF2            = bits_t(1,        TEIF2_POS),
        GIF3             = bits_t(1,         GIF3_POS),
        TCIF3            = bits_t(1,        TCIF3_POS),
        HTIF3            = bits_t(1,        HTIF3_POS),
        TEIF3            = bits_t(1,        TEIF3_POS),
        GIF4             = bits_t(1,         GIF4_POS),
        TCIF4            = bits_t(1,        TCIF4_POS),
        HTIF4            = bits_t(1,        HTIF4_POS),
        TEIF4            = bits_t(1,        TEIF4_POS),
        GIF5             = bits_t(1,         GIF5_POS),
        TCIF5            = bits_t(1,        TCIF5_POS),
        HTIF5            = bits_t(1,        HTIF5_POS),
        TEIF5            = bits_t(1,        TEIF5_POS),
        GIF6             = bits_t(1,         GIF6_POS),
        TCIF6            = bits_t(1,        TCIF6_POS),
        HTIF6            = bits_t(1,        HTIF6_POS),
        TEIF6            = bits_t(1,        TEIF6_POS),
        GIF7             = bits_t(1,         GIF7_POS),
        TCIF7            = bits_t(1,        TCIF7_POS),
        HTIF7            = bits_t(1,        HTIF7_POS),
        TEIF7            = bits_t(1,        TEIF7_POS);
    };  // struct Isr
    using isr_t = Reg<uint32_t, Isr>;
          isr_t   isr;


    struct Ifcr {
        using              pos_t = Pos<uint32_t, Ifcr>;
        static constexpr   pos_t
               CGIF1_POS = pos_t( 0),
              CTCIF1_POS = pos_t( 1),
              CHTIF1_POS = pos_t( 2),
              CTEIF1_POS = pos_t( 3),
               CGIF2_POS = pos_t( 4),
              CTCIF2_POS = pos_t( 5),
              CHTIF2_POS = pos_t( 6),
              CTEIF2_POS = pos_t( 7),
               CGIF3_POS = pos_t( 8),
              CTCIF3_POS = pos_t( 9),
              CHTIF3_POS = pos_t(10),
              CTEIF3_POS = pos_t(11),
               CGIF4_POS = pos_t(12),
              CTCIF4_POS = pos_t(13),
              CHTIF4_POS = pos_t(14),
              CTEIF4_POS = pos_t(15),
               CGIF5_POS = pos_t(16),
              CTCIF5_POS = pos_t(17),
              CHTIF5_POS = pos_t(18),
              CTEIF5_POS = pos_t(19),
               CGIF6_POS = pos_t(20),
              CTCIF6_POS = pos_t(21),
              CHTIF6_POS = pos_t(22),
              CTEIF6_POS = pos_t(23),
               CGIF7_POS = pos_t(24),
              CTCIF7_POS = pos_t(25),
              CHTIF7_POS = pos_t(26),
              CTEIF7_POS = pos_t(27);

        using              bits_t = Bits<uint32_t, Ifcr>;
        static constexpr   bits_t
        CGIF1            = bits_t(1,        CGIF1_POS),
        CTCIF1           = bits_t(1,       CTCIF1_POS),
        CHTIF1           = bits_t(1,       CHTIF1_POS),
        CTEIF1           = bits_t(1,       CTEIF1_POS),
        CGIF2            = bits_t(1,        CGIF2_POS),
        CTCIF2           = bits_t(1,       CTCIF2_POS),
        CHTIF2           = bits_t(1,       CHTIF2_POS),
        CTEIF2           = bits_t(1,       CTEIF2_POS),
        CGIF3            = bits_t(1,        CGIF3_POS),
        CTCIF3           = bits_t(1,       CTCIF3_POS),
        CHTIF3           = bits_t(1,       CHTIF3_POS),
        CTEIF3           = bits_t(1,       CTEIF3_POS),
        CGIF4            = bits_t(1,        CGIF4_POS),
        CTCIF4           = bits_t(1,       CTCIF4_POS),
        CHTIF4           = bits_t(1,       CHTIF4_POS),
        CTEIF4           = bits_t(1,       CTEIF4_POS),
        CGIF5            = bits_t(1,        CGIF5_POS),
        CTCIF5           = bits_t(1,       CTCIF5_POS),
        CHTIF5           = bits_t(1,       CHTIF5_POS),
        CTEIF5           = bits_t(1,       CTEIF5_POS),
        CGIF6            = bits_t(1,        CGIF6_POS),
        CTCIF6           = bits_t(1,       CTCIF6_POS),
        CHTIF6           = bits_t(1,       CHTIF6_POS),
        CTEIF6           = bits_t(1,       CTEIF6_POS),
        CGIF7            = bits_t(1,        CGIF7_POS),
        CTCIF7           = bits_t(1,       CTCIF7_POS),
        CHTIF7           = bits_t(1,       CHTIF7_POS),
        CTEIF7           = bits_t(1,       CTEIF7_POS);
    };  // struct Ifcr
    using ifcr_t = Reg<uint32_t, Ifcr>;
          ifcr_t   ifcr;

};  // struct Dma
static_assert(sizeof(Dma) == 8, "sizeof(Dma) != 8");



struct DmaChannel {
    struct Ccr {
        using              pos_t = Pos<uint32_t, Ccr>;
        static constexpr   pos_t
                  EN_POS = pos_t( 0),
                TCIE_POS = pos_t( 1),
                HTIE_POS = pos_t( 2),
                TEIE_POS = pos_t( 3),
                 DIR_POS = pos_t( 4),
                CIRC_POS = pos_t( 5),
                PINC_POS = pos_t( 6),
                MINC_POS = pos_t( 7),
               PSIZE_POS = pos_t( 8),
               MSIZE_POS = pos_t(10),
                  PL_POS = pos_t(12),
             MEM2MEM_POS = pos_t(14);

        using              bits_t = Bits<uint32_t, Ccr>;
        static constexpr   bits_t
        EN               = bits_t(1,           EN_POS),
        TCIE             = bits_t(1,         TCIE_POS),
        HTIE             = bits_t(1,         HTIE_POS),
        TEIE             = bits_t(1,         TEIE_POS),
        DIR              = bits_t(1,          DIR_POS),
        DIR_PERIPH2MEM   = bits_t(0,          DIR_POS),
        DIR_MEM2PERIPH   = bits_t(1,          DIR_POS),
        CIRC             = bits_t(1,         CIRC_POS),
        PINC             = bits_t(1,         PINC_POS),
        MINC             = bits_t(1,         MINC_POS),
        MEM2MEM          = bits_t(1,      MEM2MEM_POS);

        static const uint32_t
              PSIZE_MASK =       0x3U,
              MSIZE_MASK =       0x3U,
                 PL_MASK =       0x3U;

        using              mskd_t = Mskd<uint32_t, Ccr>;
        static constexpr   mskd_t
        PL_LOW           = mskd_t(      PL_MASK,     0b00,        PL_POS),
        PL_MEDIUM        = mskd_t(      PL_MASK,     0b01,        PL_POS),
        PL_HIGH          = mskd_t(      PL_MASK,     0b10,        PL_POS),
        PL_VERY_HIGH     = mskd_t(      PL_MASK,     0b11,        PL_POS),
        MSIZE_8_BITS     = mskd_t(      MSIZE_MASK,  0b00,        MSIZE_POS),
        MSIZE_16_BITS    = mskd_t(      MSIZE_MASK,  0b01,        MSIZE_POS),
        MSIZE_32_BITS    = mskd_t(      MSIZE_MASK,  0b10,        MSIZE_POS),
        PSIZE_8_BITS     = mskd_t(      PSIZE_MASK,  0b00,        PSIZE_POS),
        PSIZE_16_BITS    = mskd_t(      PSIZE_MASK,  0b01,        PSIZE_POS),
        PSIZE_32_BITS    = mskd_t(      PSIZE_MASK,  0b10,        PSIZE_POS);
    };  // struct Ccr
    using ccr_t = Reg<uint32_t, Ccr>;
          ccr_t   ccr;


    uint32_t    ndt,
                pa,
                ma;

};  // struct DmaChannel
static_assert(sizeof(DmaChannel) == 16, "sizeof(DmaChannel) != 16");



// timers
//


// timer types
//

// check to include private stm32f103xb_tim.hxx from here
#define STM32F103XB_TIM_HXX_INCLUDE

struct Tim
{
#include "stm32f103xb_tim.hxx"
};
static_assert(sizeof(Tim) == 84, "sizeof(Tim) != 84");


// timers classes
//

struct AdvTim_1
{
#define STM32F103XB_ADV_TIM_1
#include "stm32f103xb_tim.hxx"
#undef STM32F103XB_ADV_TIM_1
};  // struct AdvTim_1
static_assert(sizeof(AdvTim_1) == 84, "sizeof(AdvTim_1) != 84");

struct GenTim_2_3_4
{
#define STM32F103XB_GEN_TIM_2_3_4
#include "stm32f103xb_tim.hxx"
#undef STM32F103XB_GEN_TIM_2_3_4
};  // struct GenTim_2_3_4
static_assert(sizeof(GenTim_2_3_4) == 84, "sizeof(GenTim_2_3_4) != 84");

#undef STM32F103XB_TIM_HXX_INCLUDE



struct Usb {
    struct Epr {
        using              pos_t = Pos<uint32_t, Epr>;
        static constexpr   pos_t
                  EA_POS = pos_t( 0),
             STAT_TX_POS = pos_t( 4),
             DTOG_TX_POS = pos_t( 6),
              CTR_TX_POS = pos_t( 7),
             EP_KIND_POS = pos_t( 8),
             EP_TYPE_POS = pos_t( 9),
               SETUP_POS = pos_t(11),
             STAT_RX_POS = pos_t(12),
             DTOG_RX_POS = pos_t(14),
              CTR_RX_POS = pos_t(15);

        using              bits_t = Bits<uint32_t, Epr>;
        static constexpr   bits_t
        // DTOX_xX mskd_t for clear-by-write-0-unchange-1 and toggle semantics
    //  DTOG_TX          = bits_t(1,      DTOG_TX_POS),
        CTR_TX           = bits_t(1,       CTR_TX_POS),
        DBL_BUF          = bits_t(1,      EP_KIND_POS),
        STATUS_OUT       = bits_t(1,      EP_KIND_POS),
        SETUP            = bits_t(1,        SETUP_POS),
    //  DTOG_RX          = bits_t(1,      DTOG_RX_POS),
        CTR_RX           = bits_t(1,       CTR_RX_POS);

        static const uint32_t
                 EA_MASK = 0xf,
            STAT_TX_MASK = 0x3,
            DTOG_TX_MASK = 0x1,
             CTR_TX_MASK = 0x1,
            EP_KIND_MASK = 0x1,
            EP_TYPE_MASK = 0x3,
            STAT_RX_MASK = 0x3,
            DTOG_RX_MASK = 0x1,
             CTR_RX_MASK = 0x1;

        using   mskd_t = Mskd<uint32_t, Epr>;
        using   shft_t = Shft<uint32_t, Epr>;

        REGBITS_MSKD_RANGE("Epr::Ea",
                           EA,
                           ea,
                           EA_MASK,
                           EA_POS,
                           EA_MASK);

        static constexpr   mskd_t
        STAT_TX_DISABLED    = mskd_t(STAT_TX_MASK,   0b00,      STAT_TX_POS),
        STAT_TX_STALL       = mskd_t(STAT_TX_MASK,   0b01,      STAT_TX_POS),
        STAT_TX_NAK         = mskd_t(STAT_TX_MASK,   0b10,      STAT_TX_POS),
        STAT_TX_VALID       = mskd_t(STAT_TX_MASK,   0b11,      STAT_TX_POS),
        DTOG_TX_DATA0       = mskd_t(DTOG_TX_MASK,   0b0,       DTOG_TX_POS),
        DTOG_TX_DATA1       = mskd_t(DTOG_TX_MASK,   0b1,       DTOG_TX_POS),
        EP_TYPE_BULK        = mskd_t(EP_TYPE_MASK,   0b00,      EP_TYPE_POS),
        EP_TYPE_CONTROL     = mskd_t(EP_TYPE_MASK,   0b01,      EP_TYPE_POS),
        EP_TYPE_ISO         = mskd_t(EP_TYPE_MASK,   0b10,      EP_TYPE_POS),
        EP_TYPE_INTERRUPT   = mskd_t(EP_TYPE_MASK,   0b11,      EP_TYPE_POS),
        STAT_RX_DISABLED    = mskd_t(STAT_RX_MASK,   0b00,      STAT_RX_POS),
        STAT_RX_STALL       = mskd_t(STAT_RX_MASK,   0b01,      STAT_RX_POS),
        STAT_RX_NAK         = mskd_t(STAT_RX_MASK,   0b10,      STAT_RX_POS),
        STAT_RX_VALID       = mskd_t(STAT_RX_MASK,   0b11,      STAT_RX_POS),
        DTOG_RX_DATA0       = mskd_t(DTOG_RX_MASK,   0b0,       DTOG_RX_POS),
        DTOG_RX_DATA1       = mskd_t(DTOG_RX_MASK,   0b1,       DTOG_RX_POS);
    };  // struct Epr

    struct epr_t : public Reg<uint32_t, Epr> {
        using   bits_t = Bits<uint32_t, Epr>;
        using   mskd_t = Mskd<uint32_t, Epr>;

        // not automatically inherited from base class
        void operator=(const Bits<uint32_t, Epr>  bits) volatile {
            Reg<uint32_t, Epr>::operator=(bits);
        }

        void write(
        const mskd_t    rw_bits)
        volatile
        {
            // don't modify read/write bits or toggle toggle-only bits
            Reg<uint32_t, Epr>  current = *this;

            // clear toggle-only bits so as to not toggle (also read-only)
                          // must use mskd_t's with all bits set
            current.clr(  Epr::STAT_TX_VALID
                        | Epr::DTOG_TX_DATA1
                        | Epr::SETUP
                        | Epr::STAT_RX_VALID
                        | Epr::DTOG_RX_DATA1);

            // set two possible clear bits so by default will not clear
            current.set(Epr::CTR_TX | Epr::CTR_RX);

            // insert desired masked bits
            current /= rw_bits;

            *this = current;
        }

        void clear(
        const bits_t    clear_bits)
        volatile
        {
            // don't modify read/write bits or toggle toggle-only bits
            Reg<uint32_t, Epr>  current = *this;

            // clear toggle-only bits so as to not toggle (also read-only)
                          // must use mskd_ts with all bits set
            current.clr(  Epr::STAT_TX_VALID
                        | Epr::DTOG_TX_DATA1
                        | Epr::SETUP
                        | Epr::STAT_RX_VALID
                        | Epr::DTOG_RX_DATA1);

            // set two possible clear bits so by default will not clear
            // no, assume caller will set only desired bits to clear
            // current.set(Epr::CTR_TX | Epr::CTR_RX);

            // clear one or both bits to be cleared
            current.clr(clear_bits);

            // write back to register
            *this = current;
        }

        // not automatically inherited from base class
        void operator/=(const Mskd<uint32_t, Epr>  mskd)
        volatile
        {
            Reg<uint32_t, Epr>::operator/=(mskd);
        }

        // not automatically inherited from base class
        void operator=(const uint32_t   word)
        volatile
        {
            Reg<uint32_t, Epr>::operator=(word);
        }


        // toggle-only bits
        //
        // exclusive-or current with new
        //  was     arg     write   becomes
        //  0       0       0       0
        //  0       1       1       1
        //  1       0       1       0
        //  1       1       0       1


        void stat_tx(
        const mskd_t    tx_stat)
        volatile
        {
            // don't modify read/write bits, clear clear-only bits, or
            // toggle other toggle-only bits
            Reg<uint32_t, Epr>  current = *this;

            // clear bits which should not be toggled, cleared, or written
                          // must use mskd_t's with all bits set
            current.clr(  Epr::DTOG_TX_DATA1
                        | Epr::CTR_TX
                        | Epr::SETUP
                        | Epr::STAT_RX_VALID
                        | Epr::DTOG_RX_DATA1
                        | Epr::CTR_RX       );

            // XOR new stat bits
            current.flp(tx_stat);

            // write back to register, toggling stat bits to desired value
            *this = current;
        }


        void stat_rx(
        const mskd_t    rx_stat)
        volatile
        {
            // don't modify read/write bits, clear clear-only bits, or
            // toggle other toggle-only bits
            Reg<uint32_t, Epr>  current = *this;

            // clear bits which should not be toggled, cleared, or written
                          // must use mskd_t's with all bits set
            current.clr(  Epr::STAT_TX_VALID
                        | Epr::DTOG_TX_DATA1
                        | Epr::CTR_TX
                        | Epr::SETUP
                        | Epr::DTOG_RX_DATA1
                        | Epr::CTR_RX   );

            // XOR new stat bits
            current.flp(rx_stat);

            // write back to register, toggling stat bits to desired value
            *this = current;
        }


        void stat_tx_rx(
        const mskd_t    stat)
        volatile
        {
            // don't modify read/write bits, clear clear-only bits, or
            // toggle other toggle-only bits
            Reg<uint32_t, Epr>  current = *this;

            // clear bits which should not be toggled, cleared, or written
                          // must use mskd_t's with all bits set
            current.clr(  Epr::DTOG_TX_DATA1
                        | Epr::CTR_TX
                        | Epr::SETUP
                        | Epr::DTOG_RX_DATA1
                        | Epr::CTR_RX       );

            // XOR new stat bits
            current.flp(stat);

            // write back to register, toggling stat bits to desired value
            *this = current;
        }
    };  // struct epr_t

    static const uint32_t   NUM_ENDPOINT_REGS = 8;
    private:
    epr_t   _eprs[NUM_ENDPOINT_REGS];
    public:
    REGBITS_ARRAY_RANGE("Eprn",
                        EPRN,
                        eprn,
                        epr_t,
                        _eprs,
                        NUM_ENDPOINT_REGS);


    private:
    uint32_t    _reserved[8];
    public:


    struct Cntr {
        using              pos_t = Pos<uint32_t, Cntr>;
        static constexpr   pos_t
                FRES_POS = pos_t( 0),
                PDWN_POS = pos_t( 1),
             LP_MODE_POS = pos_t( 2),
               FSUSP_POS = pos_t( 3),
              RESUME_POS = pos_t( 4),
               ESOFM_POS = pos_t( 8),
                SOFM_POS = pos_t( 9),
              RESETM_POS = pos_t(10),
               SUSPM_POS = pos_t(11),
               WKUPM_POS = pos_t(12),
                ERRM_POS = pos_t(13),
             PMAOVRM_POS = pos_t(14),
                CTRM_POS = pos_t(15);

        using              bits_t = Bits<uint32_t, Cntr>;
        static constexpr   bits_t
        FRES             = bits_t(1,         FRES_POS),
        PDWN             = bits_t(1,         PDWN_POS),
        LP_MODE          = bits_t(1,      LP_MODE_POS),
        FSUSP            = bits_t(1,        FSUSP_POS),
        RESUME           = bits_t(1,       RESUME_POS),
        ESOFM            = bits_t(1,        ESOFM_POS),
        SOFM             = bits_t(1,         SOFM_POS),
        RESETM           = bits_t(1,       RESETM_POS),
        SUSPM            = bits_t(1,        SUSPM_POS),
        WKUPM            = bits_t(1,        WKUPM_POS),
        ERRM             = bits_t(1,         ERRM_POS),
        PMAOVRM          = bits_t(1,      PMAOVRM_POS),
        CTRM             = bits_t(1,         CTRM_POS);
    };  // struct Cntr
    using cntr_t = Reg<uint32_t, Cntr>;
          cntr_t   cntr;


    struct Istr {
        using              pos_t = Pos<uint32_t, Istr>;
        static constexpr   pos_t
               EP_ID_POS = pos_t( 0),
                 DIR_POS = pos_t( 4),
                ESOF_POS = pos_t( 8),
                 SOF_POS = pos_t( 9),
               RESET_POS = pos_t(10),
                SUSP_POS = pos_t(11),
                WKUP_POS = pos_t(12),
                 ERR_POS = pos_t(13),
              PMAOVR_POS = pos_t(14),
                 CTR_POS = pos_t(15);

        using              bits_t = Bits<uint32_t, Istr>;
        static constexpr   bits_t
        DIR              = bits_t(1,          DIR_POS),
        DIR_OUT          = bits_t(1,          DIR_POS),
        ESOF             = bits_t(1,         ESOF_POS),
        SOF              = bits_t(1,          SOF_POS),
        RESET            = bits_t(1,        RESET_POS),
        SUSP             = bits_t(1,         SUSP_POS),
        WKUP             = bits_t(1,         WKUP_POS),
        ERR              = bits_t(1,          ERR_POS),
        PMAOVR           = bits_t(1,       PMAOVR_POS),
        CTR              = bits_t(1,          CTR_POS);

        static const uint32_t
              EP_ID_MASK =       0xF;

        using      mskd_t = Mskd<uint32_t, Istr>;
        using      shft_t = Shft<uint32_t, Istr>;

        REGBITS_MSKD_RANGE("istr::EpId",
                           EP_ID,
                           ep_id,
                           EP_ID_MASK,
                           EP_ID_POS,
                           EP_ID_MASK);
    };  // struct Istr
    using istr_t = Reg<uint32_t, Istr>;
          istr_t   istr;


    struct Fnr {
        using              pos_t = Pos<uint32_t, Fnr>;
        static constexpr   pos_t
                  FN_POS = pos_t( 0),
                LSOF_POS = pos_t(11),
                 LCK_POS = pos_t(13),
                RXDM_POS = pos_t(14),
                RXDP_POS = pos_t(15);

        using              bits_t = Bits<uint32_t, Fnr>;
        static constexpr   bits_t
        LCK              = bits_t(1,          LCK_POS),
        RXDM             = bits_t(1,         RXDM_POS),
        RXDP             = bits_t(1,         RXDP_POS);

        static const uint32_t
                 FN_MASK =     0x7FFU,
               LSOF_MASK =       0x3U;

        using      shft_t = Shft<uint32_t, Fnr>;

        static constexpr   shft_t
        FN   = shft_t(  FN_MASK,   FN_POS),
        LSOF = shft_t(LSOF_MASK, LSOF_POS);
    };  // struct Fnr
    using fnr_t = Reg<uint32_t, Fnr>;
          fnr_t   fnr;


    struct Daddr {
        using              pos_t = Pos<uint32_t, Daddr>;
        static constexpr   pos_t
                 ADD_POS = pos_t( 0),
                  EF_POS = pos_t( 7);

        using              bits_t = Bits<uint32_t, Daddr>;
        static constexpr   bits_t
        EF               = bits_t(1,           EF_POS);

        static const uint32_t
                ADD_MASK =      0x7FU;

        using      mskd_t = Mskd<uint32_t, Daddr>;
        using      shft_t = Shft<uint32_t, Daddr>;

        REGBITS_MSKD_RANGE("daddr::add",
                           ADD,
                           add,
                           ADD_MASK,
                           ADD_POS,
                           ADD_MASK);
    };  // struct Daddr
    using daddr_t = Reg<uint32_t, Daddr>;
          daddr_t   daddr;


                uint16_t    btable;
    private:    uint16_t    _btable_high_bits;   public:

};  // struct Usb
static_assert(sizeof(Usb) == 84, "sizeof(Usb) != 84");



struct UsbBufDesc {
                uint16_t    addr_tx;
    private:    uint16_t    _addr_tx_high_bits;  public:


    struct CountTx {
        using              pos_t = Pos<uint32_t, CountTx>;
        static constexpr   pos_t
            COUNT_0_POS = pos_t( 0),
            COUNT_1_POS = pos_t(16);

        static const uint32_t
                COUNT_0_MASK =  0x3ff,
                COUNT_1_MASK =  0x3ff;

        using      mskd_t = Mskd<uint32_t, CountTx>;
        using      shft_t = Shft<uint32_t, CountTx>;

        REGBITS_MSKD_RANGE("UsbBufDesc::CountTx",
                           COUNT_0,
                           count_0,
                           COUNT_0_MASK,
                           COUNT_0_POS,
                           COUNT_0_MASK);
        REGBITS_MSKD_RANGE("UsbBufDesc::CountTx",
                           COUNT_1,
                           count_1,
                           COUNT_1_MASK,
                           COUNT_1_POS,
                           COUNT_1_MASK);
    };  // struct CountTx
    using count_tx_t = Reg<uint32_t, CountTx>;
          count_tx_t    count_tx;


                uint16_t    addr_rx;
    private:    uint16_t    _addr_rx_high_bits;  public:


    struct CountRx {
        using              pos_t = Pos<uint32_t, CountRx>;
        static constexpr   pos_t
                COUNT_0_POS = pos_t( 0),
            NUM_BLOCK_0_POS = pos_t(10),
               BLSIZE_0_POS = pos_t(15),
                COUNT_1_POS = pos_t(16),
            NUM_BLOCK_1_POS = pos_t(26),
               BLSIZE_1_POS = pos_t(31);

        using              bits_t = Bits<uint32_t, CountRx>;
        static constexpr   bits_t
            BLSIZE_0 = bits_t(1, BLSIZE_0_POS),
            BLSIZE_1 = bits_t(1, BLSIZE_1_POS);

        static const uint32_t
                    COUNT_0_MASK = 0x3ff,
                NUM_BLOCK_0_MASK = 0x01f,
                   BLSIZE_0_MASK = 0x001,
                    COUNT_1_MASK = 0x3ff,
                NUM_BLOCK_1_MASK = 0x01f,
                   BLSIZE_1_MASK = 0x001;

        using      mskd_t = Mskd<uint32_t, CountRx>;
        using      shft_t = Shft<uint32_t, CountRx>;

        static constexpr    mskd_t
        BLSIZE_0_2_BYTES  = mskd_t(BLSIZE_0_MASK, 0, BLSIZE_0_POS),
        BLSIZE_0_32_BYTES = mskd_t(BLSIZE_0_MASK, 1, BLSIZE_0_POS),
        BLSIZE_1_2_BYTES  = mskd_t(BLSIZE_1_MASK, 0, BLSIZE_1_POS),
        BLSIZE_1_32_BYTES = mskd_t(BLSIZE_1_MASK, 1, BLSIZE_1_POS);

        REGBITS_MSKD_RANGE("UsbBufDesc::CountRx",
                           COUNT_0,
                           count_0,
                           COUNT_0_MASK,
                           COUNT_0_POS,
                           COUNT_0_MASK);
        REGBITS_MSKD_RANGE("UsbBufDesc::CountRx",
                           NUM_BLOCK_0,
                           num_block_0,
                           NUM_BLOCK_0_MASK,
                           NUM_BLOCK_0_POS,
                           NUM_BLOCK_0_MASK);
        REGBITS_MSKD_RANGE("UsbBufDesc::CountRx",
                           COUNT_1,
                           count_1,
                           COUNT_1_MASK,
                           COUNT_1_POS,
                           COUNT_1_MASK);
        REGBITS_MSKD_RANGE("UsbBufDesc::CountRx",
                           NUM_BLOCK_1,
                           num_block_1,
                           NUM_BLOCK_1_MASK,
                           NUM_BLOCK_1_POS,
                           NUM_BLOCK_1_MASK);
    };  // struct CountRx

    struct count_rx_t : public Reg<uint32_t, CountRx> {
        // not automatically inherited from base class
        void operator=(const Mskd<uint32_t, CountRx>  mskd) volatile {
            Reg<uint32_t, CountRx>::operator=(mskd);
        }

        void set_num_blocks_0(
        const uint16_t  num_bytes)
        volatile
        {
            if (num_bytes <= 62)
                *this =   CountRx::BLSIZE_0_2_BYTES
                        | CountRx::num_block_0((num_bytes + 1) >> 1);
            else if (num_bytes <= 512)
                *this =   CountRx::BLSIZE_0_32_BYTES
                        | CountRx::num_block_0(    (   ((num_bytes + 31) & ~31)
                                                     - 32)
                                                >> 5);
            // else fail silently
        }

        void set_num_blocks_1(
        const uint16_t  num_bytes)
        volatile
        {
            if (num_bytes <= 62)
                *this =   CountRx::BLSIZE_1_2_BYTES
                        | CountRx::num_block_1((num_bytes + 1) >> 1);
            else if (num_bytes <= 512)
                *this =   CountRx::BLSIZE_1_32_BYTES
                        | CountRx::num_block_1(    (   ((num_bytes + 31) & ~31)
                                                     - 32)
                                                >> 5);
            // else fail silently
        }

        uint16_t num_bytes_0()
        volatile
        {
            uint16_t    num_blocks = this->shifted(CountRx::NUM_BLOCK_0_SHFT);

            if (this->any(CountRx::BLSIZE_0)) return (num_blocks + 1) << 5;
            else                              return  num_blocks      << 1;
        }

        uint16_t num_bytes_1()
        volatile
        {
            uint16_t    num_blocks = this->shifted(CountRx::NUM_BLOCK_1_SHFT);

            if (this->any(CountRx::BLSIZE_1)) return (num_blocks + 1) << 5;
            else                              return  num_blocks      << 1;
        }
    };  // struct count_rx_t
    count_rx_t  count_rx;

};  // struct UsbBufDesc
static_assert(sizeof(UsbBufDesc) == 16, "sizeof(UsbBufDesc) != 16");



struct Spi {
    struct Cr1 {
        using              pos_t = Pos<uint32_t, Cr1>;
        static constexpr   pos_t
                CPHA_POS = pos_t( 0),
                CPOL_POS = pos_t( 1),
                MSTR_POS = pos_t( 2),
                  BR_POS = pos_t( 3),
                 SPE_POS = pos_t( 6),
            LSBFIRST_POS = pos_t( 7),
                 SSI_POS = pos_t( 8),
                 SSM_POS = pos_t( 9),
              RXONLY_POS = pos_t(10),
                 DFF_POS = pos_t(11),
             CRCNEXT_POS = pos_t(12),
               CRCEN_POS = pos_t(13),
              BIDIOE_POS = pos_t(14),
            BIDIMODE_POS = pos_t(15);

        using              bits_t = Bits<uint32_t, Cr1>;
        static constexpr   bits_t
        CPHA             = bits_t(1,         CPHA_POS),
        CPHA_IDLE_0      = bits_t(0,         CPHA_POS),
        CPHA_IDLE_1      = bits_t(1,         CPHA_POS),
        CPOL             = bits_t(1,         CPOL_POS),
        CPOL_1ST_EDGE    = bits_t(0,         CPOL_POS),
        CPOL_2ND_EDGE    = bits_t(1,         CPOL_POS),
        MSTR             = bits_t(1,         MSTR_POS),
        SLAVE            = bits_t(0,         MSTR_POS),
        MASTER           = bits_t(1,         MSTR_POS),
        SPE              = bits_t(1,          SPE_POS),
        LSBFIRST         = bits_t(1,     LSBFIRST_POS),
        MSBFIRST         = bits_t(0,     LSBFIRST_POS),
        SSI              = bits_t(1,          SSI_POS),
        SSM              = bits_t(1,          SSM_POS),
        RXONLY           = bits_t(1,       RXONLY_POS),
        DFF              = bits_t(1,          DFF_POS),
        DFF_8            = bits_t(0,          DFF_POS),
        DFF_16           = bits_t(1,          DFF_POS),
        CRCNEXT          = bits_t(1,      CRCNEXT_POS),
        CRCEN            = bits_t(1,        CRCEN_POS),
        BIDIOE           = bits_t(1,       BIDIOE_POS),
        BIDIMODE         = bits_t(1,     BIDIMODE_POS);

        static const uint32_t
                 BR_MASK =       0x7U;

        using              mskd_t = Mskd<uint32_t, Cr1>;
        using              shft_t = Shft<uint32_t, Cr1>;

        REGBITS_MSKD_RANGE("Spi::Cr1",
                           BR,
                           br,
                           BR_MASK,
                           BR_POS,
                           BR_MASK);

        static constexpr   mskd_t
        BR_DIV_2         = mskd_t(         BR_MASK,  0b000,          BR_POS),
        BR_DIV_4         = mskd_t(         BR_MASK,  0b001,          BR_POS),
        BR_DIV_8         = mskd_t(         BR_MASK,  0b010,          BR_POS),
        BR_DIV_16        = mskd_t(         BR_MASK,  0b011,          BR_POS),
        BR_DIV_32        = mskd_t(         BR_MASK,  0b100,          BR_POS),
        BR_DIV_64        = mskd_t(         BR_MASK,  0b101,          BR_POS),
        BR_DIV_128       = mskd_t(         BR_MASK,  0b110,          BR_POS),
        BR_DIV_256       = mskd_t(         BR_MASK,  0b111,          BR_POS);
    };  // struct Cr1
    using cr1_t = Reg<uint32_t, Cr1>;
          cr1_t   cr1;


    struct Cr2 {
        using              pos_t = Pos<uint32_t, Cr2>;
        static constexpr   pos_t
             RXDMAEN_POS = pos_t( 0),
             TXDMAEN_POS = pos_t( 1),
                SSOE_POS = pos_t( 2),
               ERRIE_POS = pos_t( 5),
              RXNEIE_POS = pos_t( 6),
               TXEIE_POS = pos_t( 7);

        using              bits_t = Bits<uint32_t, Cr2>;
        static constexpr   bits_t
        RXDMAEN          = bits_t(1,      RXDMAEN_POS),
        TXDMAEN          = bits_t(1,      TXDMAEN_POS),
        SSOE             = bits_t(1,         SSOE_POS),
        ERRIE            = bits_t(1,        ERRIE_POS),
        RXNEIE           = bits_t(1,       RXNEIE_POS),
        TXEIE            = bits_t(1,        TXEIE_POS);
    };  // struct Cr2
    using cr2_t = Reg<uint32_t, Cr2>;
          cr2_t   cr2;


    struct Sr {
        using              pos_t = Pos<uint32_t, Sr>;
        static constexpr   pos_t
                RXNE_POS = pos_t( 0),
                 TXE_POS = pos_t( 1),
              CHSIDE_POS = pos_t( 2),
                 UDR_POS = pos_t( 3),
              CRCERR_POS = pos_t( 4),
                MODF_POS = pos_t( 5),
                 OVR_POS = pos_t( 6),
                 BSY_POS = pos_t( 7);

        using              bits_t = Bits<uint32_t, Sr>;
        static constexpr   bits_t
        RXNE             = bits_t(1,         RXNE_POS),
        TXE              = bits_t(1,          TXE_POS),
        CHSIDE           = bits_t(1,       CHSIDE_POS),
        UDR              = bits_t(1,          UDR_POS),
        CRCERR           = bits_t(1,       CRCERR_POS),
        MODF             = bits_t(1,         MODF_POS),
        OVR              = bits_t(1,          OVR_POS),
        BSY              = bits_t(1,          BSY_POS);
    };  // struct Sr
    using sr_t = Reg<uint32_t, Sr>;
          sr_t   sr;


    union {
        uint8_t     dr8 ;
        uint16_t    dr16;
        uint32_t    _dr ;
    };


                uint16_t    crcpr;
    private:    uint16_t    _crcpr_high_bits;    public:


                uint16_t    rxcrcr;
    private:    uint16_t    _rxcrcr_high_bits;   public:


                uint16_t    txcrcr;
    private:    uint16_t    _txcrcr_high_bits;   public:


    struct I2scfgr {
        using              pos_t = Pos<uint32_t, I2scfgr>;
        static constexpr   pos_t
              I2SMOD_POS = pos_t(11);

        using              bits_t = Bits<uint32_t, I2scfgr>;
        static constexpr   bits_t
        I2SMOD           = bits_t(1,       I2SMOD_POS);
    };  // struct I2scfgr
    using i2scfgr_t = Reg<uint32_t, I2scfgr>;
          i2scfgr_t   i2scfgr;

};  // struct Spi
static_assert(sizeof(Spi) == 32, "sizeof(Spi) != 32");



struct I2c {
    struct Cr1 {
        using              pos_t = Pos<uint32_t, Cr1>;
        static constexpr   pos_t
                  PE_POS = pos_t( 0),
               SMBUS_POS = pos_t( 1),
             SMBTYPE_POS = pos_t( 3),
               ENARP_POS = pos_t( 4),
               ENPEC_POS = pos_t( 5),
                ENGC_POS = pos_t( 6),
           NOSTRETCH_POS = pos_t( 7),
               START_POS = pos_t( 8),
                STOP_POS = pos_t( 9),
                 ACK_POS = pos_t(10),
                 POS_POS = pos_t(11),
                 PEC_POS = pos_t(12),
               ALERT_POS = pos_t(13),
               SWRST_POS = pos_t(15);

        using              bits_t = Bits<uint32_t, Cr1>;
        static constexpr   bits_t
        PE               = bits_t(1,           PE_POS),
        SMBUS            = bits_t(1,        SMBUS_POS),
        SMBTYPE          = bits_t(1,      SMBTYPE_POS),
        ENARP            = bits_t(1,        ENARP_POS),
        ENPEC            = bits_t(1,        ENPEC_POS),
        ENGC             = bits_t(1,         ENGC_POS),
        NOSTRETCH        = bits_t(1,    NOSTRETCH_POS),
        START            = bits_t(1,        START_POS),
        STOP             = bits_t(1,         STOP_POS),
        ACK              = bits_t(1,          ACK_POS),
        POS              = bits_t(1,          POS_POS),
        PEC              = bits_t(1,          PEC_POS),
        ALERT            = bits_t(1,        ALERT_POS),
        SWRST            = bits_t(1,        SWRST_POS);
    };  // struct Cr1
    using cr1_t = Reg<uint32_t, Cr1>;
          cr1_t   cr1;


    struct Cr2 {
        using              pos_t = Pos<uint32_t, Cr2>;
        static constexpr   pos_t
                FREQ_POS = pos_t( 0),
             ITERREN_POS = pos_t( 8),
             ITEVTEN_POS = pos_t( 9),
             ITBUFEN_POS = pos_t(10),
               DMAEN_POS = pos_t(11),
                LAST_POS = pos_t(12);

        using              bits_t = Bits<uint32_t, Cr2>;
        static constexpr   bits_t
        ITERREN          = bits_t(1,      ITERREN_POS),
        ITEVTEN          = bits_t(1,      ITEVTEN_POS),
        ITBUFEN          = bits_t(1,      ITBUFEN_POS),
        DMAEN            = bits_t(1,        DMAEN_POS),
        LAST             = bits_t(1,         LAST_POS);

        static const uint32_t
               FREQ_MASK =      0x3FU;

        using              mskd_t = Mskd<uint32_t, Cr2>;
        using              shft_t = Shft<uint32_t, Cr2>;

        REGBITS_MSKD_RANGE("I2c::Cr2",
                           FREQ,
                           freq,
                           FREQ_MASK,
                           FREQ_POS,
                           FREQ_MASK);
    };  // struct Cr2
    using cr2_t = Reg<uint32_t, Cr2>;
          cr2_t   cr2;


    struct Oar1 {
        using              pos_t = Pos<uint32_t, Oar1>;
        static constexpr   pos_t
                ADD0_POS = pos_t( 0),
                ADD7_POS = pos_t( 1),
               ADD10_POS = pos_t( 8),
               BIT14_POS = pos_t(14),
             ADDMODE_POS = pos_t(15);

        using              bits_t = Bits<uint32_t, Oar1>;
        static constexpr   bits_t
        BIT14            = bits_t(1,       BIT14_POS),
        ADDMODE          = bits_t(1,     ADDMODE_POS),
        ADDMODE_7        = bits_t(0,     ADDMODE_POS),
        ADDMODE_10       = bits_t(1,     ADDMODE_POS);

        static const uint32_t
                ADD7_MASK = 0x3f,
               ADD10_MASK = 0x03;

        using              mskd_t = Mskd<uint32_t, Oar1>;
        using              shft_t = Shft<uint32_t, Oar1>;

        REGBITS_MSKD_RANGE("I2c::Oar1",
                           ADD7,
                           add7,
                           ADD7_MASK,
                           ADD7_POS,
                           ADD7_MASK);
        REGBITS_MSKD_RANGE("I2c::Oar1",
                           ADD10,
                           add10,
                           ADD10_MASK,
                           ADD10_POS,
                           ADD10_MASK);
    };  // struct Oar1
    using oar1_t = Reg<uint32_t, Oar1>;
          oar1_t   oar1;


    struct Oar2 {
        using              pos_t = Pos<uint32_t, Oar2>;
        static constexpr   pos_t
              ENDUAL_POS = pos_t( 0),
                ADD2_POS = pos_t( 1);

        using              bits_t = Bits<uint32_t, Oar2>;
        static constexpr   bits_t
        ENDUAL           = bits_t(1,       ENDUAL_POS);

        static const uint32_t
               ADD2_MASK =      0x7F;

        using              mskd_t = Mskd<uint32_t, Oar2>;
        using              shft_t = Shft<uint32_t, Oar2>;

        REGBITS_MSKD_RANGE("I2c::Oar2",
                           ADD2,
                           add2,
                           ADD2_MASK,
                           ADD2_POS,
                           ADD2_MASK);
    };  // struct Oar2
    using oar2_t = Reg<uint32_t, Oar2>;
          oar2_t   oar2;


    uint32_t    dr;


    struct Sr1 {
        using              pos_t = Pos<uint32_t, Sr1>;
        static constexpr   pos_t
                  SB_POS = pos_t( 0),
                ADDR_POS = pos_t( 1),
                 BTF_POS = pos_t( 2),
               ADD10_POS = pos_t( 3),
               STOPF_POS = pos_t( 4),
                RXNE_POS = pos_t( 6),
                 TXE_POS = pos_t( 7),
                BERR_POS = pos_t( 8),
                ARLO_POS = pos_t( 9),
                  AF_POS = pos_t(10),
                 OVR_POS = pos_t(11),
              PECERR_POS = pos_t(12),
             TIMEOUT_POS = pos_t(14),
            SMBALERT_POS = pos_t(15);

        using              bits_t = Bits<uint32_t, Sr1>;
        static constexpr   bits_t
        SB               = bits_t(1,           SB_POS),
        ADDR             = bits_t(1,         ADDR_POS),
        BTF              = bits_t(1,          BTF_POS),
        ADD10            = bits_t(1,        ADD10_POS),
        STOPF            = bits_t(1,        STOPF_POS),
        RXNE             = bits_t(1,         RXNE_POS),
        TXE              = bits_t(1,          TXE_POS),
        BERR             = bits_t(1,         BERR_POS),
        ARLO             = bits_t(1,         ARLO_POS),
        AF               = bits_t(1,           AF_POS),
        OVR              = bits_t(1,          OVR_POS),
        PECERR           = bits_t(1,       PECERR_POS),
        TIMEOUT          = bits_t(1,      TIMEOUT_POS),
        SMBALERT         = bits_t(1,     SMBALERT_POS);
    };  // struct Sr1
    using sr1_t = Reg<uint32_t, Sr1>;
          sr1_t   sr1;



    struct Sr2 {
        using              pos_t = Pos<uint32_t, Sr2>;
        static constexpr   pos_t
                 MSL_POS = pos_t( 0),
                BUSY_POS = pos_t( 1),
                 TRA_POS = pos_t( 2),
             GENCALL_POS = pos_t( 4),
          SMBDEFAULT_POS = pos_t( 5),
             SMBHOST_POS = pos_t( 6),
               DUALF_POS = pos_t( 7),
                 PEC_POS = pos_t( 8);

        using              bits_t = Bits<uint32_t, Sr2>;
        static constexpr   bits_t
        MSL              = bits_t(1,          MSL_POS),
        BUSY             = bits_t(1,         BUSY_POS),
        TRA              = bits_t(1,          TRA_POS),
        GENCALL          = bits_t(1,      GENCALL_POS),
        SMBDEFAULT       = bits_t(1,   SMBDEFAULT_POS),
        SMBHOST          = bits_t(1,      SMBHOST_POS),
        DUALF            = bits_t(1,        DUALF_POS);

        static const uint32_t
                PEC_MASK =      0xFF;

        using              mskd_t = Mskd<uint32_t, Sr2>;
        using              shft_t = Shft<uint32_t, Sr2>;

        REGBITS_MSKD_RANGE("I2c::Sr2",
                           PEC,
                           pec,
                           PEC_MASK,
                           PEC_POS,
                           PEC_MASK);
    };  // struct Sr2
    using sr2_t = Reg<uint32_t, Sr2>;
          sr2_t   sr2;



    struct Ccr {
        using              pos_t = Pos<uint32_t, Ccr>;
        static constexpr   pos_t
                 CCR_POS = pos_t( 0),
                DUTY_POS = pos_t(14),
                  FS_POS = pos_t(15);

        using              bits_t = Bits<uint32_t, Ccr>;
        static constexpr   bits_t
        DUTY             = bits_t(1,         DUTY_POS),
        FS               = bits_t(1,           FS_POS);

        static const uint32_t
                CCR_MASK =     0xFFF;

        using              mskd_t = Mskd<uint32_t, Ccr>;
        using              shft_t = Shft<uint32_t, Ccr>;

        REGBITS_MSKD_RANGE("I2c::Ccr",
                           CCR,
                           ccr,
                           CCR_MASK,
                           CCR_POS,
                           CCR_MASK);
    };  // struct Ccr
    using ccr_t = Reg<uint32_t, Ccr>;
          ccr_t   ccr;


    struct Trise {
        using              pos_t = Pos<uint32_t, Trise>;
        static constexpr   pos_t
               TRISE_POS = pos_t( 0);

        static const uint32_t
              TRISE_MASK =      0x3F;

        using              mskd_t = Mskd<uint32_t, Trise>;
        using              shft_t = Shft<uint32_t, Trise>;

        REGBITS_MSKD_RANGE("I2c::Trise",
                           TRISE,
                           trise,
                           TRISE_MASK,
                           TRISE_POS,
                           TRISE_MASK);
    };  // struct Trise
    using trise_t = Reg<uint32_t, Trise>;
          trise_t   trise;

};  // struct I2c
static_assert(sizeof(I2c) == 36, "sizeof(I2c) != 36");



struct Usart {
    struct Sr {
        using              pos_t = Pos<uint32_t, Sr>;
        static constexpr   pos_t
                  PE_POS = pos_t( 0),
                  FE_POS = pos_t( 1),
                  NE_POS = pos_t( 2),
                 ORE_POS = pos_t( 3),
                IDLE_POS = pos_t( 4),
                RXNE_POS = pos_t( 5),
                  TC_POS = pos_t( 6),
                 TXE_POS = pos_t( 7),
                 LBD_POS = pos_t( 8),
                 CTS_POS = pos_t( 9);

        using              bits_t = Bits<uint32_t, Sr>;
        static constexpr   bits_t
        PE               = bits_t(1,           PE_POS),
        FE               = bits_t(1,           FE_POS),
        NE               = bits_t(1,           NE_POS),
        ORE              = bits_t(1,          ORE_POS),
        IDLE             = bits_t(1,         IDLE_POS),
        RXNE             = bits_t(1,         RXNE_POS),
        TC               = bits_t(1,           TC_POS),
        TXE              = bits_t(1,          TXE_POS),
        LBD              = bits_t(1,          LBD_POS),
        CTS              = bits_t(1,          CTS_POS);
    };  // struct Sr
    using sr_t = Reg<uint32_t, Sr>;
          sr_t   sr;


                uint8_t      dr;
    private:    uint8_t     _dr_high_bytes[3];  public:


    struct Brr {
        using              pos_t = Pos<uint32_t, Brr>;
        static constexpr   pos_t
        FRACTION_POS = pos_t( 0),
        MANTISSA_POS = pos_t( 4);

        static const uint32_t
        FRACTION_MASK =       0xFU,
        MANTISSA_MASK =     0xFFFU;

        using              mskd_t = Mskd<uint32_t, Brr>;
        using              shft_t = Shft<uint32_t, Brr>;

        REGBITS_MSKD_RANGE("Usart::Brr",
                           FRACTION,
                           fraction,
                           FRACTION_MASK,
                           FRACTION_POS,
                           FRACTION_MASK);
        REGBITS_MSKD_RANGE("Usart::Brr",
                           MANTISSA,
                           mantissa,
                           MANTISSA_MASK,
                           MANTISSA_POS,
                           MANTISSA_MASK);
    };  // struct Brr
    using brr_t = Reg<uint32_t, Brr>;
          brr_t  brr;


    struct Cr1 {
        using              pos_t = Pos<uint32_t, Cr1>;
        static constexpr   pos_t
                 SBK_POS = pos_t( 0),
                 RWU_POS = pos_t( 1),
                  RE_POS = pos_t( 2),
                  TE_POS = pos_t( 3),
              IDLEIE_POS = pos_t( 4),
              RXNEIE_POS = pos_t( 5),
                TCIE_POS = pos_t( 6),
               TXEIE_POS = pos_t( 7),
                PEIE_POS = pos_t( 8),
                  PS_POS = pos_t( 9),
                 PCE_POS = pos_t(10),
                WAKE_POS = pos_t(11),
                   M_POS = pos_t(12),
                  UE_POS = pos_t(13);

        using              bits_t = Bits<uint32_t, Cr1>;
        static constexpr   bits_t
        SBK              = bits_t(1,          SBK_POS),
        RWU              = bits_t(1,          RWU_POS),
        RE               = bits_t(1,           RE_POS),
        TE               = bits_t(1,           TE_POS),
        IDLEIE           = bits_t(1,       IDLEIE_POS),
        RXNEIE           = bits_t(1,       RXNEIE_POS),
        TCIE             = bits_t(1,         TCIE_POS),
        TXEIE            = bits_t(1,        TXEIE_POS),
        PEIE             = bits_t(1,         PEIE_POS),
        PS               = bits_t(1,           PS_POS),
        PS_EVEN          = bits_t(0,           PS_POS),
        PS_ODD           = bits_t(1,           PS_POS),
        PCE              = bits_t(1,          PCE_POS),
        WAKE             = bits_t(1,         WAKE_POS),
        M                = bits_t(1,            M_POS),
        M_8_DATA_BITS    = bits_t(0,            M_POS),
        M_9_DATA_BITS    = bits_t(1,            M_POS),
        UE               = bits_t(1,           UE_POS);
    };  // struct Cr1
    using cr1_t = Reg<uint32_t, Cr1>;
          cr1_t   cr1;


    struct Cr2 {
        using              pos_t = Pos<uint32_t, Cr2>;
        static constexpr   pos_t
                 ADD_POS = pos_t( 0),
                LBDL_POS = pos_t( 5),
               LBDIE_POS = pos_t( 6),
                LBCL_POS = pos_t( 8),
                CPHA_POS = pos_t( 9),
                CPOL_POS = pos_t(10),
               CLKEN_POS = pos_t(11),
                STOP_POS = pos_t(12),
               LINEN_POS = pos_t(14);

        using              bits_t = Bits<uint32_t, Cr2>;
        static constexpr   bits_t
        LBDL             = bits_t(1,         LBDL_POS),
        LBDIE            = bits_t(1,        LBDIE_POS),
        LBCL             = bits_t(1,         LBCL_POS),
        CPHA             = bits_t(1,         CPHA_POS),
        CPOL             = bits_t(1,         CPOL_POS),
        CLKEN            = bits_t(1,        CLKEN_POS),
        LINEN            = bits_t(1,        LINEN_POS);

        static const uint32_t
                ADD_MASK =       0xFU,
               STOP_MASK =       0x3U;

        using              mskd_t = Mskd<uint32_t, Cr2>;
        using              shft_t = Shft<uint32_t, Cr2>;

        static constexpr   mskd_t
        STOP_1_BIT       = mskd_t(STOP_MASK,     0b00,         STOP_POS),
        STOP_0_5_BITS    = mskd_t(STOP_MASK,     0b01,         STOP_POS),
        STOP_2_BITS      = mskd_t(STOP_MASK,     0b10,         STOP_POS),
        STOP_1_5_BITS    = mskd_t(STOP_MASK,     0b11,         STOP_POS);

        REGBITS_MSKD_RANGE("Usart::Cr2",
                           ADD,
                           add,
                           ADD_MASK,
                           ADD_POS,
                           ADD_MASK);
    };  // struct Cr2
    using cr2_t = Reg<uint32_t, Cr2>;
          cr2_t   cr2;


    struct Cr3 {
        using              pos_t = Pos<uint32_t, Cr3>;
        static constexpr   pos_t
                 EIE_POS = pos_t( 0),
                IREN_POS = pos_t( 1),
                IRLP_POS = pos_t( 2),
               HDSEL_POS = pos_t( 3),
                NACK_POS = pos_t( 4),
                SCEN_POS = pos_t( 5),
                DMAR_POS = pos_t( 6),
                DMAT_POS = pos_t( 7),
                RTSE_POS = pos_t( 8),
                CTSE_POS = pos_t( 9),
               CTSIE_POS = pos_t(10);

        using              bits_t = Bits<uint32_t, Cr3>;
        static constexpr   bits_t
        EIE              = bits_t(1,          EIE_POS),
        IREN             = bits_t(1,         IREN_POS),
        IRLP             = bits_t(1,         IRLP_POS),
        HDSEL            = bits_t(1,        HDSEL_POS),
        NACK             = bits_t(1,         NACK_POS),
        SCEN             = bits_t(1,         SCEN_POS),
        DMAR             = bits_t(1,         DMAR_POS),
        DMAT             = bits_t(1,         DMAT_POS),
        RTSE             = bits_t(1,         RTSE_POS),
        CTSE             = bits_t(1,         CTSE_POS),
        CTSIE            = bits_t(1,        CTSIE_POS);
    };  // struct Cr3
    using cr3_t = Reg<uint32_t, Cr3>;
          cr3_t   cr3;


    struct Gtpr {
        using              pos_t = Pos<uint32_t, Gtpr>;
        static constexpr   pos_t
                 PSC_POS = pos_t( 0),
                  GT_POS = pos_t( 8);

        static const uint32_t
                PSC_MASK =      0xFFU,
                 GT_MASK =      0xFFU;

        using              mskd_t = Mskd<uint32_t, Gtpr>;
        using              shft_t = Shft<uint32_t, Gtpr>;

        REGBITS_MSKD_RANGE("Usart::Gtpr",
                           PSC,
                           psc,
                           PSC_MASK,
                           PSC_POS,
                           PSC_MASK);
        REGBITS_MSKD_RANGE("Usart::Gtpr",
                           GT,
                           gt,
                           GT_MASK,
                           GT_POS,
                           GT_MASK);
    };  // struct Gtpr
    using gtpr_t = Reg<uint32_t, Gtpr>;
          gtpr_t   gtpr;
};  // struct Usart
static_assert(sizeof(Usart) == 28, "sizeof(Usart) != 28");



struct ElecSig {
                uint16_t    flash_size;
    private:    uint16_t    _flash_size_high_bits;   public:

    private:    uint32_t    _unused;    public:

                uint16_t    u_id_15_0,
                            u_id_31_16;
                uint32_t    u_id_63_32,
                            u_id_95_64;
};  // struct ElecSig
static_assert(sizeof(ElecSig) == 20, "sizeof(ElecSig) != 20");



struct Flash {
    struct Acr {
        using              pos_t = Pos<uint32_t, Acr>;
        static constexpr   pos_t
             LATENCY_POS = pos_t( 0),
              HLFCYA_POS = pos_t( 3),
              PRFTBE_POS = pos_t( 4),
              PRFTBS_POS = pos_t( 5);

        using              bits_t = Bits<uint32_t, Acr>;
        static constexpr   bits_t
        HLFCYA           = bits_t(1,       HLFCYA_POS),
        PRFTBE           = bits_t(1,       PRFTBE_POS),
        PRFTBS           = bits_t(1,       PRFTBS_POS);

        static const uint32_t
            LATENCY_MASK =       0x7U;

        using              mskd_t = Mskd<uint32_t, Acr>;
        static constexpr   mskd_t
        LATENCY_0_WAIT_STATES = mskd_t(LATENCY_MASK, 0b000, LATENCY_POS),
        LATENCY_1_WAIT_STATE  = mskd_t(LATENCY_MASK, 0b001, LATENCY_POS),
        LATENCY_2_WAIT_STATES = mskd_t(LATENCY_MASK, 0b010, LATENCY_POS);
    };  // struct Acr
    using acr_t = Reg<uint32_t, Acr>;
          acr_t   acr;


    uint32_t    keyr,
                optkeyr;


    struct Sr {
        using              pos_t = Pos<uint32_t, Sr>;
        static constexpr   pos_t
                 BSY_POS = pos_t( 0),
               PGERR_POS = pos_t( 2),
            WRPRTERR_POS = pos_t( 4),
                 EOP_POS = pos_t( 5);

        using              bits_t = Bits<uint32_t, Sr>;
        static constexpr   bits_t
        BSY              = bits_t(1,          BSY_POS),
        PGERR            = bits_t(1,        PGERR_POS),
        WRPRTERR         = bits_t(1,     WRPRTERR_POS),
        EOP              = bits_t(1,          EOP_POS);
    };  // struct Sr
    using sr_t = Reg<uint32_t, Sr>;
          sr_t   sr;


    struct Cr {
        using              pos_t = Pos<uint32_t, Cr>;
        static constexpr   pos_t
                  PG_POS = pos_t( 0),
                 PER_POS = pos_t( 1),
                 MER_POS = pos_t( 2),
               OPTPG_POS = pos_t( 4),
               OPTER_POS = pos_t( 5),
                STRT_POS = pos_t( 6),
                LOCK_POS = pos_t( 7),
              OPTWRE_POS = pos_t( 9),
               ERRIE_POS = pos_t(10),
               EOPIE_POS = pos_t(12);

        using              bits_t = Bits<uint32_t, Cr>;
        static constexpr   bits_t
        PG               = bits_t(1,           PG_POS),
        PER              = bits_t(1,          PER_POS),
        MER              = bits_t(1,          MER_POS),
        OPTPG            = bits_t(1,        OPTPG_POS),
        OPTER            = bits_t(1,        OPTER_POS),
        STRT             = bits_t(1,         STRT_POS),
        LOCK             = bits_t(1,         LOCK_POS),
        OPTWRE           = bits_t(1,       OPTWRE_POS),
        ERRIE            = bits_t(1,        ERRIE_POS),
        EOPIE            = bits_t(1,        EOPIE_POS);
    };  // struct Cr
    using cr_t = Reg<uint32_t, Cr>;
          cr_t   cr;


    uint32_t    ar,
                reserved0;


    struct Obr {
        using              pos_t = Pos<uint32_t, Obr>;
        static constexpr   pos_t
              OPTERR_POS = pos_t( 0),
               RDPRT_POS = pos_t( 1),
             IWDG_SW_POS = pos_t( 2),
           nRST_STOP_POS = pos_t( 3),
          nRST_STDBY_POS = pos_t( 4),
               DATA0_POS = pos_t(10),
               DATA1_POS = pos_t(18);

        using              bits_t = Bits<uint32_t, Obr>;
        static constexpr   bits_t
        OPTERR           = bits_t(1,       OPTERR_POS),
        RDPRT            = bits_t(1,        RDPRT_POS),
        IWDG_SW          = bits_t(1,      IWDG_SW_POS),
        nRST_STOP        = bits_t(1,    nRST_STOP_POS),
        nRST_STDBY       = bits_t(1,   nRST_STDBY_POS);

        static const uint32_t
              DATA0_MASK =      0xFFU,
              DATA1_MASK =      0xFFU;

        using   mskd_t = Mskd<uint32_t, Obr>;
        using   shft_t = Shft<uint32_t, Obr>;

        REGBITS_MSKD_RANGE("Flash::Obr",
                           DATA0,
                           data0,
                           DATA0_MASK,
                           DATA0_POS,
                           DATA0_MASK);
        REGBITS_MSKD_RANGE("Flash::Obr",
                           DATA1,
                           data1,
                           DATA1_MASK,
                           DATA1_POS,
                           DATA1_MASK);
    };  // struct Obr
    using obr_t = Reg<uint32_t, Obr>;
          obr_t   obr;


    uint32_t    wrpr;

};  // struct Flash
static_assert(sizeof(Flash) == 36, "sizeof(Flash) != 36");



struct DbgMcu {
    struct Idcode {
        using              pos_t = Pos<uint32_t, Idcode>;
        static constexpr   pos_t
              DEV_ID_POS = pos_t( 0),
              REV_ID_POS = pos_t(16);

        static const uint32_t
             DEV_ID_MASK =     0xFFFU,
             REV_ID_MASK =    0xFFFFU;

        using   mskd_t = Mskd<uint32_t, Idcode>;
        using   shft_t = Shft<uint32_t, Idcode>;

        REGBITS_MSKD_RANGE("DbgMcu::Idcode",
                           DEV_ID,
                           dev_id,
                           DEV_ID_MASK,
                           DEV_ID_POS,
                           DEV_ID_MASK);
        REGBITS_MSKD_RANGE("DbgMcu::Idcode",
                           REV_ID,
                           rev_id,
                           REV_ID_MASK,
                           REV_ID_POS,
                           REV_ID_MASK);
    };  // struct Idcode
    using idcode_t = Reg<uint32_t, Idcode>;
          idcode_t   idcode;

};  // struct DbgMcu
static_assert(sizeof(DbgMcu) == 4, "sizeof(DbgMcu) != 4");



static const uint32_t   PERIPH_BASE         = 0x40000000U;

static const uint32_t   APB1PERIPH_BASE     = PERIPH_BASE              ,
                        APB2PERIPH_BASE     = PERIPH_BASE + 0x00010000U,
                        AHBPERIPH_BASE      = PERIPH_BASE + 0x00020000U;

// APB1 peripherals
static const uint32_t   TIM2_BASE           = APB1PERIPH_BASE + 0x00000000U,
                        TIM3_BASE           = APB1PERIPH_BASE + 0x00000400U,
                        TIM4_BASE           = APB1PERIPH_BASE + 0x00000800U,
                        RTC_BASE            = APB1PERIPH_BASE + 0x00002800U,
                        WWDG_BASE           = APB1PERIPH_BASE + 0x00002C00U,
                        IWDG_BASE           = APB1PERIPH_BASE + 0x00003000U,
                        SPI2_BASE           = APB1PERIPH_BASE + 0x00003800U,
                        USART2_BASE         = APB1PERIPH_BASE + 0x00004400U,
                        USART3_BASE         = APB1PERIPH_BASE + 0x00004800U,
                        I2C1_BASE           = APB1PERIPH_BASE + 0x00005400U,
                        I2C2_BASE           = APB1PERIPH_BASE + 0x00005800U,
                        USB_BASE            = APB1PERIPH_BASE + 0x00005C00U,
                        USB_PMAADDR         = APB1PERIPH_BASE + 0x00006000U,
                        USB_PMASIZE         = 512                          ,
                        CAN1_BASE           = APB1PERIPH_BASE + 0x00006400U,
                        BKP_BASE            = APB1PERIPH_BASE + 0x00006C00U,
                        PWR_BASE            = APB1PERIPH_BASE + 0x00007000U;

// APB2 peripherals
static const uint32_t   AFIO_BASE           = APB2PERIPH_BASE + 0x00000000U,
                        EXTI_BASE           = APB2PERIPH_BASE + 0x00000400U,
                        GPIOA_BASE          = APB2PERIPH_BASE + 0x00000800U,
                        GPIOB_BASE          = APB2PERIPH_BASE + 0x00000C00U,
                        GPIOC_BASE          = APB2PERIPH_BASE + 0x00001000U,
                        GPIOD_BASE          = APB2PERIPH_BASE + 0x00001400U,
                        GPIOE_BASE          = APB2PERIPH_BASE + 0x00001800U,
                        ADC1_BASE           = APB2PERIPH_BASE + 0x00002400U,
                        ADC2_BASE           = APB2PERIPH_BASE + 0x00002800U,
                        TIM1_BASE           = APB2PERIPH_BASE + 0x00002C00U,
                        SPI1_BASE           = APB2PERIPH_BASE + 0x00003000U,
                        USART1_BASE         = APB2PERIPH_BASE + 0x00003800U;

// AHB peripherals
static const uint32_t   DMA1_BASE           = AHBPERIPH_BASE + 0x00000000U,
                        DMA1_CHANNEL1_BASE  = AHBPERIPH_BASE + 0x00000008U,
                        DMA1_CHANNEL2_BASE  = AHBPERIPH_BASE + 0x0000001CU,
                        DMA1_CHANNEL3_BASE  = AHBPERIPH_BASE + 0x00000030U,
                        DMA1_CHANNEL4_BASE  = AHBPERIPH_BASE + 0x00000044U,
                        DMA1_CHANNEL5_BASE  = AHBPERIPH_BASE + 0x00000058U,
                        DMA1_CHANNEL6_BASE  = AHBPERIPH_BASE + 0x0000006CU,
                        DMA1_CHANNEL7_BASE  = AHBPERIPH_BASE + 0x00000080U,
                        RCC_BASE            = AHBPERIPH_BASE + 0x00001000U,
                        FLASH_BASE          = AHBPERIPH_BASE + 0x00002000U,
                        CRC_BASE            = AHBPERIPH_BASE + 0x00003000U;

static const uint32_t   ELEC_SIG_BASE       = 0x1FFFF7E0U;

static const uint32_t   DBGMCU_BASE         = 0xE0042000U;


#define STM32F103XB_PERIPH(TYPE, PERIPH, BASE)      \
    static volatile TYPE* const                     \
    PERIPH = reinterpret_cast<volatile TYPE*>(BASE)

STM32F103XB_PERIPH( Rcc,                rcc,            RCC_BASE          );

STM32F103XB_PERIPH( Gpio,               gpioa,          GPIOA_BASE        );
STM32F103XB_PERIPH( Gpio,               gpiob,          GPIOB_BASE        );
STM32F103XB_PERIPH( Gpio,               gpioc,          GPIOC_BASE        );
STM32F103XB_PERIPH( Gpio,               gpiod,          GPIOD_BASE        );
STM32F103XB_PERIPH( Gpio,               gpioe,          GPIOE_BASE        );

STM32F103XB_PERIPH( Afio,               afio,           AFIO_BASE         );

STM32F103XB_PERIPH( Exti,               exti,           EXTI_BASE         );

STM32F103XB_PERIPH( Adc,                adc1,           ADC1_BASE         );
STM32F103XB_PERIPH( Adc,                adc2,           ADC2_BASE         );

STM32F103XB_PERIPH( Dma,                dma1,           DMA1_BASE         );

STM32F103XB_PERIPH( DmaChannel,         dma1_channel1,  DMA1_CHANNEL1_BASE);
STM32F103XB_PERIPH( DmaChannel,         dma1_channel2,  DMA1_CHANNEL2_BASE);
STM32F103XB_PERIPH( DmaChannel,         dma1_channel3,  DMA1_CHANNEL3_BASE);
STM32F103XB_PERIPH( DmaChannel,         dma1_channel4,  DMA1_CHANNEL4_BASE);
STM32F103XB_PERIPH( DmaChannel,         dma1_channel5,  DMA1_CHANNEL5_BASE);
STM32F103XB_PERIPH( DmaChannel,         dma1_channel6,  DMA1_CHANNEL6_BASE);
STM32F103XB_PERIPH( DmaChannel,         dma1_channel7,  DMA1_CHANNEL7_BASE);

STM32F103XB_PERIPH( AdvTim_1,           adv_tim_1,      TIM1_BASE         );

STM32F103XB_PERIPH( GenTim_2_3_4,       gen_tim_2,      TIM2_BASE         );
STM32F103XB_PERIPH( GenTim_2_3_4,       gen_tim_3,      TIM3_BASE         );
STM32F103XB_PERIPH( GenTim_2_3_4,       gen_tim_4,      TIM4_BASE         );

STM32F103XB_PERIPH( Usb,                usb,            USB_BASE          );

STM32F103XB_PERIPH( Spi,                spi1,           SPI1_BASE         );
STM32F103XB_PERIPH( Spi,                spi2,           SPI2_BASE         );

STM32F103XB_PERIPH( I2c,                i2c1,           I2C1_BASE         );
STM32F103XB_PERIPH( I2c,                i2c2,           I2C2_BASE         );

STM32F103XB_PERIPH( Usart,              usart2,         USART2_BASE       );
STM32F103XB_PERIPH( Usart,              usart3,         USART3_BASE       );
STM32F103XB_PERIPH( Usart,              usart1,         USART1_BASE       );

STM32F103XB_PERIPH( ElecSig,            elec_sig,       ELEC_SIG_BASE     );

STM32F103XB_PERIPH( Flash,              flash,          FLASH_BASE        );

STM32F103XB_PERIPH( DbgMcu,             dbg_mcu,        DBGMCU_BASE       );

#undef STM32F103XB_PERIPH


template <unsigned NUM_ENDPOINTS, unsigned BTABLE_OFFSET> class UsbPmaDescs {
  public:
    REGBITS_ARRAY_RANGE("UsbPmaDescs",
                        EPRN,
                        eprn,
                        UsbBufDesc,
                        reinterpret_cast<UsbBufDesc* const>(  USB_PMAADDR
                                                            + BTABLE_OFFSET),
                        NUM_ENDPOINTS);
    // GCC bug prevents statically initializing protected UsbBufDesc* const
    // member for reuse (works if dynamically constructed on stack).
    // Above works for both, optimally efficient object code.
};

}  // namespace stm32f103xb


namespace arm {
enum class NvicIrqn {
    // Cortex-M3 Processor Exceptions Numbers
    NonMaskableInt   = -14, // 2 Non Maskable Interrupt
    HardFault        = -13, // 3 Cortex-M3 Hard Fault Interrupt
    MemoryManagement = -12, // 4 Cortex-M3 Memory Management Interrupt
    BusFault         = -11, // 5 Cortex-M3 Bus Fault Interrupt
    UsageFault       = -10, // 6 Cortex-M3 Usage Fault Interrupt
    SVCall           =  -5, // 11 Cortex-M3 SV Call Interrupt
    DebugMonitor     =  -4, // 12 Cortex-M3 Debug Monitor Interrupt
    PendSV           =  -2, // 14 Cortex-M3 Pend SV Interrupt
    SysTick          =  -1, // 15 Cortex-M3 System Tick Interrupt

    // STM32 specific Interrupt Numbers
    WWDG             =  0,  // Window WatchDog Interrupt
    PVD              =  1,  // PVD through EXTI Line detection Interrupt
    TAMPER           =  2,  // Tamper Interrupt
    RTC              =  3,  // RTC global Interrupt
    FLASH            =  4,  // FLASH global Interrupt
    RCC              =  5,  // RCC global Interrupt
    EXTI0            =  6,  // EXTI Line0 Interrupt
    EXTI1            =  7,  // EXTI Line1 Interrupt
    EXTI2            =  8,  // EXTI Line2 Interrupt
    EXTI3            =  9,  // EXTI Line3 Interrupt
    EXTI4            = 10,  // EXTI Line4 Interrupt
    DMA1_Channel1    = 11,  // DMA1 Channel 1 global Interrupt
    DMA1_Channel2    = 12,  // DMA1 Channel 2 global Interrupt
    DMA1_Channel3    = 13,  // DMA1 Channel 3 global Interrupt
    DMA1_Channel4    = 14,  // DMA1 Channel 4 global Interrupt
    DMA1_Channel5    = 15,  // DMA1 Channel 5 global Interrupt
    DMA1_Channel6    = 16,  // DMA1 Channel 6 global Interrupt
    DMA1_Channel7    = 17,  // DMA1 Channel 7 global Interrupt
    ADC1_2           = 18,  // ADC1 and ADC2 global Interrupt
    USB_HP_CAN1_TX   = 19,  // USB Device High Priority or CAN1 TX Interrupts
    USB_LP_CAN1_RX0  = 20,  // USB Device Low Priority or CAN1 RX0 Interrupts
    CAN1_RX1         = 21,  // CAN1 RX1 Interrupt
    CAN1_SCE         = 22,  // CAN1 SCE Interrupt
    EXTI9_5          = 23,  // External Line[9:5] Interrupts
    TIM1_BRK         = 24,  // TIM1 Break Interrupt
    TIM1_UP          = 25,  // TIM1 Update Interrupt
    TIM1_TRG_COM     = 26,  // TIM1 Trigger and Commutation Interrupt
    TIM1_CC          = 27,  // TIM1 Capture Compare Interrupt
    TIM2             = 28,  // TIM2 global Interrupt
    TIM3             = 29,  // TIM3 global Interrupt
    TIM4             = 30,  // TIM4 global Interrupt
    I2C1_EV          = 31,  // I2C1 Event Interrupt
    I2C1_ER          = 32,  // I2C1 Error Interrupt
    I2C2_EV          = 33,  // I2C2 Event Interrupt
    I2C2_ER          = 34,  // I2C2 Error Interrupt
    SPI1             = 35,  // SPI1 global Interrupt
    SPI2             = 36,  // SPI2 global Interrupt
    USART1           = 37,  // USART1 global Interrupt
    USART2           = 38,  // USART2 global Interrupt
    USART3           = 39,  // USART3 global Interrupt
    EXTI15_10        = 40,  // External Line[15:10] Interrupts
    RTC_Alarm        = 41,  // RTC Alarm through EXTI Line Interrupt
    USBWakeUp        = 42,  // USB Device WakeUp from suspend through EXTI Line Interrupt
}; // enum NvicIrqn
}  // namespace arm


#endif  // #ifndef STM32F103XB_HXX
