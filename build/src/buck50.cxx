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


#include <cstdint>

#include <core_cm3.hxx>

#include <stm32f103xb.hxx>

#include <sys_tick_timer.hxx>

#include <usb_dev_cdc_acm.hxx>

#if STM32F103XB_MAJOR_VERSION == 1
#if STM32F103XB_MINOR_VERSION  < 3
#warning STM32F103XB_MINOR_VERSION < 3
#endif
#else
#error STM32F103XB_MAJOR_VERSION != 1
#endif


using namespace stm32f103xb      ;
using namespace stm32f10_12357_xx;



#if 1  // (section identification for code browsing)
//
// constants
//

#define _JBLEN      (10)
typedef uint32_t    jmp_buf[_JBLEN];

static const uint32_t   IDENTITY              = 0xea017af5;
static const uint8_t    MAX_BRIDGE_DATA_LEN   = 62        ,
                        MAX_ADC_CHANNEL_NUM   =  7        ,
                        VERSION            [] = {0, 9, 2},
                        CONNECT_SIG_BYTE_0    = 0xf2      ;

// See wait_connect_signature()
// All with high bit set to hopefully avoid ascii control chars
static const unsigned     CONNECT_SIGNATURE_LENGTH = 17;
static const uint8_t      CONNECT_SIGNATURE[CONNECT_SIGNATURE_LENGTH]
                        = {CONNECT_SIG_BYTE_0    ,
                           0x9e, 0xc4, 0xaa, 0xdf,
                           0xd8, 0xca, 0x8f, 0xbd,
                           0xbe, 0xa9, 0xfe, 0x83,
                           0x99, 0xd1, 0xae, 0xeb};

// access USB send buf as uint32_t or uint16_t
static const unsigned   RECV_BUF_UINT32S = UsbDevCdcAcm::CDC_OUT_DATA_SIZE >> 2,
                        SEND_BUF_UINT32S = UsbDevCdcAcm::CDC_IN_DATA_SIZE  >> 2,
                        RECV_BUF_UINT16S = UsbDevCdcAcm::CDC_OUT_DATA_SIZE >> 1,
                        SEND_BUF_UINT16S = UsbDevCdcAcm::CDC_IN_DATA_SIZE  >> 1;

static const unsigned   MAX_TRIGGERS = 256;

// Fixed value. For safety  against infinite loops (interruptible by user
// input over USB from host anyway). Used only by live() (asm analog sampling
// has no check, can't take time to set timer, etc) so not realtime-critical.
// Max conversion time is 252 12MHz cycles, so in 72MHz main CPU clocks ...
uint32_t    ADC_TIMEOUT = 300 * 72 / 12;  // 25 microseconds

extern uint32_t     STORAGE    ,  // from .ld file
                    STORAGE_END;  //  "    "   "

#endif  // #if 1 (constants)




#if 1  // (section identification for code browsing)
//
// types
//

namespace Command {    // enum class would require casting to/from uint8_t
    static const    uint8_t     IDENTITY         =  1,
                                VERSION          =  2,
                                RESET            =  3,
                                LIVE             =  4,
                                HALT             =  5,
                                DIGITAL_SAMPLING =  6,
                                ANALOG_SAMPLING  =  7,
                                SEND_SAMPLES     =  8,
                                PWM              =  9,
                                PARALLEL_BRIDGE  = 10,
                                USART_BRIDGE     = 11,
                                SPI_BRIDGE       = 12,
                                COUNTER          = 13,
                                FLASH_WAIT_PRE   = 14,
                                I2C_BRIDGE       = 15,
                                SERIAL_NUMBER    = 16,
                                BLINK_USER_LED   = 17,
                                CONNECT_SIG      = CONNECT_SIG_BYTE_0;
}

// namespaced static const enum classes would require casting to/from uint8_t
//
namespace HaltCode {
    static const uint8_t
                            SETJMP   = 0,  // setjmp() API
                            NONE     = 0,
                            MEMORY   = 1,
                            DURATION = 2,
                            USB      = 3;
}

namespace SamplingMode {
    static const uint8_t    MHZ_6     =   0 ,
                            IRREGULAR =   1 ,
                            UNIFORM   =   2 ,
                            MHZ_4     =   3 ,
                            ANALOG    = 0x0f,
                            UNSET     = 0xff;
}

namespace InProgress {
                            // no good way to access static consts
                            //   or #defines in asm
                            // manually search for "InProgress::" and
                            //   edit if any change
    static const uint16_t   IDLE         = 0x0000        ,
                            TRIGGER_MASK = 0x00ff        ,
                            TRIGGERING   = 0x0100        ,
                            EXTERN_TRIG  = 0x0200       ,
                            ANALOG       = 0x0400        ,
                            TRIGGERED    = 0x0800        ,
                            SAMPLING     = 0x1000        ,
                            COUNTING     = 0x2000        ,
                            SAMPLING_ETC =   TRIGGER_MASK
                                           | TRIGGERING
                                           | EXTERN_TRIG
                                           | ANALOG
                                           | TRIGGERED
                                           | SAMPLING    ,
                            IN_PROGRESS =    TRIGGERING
                                           | ANALOG
                                           | TRIGGERED
                                           | SAMPLING
                                           | COUNTING    ;
}


namespace AnalogSlope {
    static const uint8_t    NEGATIVE = 0,
                            POSITIVE = 1;
}


// ordered from "best" to "worst"
// USART_n and I2C_n are context dependent, don't conflict with each other
enum class PeriphStatus {
    OK = 0       ,  //  0
    EMPTY        ,  //  1
    HALTED       ,  //  2
    TIMEOUT      ,  //  3
    BUSY         ,  //  4
    OVERRUN      ,  //  5
    ERROR        ,  //  6
    USART_BREAK  ,  //  7
    USART_NOISE  ,  //  8
    USART_PARITY ,  //  9
    USART_FRAMING,  // 10
    I2C_NOSTART  ,  // 11
    I2C_NOADDR   ,  // 12
    I2C_NACK     ,  // 13
    I2C_NOBTF    ,  // 14
    I2C_NORXNE   ,  // 15
    I2C_NOSTOP   ,  // 16
};



struct Trigger {
    union {
        struct {
            uint8_t     mask,
                        pass,
                        fail,
                        bits;
        };
        uint32_t        word;
    };
};



/* Circular buffer. Assume that host will never send message(s) totalling
   more than _SIZE (64 byte CDC_OUT_DATA_SIZE) before firmware consumes
   them by calling flush(), but don't assume that host CDC-ACM driver always
   sends complete message in one USB packet. See fill() which doesn't call
   _fill() if client's requested size is already buffered, so _fill()
   doesn't call UsbDev::recv_done() to unblock USB flow control (NAKs).
   Also assume that host always sends modulo 4 bytes sized packets -- but
   check and handle if not, as per when CDC-ACM startup packets.
*/
class UsbRecv {
  public:
    UsbRecv()
    :
        _begin(0),
        _level(0)
    {}

    uint8_t  byte(unsigned ndx) const;
    uint16_t shrt(unsigned ndx) const;
    uint32_t word(unsigned ndx) const;

    unsigned    fill(unsigned   need);

    void        flush(unsigned  used);  // move inline

  protected:
    static const uint8_t
    _SIZE              = UsbDevCdcAcm::CDC_OUT_DATA_SIZE,    // must be 64
    _MODULO_BYTES_MASK = 0x3f                           ,
    _MODULO_SHRTS_MASK = _MODULO_BYTES_MASK >> 1        ,
    _MODULO_WORDS_MASK = _MODULO_BYTES_MASK >> 2        ;

    void    _fill();

    union {
        uint8_t     _bytes[_SIZE     ];
        uint16_t    _shrts[_SIZE >> 1];
        uint32_t    _words[_SIZE >> 2];
    };
    unsigned        _begin,
                    _level;
};



class Sbrk {
  public:
    Sbrk()
    :   _brk(reinterpret_cast<uint8_t*>(&STORAGE))
    {}

    uint8_t* operator()(
    const unsigned      bytes)
    {
        uint8_t*    current = _brk;
        _brk += (bytes + 3) & 0xfffffffc;  // keep 32 bit aligned
        return current;
    }

  protected:
    uint8_t*    _brk;
};



union AdcLive {
    struct {
        unsigned    channel    : 4,
                    samp_hold  : 4,
                    hysteresis : 8,
                    exponent   : 8,
                    weight     : 8;
    };
    unsigned        word;
};



static const uint8_t    NIBBLE_COUNTS[16] = {0, 1, 1, 2,
                                             1, 2, 2, 3,
                                             1, 2, 2, 3,
                                             2, 3, 3, 4};
INLINE_DECL uint8_t INLINE_ATTR num_bits_set(
const uint8_t       bits)
{
    return NIBBLE_COUNTS[bits >> 4] + NIBBLE_COUNTS[bits & 0xf];
}

#endif  // #if 1 (types)




#if 1  // (section identification for code browsing)
//
// command message layouts
//

namespace gpioa_command {
    static const uint8_t
    COMMAND     = 0,
    GPIO_SPEED  = 1,    // GPIO output max speed
    OPEN_PULL   = 2;    // (0,1) push-pull/open-drain
}


namespace spi_command {
    static const uint8_t
    // usb_recv.bytes(NDX)
    COMMAND     =  0,       // Command::SPI_BRIDGE or (0,1) enable/disable
    MASTER      =  1,       // (0,1) slave,master
    BITS        =  2,       // aggregate of ...

    // bits positions
    XMIT_ONLY   =  5,       // (0,1) xmit&recv,xmit_only
    SELECT      =  4,       // (0,1) software,hardware
    ENDIAN      =  3,       // (0,1) MSB,LSB first
    POLARITY    =  2,       // (0,1) line at idle
    PHASE       =  1,       // (0,1) clock latch edge
    MISO        =  0,       // (0,1) pushpull/opendrain

    // usb_recv.bytes(NDX)
    BAUD        =  3,       // (0-7) clock/pow(2,baud)
    PULL        =  4,       // (0,1,2) floating/up/down
    SPEED       =  5,       // GPIO output max speed
    TX_LEN      =  6,       // data length
    NSS         =  7,       // (0,1,2) floatg/low/active

    // usb_recv.word(NDX)
    NSS_DLAY    =  2,       // gpio, PA-4
    TX_TMOUT    =  3,       // Spi::Sr::TXE timeout
    RATE        =  4,       // master send rate
    RX_WAIT     =  5,       // slave wait contiguous MOSI

    // usb_recv.bytes(NDX)
    CMD_LEN     = 24;       // command length

}  // namespace spi_command


namespace i2c_command {
    static const uint8_t
    // usb_recv.bytes(NDX)
    COMMAND     =  0,   // Command::I2C_BRIDGE or (0,1) enable/disable

    // usb_recv.bytes(NDX)
    BITS        =  1,   // aggregate of ...

    // bits positions
    MASTER      =  3,   // (0,1),slave,master
    STD_FAST    =  2,   // (0,1) I2C standard,fast mode
    DUTY        =  1,   // (0,1) 50-50,16/9 duty cycle
    GEN_CALL    =  0,   // (0,1) disable/enable general call address

    // usb_recv.bytes(NDX)
    GPIO        =  2,   // (2,1,3) 2,10,50 MHz gpio speed
    DEST        =  3,   // I2C destination address
    OAR1        =  4,   // own address 1
    OAR2        =  5,   //  "     "    2 or >127 for "unused"
    DFLT_SIZE   =  6,   // length of default tx data
    RX_SIZE     =  7,   // number of bytes to receive

    // recv_unit16s[NX}
    CCR         =  5,   // ccr field of ccr register

    // usb_recv.word(NDX)
    TIMEOUT     =  3,   // master send timeout

    // usb_recv.bytes(NDX)
    CMD_LEN     = 16,

    // limits
    MAX_TX_DFLT = 16,
    MAX_TX_RX   = 60;

}  // namespace i2c_command


namespace usart_command {
    static const uint8_t
    // usb_recv.bytes(NDX)
    CMD         =  0,   // placeholder/alignment
    BITS_1      =  1,   // aggregate of ...
    BITS_2      =  2,   // aggregate of ...

    // bits_1 positions
    XMIT        =  6,   // (0,1) disabled,enabled
    RECV        =  5,   //   "      "        "
    CLOCK       =  4,   // (0,1) async,synchro
    LENGTH      =  3,   // (0,1) 8,9 bits
    POLRTY      =  2,   // (0,1) line at idle
    PHASE       =  1,   // (0,1) 1st,2nd latch edge
    LSTCLK      =  0,   // (0,1) 1st,2nd latch edge
    // bits_2 positions
    RTS         =  2,   // (0,1) disabled,enabled
    CTS         =  1,   // (0,1) disabled,enabled
    USART       =  0,   // (0,1) PA0-3 or PA8-10

    // usb_recv.bytes(NDX)
    PARITY      =  3,   // (0,2,3) none,even,odd
    STOP        =  4,   // (0-3) 1, 0.5, 2, or 1.5
    SPEED       =  5,   // GPIO output max speed
    TX_DFLT     =  6,   // data to xmit in live() or usart_bridge if  synchro
    RX_LEN      =  7,   // max rx data len

    // usb_recv.shrt(NDX)
    BAUD        =  5,   // 16 bits

    // usb_recv.word(NDX)
    TX_TMO      =  3,   // TX timeout
    RX_WAIT     =  4,   // concat time
    RATE        =  5,   // rate time

    // usb_recv.bytes(NDX)
    CMD_LEN     = 24;   // command length
}  // namespace usart_command


namespace live_command {
    static const uint8_t
    // usb_recv.bytes(NDX)
    CMD           =  0,  // placeholder/alignment
    GPIO          =  1,  // (0,1) gpioa disabled/enabled
    USART         =  2,  // (0,1) usart disabled/enabled
    SPI           =  3,  // (0,1) spi   disabled/enabled
    I2C           =  4,  // (0,1) i2c   disabled/enabled
    ADCS          =  5,  // (0,8) adcs  enabled (count)

    // usb_recv.word(NDX)
    DURATION_LO   =  2,  // (0-0xffffffff)
    DURATION_HI   =  3,  // (0-0xffffffff)
    RATE_LO       =  4,  // (0-0xffffffff)
    RATE_HI       =  5,  // (0-0xffffffff)

    // usb_recv.bytes(NDX)
          CMD_LEN = 24;  // command length

}  // namespace live_command


namespace adc_command {
    static const uint8_t
    // usb_recv.bytes(NDX)
    CHAN_RATE = 0,  // (((0-7) channel) << 4) | ((0-15 ) sample rate code)
    HYST      = 1,  // (0-255) hysteresis
    EXPN      = 2,  // (0-255) filter exponent
    WGHT      = 3,  // (0-255) filter weight
    CMD_LEN   = 4;

}  // namespace adc_command

#endif  // #if 1 (command message layouts_




#if 1  // (section identification for code browsing)
//
// globals
//

// ordered by size to avoid unnecessary memory-wasting padding

uint8_t     send_buf[UsbDevCdcAcm::CDC_IN_DATA_SIZE ] __attribute__
                                                      ((aligned(4)));
UsbRecv     usb_recv;
uint32_t    *send_uint32s = reinterpret_cast<uint32_t*>(send_buf);
uint16_t    *send_uint16s = reinterpret_cast<uint16_t*>(send_buf);

jmp_buf     longjump_buf = {0};

// 128 sufficient size from analysis of assembly output and experimentation
// add 16 (words) for safety
uint32_t     STACK[144] __attribute__((section (".stack")));

UsbDevCdcAcm        usb_dev;
arm::SysTickTimer   sys_tick_timer;

Trigger     *triggers = reinterpret_cast<Trigger*>(&STORAGE_END) - MAX_TRIGGERS;

            // must be global for asm and send_samples() access
uint32_t    sampling_mode = SamplingMode::UNSET;

uint32_t    *samples     = &STORAGE_END,    // init in case SEND_SAMPLES
            *samples_end = &STORAGE_END;    //   before START_SAMPLING

// globals for setting in analog_sampling() and returning in send_samples()
uint32_t    analog_sample_rate;  // s/h+adc for host to calculate rate
uint16_t    num_analog_words  ;  // two samples/word, either two single channel
                                 //   samples, or one from each dual channel

// high byte == InProgress code,  low byte == state number where triggered at
uint16_t    in_progress = InProgress::IDLE;

// reason why sampling ended, see HaltCode, above
uint8_t     halt_code = HaltCode::NONE;

// more globals for setting in analog_sampling() and returning in send_samples()
union {
    struct {
        unsigned    trigger : 4,
                    second  : 4;
    };
    uint8_t         byte       ;
} analog_channels;
uint8_t     num_analog_channels;

#endif  // #if 1 (globals)





#if 1  // (section identification for code browsing)
//
// general utilities
//

INLINE_DECL int INLINE_ATTR memcmp(
const uint8_t   *one,
const uint8_t   *two,
const unsigned   len)
{
    for (unsigned ndx = 0 ; ndx < len ; ++ndx)
        if (one[ndx] != two[ndx]) return one[ndx] - two[ndx];
    return 0;
}

INLINE_DECL uint8_t INLINE_ATTR *memcpy(
      uint8_t       *dest  ,
const uint8_t       *source,
const unsigned       length)
{
    for (unsigned ndx = 0 ; ndx < length ; ++ndx)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        dest[ndx] = source[ndx];
#pragma GCC diagnostic pop
    return dest;
}



INLINE_DECL uint8_t INLINE_ATTR UsbRecv::byte(
unsigned ndx) const
{
    return _bytes[(_begin + ndx) & _MODULO_BYTES_MASK];
}

INLINE_DECL uint16_t INLINE_ATTR UsbRecv::shrt(
unsigned ndx) const
{
    return _shrts[((_begin >> 1) + ndx) & _MODULO_SHRTS_MASK];
}

INLINE_DECL uint32_t INLINE_ATTR UsbRecv::word(
unsigned ndx) const
{
    return _words[((_begin >> 2) + ndx) & _MODULO_WORDS_MASK];
}


void UsbRecv::_fill()
{
    unsigned    rcvd  = usb_dev.recv_lnth(UsbDevCdcAcm::CDC_ENDPOINT_OUT);

    if (!rcvd)
        return;

    // Should only be getting non-modulo-4-sized packets from CDC-ACM
    //    startup, which are ignored, so okay to pad.
    // But also pad in case host sends non-mod-4
    rcvd = (rcvd + 3) & 0xfffffffc;

    if (_level + rcvd > _SIZE)
        rcvd = _SIZE - _level;

    unsigned    end = (_begin + _level) & _MODULO_BYTES_MASK;

    _level += rcvd;

    unsigned    end16 = end >> 1;

    // OK if write extra LSB of last uint16_t because just overwriting
    // already-used byte, or at worst completely filling with _SIZE
    // and stops just short of wraparound
    for (unsigned ndx = 0 ; ndx < (rcvd  >> 1) ; ++ndx) {
        _shrts[end16] = usb_dev.read(UsbDevCdcAcm::CDC_ENDPOINT_OUT, ndx);
        end16         = (end16 + 1) & _MODULO_SHRTS_MASK                 ;
    }

    usb_dev.recv_done(UsbDevCdcAcm::CDC_ENDPOINT_OUT);
}



unsigned UsbRecv::fill(
unsigned     need)   // must be <= UsbDevCdcAcm::CDC_OUT_DATA_SIZE
{
    if (need == 0 && _level == 0)
        _fill();   // poll/check

    while (_level < need)
        _fill();

    return _level;

}

void UsbRecv::flush(
unsigned     used)
{
    // only flush modulo 4 bytes
    used = (used + 3) & 0xfffffffc;

    if (used > _level)
        used = _level;

    _begin  = (_begin + used) & _MODULO_BYTES_MASK;
    _level -= used                                ;
}



template <typename ENR, typename RSTR, typename ENR_BITS, typename RSTR_BITS>
inline void __attribute__((always_inline)) rcc_periph_enable_and_reset(
volatile ENR        &enr      ,
volatile RSTR       &rstr     ,
const    ENR_BITS    enr_bits ,
const    RSTR_BITS   rstr_bits)
{
    // "best practices"
    //

    enr |= enr_bits              ;  // enable
    volatile ENR    enr_read(enr);  // read ensures peripheral has enabled

    rstr |= rstr_bits;  // put into reset
    rstr -= rstr_bits;  // take out of reset
}



inline uint16_t __attribute__((always_inline, optimize(3))) abs(
const int   value)
{
    return value < 0 ? -value : value;
}



void inline __attribute__((always_inline)) user_led_on()
{
    gpioc->bsrr = Gpio::Bsrr::BR13;  // set low turn on user LED
}

void inline __attribute__((always_inline)) user_led_off()
{
    gpioc->bsrr = Gpio::Bsrr::BS13;  // set low turn off user LED
}



INLINE_DECL void INLINE_ATTR usb_send(
uint8_t     length)
{
    while (!usb_dev.send(UsbDevCdcAcm::CDC_ENDPOINT_IN, send_buf, length))
        asm("wfi");
}



void usb_send_w_zlp(
uint8_t     length)
{
    usb_send(length);

    if (length == UsbDevCdcAcm::CDC_OUT_DATA_SIZE)
        // send zero length packet to indicate end of data
        usb_send(0);
}



void duration_timer(
const uint32_t  duration    ,
const bool      start = true)
{
    rcc_periph_enable_and_reset(rcc->apb1enr          ,
                                rcc->apb1rstr         ,
                                Rcc::Apb1enr ::TIM3EN ,
                                Rcc::Apb1rstr::TIM3RST);

    gen_tim_3->psc = 0xffff;  // 72MHz/0x10000 = 0.000910222... secs/tick

    // generate "update event" to load PSC register into actual
    //   prescaler counter shadow register
    gen_tim_3->egr = GenTim_2_3_4::Egr::UG;

    // clear timer, especially UIF so following doesn't generate
    // immediate (or slightly delayed) interrupt)
    gen_tim_3->sr = 0;

    // but ST craziness:
    //   1) need to set prescaler
    //   2) need to set UG bit in EGR register to actually take effect
    //   3) works first time -- doesn't generate pending interrupt
    //   4) PSC isn't change by code, and peripheral maintains even in face
    //      of enabling/disabling in APB1ENR
    //   5) second and subsequent times immediately generates
    // so need to clear interrupt (doesn't hurt to do first time)
    arm::nvic->icpr.set(arm::NvicIrqn::TIM3);

    gen_tim_3->arr = duration;

    gen_tim_3->dier = GenTim_2_3_4::Dier::UIE;
    arm::nvic->iser.set(arm::NvicIrqn::TIM3);

    if (start)
        gen_tim_3->cr1 = GenTim_2_3_4::Cr1::OPM | GenTim_2_3_4::Cr1::CEN;
}

#endif  // #if 1 (general utilities)




#if 1  // (section identification for code browsing)
//
// interrupt handler utilities
//

// noinline because needs to be callable from assembly
extern "C" void __attribute__((noinline)) halt_timers()
{
    gen_tim_3->cr1 = 0;  // always stop duration timer
    gen_tim_3->sr  = 0;  // clear interrupt flags else keeps interrupting
    arm::nvic->icpr.set(arm::NvicIrqn::TIM3);  // in case interrupt pending
    arm::nvic->icer.set(arm::NvicIrqn::TIM3);  // disable interrupt
    rcc->apb1enr -= Rcc::Apb1enr::TIM3EN;

    adv_tim_1->cr1  = 0;    // stop SysTick overflow timer
    rcc->apb2enr   -= Rcc::Apb2enr::TIM1EN;
}

#endif  // #if 1 (interrupt handler utilities)




#if 1  // (section identification for code browsing)
//
// global initialization
//

void usb_mcu_init()
{
#ifdef USB_DEV_FLASH_WAIT_STATES
#if USB_DEV_FLASH_WAIT_STATES == 1
    // enable flash prefetch buffer, one wait state
    flash->acr |= Flash::Acr::PRFTBE | Flash::Acr::LATENCY_1_WAIT_STATE;
#elif USB_DEV_FLASH_WAIT_STATES == 2
    // enable flash prefetch buffer, two wait states
    flash->acr |= Flash::Acr::PRFTBE | Flash::Acr::LATENCY_2_WAIT_STATES;
#endif
#endif

    rcc->cr |= Rcc::Cr::HSEON;
    while(!rcc->cr.any(Rcc::Cr::HSERDY));

    rcc->cfgr |=   Rcc::Cfgr::HPRE_DIV_1
                 | Rcc::Cfgr::PPRE2_DIV_1
                 | Rcc::Cfgr::PPRE1_DIV_2;  // apb1 max 36 MHz
                                            // but tim2-5 automatic 2x back
                                            //   to 72 MHz

    rcc->cfgr.ins(  Rcc::Cfgr::PLLSRC     // PLL input from HSE (8 MHz)
                  | Rcc::Cfgr::PLLMULL_9);// Multiply by 9 (8*9=72 MHz)

    rcc->cr |= Rcc::Cr::PLLON;
    while(!rcc->cr.any(Rcc::Cr::PLLRDY));

    rcc->cfgr.ins(Rcc::Cfgr::SW_PLL);           // use PLL as system clock
    while(!rcc->cfgr.all(Rcc::Cfgr::SWS_PLL));  // wait for confirmation

    rcc->cfgr.clr(Rcc::Cfgr::USBPRE);           // 1.5x USB prescaler

     rcc_periph_enable_and_reset(rcc->apb1enr          ,
                                 rcc->apb1rstr         ,
                                 Rcc::Apb1enr ::USBEN  ,
                                 Rcc::Apb1rstr::USBRST);

    // enable ports and alternate functions (leave enabled)
    rcc_periph_enable_and_reset(
        rcc->apb2enr           ,
        rcc->apb2rstr          ,
          Rcc::Apb2enr ::AFIOEN     // peripherals
        | Rcc::Apb2enr ::IOPAEN     // USB,/usart/tim2/spi/adc
        | Rcc::Apb2enr ::IOPBEN     // sampling/tim1/I2C
        | Rcc::Apb2enr ::IOPCEN,    // user LED
          Rcc::Apb2rstr::AFIORST    // peripherals
        | Rcc::Apb2rstr::IOPARST    // USB/usart/tim2/spi/adc
        | Rcc::Apb2rstr::IOPBRST    // sampling/tim1/I2C
        | Rcc::Apb2rstr::IOPCRST);  // user LED

    arm::nvic->iser.set(arm::NvicIrqn::USB_LP_CAN1_RX0);


    // full main clock speed
    sys_tick_timer.init(arm::SysTick::Ctrl::CLK_SRC_CPU);

}  // usb_mcu_init()



void gpio_init()
{
    afio->mapr = Afio::Mapr::SWJ_CFG_NO_NJTRST;  // get NJRST off PB4

    // enable USB alternate function on USB+ and USB- data pins
    //   alternate function output, open-drain (for paranoia), speed 50MHz
    gpioa->crh.ins(  Gpio::Crh::CNF11_ALTFUNC_OPEN_DRAIN
                   | Gpio::Crh::CNF12_ALTFUNC_OPEN_DRAIN
                   | Gpio::Crh::MODE11_OUTPUT_50_MHZ
                   | Gpio::Crh::MODE12_OUTPUT_50_MHZ    );

    // all pull down, including PB14 (wait all high until trigger start)
    //   and PB15 (wait all high to ackknowlege triggered)
    // is 0x00000010 by default despite RM0008 claiming 0x00000000
    gpiob->odr = 0;

    // enable digital input pins as pull/pull-down (latter default in ODR)
    gpiob->crl.ins(  Gpio::Crl::  CNF0_INPUT_PULL_UP_DOWN
                   | Gpio::Crl::  CNF1_INPUT_PULL_UP_DOWN
                   | Gpio::Crl::  CNF2_INPUT_FLOATING     // BOOT1 jumpered low
                   | Gpio::Crl::  CNF3_INPUT_PULL_UP_DOWN
                   | Gpio::Crl::  CNF4_INPUT_PULL_UP_DOWN
                   | Gpio::Crl::  CNF5_INPUT_PULL_UP_DOWN
                   | Gpio::Crl::  CNF6_INPUT_PULL_UP_DOWN
                   | Gpio::Crl::  CNF7_INPUT_PULL_UP_DOWN
                   | Gpio::Crl:: MODE0_INPUT
                   | Gpio::Crl:: MODE1_INPUT
                   | Gpio::Crl:: MODE2_INPUT
                   | Gpio::Crl:: MODE3_INPUT
                   | Gpio::Crl:: MODE4_INPUT
                   | Gpio::Crl:: MODE5_INPUT
                   | Gpio::Crl:: MODE6_INPUT
                   | Gpio::Crl:: MODE7_INPUT             );
    gpiob->crh.ins(  Gpio::Crh::  CNF8_INPUT_PULL_UP_DOWN
                   | Gpio::Crh::  CNF9_INPUT_PULL_UP_DOWN
                   | Gpio::Crh:: CNF10_INPUT_PULL_UP_DOWN
                   | Gpio::Crh:: CNF11_INPUT_PULL_UP_DOWN
                   | Gpio::Crh:: CNF12_INPUT_PULL_UP_DOWN
                   | Gpio::Crh:: CNF13_INPUT_PULL_UP_DOWN
                   | Gpio::Crh:: CNF14_INPUT_PULL_UP_DOWN
                   | Gpio::Crh:: CNF15_INPUT_PULL_UP_DOWN
                   | Gpio::Crh:: MODE8_INPUT
                   | Gpio::Crh:: MODE9_INPUT
                   | Gpio::Crh::MODE10_INPUT
                   | Gpio::Crh::MODE11_INPUT
                   | Gpio::Crh::MODE12_INPUT
                   | Gpio::Crh::MODE13_INPUT
                   | Gpio::Crh::MODE14_INPUT
                   | Gpio::Crh::MODE15_INPUT             );

    // enable on-board user LED, turned off
    gpioc->bsrr = Gpio::Bsrr::BS13;
    gpioc->crh.ins(  Gpio::Crh::CNF13_OUTPUT_OPEN_DRAIN
                   | Gpio::Crh::MODE13_OUTPUT_2_MHZ    );

}

#endif  // #if 1 (global initialization)




#if 1  // (section identification for code browsing)
//
// peripheral setup utilities
//

void spi_activate()
{
    namespace spi = spi_command;

    const uint8_t   spi_bits   = usb_recv.byte   (spi::BITS    ),
                    baud       = usb_recv.byte   (spi::BAUD    ),
                    pull       = usb_recv.byte   (spi::PULL    ),
                    gpio_speed = usb_recv.byte   (spi::SPEED   ),
                    nss        = usb_recv.byte   (spi::NSS     );
    const bool      master     = usb_recv.byte   (spi::MASTER  ),
                    select     = spi_bits & (1 << spi::SELECT  ),
                    endian     = spi_bits & (1 << spi::ENDIAN  ),
                    polarity   = spi_bits & (1 << spi::POLARITY),
                    phase      = spi_bits & (1 << spi::PHASE   ),
                    miso       = spi_bits & (1 << spi::MISO    );

    const Gpio::Crl::mskd_t speed4 = Gpio::Crl::mskd_t(Gpio::Crl::MASK      ,
                                                       gpio_speed           ,
                                                       Gpio::Crl::MODE4_POS ),
                            speed5 = Gpio::Crl::mskd_t(Gpio::Crl::MASK      ,
                                                       gpio_speed           ,
                                                       Gpio::Crl::MODE5_POS ),
                            speed6 = Gpio::Crl::mskd_t(Gpio::Crl::MASK      ,
                                                       gpio_speed           ,
                                                       Gpio::Crl::MODE6_POS ),
                            speed7 = Gpio::Crl::mskd_t(Gpio::Crl::MASK      ,
                                                       gpio_speed           ,
                                                       Gpio::Crl::MODE7_POS );

    rcc_periph_enable_and_reset(rcc->apb2enr          ,
                                rcc->apb2rstr         ,
                                Rcc::Apb2enr ::SPI1EN ,
                                Rcc::Apb2rstr::SPI1RST);

    if (master) {
        spi1->cr1 =   Spi::Cr1::SPE
                    | Spi::Cr1::MASTER
                    | Spi::Cr1::SSM
                    | Spi::Cr1::SSI
                    | Spi::Cr1::bits_t( endian,   Spi::Cr1::LSBFIRST_POS)
                    | Spi::Cr1::bits_t(polarity,  Spi::Cr1:: CPOL_POS   )
                    | Spi::Cr1::bits_t(   phase,  Spi::Cr1:: CPHA_POS   )
                    | Spi::Cr1::br    (    baud & Spi::Cr1::  BR_MASK   );

        spi1->cr2 = Spi::Cr2::SSOE;  // drive NSS pin low to select slave

        // connect to I/O pins
        //
        Gpio::Crl::mskd_t   miso_pin,
                            nss_pin ;

        switch (nss) {
            case 0:
                // floating
                nss_pin =   Gpio::Crl::MODE4_INPUT
                          | Gpio::Crl:: CNF4_INPUT_FLOATING;
                break;
            case 1:
                // permanently low
                nss_pin      = speed4 | Gpio::Crl::CNF4_OUTPUT_PUSH_PULL;
                gpioa->bsrr |= Gpio::Bsrr::BR4                          ;
                break;
            case 2:
            default:
                // SPI sets low when CLOCK+MOSI active
                // NO!! Contrary to  RM0008 obfuscation, peripheral does not
                //      control NSS pin on a per-"frame" (8/16 bit xfer) basis
                // nss_pin = speed4 | Gpio::Crl:: CNF4_ALTFUNC_PUSH_PULL;
                nss_pin      = speed4 | Gpio::Crl::CNF4_OUTPUT_PUSH_PULL;
                gpioa->bsrr |= Gpio::Bsrr::BS4  /* high */              ;
                break;
        }

        if (pull == 0)
            miso_pin = Gpio::Crl:: CNF6_INPUT_FLOATING;
        else {
            miso_pin     =   Gpio::Crl:: MODE6_INPUT
                           | Gpio::Crl:: CNF6_INPUT_PULL_UP_DOWN;
            gpioa->bsrr |=   pull == 1
                           ? Gpio::Bsrr::BS6   // miso pull up
                           : Gpio::Bsrr::BR6;  // miso pull down, pull==2,
                                               //   shouldn't use
        }

        gpioa->crl.ins(  speed5                               // spi1 sclk, PA5
                       | speed7                               // spi1 mosi, PA7
                       | nss_pin                              // spi1 nss , PA4
                       | Gpio::Crl:: CNF5_ALTFUNC_PUSH_PULL   // spi1 sclk, PA5
                       | miso_pin                             // spi1 miso, PA6
                       | Gpio::Crl:: CNF7_ALTFUNC_PUSH_PULL); // spi1 mosi, PA7

    }
    else {  // slave
        Spi::Cr1::bits_t    select_bit =   select
                                         ? Spi::Cr1::bits_t(0)   // hardware
                                         : Spi::Cr1::SSM      ;  // software

        spi1->cr1 =   Spi::Cr1::SPE
                    | Spi::Cr1::SLAVE
                    | select_bit
                    | Spi::Cr1::bits_t(  endian,  Spi::Cr1::LSBFIRST_POS)
                    | Spi::Cr1::bits_t(polarity,  Spi::Cr1::    CPOL_POS)
                    | Spi::Cr1::bits_t(   phase,  Spi::Cr1::    CPHA_POS)
                    | Spi::Cr1::br    (    baud & Spi::Cr1::     BR_MASK);

        spi1->cr2 = 0;  // NSS


        // connect to I/O pins
        //
        if (miso)   // bit set to 1, open-drain
            gpioa->crl.ins(  Gpio::Crl::MODE4_INPUT                 // spi1 nss
                           | Gpio::Crl::MODE5_INPUT                 // spi1 sclk
                           | speed6                                 // spi1 miso
                           | Gpio::Crl::MODE7_INPUT                 // spi1 mosi
                           | Gpio::Crl:: CNF4_INPUT_PULL_UP_DOWN    // spi1 nss
                           | Gpio::Crl:: CNF5_INPUT_FLOATING        // spi1 sclk
                           | Gpio::Crl:: CNF6_ALTFUNC_OPEN_DRAIN    // spi1 miso
                           | Gpio::Crl:: CNF7_INPUT_FLOATING     ); // spi1 mosi
        else        // bit set to 0, push-pull
            gpioa->crl.ins(  Gpio::Crl::MODE4_INPUT                 // spi1 nss
                           | Gpio::Crl::MODE5_INPUT                 // spi1 sclk
                           | speed6                                 // spi1 miso
                           | Gpio::Crl::MODE7_INPUT                 // spi1 mosi
                           | Gpio::Crl:: CNF4_INPUT_PULL_UP_DOWN    // spi1 nss
                           | Gpio::Crl:: CNF5_INPUT_FLOATING        // spi1 sclk
                           | Gpio::Crl:: CNF6_ALTFUNC_PUSH_PULL     // spi1 miso
                           | Gpio::Crl:: CNF7_INPUT_FLOATING     ); // spi1 mosi
    }
}  // spi_activate()

void spi_disable()
{
    // disconnect from I/O pins
    //
    gpioa->crl.ins(  Gpio::Crl::MODE4_INPUT             // spi1 nss
                   | Gpio::Crl::MODE5_INPUT             // spi1 sclk
                   | Gpio::Crl::MODE6_INPUT             // spi1 miso
                   | Gpio::Crl::MODE7_INPUT             // spi1 mosi
                   | Gpio::Crl:: CNF4_INPUT_FLOATING    // spi1 nss
                   | Gpio::Crl:: CNF5_INPUT_FLOATING    // spi1 sclk
                   | Gpio::Crl:: CNF6_INPUT_FLOATING    // spi1 miso
                   | Gpio::Crl:: CNF7_INPUT_FLOATING);  // spi1 mosi
    gpioa->odr = 0;

    spi1->cr1     = 0                   ;   // disable
    rcc->apb2enr -= Rcc::Apb2enr::SPI1EN;   // turn off peripheral clock
}  // spi_disable()



void i2c_activate()
{
    namespace i2c = i2c_command;

    const uint16_t  ccr        = usb_recv.shrt    (i2c::CCR     );
    const uint8_t   i2c_bits   = usb_recv.byte    (i2c::BITS    ),
                    oar1       = usb_recv.byte    (i2c::OAR1    ),
                    oar2       = usb_recv.byte    (i2c::OAR2    );
    const bool      master     = i2c_bits &  (1 << i2c::MASTER  ),
                    fast       = i2c_bits &  (1 << i2c::STD_FAST),
                    duty       = i2c_bits &  (1 << i2c::DUTY    ),
                    gen_call   = i2c_bits &  (1 << i2c::GEN_CALL);

    rcc_periph_enable_and_reset(rcc->apb1enr          ,
                                rcc->apb1rstr         ,
                                Rcc::Apb1enr ::I2C2EN ,
                                Rcc::Apb1rstr::I2C2RST);

    uint8_t             gpio_speed = usb_recv.byte(i2c::GPIO)               ;
    Gpio::Crh::mskd_t   speed10    = Gpio::Crh::mskd_t(Gpio::Crh::MASK      ,
                                                       gpio_speed           ,
                                                       Gpio::Crh::MODE10_POS),
                        speed11    = Gpio::Crh::mskd_t(Gpio::Crh::MASK      ,
                                                       gpio_speed           ,
                                                       Gpio::Crh::MODE11_POS);
    gpiob->crh.ins(  speed10 | Gpio::Crh::CNF10_ALTFUNC_OPEN_DRAIN
                   | speed11 | Gpio::Crh::CNF11_ALTFUNC_OPEN_DRAIN);

    // must be disabled to set some registers/fields
    i2c2->cr1 = 0;   // -= I2c::Cr1::PE;

    // both master and slave
    i2c2->cr2   =   I2c::Cr2::freq(36);  // APB1 bus frequency, 72 MHz div 2
    i2c2->ccr   =   I2c::Ccr::bits_t(fast, I2c::Ccr::  FS_POS)
                  | I2c::Ccr::bits_t(duty, I2c::Ccr::DUTY_POS)
                  | I2c::Ccr::ccr   (ccr                     );
    i2c2->trise =   fast    // max rise        pCLCK1
                  ? 10 + 1 //   300e-9  / (1 / 36e6) == 10.8
                  : 36 + 1; // 1000e-9  / (1 / 36e6) == 36.0  ;

    // slave (but doesn't hurt if master)
    i2c2->oar1 =   I2c::Oar1::add7(oar1)
                 | I2c::Oar1::BIT14
                 | I2c::Oar1::ADDMODE_7                             ;
    i2c2->oar2 =   (  oar2 < 128
                    ? I2c::Oar2::ENDUAL
                    : I2c::Oar2::bits_t(0, I2c::Oar2::ENDUAL_POS))
                 |    I2c::Oar2::add2(oar2)                         ;

    if (master)
        i2c2->cr1   = I2c::Cr1::PE                                   ;
    else  // slave
        i2c2->cr1  =   I2c::Cr1::bits_t(gen_call, I2c::Cr1::ENGC_POS)
                     | I2c::Cr1::ACK  // permanently enabled
                     | I2c::Cr1::PE                                  ;

}  // i2c_activate()


void i2c_disable()
{
    if (!rcc->apb1enr.any(Rcc::Apb1enr::I2C2EN))
        return;  // currently disabled

    // disconnect from I/O pins
    //
    gpiob->crh.ins(  Gpio::Crh::MODE10_INPUT
                   | Gpio::Crh::MODE11_INPUT
                   | Gpio::Crh:: CNF10_INPUT_PULL_UP_DOWN
                   | Gpio::Crh:: CNF11_INPUT_PULL_UP_DOWN); // i2c1 mosi
    gpiob->bsrr |=   Gpio::Bsrr::BR10
                   | Gpio::Bsrr::BR11;

    i2c2->cr1     = 0                   ;   // disable
    rcc->apb1enr -= Rcc::Apb1enr::I2C2EN;   // turn off peripheral clock
}  // i2c_disable()



volatile Usart* usart_activate()
{
    namespace usart = usart_command;

    const uint16_t  baud        =  usb_recv.shrt(usart::BAUD  );
    const uint8_t   bits_1      =  usb_recv.byte(usart::BITS_1),
                    bits_2      =  usb_recv.byte(usart::BITS_2),
                    parity      =  usb_recv.byte(usart::PARITY),
                    stop_bits   =  usb_recv.byte(usart::STOP  ),
                    gpio_speed  =  usb_recv.byte(usart::SPEED );
    const bool      usart_0_3   = bits_2 & (1 << usart::USART ),
                    xmit        = bits_1 & (1 << usart::XMIT  ),
                    recv        = bits_1 & (1 << usart::RECV  ),
                    clock       = bits_1 & (1 << usart::CLOCK ),
                    num_bits    = bits_1 & (1 << usart::LENGTH),
                    polarity    = bits_1 & (1 << usart::POLRTY),
                    phase       = bits_1 & (1 << usart::PHASE ),
                    last_clck   = bits_1 & (1 << usart::LSTCLK),
                    cts         = bits_2 & (1 << usart::CTS   ),
                    rts         = bits_2 & (1 << usart::RTS   );

    volatile Usart  *usart_n;

    if (usart_0_3) {
        void    pwm_disable();   // forward reference

        // check for port conflicts with PWM running on PA1-3 and kill if so
        if (rcc->apb1enr.any(Rcc::Apb1enr ::TIM2EN))
            pwm_disable();

        usart_n = usart2;
        rcc_periph_enable_and_reset(rcc->apb1enr            ,
                                    rcc->apb1rstr           ,
                                    Rcc::Apb1enr ::USART2EN ,
                                    Rcc::Apb1rstr::USART2RST);
    }
    else {
        usart_n = usart1;
        rcc_periph_enable_and_reset(rcc->apb2enr            ,
                                    rcc->apb2rstr           ,
                                    Rcc::Apb2enr ::USART1EN ,
                                    Rcc::Apb2rstr::USART1RST);
    }

    // usart2 (usart_0_3 ) is on APB1 bus @ 36MHz vs
    // usart1 (usart_8_10)    on APB2 bus @ 72MHz
    // so need to scale baud rate to match
    usart_n->brr = usart_0_3 ? (baud >> 1) : baud;

    usart_n->cr2 =  Usart::Cr2::bits_t(last_clck, Usart::Cr2:: LBCL_POS)
                  | Usart::Cr2::bits_t(    phase, Usart::Cr2:: CPHA_POS)
                  | Usart::Cr2::bits_t( polarity, Usart::Cr2:: CPOL_POS)
                  | Usart::Cr2::bits_t(    clock, Usart::Cr2::CLKEN_POS)
                  | Usart::Cr2::mskd_t(Usart::Cr2::STOP_MASK,
                                       stop_bits,  Usart::Cr2::STOP_POS);

    usart_n->cr1 =  Usart::Cr1::bits_t(     recv, Usart::Cr1:: RE_POS)
                  | Usart::Cr1::bits_t(     xmit, Usart::Cr1:: TE_POS)
                  | Usart::Cr1::bits_t(   parity, Usart::Cr1:: PS_POS) // w/ PCE
                  | Usart::Cr1::bits_t( num_bits, Usart::Cr1::  M_POS)
                  | Usart::Cr1::UE                                      ;


    // connect to I/O pins
    //

    if (usart_0_3) {
        usart2->cr3 |=   Usart::Cr3::bits_t(cts, Usart::Cr3::CTSE_POS)
                       | Usart::Cr3::bits_t(rts, Usart::Cr3::RTSE_POS);

        const
        Gpio::Crl::mskd_t   speed1 = Gpio::Crl::mskd_t(Gpio::Crl::MASK      ,
                                                       gpio_speed           ,
                                                       Gpio::Crl::MODE1_POS ),
                            speed2 = Gpio::Crl::mskd_t(Gpio::Crl::MASK      ,
                                                       gpio_speed           ,
                                                       Gpio::Crl::MODE2_POS );

        gpioa->crl.ins(  Gpio::Crl::MODE0_INPUT                 // usart2 CTS
                       | speed1                                 // usart2 RTS
                       | speed2                                 // usart2 TX
                       | Gpio::Crl::MODE3_INPUT                 // usart2 RX
                       | Gpio::Crl:: CNF0_INPUT_PULL_UP_DOWN    // usart2 CTS
                       | Gpio::Crl:: CNF1_ALTFUNC_PUSH_PULL     // usart2 RTS
                       | Gpio::Crl:: CNF2_ALTFUNC_PUSH_PULL     // usart2 TX
                       | Gpio::Crl:: CNF3_INPUT_PULL_UP_DOWN);  // usart2 RX

        gpioa->bsrr =   Gpio::Bsrr::BS0     // usart2 CTS pull up
                      | Gpio::Bsrr::BS3;    // usart2 RX pull up
    }
    else {
        const
        Gpio::Crh::mskd_t   speed8 = Gpio::Crh::mskd_t(Gpio::Crh::MASK      ,
                                                       gpio_speed           ,
                                                       Gpio::Crh::MODE8_POS ),
                            speed9 = Gpio::Crh::mskd_t(Gpio::Crh::MASK      ,
                                                       gpio_speed           ,
                                                       Gpio::Crh::MODE9_POS );

        if (clock)  // synchronous (USART) mode
            gpioa->crh.ins(  speed8                                 // usart1 ck
                           | speed9                                 // usart1 tx
                           | Gpio::Crh::MODE10_INPUT                // usart1 rx
                           | Gpio::Crh::  CNF8_ALTFUNC_PUSH_PULL    // usart1 ck
                           | Gpio::Crh::  CNF9_ALTFUNC_PUSH_PULL    // usart1 tx
                           | Gpio::Crh:: CNF10_INPUT_PULL_UP_DOWN); // usart1 rx

        else  // async (UART) mode
            gpioa->crh.ins(  Gpio::Crh:: MODE8_INPUT                // usart1 ck
                           | speed9                                 // usart1 tx
                           | Gpio::Crh::MODE10_INPUT                // usart1 rx
                           | Gpio::Crh::  CNF8_INPUT_PULL_UP_DOWN   // usart1 ck
                           | Gpio::Crh::  CNF9_ALTFUNC_PUSH_PULL    // usart1 tx
                           | Gpio::Crh:: CNF10_INPUT_PULL_UP_DOWN); // usart1 rx
        // either mode
        gpioa->bsrr = Gpio::Bsrr::BS10; // usart1 RX pull up
    }

    return usart_n;

}  // usart_activate()

void usart_disable(
volatile Usart*     usart_n)
{

    if (usart_n == usart1)
        gpioa->crh.ins(  Gpio::Crh:: MODE8_INPUT            // usart1 ck
                       | Gpio::Crh:: MODE9_INPUT            // usart1 tx
                       | Gpio::Crh::MODE10_INPUT            // usart1 rx
                       | Gpio::Crh::  CNF8_INPUT_FLOATING   // usart1 ck
                       | Gpio::Crh::  CNF9_INPUT_FLOATING   // usart1 tx
                       | Gpio::Crh:: CNF10_INPUT_FLOATING); // usart1 rx
    else  // usart2
        gpioa->crl.ins(  Gpio::Crl:: MODE0_INPUT            // usart2 cts
                       | Gpio::Crl:: MODE1_INPUT            // usart2 rts
                       | Gpio::Crl:: MODE2_INPUT            // usart2 tx
                       | Gpio::Crl:: MODE3_INPUT            // usart2 rx
                       | Gpio::Crl::  CNF0_INPUT_FLOATING   // usart2 cts
                       | Gpio::Crl::  CNF1_INPUT_FLOATING   // usart2 rts
                       | Gpio::Crl::  CNF2_INPUT_FLOATING   // usart2 tx
                       | Gpio::Crl::  CNF3_INPUT_FLOATING); // usart2 rx
    gpioa->odr = 0;

    usart_n->cr1 = 0;  // disable

    if (usart_n == usart1)
        rcc->apb2enr -= Rcc::Apb2enr::USART1EN;  // turn off peripheral clock
    else
        rcc->apb1enr -= Rcc::Apb1enr::USART2EN;  // turn off peripheral clock
}



void adc_activate(
uint8_t     num_adcs ,
uint8_t     port_bits,   // bitmask of GPIOA ports to enable
uint8_t     *chans   ,
uint8_t     *rates   )
{
    // max 14 MHz ADC clock. 12 MHz is closest possible (default is
    //   DIV_2 == 36 MHz)
    rcc->cfgr.ins(Rcc::Cfgr::ADCPRE_DIV_6);

    uint8_t     bit = 0x01;
    for (unsigned port = 0 ; port < 8 ; ++port, bit <<= 1)
        if (port_bits & bit)
            gpioa->crl.ins(  Gpio::Crl::mskd_t(Gpio::Crl::MASK               ,
                                               Gpio::Crl::INPUT              ,
                                               Gpio::Crl::pos_t(  (port << 2)
                                                                + 2          ))
                           | Gpio::Crl::mskd_t(Gpio::Crl::MASK               ,
                                               Gpio::Crl::INPUT_ANALOG       ,
                                               Gpio::Crl::pos_t(   port << 2)));

    rcc_periph_enable_and_reset(rcc->apb2enr          ,
                                rcc->apb2rstr         ,
                                Rcc::Apb2enr ::ADC1EN ,
                                Rcc::Apb2rstr::ADC1RST);

    uint32_t    sqr3  = 0,
                sqr2  = 0,
                smpr2 = 0;
    for (unsigned ndx = 0 ; ndx < num_adcs ; ++ndx) {
        if (chans[ndx] <= 6) sqr3 |= chans[ndx] << ( ndx      * 5);
        else                 sqr2 |= chans[ndx] << ((ndx - 6) * 5);
        smpr2                     |= rates[ndx] << ( ndx      * 3);
    }
    adc1->sqr3  = sqr3                       ;
    adc1->sqr2  = sqr2                       ;
    adc1->smpr2 = smpr2                      ;
    adc1->sqr1  = Adc::Sqr1 ::l(num_adcs - 1);


    // calibrate
    //

    // reset calibration registers
    adc1->cr2 |= Adc::Cr2::ADON;
    // RM0008: "at least two ADC clock cycles"
    sys_tick_timer.delay32(36);  // 3 for safety, 12 main clock per 1 ADC clock
    adc1->cr2 |= Adc::Cr2::ADON | Adc::Cr2::RSTCAL;
    while (adc1->cr2.any(Adc::Cr2::RSTCAL))  // need timeout safety
        asm("nop");

    // calibrate
    adc1->cr2 |= Adc::Cr2::ADON | Adc::Cr2::CAL;
    // wait for calibration finish
    while (adc1->cr2.any(Adc::Cr2::CAL))     // need timeout safety
        asm("nop");

    adc1->cr1 =   Adc::Cr1::DISCEN
                | Adc::Cr1::discnum(num_adcs);
    adc1->cr2 =   Adc::Cr2::EXTTRIG
                | Adc::Cr2::EXTSEL_SWSTART
                | Adc::Cr2::ADON             ;
}  // adc_activate()

void adc_disable()
{
    dma1_channel1->ccr = 0;  // if in use by analog_capture(), ok if not

    adc1->cr2 = 0;

    rcc->ahbenr  -= Rcc::Ahbenr ::DMA1EN;
    rcc->apb2enr -= Rcc::Apb2enr::ADC1EN;

    // reset all, regardless of how many were set to ADC ...
    // ... except not PA1-PA3 because might be running pwm()
    gpioa->crl.ins(  Gpio::Crl::MODE0_INPUT
                   | Gpio::Crl::MODE4_INPUT
                   | Gpio::Crl::MODE5_INPUT
                   | Gpio::Crl::MODE6_INPUT
                   | Gpio::Crl::MODE7_INPUT
                   | Gpio::Crl:: CNF0_INPUT_FLOATING
                   | Gpio::Crl:: CNF4_INPUT_FLOATING
                   | Gpio::Crl:: CNF5_INPUT_FLOATING
                   | Gpio::Crl:: CNF6_INPUT_FLOATING
                   | Gpio::Crl:: CNF7_INPUT_FLOATING);

    if (!rcc->apb1enr.any(Rcc::Apb1enr ::TIM2EN)) {
        // pwm() not running, gen_tim_2 isn't using PA1-3
        gpioa->crl.ins(  Gpio::Crl::MODE1_INPUT
                       | Gpio::Crl::MODE2_INPUT
                       | Gpio::Crl::MODE3_INPUT
                       | Gpio::Crl:: CNF1_INPUT_FLOATING
                       | Gpio::Crl:: CNF2_INPUT_FLOATING
                       | Gpio::Crl:: CNF3_INPUT_FLOATING);
    }

    gpioa->odr = 0;
}



uint8_t gpioa_activate()
{
    gpioa->odr = 0;

    const uint8_t     speed  = usb_recv.byte(gpioa_command::GPIO_SPEED),
                   open_pull = usb_recv.byte(gpioa_command::OPEN_PULL );

        gpioa->crl
    /=   Gpio::Crl::mskd_t(Gpio::Crl::MASK, open_pull, Gpio::Crl:: CNF0_POS)
       | Gpio::Crl::mskd_t(Gpio::Crl::MASK, open_pull, Gpio::Crl:: CNF4_POS)
       | Gpio::Crl::mskd_t(Gpio::Crl::MASK, open_pull, Gpio::Crl:: CNF5_POS)
       | Gpio::Crl::mskd_t(Gpio::Crl::MASK, open_pull, Gpio::Crl:: CNF6_POS)
       | Gpio::Crl::mskd_t(Gpio::Crl::MASK, open_pull, Gpio::Crl:: CNF7_POS)
       | Gpio::Crl::mskd_t(Gpio::Crl::MASK, speed    , Gpio::Crl::MODE0_POS)
       | Gpio::Crl::mskd_t(Gpio::Crl::MASK, speed    , Gpio::Crl::MODE4_POS)
       | Gpio::Crl::mskd_t(Gpio::Crl::MASK, speed    , Gpio::Crl::MODE5_POS)
       | Gpio::Crl::mskd_t(Gpio::Crl::MASK, speed    , Gpio::Crl::MODE6_POS)
       | Gpio::Crl::mskd_t(Gpio::Crl::MASK, speed    , Gpio::Crl::MODE7_POS);

    if (rcc->apb1enr.any(Rcc::Apb1enr ::TIM2EN))
        // pwm() is running, gen_tim_2 has PA1-3
        return 0xf1;  // mask for parallel_bridge() and counter() stay off PA1-3

    // active rest of ports (since gen_tim_2 not active and using them
        gpioa->crl
    /=   Gpio::Crl::mskd_t(Gpio::Crl::MASK, open_pull, Gpio::Crl:: CNF1_POS)
       | Gpio::Crl::mskd_t(Gpio::Crl::MASK, open_pull, Gpio::Crl:: CNF2_POS)
       | Gpio::Crl::mskd_t(Gpio::Crl::MASK, open_pull, Gpio::Crl:: CNF3_POS)
       | Gpio::Crl::mskd_t(Gpio::Crl::MASK, speed    , Gpio::Crl::MODE1_POS)
       | Gpio::Crl::mskd_t(Gpio::Crl::MASK, speed    , Gpio::Crl::MODE2_POS)
       | Gpio::Crl::mskd_t(Gpio::Crl::MASK, speed    , Gpio::Crl::MODE3_POS);

    return 0xff;  // mask for parallel_bridge() and counter to use PA0-7
}  // gpioa_activate()

void gpioa_disable()
{
    gpioa->odr = 0;  // mysteriously gets set to 0x45, so reset

    // need to do first otherwise residual charge on pins  (or something)
    // (even if INPUT_PUSH_PULL below) causes lines to oscillate
    gpioa->crl /=   Gpio::Crl:: CNF0_OUTPUT_OPEN_DRAIN
                  | Gpio::Crl:: CNF1_OUTPUT_OPEN_DRAIN
                  | Gpio::Crl:: CNF2_OUTPUT_OPEN_DRAIN
                  | Gpio::Crl:: CNF3_OUTPUT_OPEN_DRAIN
                  | Gpio::Crl:: CNF4_OUTPUT_OPEN_DRAIN
                  | Gpio::Crl:: CNF5_OUTPUT_OPEN_DRAIN
                  | Gpio::Crl:: CNF6_OUTPUT_OPEN_DRAIN
                  | Gpio::Crl:: CNF7_OUTPUT_OPEN_DRAIN
                  | Gpio::Crl::MODE0_OUTPUT_2_MHZ
                  | Gpio::Crl::MODE1_OUTPUT_2_MHZ
                  | Gpio::Crl::MODE2_OUTPUT_2_MHZ
                  | Gpio::Crl::MODE3_OUTPUT_2_MHZ
                  | Gpio::Crl::MODE4_OUTPUT_2_MHZ
                  | Gpio::Crl::MODE5_OUTPUT_2_MHZ
                  | Gpio::Crl::MODE6_OUTPUT_2_MHZ
                  | Gpio::Crl::MODE7_OUTPUT_2_MHZ      ;

    // now set to desired state
    gpioa->crl /=   Gpio::Crl:: CNF0_INPUT_FLOATING
                  | Gpio::Crl:: CNF1_INPUT_FLOATING
                  | Gpio::Crl:: CNF2_INPUT_FLOATING
                  | Gpio::Crl:: CNF3_INPUT_FLOATING
                  | Gpio::Crl:: CNF4_INPUT_FLOATING
                  | Gpio::Crl:: CNF5_INPUT_FLOATING
                  | Gpio::Crl:: CNF6_INPUT_FLOATING
                  | Gpio::Crl:: CNF7_INPUT_FLOATING
                  | Gpio::Crl::MODE0_INPUT
                  | Gpio::Crl::MODE1_INPUT
                  | Gpio::Crl::MODE2_INPUT
                  | Gpio::Crl::MODE3_INPUT
                  | Gpio::Crl::MODE4_INPUT
                  | Gpio::Crl::MODE5_INPUT
                  | Gpio::Crl::MODE6_INPUT
                  | Gpio::Crl::MODE7_INPUT          ;
}



void pwm_disable() {
    // disconnect from I/O pins
    gpioa->crl.ins(  Gpio::Crl::MODE1_INPUT                 // tim2 ch2
                   | Gpio::Crl::MODE2_INPUT                 // tim2 ch3
                   | Gpio::Crl::MODE3_INPUT                 // tim2 ch4
                   | Gpio::Crl:: CNF1_INPUT_PULL_UP_DOWN    // tim2 ch2
                   | Gpio::Crl:: CNF2_INPUT_PULL_UP_DOWN    // tim2 ch3
                   | Gpio::Crl:: CNF3_INPUT_PULL_UP_DOWN);  // tim2 ch4
    gpioa->bsrr |=   Gpio::Bsrr::BR1     // pull down
                   | Gpio::Bsrr::BR2     // pull down
                   | Gpio::Bsrr::BR3;    // pull down

    gen_tim_2->cr1  = 0                   ;  // disable
    rcc->apb1enr   -= Rcc::Apb1enr::TIM2EN;  // turn off peripheral clock
}



void reset_timer_ganged_ports()
{
    // reset timer output pin
    gpiob->crh.ins(  Gpio::Crh:: CNF13_INPUT_FLOATING
                   | Gpio::Crh::MODE13_INPUT         );
    // in case ganged enabled (and okay if not)
    gpiob->bsrr =    Gpio::Bsrr::BR14   // clear
                   | Gpio::Bsrr::BR15;  //   "
}

#endif  // #if 1 (peripheral setup utilities)



#if 1  // (section identification for code browsing)
//
// peripheral I/O utilities
//

PeriphStatus usart_tx(
volatile Usart  *usart    ,
const uint8_t    send_data,
const uint32_t   timeout  )
{
    arm::SysTickTimer   timer(32);

    while (!usart->sr.any(Usart::Sr::TXE) && timer.elapsed32() < timeout)
        asm("nop");
    if (!usart->sr.any(Usart::Sr::TXE))
        return PeriphStatus::BUSY;

    usart->dr = send_data;

    return PeriphStatus::OK;

}  // usart_tx()



PeriphStatus usart_rx(
volatile Usart  *usart    ,
      uint8_t   &rcvd_data,
const uint32_t   timeout  )

{
    PeriphStatus        status =   usart->sr.any(Usart::Sr::ORE)
                                 ? PeriphStatus::OVERRUN
                                 : PeriphStatus::OK             ;
    arm::SysTickTimer   timer                                   ;

    if (usart->cr2.any(Usart::Cr2::CLKEN)) {
        // wait for synchronous xmit to complete
        timer.begin32();
        while (!usart->sr.any(Usart::Sr::RXNE) && timer.elapsed32() < timeout)
            asm("nop");

        if (!usart->sr.any(Usart::Sr::RXNE))
            return PeriphStatus::TIMEOUT;
    }

    if (usart->sr.any(Usart::Sr::RXNE)) {
        // Must do before read of DR because that clears (after read of SR)
        // Can have multiple flags but status is single valued, so
        //   order so "worse" overrides "not as bad"
        if (usart->sr.any(Usart::Sr::LBD)) status = PeriphStatus::USART_BREAK  ;
        if (usart->sr.any(Usart::Sr::NE )) status = PeriphStatus::USART_NOISE  ;
        if (usart->sr.any(Usart::Sr::PE )) status = PeriphStatus::USART_PARITY ;
        if (usart->sr.any(Usart::Sr::FE )) status = PeriphStatus::USART_FRAMING;

        if (usart->sr.any(Usart::Sr::LBD))  // different from others ...
            usart->sr -= Usart::Sr::LBD;    // ... must be explicitly cleared

        // mask out parity bit 9 in case parity enabled
        rcvd_data = usart->dr & 0xff;

        return status        ; // might also be OVERRUN from above
    }

    return PeriphStatus::EMPTY;

}  // usart_rx()



PeriphStatus usart_buf_tx(
volatile Usart  *usart     ,
const uint8_t   *data      ,
const unsigned   length    ,
const uint32_t   tx_timeout,
const uint32_t   rate      ,
const uint8_t    halt_byte ,
      unsigned  &sent      )
{
    PeriphStatus        status = PeriphStatus::OK;

    for (sent = 0 ; sent < length ; ++sent) {
        if (   (status = usart_tx(usart, data[sent], tx_timeout))
            != PeriphStatus::OK)
            break;

        if (rate) {
            arm::SysTickTimer   timer(32);
            while (timer.elapsed32() < rate)
                // check for user interrupt
                if (usb_recv.fill(0) && usb_recv.byte(0) == halt_byte)
                    return status;
        }
    }

    return status;
}



PeriphStatus usart_buf_tx_rx(
volatile Usart  *usart     ,
const uint8_t   *send      ,
      uint8_t   *recv      ,
const unsigned   length    ,
const uint32_t   tx_timeout,
const uint32_t   rate      ,
const uint8_t    halt_byte ,
      unsigned  &sent      )
{
    PeriphStatus    status = PeriphStatus::OK;

    for (sent = 0 ; sent < length ; ++sent) {
        if (   (status = usart_tx(usart, send[sent], tx_timeout))
            != PeriphStatus::OK                                  )
            break;

        if (   (status = usart_rx(usart, recv[sent], tx_timeout))
            != PeriphStatus::OK                                  )
            break;

        if (rate) {
            arm::SysTickTimer   timer(32);
            while (timer.elapsed32() < rate)
                // check for user interrupt
                if (usb_recv.fill(0) && usb_recv.byte(0) == halt_byte)
                    return PeriphStatus::HALTED;
        }
    }

    return status;
}



PeriphStatus usart_buf_rx(
volatile Usart  *usart    ,
      uint8_t   *data     ,
const unsigned   length   ,
const uint32_t   rx_wait  ,
const uint8_t    halt_byte,
      unsigned  &rcvd     )
{
    PeriphStatus        status = PeriphStatus::OK,
                        worst  = PeriphStatus::OK;
    arm::SysTickTimer   timer                    ;

    for (rcvd = 0 ; rcvd < length ; ++rcvd) {
        timer.begin32();
        while (      (status = usart_rx(usart, data[rcvd], 0))
                  == PeriphStatus::EMPTY
               && timer.elapsed32() < rx_wait                 )
            // check for user interrupt
            if (usb_recv.fill(0) && usb_recv.byte(0) == halt_byte)
                return   worst > PeriphStatus::HALTED
                       ? worst
                       :         PeriphStatus::HALTED;
        if (status == PeriphStatus::EMPTY) break         ;
        if (status >  worst              ) worst = status;
    }

    return status > worst ? status : worst;
}



PeriphStatus spi_tx_rx_mstr(
const uint8_t    send_data ,
      uint8_t   &rcvd_data ,
const uint32_t   tx_timeout,
const uint32_t  nss_delay  )
{
    arm::SysTickTimer   timer(32);

    while (!spi1->sr.any(Spi::Sr::TXE) && timer.elapsed32() < tx_timeout)
        asm("nop");
    if (!spi1->sr.any(Spi::Sr::TXE))
        return PeriphStatus::BUSY;

    if (nss_delay) {
        gpioa->bsrr |= Gpio::Bsrr::BR4;  // spi1 nss low
        timer.begin32();
        while (timer.elapsed32() < nss_delay)
            asm("nop");
    }

    spi1->dr8 = send_data;

    // wait for MOSI/MISO complete
    timer.begin32();
    while (!spi1->sr.any(Spi::Sr::RXNE) && timer.elapsed32() < tx_timeout)
        asm("nop");

    if (nss_delay) {
        timer.begin32();
        while (timer.elapsed32() < nss_delay)
            asm("nop");
        gpioa->bsrr |= Gpio::Bsrr::BS4;  // spi1 nss high
    }

    if (spi1->sr.any(Spi::Sr::RXNE)) {
        rcvd_data = spi1->dr8  ;
        return PeriphStatus::OK;
    }
    else
        return PeriphStatus::TIMEOUT;
}



PeriphStatus spi_tx_rx_slav(
const uint8_t    send_data,
      uint8_t   &rcvd_data)
{
    if (spi1->sr.any(Spi::Sr::TXE))
        spi1->dr8 = send_data;

    PeriphStatus        status =   spi1->sr.any(Spi::Sr::OVR)
                                 ? PeriphStatus::OVERRUN
                                 : PeriphStatus::OK          ;

    if (spi1->sr.any(Spi::Sr::RXNE))
        rcvd_data = spi1->dr8;
    else
        status = PeriphStatus::EMPTY;

    return status;
}



PeriphStatus spi_tx_rx_buf_mstr(
const uint8_t   *mosi      ,
      uint8_t   *miso      ,
const unsigned   length    ,
const uint32_t   tx_timeout,
const uint32_t   nss_delay ,
const uint32_t   rate      ,
const uint8_t    halt_byte ,
      unsigned  &sent      )
{
    PeriphStatus        worst = PeriphStatus::OK,
                        status                  ;

    for (sent = 0 ; sent < length ; ++sent) {
        if (  (status = spi_tx_rx_mstr(mosi[sent],
                                       miso[sent],
                                       tx_timeout,
                                       nss_delay ))
            != PeriphStatus::OK                    ) {
            if (status > worst) worst = status;
            break                             ;
        }

        if (rate) {
            arm::SysTickTimer   timer(32);
            while (timer.elapsed32() < rate)
                // check for user interrupt
                if (usb_recv.fill(0) && usb_recv.byte(0) == halt_byte)
                    return   worst > PeriphStatus::HALTED
                           ? worst
                           :         PeriphStatus::HALTED;
        }
    }

    return worst;
}



PeriphStatus spi_tx_rx_buf_slav(
const uint8_t   *miso_1   ,
const uint8_t   *miso_2   ,
      uint8_t   *mosi     ,
const unsigned   length_1 ,
const unsigned   length_2 ,
const uint32_t   rx_wait  ,
const uint8_t    halt_byte,
      unsigned  &rcvd     )
{
    PeriphStatus        status                   ,
                        worst  = PeriphStatus::OK;
    arm::SysTickTimer   timer                    ;

    for (rcvd = 0 ; rcvd < length_1 + length_2 ; ++rcvd) {
        uint8_t     miso_byte;

        if (rcvd < length_1) miso_byte = miso_1[rcvd           ];
        else                 miso_byte = miso_2[rcvd - length_1];

        timer.begin32();
        while (      (status = spi_tx_rx_slav(miso_byte, mosi[rcvd]))
                  == PeriphStatus::EMPTY
               && timer.elapsed32() < rx_wait                           )
            // check for user interrupt
            if (usb_recv.fill(0) && usb_recv.byte(0) == halt_byte)
                break;

        if (status >  worst              ) worst = status;
        if (status == PeriphStatus::EMPTY) break         ;
    }

    return worst;
}



PeriphStatus i2c_master_start(
const uint32_t   timeout)
{
    i2c2->cr1 = I2c::Cr1::START | I2c::Cr1::PE;

    arm::SysTickTimer   timer(32);
    while (!i2c2->sr1.any(I2c::Sr1::SB) && timer.elapsed32() < timeout)
        asm("nop");
    if (i2c2->sr1.any(I2c::Sr1::SB))
        return PeriphStatus::OK;
    else
        return PeriphStatus::I2C_NOSTART;
}




PeriphStatus i2c_master_addr(
const uint32_t   timeout     ,
const uint8_t    dest_addr   ,
const uint8_t    rx_bit      ,
const uint8_t    rx_bytes = 0)  // 1, 2, or 3+
{
    PeriphStatus    status = PeriphStatus::OK;

    // writing to DR after read of SR1 in i2c_master_start() clears SB
    i2c2->dr = (dest_addr << 1) | rx_bit;

    arm::SysTickTimer   timer(32);
    while (!i2c2->sr1.any(I2c::Sr1::ADDR) && timer.elapsed32() < timeout)
        if (i2c2->sr1.any(I2c::Sr1::AF | I2c::Sr1::ARLO | I2c::Sr1::BERR)) {
            // no slave acknowledged with ACK (SDA low), or other error
            // note that ADDR does not get set
            // clear all
            i2c2->sr1 -= I2c::Sr1::AF | I2c::Sr1::ARLO | I2c::Sr1::BERR;
            status = PeriphStatus::ERROR;
        }
    // Can't check "if (!i2c2->sr1.any(I2c::Sr1::ADDR))" here because ADDR
    //   self-clears ("Schrodinger's Cat" bit).
    // Have to check if timer elapsed which is slightly risky because
    //   might have rolled over if max timeout.
    if (timer.elapsed32() >= timeout) {
        status = PeriphStatus::I2C_NOADDR;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
    volatile uint32_t   reader;
#pragma GCC diagnostic pop


    // wacky RM0008, AN2824, and stm32f1xx_hal_i2c.c methodology
    //
    if (rx_bit == 1) {  // master rx
        switch (rx_bytes) {
            case 1:
                reader     = i2c2->sr1.word();  // clear ADDR
                reader     = i2c2->sr2.word();  // 2nd phase of ADDR clear
                [[gnu::fallthrough]];
            case 0:  // impossible, wouldn't have been called
                i2c2->cr1 |= I2c::Cr1::STOP;
                break;
            case 2:
                i2c2->cr1 |= I2c::Cr1::ACK   ;
                i2c2->cr1 |= I2c::Cr1::POS   ;
                reader     = i2c2->sr1.word();  // clear ADDR
                reader     = i2c2->sr2.word();  // 2nd phase of ADDR clear
                i2c2->cr1 -= I2c::Cr1::ACK   ;
                break;
            default:
                i2c2->cr1 |= I2c::Cr1::ACK   ;
                reader     = i2c2->sr1.word();  // clear ADDR
                reader     = i2c2->sr2.word();  // 2nd phase of ADDR clear
                break;
        }
    }
    else {  // master tx
        reader = i2c2->sr1.word();  // already read, but doing because HAL does
        reader = i2c2->sr2.word();  // 2nd phase of ADDR clear
    }

    return status;
}



PeriphStatus i2c_master(
const uint32_t   timeout  ,
const uint8_t   *tx_data  ,
      uint8_t   *rx_data  ,
const uint8_t    dest_addr,
const uint8_t    tx_size  ,
const uint8_t    rx_size  ,
      uint8_t   &tx_sent  ,
      uint8_t   &rx_rcvd  )
{
    PeriphStatus        status = PeriphStatus::OK;
    arm::SysTickTimer   timer                    ;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
    volatile uint8_t    reader                   ;
#pragma GCC diagnostic pop
    bool                do_stop = true           ;

    tx_sent = 0;
    rx_rcvd = 0;

    timer.begin32();
    while (i2c2->sr2.any(I2c::Sr2::BUSY) && timer.elapsed32() < timeout)
        asm("nop");
    if (i2c2->sr2.any(I2c::Sr2::BUSY)) {
        status  = PeriphStatus::BUSY;
        goto i2c_stop;
    }

    if (tx_size > 0) {
        if ((status = i2c_master_start(timeout)) != PeriphStatus::OK)
            goto i2c_stop;

        if (  (status = i2c_master_addr(timeout, dest_addr, 0))
            != PeriphStatus::OK                                )
            goto i2c_stop;

        bool    btf_seen = true;
        while (tx_sent < tx_size) {
            btf_seen = false;
            timer.begin32();
            while (   !i2c2->sr1.any(  I2c::Sr1::TXE
                                     | I2c::Sr1::BTF
                                     | I2c::Sr1::AF )
                   && timer.elapsed32() < timeout)
                asm("nop");
            if (i2c2->sr1.any(I2c::Sr1::AF)) {
                // slave didn't ACK -- maybe doesn't want more bytes?
                // if just busy should have stretched clock instead
                status     = PeriphStatus::I2C_NACK ;
                i2c2->sr1 -= I2c::Sr1::AF           ;   // clear flag
                break;
            }
            if (!i2c2->sr1.any(I2c::Sr1::TXE | I2c::Sr1::BTF)) {
                status = PeriphStatus::TIMEOUT;
                goto i2c_stop;
            }
            if (i2c2->sr1.any(I2c::Sr1::BTF))
                btf_seen = true;
            i2c2->dr = tx_data[tx_sent++];
        }

        if (!btf_seen) {
            timer.begin32();
            while (!i2c2->sr1.any(I2c::Sr1::BTF) && timer.elapsed32() < timeout)
                asm("nop");
            if (i2c2->sr1.any(I2c::Sr1::BTF))
                reader = i2c2->dr;   // clear BTF (not necessary?)
            else {
                status  = PeriphStatus::I2C_NOBTF;
                goto i2c_stop;
            }
        }
    }

    if (rx_size > 0) {
        if ((status = i2c_master_start(timeout)) != PeriphStatus::OK)
            goto i2c_stop ;

        if (   (status = i2c_master_addr(timeout, dest_addr, 1, rx_size))
            != PeriphStatus::OK                                          )
            goto i2c_stop ;

        // will be set conditionally
        do_stop = false;  //

        // wacky RM0008, AN2824, and stm32f1xx_hal_i2c.c methodology
        //
        while (rx_rcvd < rx_size) {
            switch (rx_size - rx_rcvd) {
                case 0:  // can't happen
                case 1:
                    timer.begin32();
                    while (   !i2c2->sr1.any(I2c::Sr1::RXNE)
                           && timer.elapsed32() < timeout)
                        asm("nop");
                    if (!i2c2->sr1.any(I2c::Sr1::RXNE)) {
                        status  = PeriphStatus::I2C_NORXNE;
                        do_stop = true                    ;
                        goto i2c_stop                     ;
                    }
                    rx_data[rx_rcvd++] = i2c2->dr;
                    break;

                case 2:
                    timer.begin32();
                    while (   !i2c2->sr1.any(I2c::Sr1::BTF)
                           && timer.elapsed32() < timeout)
                        asm("nop");
                    if (!i2c2->sr1.any(I2c::Sr1::BTF)) {
                        status  = PeriphStatus::I2C_NOBTF;
                        do_stop = true                   ;
                        goto i2c_stop                    ;
                    }
                    i2c2->cr1 -= I2c::Cr1::ACK;
                    i2c2->cr1 |= I2c::Cr1::STOP;
                    // could this just read once and continue loop to case 1?
                    rx_data[rx_rcvd++] = i2c2->dr;  // totally ...
                    break;

                case 3:
                    timer.begin32();
                    while (   !i2c2->sr1.any(I2c::Sr1::BTF)
                           && timer.elapsed32() < timeout)
                        asm("nop");
                    if (!i2c2->sr1.any(I2c::Sr1::BTF)) {
                        status  = PeriphStatus::I2C_NOBTF;
                        do_stop = true                   ;
                        goto i2c_stop                    ;
                    }
                    rx_data[rx_rcvd++]  = i2c2->dr     ;
                    i2c2->cr1          -= I2c::Cr1::ACK;
                    break;

                default:
                    timer.begin32();
                    while (   !i2c2->sr1.any(I2c::Sr1::RXNE)
                           && timer.elapsed32() < timeout)
                        asm("nop");
                    if (!i2c2->sr1.any(I2c::Sr1::RXNE)) {
                        status  = PeriphStatus::I2C_NORXNE;
                        do_stop = true                    ;
                        goto i2c_stop                     ;
                    }
                    rx_data[rx_rcvd++] = i2c2->dr;
            }
        }
    }

    i2c_stop:
    if (do_stop) {
        i2c2->cr1 |= I2c::Cr1::STOP;
    }

    timer.begin32();
    // wait for stop to clear as per AN2824 "STM32F10xxx I2C optimized examples"
    // or should it be Sr1::STOPF ??
    while (i2c2->cr1.any(I2c::Cr1::STOP) && timer.elapsed32() < timeout)
        asm("nop");
    if (i2c2->cr1.any(I2c::Cr1::STOP))
        if (status == PeriphStatus::OK)
            status = PeriphStatus::I2C_NOSTOP;  // don't overwrite other error
    // clear STOP bit if set??
    i2c2->cr1 = I2c::Cr1::PE;

    return status;

}  // i2c_master()



PeriphStatus i2c_slave(
const uint32_t   timeout     ,
const uint8_t   *tx_data     ,
const uint8_t   *tx_dflt     ,
      uint8_t   *rx_data     ,
const uint8_t    tx_size     ,
const uint8_t    tx_dflt_size,
const uint8_t    rx_size     ,
      uint8_t   &oar_gc      ,
      uint8_t   &rx_rcvd     ,
      uint8_t   &tx_sent     )
{
    if (!i2c2->sr1.any(I2c::Sr1::ADDR))
        return PeriphStatus::EMPTY;

    PeriphStatus        status = PeriphStatus::OK;
    arm::SysTickTimer   timer;

    rx_rcvd = 0;
    tx_sent = 0;

    while (i2c2->sr1.any(I2c::Sr1::ADDR)) {
        // 2nd phase of ADDR clear and get master's tx/rx bit
        // TRA bit is backwards from reading of RM0008 (1==master tx, 0==rx
        I2c::sr2_t  sr2      =  i2c2->sr2             ;
        bool        slave_rx = !sr2.any(I2c::Sr2::TRA);

             if (sr2.any(I2c::Sr2::GENCALL)) oar_gc = 0;   // addressed as GC
        else if (sr2.any(I2c::Sr2::DUALF  )) oar_gc = 2;   //      "     " OAR2
        else                                 oar_gc = 1;   //      "     " OAR1

        if (slave_rx) {
            while (true) {
                timer.begin32();
                while (    !i2c2->sr1.any(  I2c::Sr1::RXNE
                                          | I2c::Sr1::BTF
                                          | I2c::Sr1::STOPF
                                          | I2c::Sr1::ADDR
                                          | I2c::Sr1::BERR)
                        && timer.elapsed32() < timeout     )
                    asm("nop");
                if (i2c2->sr1.any(I2c::Sr1::ADDR)) {
                    // repeated START for tx or (crazy) another rx
                    break;
                }
                if (i2c2->sr1.any(I2c::Sr1::STOPF)) {
                    i2c2->cr1 |= I2c::Cr1::PE;  // 2d phase of clearing STOPF
                    return status;
                }
                if (i2c2->sr1.any(I2c::Sr1::BERR)) {
                    // clear, and other flags in case set
                    i2c2->sr1 -= I2c::Sr1::BERR | I2c::Sr1::STOPF;
                    return PeriphStatus::ERROR;  // no waiting for STOP
                }
                if (timer.elapsed32() >= timeout) {
                    status = PeriphStatus::I2C_NORXNE;
                    break;
                }
                uint8_t     data = i2c2->dr;
                if (rx_rcvd < rx_size)
                    rx_data[rx_rcvd++] = data;
            }
        }
        else {
            uint8_t     dflt_ndx = 0    ;

            while (true) {
                timer.begin32();
                while (    !i2c2->sr1.any(  I2c::Sr1::TXE
                                          | I2c::Sr1::BTF
                                          | I2c::Sr1::AF
                                          | I2c::Sr1::STOPF
                                          | I2c::Sr1::ADDR
                                          | I2c::Sr1::BERR)
                        && timer.elapsed32() < timeout     )
                    asm("nop");
                if (i2c2->sr1.any(I2c::Sr1::STOPF)) {
                    i2c2->cr1 |= I2c::Cr1::PE;  // 2d phase of clearing STOPF
                    break;
                }
                if (i2c2->sr1.any(I2c::Sr1::ADDR)) {
                    // repeated START for (crazy) another tx or, worse, rx
                    break;
                }
                if (i2c2->sr1.any(I2c::Sr1::BERR)) {
                    // clear, and other flags in case set
                    i2c2->sr1 -= I2c::Sr1::BERR | I2c::Sr1::STOPF;
                    return PeriphStatus::ERROR;  // no waiting for STOP
                }
                if (i2c2->sr1.any(I2c::Sr1::AF))
                    break;
                if (timer.elapsed32() >= timeout)
                    return PeriphStatus::TIMEOUT;
                if (tx_sent < tx_size)
                    i2c2->dr = tx_data[tx_sent];
                else {
                    i2c2->dr = tx_dflt[dflt_ndx++];
                    if (dflt_ndx >= tx_dflt_size)
                        dflt_ndx  = 0;
                }
                ++tx_sent;  // always, even if sent default

                timer.begin32();
                while (    !i2c2->sr1.any(I2c::Sr1::BTF | I2c::Sr1::AF)
                        && timer.elapsed32() < timeout                 )
                    asm("nop");

                if (i2c2->sr1.any(I2c::Sr1::AF))
                    break;
            }

            timer.begin32();
            while (!i2c2->sr1.any(  I2c::Sr1::AF))
                asm("nop");
            if (!i2c2->sr1.any( I2c::Sr1::AF))
                status = PeriphStatus::TIMEOUT;
            i2c2->sr1 -= I2c::Sr1::AF;  // always do, idempotent if not set
        }
    }

    return status;

}  // i2c_slave()



/*
   Read USB packet containing header with data length, and then
   that amount of data. Fill in header and data buffers. Return
   data length, or zero if polling USB was empty
*/
unsigned bridge_recv(
uint8_t* const  header     ,
uint8_t* const  data       ,
const uint8_t   header_len ,
const uint8_t   size_offset,
const uint8_t   max_data   )
{
    if (!usb_recv.fill(0))
        return 0;

    usb_recv.fill(header_len);
    for (unsigned ndx = 0 ; ndx < header_len ; ++ndx)
        header[ndx] = usb_recv.byte(ndx);

    unsigned    data_len = header[size_offset];

    usb_recv.fill(header_len + data_len);
    for (unsigned ndx = 0 ; ndx < data_len && ndx < max_data ; ++ndx)
        data[ndx] = usb_recv.byte(header_len + ndx);

    usb_recv.flush(header_len + data_len);

    return header_len + data_len;
}



INLINE_DECL uint8_t INLINE_ATTR counter_increment(
const uint8_t       current  ,
const  int8_t       increment,
const uint8_t       low      ,
const uint8_t       high     )
{
    int32_t     incremented = current + increment;

    if (incremented > high) incremented = low ;
    if (incremented < low ) incremented = high;

    return incremented;
}



void rollover_triggering_timer_enable()
{
    gpiob->crh.ins(  Gpio::Crh:: CNF13_ALTFUNC_PUSH_PULL
                   | Gpio::Crh::MODE13_OUTPUT_2_MHZ     );  // 4Hz toggling

    // SysTick rollover timer
    // flip PB13 == adv_tim_1 channel 1N every time sys_tick rolls over
    // default/reset is timers/channel output, preload delay off
    // also used as blinking notification output while sampling (analog)

    // enable timer
    rcc_periph_enable_and_reset(rcc->apb2enr          ,
                                rcc->apb2rstr         ,
                                Rcc::Apb2enr ::TIM1EN ,
                                Rcc::Apb2rstr::TIM1RST);
    // clear timer
    adv_tim_1->sr  = 0;
    // divide 72 MHz main clock to 0.5 ms per tick
    adv_tim_1->psc = 35999;
    //  prescaler counter shadow register
    // generate "update event" to load shadow register into working
    adv_tim_1->egr = AdvTim_1::Egr::UG;
    // clear timer, especially UIF, so setting ARR doesn't start immediately
    // (or delayed ARR/ticks) generate interrupt
    adv_tim_1->sr = 0;
    // total cycle period, slightly faster than double SysTick rollover
    // of 0x1000000 / 72000000 = 0.233016888... seconds
    // 920 ticks * (36e3/72e3) seconds/tick == 0.46 seconds
    adv_tim_1->arr = 920;
    // default/reset is CCnS_OUT==0 (output)
    adv_tim_1->ccmr1 = AdvTim_1::Ccmr1::OC1M_PWM_MODE_2;
    // set "on" duty cycle to half of total period i.e. 0.23 seconds
    adv_tim_1->ccr1 = 460;
    // enable correct output channel on timer
    adv_tim_1->ccer = AdvTim_1::Ccer::CC1NE;
    // fix secret "make it not work" bit
    adv_tim_1->bdtr = AdvTim_1::Bdtr::MOE;
}

#endif  // #if 1 (peripheral I/O utilities)




#if 1  // (section identification for code browsing)
//
// host commands
//

void reset_ganged()
{
    static const uint8_t    // usb_recv.byte(NDX)
                            RESET_NDX = 1;   // (0,1) input, output open-drain

    usb_recv.fill(RESET_NDX + 1);

    if (usb_recv.byte(RESET_NDX))
        gpiob->crh.ins(  Gpio::Crh :: CNF14_OUTPUT_OPEN_DRAIN
                       | Gpio::Crh :: CNF15_OUTPUT_OPEN_DRAIN
                       | Gpio::Crh ::MODE14_OUTPUT_2_MHZ
                       | Gpio::Crh ::MODE15_OUTPUT_2_MHZ      );
    else
        gpiob->crh.ins(  Gpio::Crh :: CNF14_INPUT_PULL_UP_DOWN
                       | Gpio::Crh :: CNF15_INPUT_PULL_UP_DOWN
                       | Gpio::Crh ::MODE14_INPUT
                       | Gpio::Crh ::MODE15_INPUT             );

    // either:
    //   open-drain: PB14: init low, start sampling when all raise
    //                     external trigger when any lower
    //               PB15: init_low, ack external trigger when all raise
    //   input: nothing connected, ports read as constant zero
    gpiob->bsrr =    Gpio::Bsrr::BR14   // pull-down
                   | Gpio::Bsrr::BR15;  //  "    "

    usb_recv.flush(RESET_NDX + 1);
}



void flash_wait_pre()
{
    static const uint8_t    // usb_recv.byte(NDX)
                            WAIT_NDX = 1,  // (0,1,2) Flash::Acr::LATENCY_n_
                            FTCH_NDX = 2;  // (0,1) flash prefetch buffer enable

    usb_recv.fill(FTCH_NDX + 1);

    Flash::Acr ::bits_t       ftch
                            = Flash::Acr ::bits_t(usb_recv.byte(FTCH_NDX),
                                                  Flash::Acr ::PRFTBE_POS);
    Flash::Acr ::mskd_t       wait
                            = Flash::Acr ::mskd_t(Flash::Acr ::LATENCY_MASK,
                                                  usb_recv.byte(WAIT_NDX)  ,
                                                  Flash::Acr ::LATENCY_POS );

    if (!flash->acr.all(ftch | wait))
        flash->acr = ftch | wait;  // never using HLFCYA

    usb_recv.flush(FTCH_NDX + 1);
}



extern "C" {
    void trigger_and_sample_plain (const uint8_t    flash_or_ram   ,
                                   const uint16_t   num_samples    );
                                // const bool       dura_enabled
    void trigger_and_sample_ganged(const uint8_t    flash_or_ram   ,
                                   const uint16_t   num_samples    );
                                // const bool       dura_enabled
}



INLINE_DECL uint16_t INLINE_ATTR dma_adc()
{
    adc1->cr2 = Adc::Cr2::ADON;
    while (!adc1->sr.any(Adc::Sr::EOC))
        asm("nop");
    return adc1->dr.data;
}



void analog_sampling()
{
                            // usb_recv.byte(NDX)
    static const uint8_t    TRIGGER_CHANNEL_NDX =  1, // (0-7)
                             SECOND_CHANNEL_NDX =  2, // (0-7,>7) (!= trigger)
                              TRIGGER_SLOPE_NDX =  3, // (0,1,2) see below
                                       RATE_NDX =  4, // (0,7) s&h+adc code
                                     GANGED_NDX =  5, // (0,1) disabled/enabled
                            // usb_recv.shrt(NDX)
                                NUM_SAMPLES_NDX =  3, // (0-0xffff)
                                 TRIGGER_LO_NDX =  4, // (0-0xfff)
                                 TRIGGER_HI_NDX =  5, // (0-0xfff)
                            // other constants
                                COMMAND_LENGTH  = 12,
                // unneeded     SLOPE_NONE      =  0,  // no trigger, immediate
                                SLOPE_POSITIVE  =  1,  // trigger slope
                                SLOPE_NEGATIVE  =  2;  //    "      "

    usb_recv.fill(COMMAND_LENGTH);

    // locals
    const uint16_t  trigger_lo          = usb_recv.shrt(      TRIGGER_LO_NDX),
                    trigger_hi          = usb_recv.shrt(      TRIGGER_HI_NDX);
    const uint8_t   trigger_channel     = usb_recv.byte( TRIGGER_CHANNEL_NDX),
                    second_channel      = usb_recv.byte(  SECOND_CHANNEL_NDX),
                    trigger_slope       = usb_recv.byte(   TRIGGER_SLOPE_NDX);
    const bool      ganged              = usb_recv.byte(          GANGED_NDX);
    // globals
                    // raw memory words, one or two samples per word
                    num_analog_words    = usb_recv.shrt(     NUM_SAMPLES_NDX),
                    analog_sample_rate  = usb_recv.byte(            RATE_NDX),
                    num_analog_channels =     second_channel
                                            > MAX_ADC_CHANNEL_NUM
                                          ? 1
                                          : 2                                ;

    usb_recv.flush(COMMAND_LENGTH);

    if (num_analog_words == 0) return;

    analog_channels.trigger = trigger_channel;
    analog_channels.second  =  second_channel;

    // check for port conflicts with PWM running on PA1-3 and kill if so
    if (rcc->apb1enr.any(Rcc::Apb1enr ::TIM2EN)) {
        // pwm() is running, gen_tim_2 has PA1-3
        if (   (trigger_channel >= 1 && trigger_channel <= 3)
            || ( second_channel >= 1 &&  second_channel <= 3))
            pwm_disable();
    }


    // setup sample memory
    // all calculations in uint32_t word size
    // each channel requires uint16_t (to store 12 bit ADC values)
    unsigned    memory_available = static_cast<unsigned>(  &STORAGE_END
                                                         - &STORAGE    );
    if (num_analog_words > memory_available)
        num_analog_words = memory_available;

    // samples and samples_end are uint32_t*
    samples     = &STORAGE_END - num_analog_words;
    samples_end = &STORAGE_END                   ;

    // set port(s) to analog
    //
    gpioa->crl.ins(
      Gpio::Crl::mskd_t(Gpio::Crl::MASK ,
                        Gpio::Crl::INPUT,
                        Gpio::Crl::pos_t((trigger_channel << 2) + 2))
    | Gpio::Crl::mskd_t(Gpio::Crl::MASK                   ,
                        Gpio::Crl::INPUT_ANALOG           ,
                        Gpio::Crl::pos_t( trigger_channel << 2     )));

    if (num_analog_channels == 2)
        gpioa->crl.ins(
          Gpio::Crl::mskd_t(Gpio::Crl::MASK ,
                            Gpio::Crl::INPUT,
                            Gpio::Crl::pos_t((second_channel << 2) + 2))
        | Gpio::Crl::mskd_t(Gpio::Crl::MASK                   ,
                            Gpio::Crl::INPUT_ANALOG           ,
                            Gpio::Crl::pos_t( second_channel << 2     )));



    // ADC clock: max 14 MHz. 12 MHz is closest possible (default is
    //   DIV_2 == 36 MHz)
    rcc->cfgr.ins(Rcc::Cfgr::ADCPRE_DIV_6);

    // enable ADC peripheral(s)
    rcc_periph_enable_and_reset(rcc->apb2enr          ,
                                rcc->apb2rstr         ,
                                Rcc::Apb2enr ::ADC1EN ,
                                Rcc::Apb2rstr::ADC1RST);
    if (num_analog_channels == 2)
        rcc_periph_enable_and_reset(rcc->apb2enr          ,
                                    rcc->apb2rstr         ,
                                    Rcc::Apb2enr ::ADC2EN ,
                                    Rcc::Apb2rstr::ADC2RST);


    // calibrate ADC(s)
    //

    // reset calibration register
    adc1->cr2 |= Adc::Cr2::ADON;
    // RM0008: "at least two ADC clock cycles"
    sys_tick_timer.delay32(36);  // 3 for safety, 12 main clock per 1 ADC clock
    adc1->cr2 |= Adc::Cr2::ADON | Adc::Cr2::RSTCAL;
    while (adc1->cr2.any(Adc::Cr2::RSTCAL))  // need timeout safety
        asm("nop");

    // calibrate
    adc1->cr2 |= Adc::Cr2::ADON | Adc::Cr2::CAL;
    // wait for calibration finish
    while (adc1->cr2.any(Adc::Cr2::CAL))     // need timeout safety
        asm("nop");

    if (num_analog_channels == 2) {  // same procedure as adc1
        adc2->cr2 |= Adc::Cr2::ADON;
        sys_tick_timer.delay32(36);
        adc2->cr2 |= Adc::Cr2::ADON | Adc::Cr2::RSTCAL;
        while (adc2->cr2.any(Adc::Cr2::RSTCAL))  // need timeout safety
            asm("nop");

        adc2->cr2 |= Adc::Cr2::ADON | Adc::Cr2::CAL;
        while (adc2->cr2.any(Adc::Cr2::CAL))     // need timeout safety
            asm("nop");
    }


    // set up ADC(s)
    //

    Adc::Cr1::bits_t    adc1_cr1;
    Adc::Cr2::bits_t    adc1_cr2;

    if (num_analog_channels == 1) {
        adc1_cr2 =    Adc::Cr2::EXTTRIG
                    | Adc::Cr2::EXTSEL_SWSTART
                    | Adc::Cr2::DMA
                    | Adc::Cr2::CONT
                    | Adc::Cr2::ADON                 ;
        adc1_cr1 =    Adc::Cr1::SCAN                 ;
    }
    else {  // num_analog_channels == 2
        adc1_cr2 =    Adc::Cr2::EXTTRIG
                    | Adc::Cr2::EXTSEL_SWSTART
                    | Adc::Cr2::DMA
                    | Adc::Cr2::CONT
                    | Adc::Cr2::ADON                 ;
        adc1_cr1 =    Adc::Cr1::DUALMOD_REGULAR_SIMUL
                    | Adc::Cr1::SCAN                 ;
        adc2->cr2 =   Adc::Cr2::EXTTRIG
                    | Adc::Cr2::EXTSEL_SWSTART
                    | Adc::Cr2::CONT
                    | Adc::Cr2::ADON                 ;
        adc2->cr1 =   Adc::Cr1::SCAN                 ;

        adc2->sqr1  = Adc::Sqr1::L<0>       (                  );  // 1 channel
        adc2->sqr3  = Adc::Sqr3::sq1        (second_channel    );  // 1st chan
        adc2->smpr2 = analog_sample_rate << (second_channel * 3);  // chan# pos
    }

    // adc1 is either master ADC (if num_analog_channels==1) else is only ADC
    adc1->sqr1  = Adc::Sqr1::L<0>       (                   );  // 1 channel
    adc1->sqr3  = Adc::Sqr3::sq1        (trigger_channel    );  // 1st chan
    adc1->smpr2 = analog_sample_rate << (trigger_channel * 3);  // chan# pos


    // set up DMA
    //

    // enable DMA peripheral; ahbenr has no corresponding "rstr" as per apb1,2
    rcc->ahbenr |= Rcc::Ahbenr::DMA1EN;

    // must be disabled when setting parameters/flags/bits
    dma1_channel1->ccr = 0;

    dma1_channel1->pa  = reinterpret_cast<uint32_t>(&adc1->dr.dr);
    dma1_channel1->ma  = reinterpret_cast<uint32_t>(samples     );  // 16bit OK

    // finish and enable
    uint32_t      dma_ccr;   // save for fast resetting after triggering
    if (num_analog_channels == 1)
        dma_ccr = (  DmaChannel::Ccr::MINC
                   | DmaChannel::Ccr::DIR_PERIPH2MEM
                   | DmaChannel::Ccr::PSIZE_16_BITS
                   | DmaChannel::Ccr::MSIZE_16_BITS
                   | DmaChannel::Ccr::PL_LOW
                   | DmaChannel::Ccr::EN             ).bits();
    else  //  num_analog_channels == 2
        dma_ccr = (  DmaChannel::Ccr::MINC
                    | DmaChannel::Ccr::DIR_PERIPH2MEM
                    | DmaChannel::Ccr::PSIZE_32_BITS
                    | DmaChannel::Ccr::MSIZE_32_BITS
                    | DmaChannel::Ccr::PL_LOW
                    | DmaChannel::Ccr::EN            ).bits();
    // set for triggering
    dma1_channel1->ndt =   num_analog_channels == 1
                         ? num_analog_words << 1    // two samples per word
                         : num_analog_words         ;
    dma1_channel1->ccr = dma_ccr                    ;


    // optional ganged sync
    if (ganged) {
        // set trigger ready high to inform others self is ready
        gpiob->bsrr = Gpio::Bsrr::BS14;
        // wait for open-drain line to go high indicating all are ready
        while (!gpiob->idr.any(Gpio::Idr::IDR14))
            asm("nop");
    }


    // triggering
    //

    // set global state variables
    sampling_mode = SamplingMode::ANALOG                         ;
    in_progress   =   InProgress::ANALOG | InProgress::TRIGGERING;

    // wait for trigger
    // uint32_t* samples will have either trigger channel value in
    //   low 12 bits of low 16 bits of little-endian 32 bit word (if
    //   num_analog_channels==2) or will be treated as uint16_t* by DMA
    //   engine and again have trigger channel 12 bits
    if (ganged)
        switch (trigger_slope) {
            case SLOPE_POSITIVE:
                while (   dma_adc() >= trigger_lo
                       && gpiob->idr.any(Gpio::Idr::IDR14));
                while (   dma_adc() <  trigger_hi
                       && gpiob->idr.any(Gpio::Idr::IDR14));
                break;

            case SLOPE_NEGATIVE:
                while (   dma_adc() <= trigger_hi
                       && gpiob->idr.any(Gpio::Idr::IDR14));
                while (   dma_adc() >  trigger_lo
                       && gpiob->idr.any(Gpio::Idr::IDR14));
                break;

            default:  // SLOPE_NONE, immediate start
                break;
        }
    else
        switch (trigger_slope) {
            case SLOPE_POSITIVE:
                while (dma_adc() >= trigger_lo);
                while (dma_adc() <  trigger_hi);
                break;

            case SLOPE_NEGATIVE:
                while (dma_adc() <= trigger_hi);
                while (dma_adc() >  trigger_lo);
                break;

            default:  // SLOPE_NONE, immediate start
                break;
        }

    // DMA "finished" bit set for some reason immediately after
    // enabling peripheral, and also after sampling. Clear to
    // prevent premature finish of sampling.
    dma1->ifcr = Dma::Ifcr::CGIF1;  // write 1 to clear Isr::TCIF1 bit

    // optional ganged sync
    if (ganged) {
        if (gpiob->idr.any(Gpio::Idr::IDR14))
            // signal others that have triggered on own level/hysteresis
            gpiob->bsrr = Gpio::Bsrr::BR14;
        else
            // got external trigger from another buck50
            in_progress |= InProgress::EXTERN_TRIG;
        // set sync high to inform others self is ready
        gpiob->bsrr = Gpio::Bsrr::BS15;
        // wait for open-drain line to go high indicating all are ready
        while (!gpiob->idr.any(Gpio::Idr::IDR15))
            asm("nop");
    }


    // sampling
    //

    // set to do DMA ADC
    adc1->cr1 = adc1_cr1;

    in_progress &=   ~InProgress::TRIGGERING;
    in_progress |=    InProgress::ANALOG
                    | InProgress::TRIGGERED
                    | InProgress::SAMPLING  ;

    // start sampling and wait for finish
    adc1->cr2 =   Adc::Cr2::SWSTART
                | Adc::Cr2::EXTTRIG
                | Adc::Cr2::EXTSEL_SWSTART
                | Adc::Cr2::DMA
                | Adc::Cr2::CONT
                | Adc::Cr2::ADON          ;
    while (!dma1->isr.any(Dma::Isr::TCIF1))
        asm("nop");

    // clean up
    adc_disable             ();
    reset_timer_ganged_ports();

    // send results to host
    send_buf    [0] = halt_code = HaltCode::MEMORY;
    send_buf    [1] = num_analog_channels         ;
    send_buf    [2] = analog_channels.byte        ;
    send_buf    [3] = analog_sample_rate          ;
    send_uint16s[2] = num_analog_words            ;
    send_uint16s[3] = in_progress                 ;
    usb_send(8);

    in_progress = InProgress::IDLE;

}  // analog_sampling()



void digital_sampling()
{
                            // usb_recv.byte(NDX)
    static const uint8_t    SAMPLING_MODE_NDX =  1,  // SamplingMode
                              MAX_TRIGGER_NDX =  2,
                                     GANG_NDX =  3,  // (0,1) disabled/enabled
                             DURA_ENABLED_NDX =  4,  // (0,1) disabled/enabled
                              CODE_MEMORY_NDX =  5,  // (0,1) ram/flash
                            // usb_recv.shrt(NDX)
                                 DURATION_NDX =  3,  // gen_tim_3 arr
                              NUM_SAMPLES_NDX =  4,  // num_samples
                            // other constant
                                      CMD_LEN = 12;  // 32-bit align triggers

    usb_recv.fill(CMD_LEN);

    sampling_mode = usb_recv.byte(SAMPLING_MODE_NDX);

    uint16_t        num_samples   = usb_recv.shrt( NUM_SAMPLES_NDX);
    const uint16_t  duration      = usb_recv.shrt(    DURATION_NDX);
    const bool      dura_enabled  = usb_recv.byte(DURA_ENABLED_NDX),
                    ganged        = usb_recv.byte(        GANG_NDX);
    const uint8_t   num_triggers  = usb_recv.byte( MAX_TRIGGER_NDX),
                    code_memory   = usb_recv.byte( CODE_MEMORY_NDX);

    usb_recv.flush(CMD_LEN);

    i2c_disable();  // in case running on PB10,11

    // sanity check: always store sample which caused trigger, plus two more
    // allow asm code chance to set in_progress=SAMPLING for hardfault handler
    if (num_samples < 3)
        num_samples = 3;

    // triggers
    for (unsigned ndx= 0 ; ndx < num_triggers ; ++ndx) {
        usb_recv.fill(sizeof(Trigger));
        triggers[ndx].word = usb_recv.word(0);
        usb_recv.flush(sizeof(Trigger));
    }

    // GPIOB CRL 4 through 11  sampling pins: input, pull-down (default)
    // GPIOB 2 is BOOT1, tied to GND (normally) or Vdd, is OK, constant
    // all others disconnected same, (default)
    // except GPIOB 13, adv_tim_1, CH1N, for systick rollover toggling
    // same for GPIOB 0,1,3,12-15 (unconnected need to be constant)
    rollover_triggering_timer_enable();

    in_progress = InProgress::TRIGGERING;

    if (dura_enabled) duration_timer(duration, true);

    user_led_off();  // triggering/sampling started

    // in_progress already set at function entry
    if (ganged) {
        gpiob->bsrr = Gpio::Bsrr::BS14; // raise, tell others self is ready
        trigger_and_sample_ganged(code_memory, num_samples); // dura_enabled
    }
    else
        trigger_and_sample_plain (code_memory, num_samples); // dura_enabled

}  // digital_sampling()



void wait_connect_signature()
{
    // Wait until user interface  program sends CONNECT_SIGNATURE before
    //   entering main command loop because Linux CDC/ACM driver sends
    //   UART configuration garbage when /dev/ACMx opened
    // Can be misinterpreted as command (have seen 13/0xd/'\r')

    unsigned    bytes_matched = 0;
    while (bytes_matched < CONNECT_SIGNATURE_LENGTH) {
        unsigned    rcvd = usb_recv.fill(CONNECT_SIGNATURE_LENGTH);

        for (uint8_t ndx = 0 ; ndx < rcvd ; ++ndx) {
            if (usb_recv.byte(ndx) == CONNECT_SIGNATURE[bytes_matched])
                ++bytes_matched;
            else
                bytes_matched = 0;  // incorrect byte, start from beginning
        }

        usb_recv.flush(rcvd);
    }

    // confirm received signature to host
    send_uint32s[0] = IDENTITY;
    usb_send(4);
}



void send_samples()
{
    static const uint8_t    // usb_recv.shrt(NDX)
                            FIRST_NDX = 1,
                            COUNT_NDX = 2;

    usb_recv.fill((COUNT_NDX + 1) << 1);

    uint16_t    first = usb_recv.shrt(FIRST_NDX),
                count = usb_recv.shrt(COUNT_NDX);

    usb_recv.flush((COUNT_NDX + 1) << 1);

    if (   sampling_mode          == SamplingMode::ANALOG
        && analog_channels.second  > MAX_ADC_CHANNEL_NUM ) {
        // two samples per uint32_t
        first >>= 1;
        count >>= 1;
    }

    uint32_t    *smpl = samples + first;
    if (smpl > samples_end)
        smpl = samples_end;

    uint32_t    *last = smpl + count;
    if (last > samples_end)
        last = samples_end;

    send_uint16s[ 0] = smpl         - samples ;  // index of first sample
    send_uint16s[ 1] = last         - smpl    ;  // count of samples to upload
    send_uint16s[ 2] = samples_end  - samples ;  // number of captured samples
    send_uint16s[ 3] = &STORAGE_END - &STORAGE;  // sample memory total size
    send_buf    [ 8] = sampling_mode          ;
    send_buf    [ 9] = num_analog_channels    ;
    send_buf    [10] = analog_channels.byte   ;
    send_buf    [11] = analog_sample_rate     ;
    send_uint16s[ 6] = num_analog_words       ;
    usb_send(14);

    unsigned    buf_ndx = 0;

    while (smpl < last) {
        send_uint32s[buf_ndx++] = *smpl++;

        if (buf_ndx == SEND_BUF_UINT32S) {
            usb_send(UsbDevCdcAcm::CDC_IN_DATA_SIZE);
            buf_ndx = 0;
        }
        else if (smpl == last)
            usb_send(buf_ndx << 2);
    }

    if (buf_ndx == 0)
        // last send was exactly IN endpoint size
        // have to send zero-length xfer to let host USB driver know is end
        usb_send(0);
}



void live()
{
    namespace live  =  live_command;
    namespace usart = usart_command;
    namespace spi   =   spi_command;
    namespace i2c   =   i2c_command;
    namespace adc   =   adc_command;

    usb_recv.fill(live::CMD_LEN);

    Sbrk         sbrk                                                       ;
    const
    uint64_t     duration      =   (static_cast<uint64_t>
                                   (usb_recv.word(live::DURATION_HI)) << 32)
                                 |  usb_recv.word(live::DURATION_LO)        ,
                 live_speed    =   (static_cast<uint64_t>
                                   (usb_recv.word(live::RATE_HI)) << 32)
                                 |  usb_recv.word(live::RATE_LO)            ;
    volatile
    Usart           *usart_n                                                ;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    AdcLive         *adcs                                                   ;
    PeriphStatus    *adc_stati                                              ;
    uint16_t        *adc_fltds                                              ,
                    *adc_datas                                              ,
                    *adc_prevs                                              ;
    uint8_t         *adc_chans                                              ;
#pragma GCC diagnostic pop
    unsigned         num_adcs      = 0                                      ;
    uint32_t         nss_delay                                              ,
                     usart_timeout                                          ,
                     usart_rate                                             ,
                     usart_wait                                             ,
                     spi_timeout                                            ,
                     spi_rx_wait                                            ,
                     spi_rate                                               ,
                     i2c_timeout                                            ;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    uint8_t          usart_tx_dflt                                          ,
                    *usart_rx_data                                          ,
                    *  spi_tx_data                                          ,
                    *  spi_rx_data                                          ,
                    *  i2c_tx_data                                          ,
                    *  i2c_rx_data                                          ;
    const uint8_t    adcs_enabled  = usb_recv.byte(live::ADCS )             ;
    uint8_t          gpio_mask     = 0xff /* maybe mask out I2C lines */    ,
                     usart_rx_len                                           ,
                     spi_tx_len                                             ,
                     i2c_tx_len                                             ,
                     i2c_rx_len                                             ,
                     i2c_oar_gc    = 3  /* master/unused */                 ,
                     i2c_dest                                               ;
#pragma GCC diagnostic pop
    const bool       gpio_enable   = usb_recv.byte(live::GPIO )             ,
                     usart_enable  = usb_recv.byte(live::USART)             ,
                     spi_enable    = usb_recv.byte(live::SPI  )             ,
                     i2c_enable    = usb_recv.byte(live::I2C  )             ;
    bool             usart_synchro                                          ,
                     spi_master                                             ,
                     i2c_is_mstr                                            ;

    usb_recv.flush(live::CMD_LEN);

    if (usart_enable) {
        usb_recv.fill(usart::CMD_LEN);

        uint8_t usart_bits = usb_recv.byte(usart::BITS_1);

        usart_tx_dflt = usb_recv.byte(usart::TX_DFLT    );
        usart_rx_len  = usb_recv.byte(usart::RX_LEN     );
        usart_timeout = usb_recv.word(usart::TX_TMO     );
        usart_wait    = usb_recv.word(usart::RX_WAIT    );
        usart_rate    = usb_recv.word(usart::RATE       );
        usart_synchro = usart_bits & (1 << usart::CLOCK );

        usart_rx_data = sbrk(usart_rx_len);

        usart_n = usart_activate();

        usb_recv.flush(usart::CMD_LEN);
    }

    if (spi_enable) {
        usb_recv.fill(spi::CMD_LEN);

        spi_master = usb_recv.byte(spi::MASTER   );
        spi_timeout = usb_recv.word(spi::TX_TMOUT);
        spi_rate    = usb_recv.word(spi::RATE    );
        spi_rx_wait = usb_recv.word(spi::RX_WAIT );
        nss_delay   = usb_recv.word(spi::NSS_DLAY);
        spi_tx_len  = usb_recv.byte(spi::TX_LEN  );

        usb_recv.fill(spi::CMD_LEN + spi_tx_len);

        spi_tx_data = sbrk(spi_tx_len);
        spi_rx_data = sbrk(spi_tx_len);  // SPI always 1-to-1 MOSI-MISO

        for (unsigned ndx = 0 ; ndx < spi_tx_len ; ++ndx)
            spi_tx_data[ndx] = usb_recv.byte(spi::CMD_LEN + ndx);

        spi_activate();

        usb_recv.flush(spi::CMD_LEN + spi_tx_len);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        if (!spi_master)
            // preload MISO buffer ASAP, ignore any MOSI
            spi_tx_rx_slav(*spi_tx_data, *spi_rx_data);
#pragma GCC diagnostic pop
    }

    if (i2c_enable) {
        gpio_mask &= 0b00111111;  // mask out PB10 and 11, I2C lines

        usb_recv.fill(i2c::CMD_LEN);

        uint8_t i2c_bits = usb_recv.byte(i2c::BITS);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        i2c_is_mstr = i2c_bits  &  (1 << i2c::MASTER   );   // bool
        i2c_dest    = usb_recv.byte(     i2c::DEST     );
        i2c_timeout = usb_recv.word(     i2c::TIMEOUT  );
        i2c_tx_len  = usb_recv.byte(     i2c::DFLT_SIZE);
        i2c_rx_len  = usb_recv.byte(     i2c::  RX_SIZE);
#pragma GCC diagnostic pop

        usb_recv.fill(i2c::CMD_LEN + i2c_tx_len);

        i2c_tx_data = sbrk(i2c_tx_len);
        i2c_rx_data = sbrk(i2c_rx_len);

        i2c_tx_data[0] = 0;  // in case no tx data from host
        for (unsigned ndx = 0 ; ndx < i2c_tx_len ; ++ndx)
            i2c_tx_data[ndx] = usb_recv.byte(i2c::CMD_LEN + ndx);

        i2c_activate();

        usb_recv.flush(i2c::CMD_LEN + i2c_tx_len);
    }

    if (adcs_enabled) {
        num_adcs  = num_bits_set(adcs_enabled)                          ;
        adcs      = reinterpret_cast<     AdcLive*>(sbrk(num_adcs << 2));//32bit
        adc_chans =                                 sbrk(num_adcs      );
        adc_stati = reinterpret_cast<PeriphStatus*>(sbrk(num_adcs     ));
        adc_datas = reinterpret_cast<    uint16_t*>(sbrk(num_adcs << 1));
        adc_fltds = reinterpret_cast<    uint16_t*>(sbrk(num_adcs << 1));
        adc_prevs = reinterpret_cast<    uint16_t*>(sbrk(num_adcs << 1));

        bool        pwm_conflict = false;
        unsigned    adcs_ndx     = 0    ;
        for (unsigned channel = 0 ; channel < 8 ; ++channel) // max 8 chans/bits
            if (adcs_enabled & (1 << channel)) {
                if (adcs_ndx >= 1 && adcs_ndx <= 3)
                    pwm_conflict = true;
                adc_chans[adcs_ndx] = channel;
                                      usb_recv.fill (adc::CMD_LEN);
                adcs[adcs_ndx].word = usb_recv.word (0           );
                                      usb_recv.flush(adc::CMD_LEN);
                ++adcs_ndx;
            }

        if (pwm_conflict) pwm_disable();

        uint8_t     *adc_rates = sbrk(num_adcs);
        for (unsigned ndx = 0 ; ndx < num_adcs ; ++ndx) {
            adc_rates[ndx] = adcs[ndx].samp_hold & 0xf;  // is samp_hold:4, LSBs
            adc_prevs[ndx] = 0xffff;
            // don't need to init adc_stati or adc_fltds
        }

        adc_activate(num_adcs, adcs_enabled, adc_chans, adc_rates);

        adc1->cr2 |= Adc::Cr2::SWSTART;
        for (unsigned ndx = 0 ; ndx < num_adcs ; ++ndx) {
            while (!adc1->sr.any(Adc::Sr::EOC))
                asm("nop");
            adc_fltds[ndx] = adc1->dr.data;
        }
    }


    // initial values other than ADCs
    //
    PeriphStatus           spi_status   = PeriphStatus::OK,
                           i2c_status   = PeriphStatus::OK,
                         usart_status   = PeriphStatus::OK;
    arm::SysTickTimer    slowdown_timer                   ;
    unsigned             usart_rcvd     = 0               ,
                         spi_sent_rcvd  = 0               ;
    bool                 slowing        = false           ,
                           adc_chng     = false           ,
                          gpio_chng     = false           ,
                           spi_chng     = false           ,
                         usart_chng     = false           ,
                           i2c_chng     = false           ;
    uint16_t              gpio_prev     = 0x0100          ; // force != 1st time
    uint8_t              gpio_crnt      = 0               ,
                         i2c_sent                         ,
                         i2c_rcvd       = 0               ;

    sys_tick_timer.begin64();  // timestamp
    while (sys_tick_timer.elapsed64() < duration) {
        sys_tick_timer.update64();
        if (slowing && slowdown_timer.elapsed64() >= live_speed)
            slowing = false;

        if (usb_recv.fill(0))
            break;

        // check passive inputs
        //
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        if (usart_enable && !usart_synchro && !usart_rcvd)
            usart_status = usart_buf_rx(usart_n      ,
                                        usart_rx_data,
                                        usart_rx_len ,
                                        usart_wait   ,
                                        Command::HALT,
                                        usart_rcvd   );

        if (spi_enable && !spi_master && !spi_sent_rcvd)
            spi_status = spi_tx_rx_buf_slav(spi_tx_data ,
                                            0           ,
                                            spi_rx_data ,
                                            spi_tx_len  ,
                                            0           ,
                                            spi_rx_wait ,
                                            Command::HALT,
                                            spi_sent_rcvd);

        if (i2c_enable && !i2c_is_mstr && !i2c_rcvd)
            i2c_status = i2c_slave(i2c_timeout ,
                                   0           ,     // only default data
                                   i2c_tx_data ,
                                   i2c_rx_data ,
                                   0           ,     // only default
                                   i2c_tx_len  ,
                                   i2c_rx_len  ,     // discard any over
                                   i2c_oar_gc  ,
                                   i2c_rcvd    ,
                                   i2c_sent    );
#pragma GCC diagnostic pop

        if (slowing && !usart_rcvd && !spi_sent_rcvd && !i2c_rcvd)
            continue;

        // read active inputs
        //

        // bits 4-11 (others guaranteed constant (zero? might need mask)
        if (gpio_enable) {
            gpio_crnt = (gpiob->idr.word() >> 4) & gpio_mask;
            gpio_chng = (gpio_crnt != gpio_prev)            ;
        }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        if (spi_enable && spi_master)
            spi_status = spi_tx_rx_buf_mstr(spi_tx_data  ,
                                            spi_rx_data  ,
                                            spi_tx_len   ,
                                            spi_timeout  ,
                                            nss_delay    ,
                                            spi_rate     ,
                                            Command::HALT,
                                            spi_sent_rcvd);

        if (usart_enable && usart_synchro)
            usart_status = usart_buf_tx_rx( usart_n      ,
                                           &usart_tx_dflt,
                                            usart_rx_data,
                                            1            ,
                                            usart_timeout,
                                            usart_rate   ,
                                            Command::HALT,
                                            usart_rcvd   );

        if (i2c_enable && i2c_is_mstr)
            i2c_status = i2c_master(i2c_timeout ,
                                    i2c_tx_data ,
                                    i2c_rx_data ,
                                    i2c_dest    ,
                                    i2c_tx_len  ,
                                    i2c_rx_len  ,
                                    i2c_sent    ,
                                    i2c_rcvd    );

        if (adcs_enabled) {
            adc_chng = false;

            adc1->cr2 |= Adc::Cr2::SWSTART;
            for (unsigned ndx = 0 ; ndx < num_adcs ; ++ndx) {
                while (!adc1->sr.any(Adc::Sr::EOC))
                    asm("nop");
                adc_datas[ndx] = adc1->dr.data;
            }

            // separate loop so above can be fast
            for (unsigned ndx = 0 ; ndx < num_adcs ; ++ndx) {
                AdcLive     *adc_live = adcs + ndx;
                unsigned    adc_expn =      adc_live->exponent  ,
                            adc_wndw = 1 << adc_live->exponent  ,
                            adc_wght =      adc_live->weight    ,
                            adc_hyst =      adc_live->hysteresis;

                adc_fltds[ndx] =   (  adc_fltds[ndx] * (adc_wndw - adc_wght)
                                    + adc_datas[ndx] * (           adc_wght))
                                 >> adc_expn                                 ;

                if (abs(adc_fltds[ndx] - adc_prevs[ndx]) > adc_hyst) {
                    adc_prevs[ndx] = adc_fltds[ndx];
                    adc_chng       = true          ;
                }

                adc_stati[ndx] = PeriphStatus::OK;
            }
        }

        if (usart_enable &&    usart_rcvd) usart_chng = true;
        if (  spi_enable && spi_sent_rcvd)   spi_chng = true;
        if (  i2c_enable &&      i2c_rcvd)   i2c_chng = true;
#pragma GCC diagnostic pop

        if (    gpio_chng
            ||   adc_chng
            || usart_chng
            ||   spi_chng
            ||   i2c_chng) {

            uint64_t    timestamp      = sys_tick_timer.elapsed64();
            uint8_t     gpio_adc_words = 3                         ;

            send_uint32s[0] = timestamp  & 0xffffffff;
            send_uint32s[1] = timestamp >> 32        ;

            // always send (and update _prev) even if not enabled
            // to keep aligned and fixed initial size
            send_uint32s[2] = gpio_crnt;
            gpio_prev       = gpio_crnt;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
            for (unsigned ndx = 0 ; ndx < num_adcs ; ++ndx)
                  send_uint32s[gpio_adc_words++]
                =   (adc_chans[ndx]                       << 24)
                  | (static_cast<uint8_t>(adc_stati[ndx]) << 16)
                  | adc_fltds[ndx]                              ;

            usb_send_w_zlp(gpio_adc_words << 2);

            if (usart_enable) {
                send_buf[0] =                      usart_rx_len ;
                send_buf[1] = static_cast<uint8_t>(usart_status);

                unsigned ndx = 0;
                for ( ; ndx < usart_rcvd && ndx < usart_rx_len ; ++ndx)
                    send_buf[2 + ndx] = usart_rx_data[ndx];
                while (ndx < usart_rx_len)
                    send_buf[2 + ndx++] = 0;
                usb_send_w_zlp(ndx + 2);
                usart_rcvd = 0;
            }

            if (spi_enable) {
                send_buf[0] =                      spi_tx_len ;
                send_buf[1] = static_cast<uint8_t>(spi_status);

                unsigned ndx = 0;
                for ( ; ndx < spi_sent_rcvd && ndx < spi_tx_len ; ++ndx)
                    send_buf[2 + ndx] = spi_rx_data[ndx];
                while (ndx < spi_tx_len)
                    send_buf[2 + ndx++] = 0;
                usb_send_w_zlp(ndx + 2);
                spi_sent_rcvd = 0      ;
            }

            if (i2c_enable) {
                send_buf[0]  = static_cast<uint8_t>(i2c_status);
                send_buf[1]  =                      i2c_oar_gc ;
                send_buf[2]  =                      i2c_rx_len ;
                unsigned ndx = 0                               ;
                for ( ; ndx < i2c_rcvd && ndx < i2c_rx_len ; ++ndx)
                    send_buf[3 + ndx] = i2c_rx_data[ndx];
                while (ndx < i2c_rx_len)
                    send_buf[3 + ndx++] = 0;
                usb_send_w_zlp(ndx + 3);
                i2c_rcvd = 0;
            }
#pragma GCC diagnostic pop
        }

        // faster to do than to check if necessary because xxx_enabled
        usart_chng = false;
          spi_chng = false;
          i2c_chng = false;

        slowing = true          ;
        slowdown_timer.begin64();
    }  // while (true)

    // disable
    if (  adcs_enabled)   adc_disable(       );
    if ( usart_enable ) usart_disable(usart_n);
    if (   spi_enable )   spi_disable(       );
    if (   i2c_enable )   i2c_disable(       );

}  // live()



void pwm()
{
                                // usb_recv.byte(NDX)
    static const uint8_t            ENABLE_NDX =  1,  // (0,1) enable,disable
                                    MODE_2_NDX =  2,  // (0-7) channel 2
                                    MODE_3_NDX =  3,  // (0-7) channel 3
                                    MODE_4_NDX =  4,  // (0-7) channel 3
                                                      // GPIO pin numbers
                                PP_OD_HI_1_NDX =  5,  // (0,1,2) p-p/opn/hiz
                                PP_OD_HI_2_NDX =  6,  // (0,1,2) p-p/opn/hiz
                                PP_OD_HI_3_NDX =  7,  // (0,1,2) p-p/opn/hiz
                                   SPEED_1_NDX =  8,  // (1,2,3) 10/2/50 MHz max
                                   SPEED_2_NDX =  9,  // (1,2,3) 10/2/50 MHz max
                                   SPEED_3_NDX = 10,  // (1,2,3) 10/2/50 MHz max
                                // usb_recv.shrt(NDX)
                                       PSC_NDX =   6,
                                       ARR_NDX  =  7,
                                     CCR_2_NDX  =  8,
                                     CCR_3_NDX  =  9,
                                     CCR_4_NDX  = 10,
                                // padded to modulo 4 bytes
                                        CMD_LEN = 24;

    usb_recv.fill(CMD_LEN);

    if (!usb_recv.byte(ENABLE_NDX)) {
        pwm_disable   (       );
        usb_recv.flush(CMD_LEN);
        return;
    }

    rcc_periph_enable_and_reset(rcc->apb1enr          ,
                                rcc->apb1rstr         ,
                                Rcc::Apb1enr ::TIM2EN ,
                                Rcc::Apb1rstr::TIM2RST);

    gen_tim_2->psc  = usb_recv.shrt(  PSC_NDX);  // prescaler
    gen_tim_2->sr   = 0                       ;  // clear timer
    gen_tim_2->arr  = usb_recv.shrt(  ARR_NDX);  // total period
    gen_tim_2->ccr2 = usb_recv.shrt(CCR_2_NDX);
    gen_tim_2->ccr3 = usb_recv.shrt(CCR_3_NDX);
    gen_tim_2->ccr4 = usb_recv.shrt(CCR_4_NDX);


    uint8_t
        mode2 = usb_recv.byte(MODE_2_NDX) & GenTim_2_3_4::Ccmr1::OC2M_MASK,
        mode3 = usb_recv.byte(MODE_3_NDX) & GenTim_2_3_4::Ccmr2::OC3M_MASK,
        mode4 = usb_recv.byte(MODE_4_NDX) & GenTim_2_3_4::Ccmr2::OC4M_MASK;

      gen_tim_2->ccmr1
    = GenTim_2_3_4::Ccmr1::mskd_t(GenTim_2_3_4::Ccmr1::OC2M_MASK,
                                  mode2                         ,
                                  GenTim_2_3_4::Ccmr1::OC2M_POS );

      gen_tim_2->ccmr2
    =   GenTim_2_3_4::Ccmr2::mskd_t(GenTim_2_3_4::Ccmr2::OC3M_MASK,
                                    mode3                         ,
                                    GenTim_2_3_4::Ccmr2::OC3M_POS )
      | GenTim_2_3_4::Ccmr2::mskd_t(GenTim_2_3_4::Ccmr2::OC4M_MASK,
                                    mode4                         ,
                                    GenTim_2_3_4::Ccmr2::OC4M_POS);

    Gpio::Crl::mskd_t   speed1  =   Gpio::Crl::mskd_t(Gpio::Crl::MASK      ,
                                                       usb_recv
                                                      .byte(SPEED_1_NDX),
                                                      Gpio::Crl::MODE1_POS),
                        speed2  =   Gpio::Crl::mskd_t(Gpio::Crl::MASK      ,
                                                       usb_recv
                                                      .byte(SPEED_2_NDX),
                                                      Gpio::Crl::MODE2_POS),
                        speed3  =   Gpio::Crl::mskd_t(Gpio::Crl::MASK      ,
                                                       usb_recv
                                                      .byte(SPEED_3_NDX),
                                                      Gpio::Crl::MODE3_POS),
                        port1                                              ,
                        port2                                              ,
                        port3                                              ;


    switch(usb_recv.byte(PP_OD_HI_1_NDX)) {
        case 0 : port1 = speed1 | Gpio::Crl::CNF1_ALTFUNC_PUSH_PULL ; break;
        case 1 : port1 = speed1 | Gpio::Crl::CNF1_ALTFUNC_OPEN_DRAIN; break;
        default:
            port1 = Gpio::Crl::CNF1_INPUT_FLOATING | Gpio::Crl::MODE1_INPUT;
            break;
    }
    switch(usb_recv.byte(PP_OD_HI_2_NDX)) {
        case 0 : port2 = speed2 | Gpio::Crl::CNF2_ALTFUNC_PUSH_PULL ; break;
        case 1 : port2 = speed2 | Gpio::Crl::CNF2_ALTFUNC_OPEN_DRAIN; break;
        default:
            port2 = Gpio::Crl::CNF2_INPUT_FLOATING | Gpio::Crl::MODE2_INPUT;
            break;
    }
    switch(usb_recv.byte(PP_OD_HI_3_NDX)) {
        case 0 : port3 = speed3 | Gpio::Crl::CNF3_ALTFUNC_PUSH_PULL ; break;
        case 1 : port3 = speed3 | Gpio::Crl::CNF3_ALTFUNC_OPEN_DRAIN; break;
        default:
            port3 = Gpio::Crl::CNF3_INPUT_FLOATING | Gpio::Crl::MODE3_INPUT;
            break;
    }


    gen_tim_2->ccer  =   GenTim_2_3_4::Ccer::CC2E    // enable output
                       | GenTim_2_3_4::Ccer::CC3E    //   "      "
                       | GenTim_2_3_4::Ccer::CC4E;   //   "      "
    gen_tim_2->cr1   =   GenTim_2_3_4::Cr1 ::CEN ;   // enable timer


    // connect to I/O pins
    gpioa->crl.ins(  port1      // channel 2
                   | port2      //    "    3
                   | port3);    //    "    4

    usb_recv.flush(CMD_LEN);

}  // pwm()



void spi_bridge()
{
    namespace spi = spi_command;

    usb_recv.fill(spi::CMD_LEN);
    const uint8_t    miso_len = usb_recv.byte(spi::TX_LEN);
    usb_recv.fill(spi::CMD_LEN + miso_len);

    spi_activate();

    arm::SysTickTimer    rate_timer                                     ,
                         concat_timer                                   ;
    Sbrk                 sbrk                                           ;
    const uint32_t       rate         = usb_recv.word(   spi::RATE     ),
                         nss_delay    = usb_recv.word(   spi::NSS_DLAY ),
                         tx_timeout   = usb_recv.word(   spi::TX_TMOUT ),
                         concat_time  = usb_recv.word(   spi::RX_WAIT  );
    uint8_t             *tx_data                                        ,
                        *miso_data                                      ,
                        *xmit_ptr                                       ,
                        *xmit_end                                       ;
    const uint8_t        spi_bits     = usb_recv.byte(   spi::BITS     );
    const bool           master       = usb_recv.byte(   spi::MASTER   ),
                         xmit_only    = spi_bits & (1 << spi::XMIT_ONLY);

    // save default tx data
    miso_data = sbrk(miso_len);

    for (unsigned ndx = 0 ; ndx < miso_len ; ++ndx)
        miso_data[ndx] = usb_recv.byte(spi::CMD_LEN + ndx);

    usb_recv.flush(spi::CMD_LEN + miso_len);

    xmit_end = xmit_ptr = tx_data = sbrk(0);     // use rest of storage

    while (true) {
        uint8_t     data_len;
        if (bridge_recv(&data_len           ,
                         xmit_end           ,
                         sizeof(data_len)   ,
                         0                  ,    // len offset in msg
                         MAX_BRIDGE_DATA_LEN)) {
            if (data_len == 0)
                break;
            xmit_end += data_len;
        }

        unsigned        sent_or_rcvd = 0;
        PeriphStatus    status          ;

        if (master && xmit_ptr != xmit_end) {
            status = spi_tx_rx_buf_mstr(xmit_ptr           ,
                                        send_buf + 2       ,
                                        xmit_end - xmit_ptr,
                                        tx_timeout         ,
                                        nss_delay          ,
                                        rate               ,
                                        0                  ,  // flag to end
                                        sent_or_rcvd       );
        }

        if (!master) {  // slave
            unsigned    xmit_len = xmit_end - xmit_ptr;

            status = spi_tx_rx_buf_slav(xmit_ptr     ,
                                        miso_data    ,
                                        send_buf + 2 ,
                                        xmit_len     ,
                                        miso_len     ,
                                        concat_time  ,
                                        Command::HALT,
                                        sent_or_rcvd );

            // Can receive less then maximum so is not really error
            if (   status       == PeriphStatus::EMPTY
                && sent_or_rcvd <   xmit_len + miso_len)
                status = PeriphStatus::OK;
        }

        if (sent_or_rcvd != 0 && !xmit_only) {
            send_buf[0] = sent_or_rcvd                ;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
            send_buf[1] = static_cast<uint8_t>(status);
#pragma GCC diagnostic pop
            usb_send_w_zlp(sent_or_rcvd + 2)          ;
        }

        if ((xmit_ptr += sent_or_rcvd) >= xmit_end)
            xmit_ptr  = xmit_end = tx_data;
    }

    spi_disable();

}  // spi_bridge()



void i2c_bridge()
{
    namespace i2c = i2c_command;

    static uint8_t* const   DFLT_BEG = reinterpret_cast<uint8_t*>(&STORAGE    );
    static uint8_t* const   XMIT_BEG = DFLT_BEG + i2c::MAX_TX_DFLT             ;

    usb_recv.fill(i2c::CMD_LEN);
    const uint8_t    dflt_size = usb_recv.byte(i2c::DFLT_SIZE);
    usb_recv.fill(i2c::CMD_LEN + dflt_size);

    i2c_activate();

    const uint32_t   timeout   = usb_recv.word   (i2c::TIMEOUT );
    PeriphStatus     status    = PeriphStatus::OK                ;
    const uint8_t   i2c_bits   = usb_recv.byte   (i2c::BITS     );
    uint8_t          tx_size   = 0                               ,
                     rx_size   = 0                               ,
                     oar_gc    = 3 /*0,1,2,3==gc,oar1,oar2,mstr*/,
                     tx_sent                                     ,
                     rx_rcvd                                     ,
                    dest_addr  = usb_recv.byte   (i2c::DEST     ),
                    *xmit_ptr  = XMIT_BEG                        ,
                    *xmit_end  = xmit_ptr                        ;
    const bool      master     = i2c_bits & (1 << i2c::MASTER)   ;

    // save default tx data
    for (unsigned ndx = 0 ; ndx < dflt_size ; ++ndx)
        DFLT_BEG[ndx] = usb_recv.byte(i2c::CMD_LEN + ndx);

    usb_recv.flush(i2c::CMD_LEN + dflt_size);

    while (true) {
        uint8_t     header[3];
        if (bridge_recv(header             ,
                        xmit_end           ,
                        sizeof(header)     ,
                        2                  ,  // len offset in msg
                        MAX_BRIDGE_DATA_LEN)) {
            dest_addr = header[0],
            rx_size   = header[1];
            tx_size   = header[2];

            if (rx_size == 0 && tx_size == 0)
                break;

            xmit_end += tx_size;

            // rationalize sizes for USB send
            if (tx_size > i2c::MAX_TX_RX) tx_size = i2c::MAX_TX_RX;
            if (rx_size > i2c::MAX_TX_RX) rx_size = i2c::MAX_TX_RX;
        }


        if (master) {
            // is end condition if received, but idle condition when running
            if (rx_size == 0 && tx_size == 0)
                continue;

            status = i2c_master(timeout     ,
                                xmit_ptr    ,
                                send_buf + 4,
                                dest_addr   ,
                                tx_size     ,
                                rx_size     ,
                                tx_sent     ,
                                rx_rcvd     );

            // always consider all sent, even if partial or complete failure
            tx_size  = 0       ;
            rx_size  = 0       ;
            xmit_ptr = XMIT_BEG;
            xmit_end = XMIT_BEG;
        }
        else {  // slave
            status = i2c_slave(timeout            ,
                               xmit_ptr           ,
                               DFLT_BEG           ,
                               send_buf + 4       ,
                               xmit_end - xmit_ptr,
                               dflt_size          ,
                               rx_size            ,  // discard any over this
                               oar_gc             ,
                               rx_rcvd            ,
                               tx_sent            );

            if (status == PeriphStatus::EMPTY)
                continue;

            // competely driven by master
            // any tx_sent < tx_size remain in queue
            if ((xmit_ptr += tx_sent) >= xmit_end)
                xmit_ptr = xmit_end = reinterpret_cast<uint8_t*>(XMIT_BEG);
        }


        // must be <= tx_size,rx_size respectively
        send_buf[0] = static_cast<uint8_t>(status );
        send_buf[1] =                      oar_gc  ;
        send_buf[2] =                      tx_sent ;
        send_buf[3] =                      rx_rcvd ;
        usb_send_w_zlp(rx_rcvd + 4)                ;
    }

    i2c_disable();

}  // i2c_bridge()



void usart_bridge()
{
    namespace usart = usart_command;

    usb_recv.fill(usart::CMD_LEN);

    volatile Usart      *usart_n     = usart_activate()                      ;
    arm::SysTickTimer    rate_timer                                          ,
                         concat_timer                                        ;
    Sbrk                 sbrk                                                ;
    const uint32_t       rate         = usb_recv.word(     usart::RATE      ),
                         timeout      = usb_recv.word(     usart::TX_TMO    ),
                         rx_wait      = usb_recv.word(     usart::RX_WAIT   );
    uint8_t             *tx_data      = sbrk(UsbDevCdcAcm::CDC_OUT_DATA_SIZE),
                        *xmit_ptr     = tx_data                              ,
                        *xmit_end     = tx_data                              ;
    const uint8_t        usart_bits   = usb_recv.byte(     usart::BITS_1 )   ,
                         rx_len       = usb_recv.byte(     usart::RX_LEN )   ;
    const bool           xmit_enable  = usart_bits & (1 << usart::XMIT   )   ,
                         recv_enable  = usart_bits & (1 << usart::RECV   )   ,
                         synchronous  = usart_bits & (1 << usart::CLOCK  )   ;

    usb_recv.flush(usart::CMD_LEN);

    while (true) {
        uint8_t     data_len;
        if (bridge_recv(&data_len           ,
                         xmit_end           ,
                         sizeof(data_len)   ,
                         0                  ,   // len offset in msg
                         MAX_BRIDGE_DATA_LEN)) {
            if (data_len == 0)
                break;
            xmit_end += data_len;
        }


        PeriphStatus    status    ;
        unsigned        rcvd   = 0;

        if (xmit_enable && xmit_ptr < xmit_end) {
            unsigned    sent = 0;

            if (recv_enable && synchronous) {
                status = usart_buf_tx_rx(usart_n            ,
                                         xmit_ptr           ,
                                         send_buf + 2       ,
                                         xmit_end - xmit_ptr,
                                         timeout            ,
                                         rate               ,
                                         0                  ,
                                         rcvd               );
                sent = rcvd;
            }
            else
                status = usart_buf_tx   (usart_n            ,
                                         xmit_ptr           ,
                                         xmit_end - xmit_ptr,
                                         timeout            ,
                                         rate               ,
                                         0                  ,
                                         sent               );

            if ((xmit_ptr += sent) >= xmit_end)
                xmit_ptr = xmit_end = tx_data;
        }

        if (recv_enable && !(xmit_enable && synchronous)) {
            status = usart_buf_rx(usart_n     ,
                                  send_buf + 2,
                                  rx_len      ,
                                  rx_wait     ,
                                  0           ,
                                  rcvd        );
            // Very likely that zero-length will come during rx_wait
            // in usart_buf_rx() instead of in bridge_recv(), above.
            if (status == PeriphStatus::HALTED)
                break;

            // Likely that receive less then rx_len (host default of 62)
            // so is not really error
            if (status == PeriphStatus::EMPTY && rcvd < rx_len)
                status = PeriphStatus::OK;
        }

        if (rcvd) {
            send_buf[0] = rcvd                        ;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
            send_buf[1] = static_cast<uint8_t>(status);
#pragma GCC diagnostic pop
            usb_send_w_zlp(rcvd + 2)                  ;
        }
    }

    usart_disable(usart_n);

}  // usart_bridge()



void parallel_bridge()
{
    static const uint8_t    // usb_recv.word(NDX)
                               RATE_NDX  = 1,    // byte send rate
                            // is inherently modulo 4
                                CMD_LEN  = 8;

    usb_recv.fill(CMD_LEN);

    const uint32_t   rate      = usb_recv.word(RATE_NDX)             ;
    const uint8_t    port_mask = gpioa_activate()                    ;
    uint8_t         *xmit_ptr  = reinterpret_cast<uint8_t*>(&STORAGE),
                    *xmit_end  = xmit_ptr                            ;
    bool             rate_wait = false                               ;

    usb_recv.flush(CMD_LEN);

    while (true) {
        uint8_t     data_len;
        if (bridge_recv(&data_len          ,
                          xmit_end         ,
                          sizeof(data_len),
                         0                  ,    // len offset in msg
                         MAX_BRIDGE_DATA_LEN)) {
            if (data_len == 0)
                break;
            xmit_end += data_len;
        }

        if (rate_wait && sys_tick_timer.elapsed32() > rate)
            rate_wait = false;

        if (xmit_ptr == xmit_end || rate_wait)
            continue;

        gpioa->odr = *xmit_ptr++ & port_mask;

        if (xmit_ptr == xmit_end)
            xmit_ptr  = xmit_end = reinterpret_cast<uint8_t*>(&STORAGE);

        if (rate) {
            sys_tick_timer.begin32();
            rate_wait = true    ;
        }
    }

    gpioa_disable();

}  // parallel_bridge()



void counter()
{
    static const uint8_t    // usb_recv.byte(NDX)
                // unneeded   GPIO_SPEED_NDX = 1,
                // unneeded   GPIO_OPNPP_NDX = 2,   //  "        "
                                   TIMED_NDX = 3,   // (0,1) infinite/duration
                                    MODE_NDX = 4,   // (0,1) binary/gray
                               INCREMENT_NDX = 5,   // (-127,127)
                                     LOW_NDX = 6,   // (0,255)
                                    HIGH_NDX = 7,   // (0,255)
                            // usb_recv.word(NDX)
                                    RATE_NDX = 2,
                            DURATION_LOW_NDX = 3,   // (0-0xffffffff)
                            DURATION_HGH_NDX = 4,   // (0-0xffffffff)
                            // inherently modulo 4
                                    CMD_LEN = 20;

    usb_recv.fill(CMD_LEN);

    pwm_disable   ();   // in case running
    gpioa_activate();   // ignore return value

    const uint32_t  rate      = usb_recv.word(RATE_NDX        ),
                    dura_low  = usb_recv.word(DURATION_LOW_NDX),
                    dura_hgh  = usb_recv.word(DURATION_HGH_NDX);
    const int8_t    increment = usb_recv.byte(INCREMENT_NDX   );
    const uint8_t   low       = usb_recv.byte(LOW_NDX         ),
                    high      = usb_recv.byte(HIGH_NDX        );
    uint8_t         count                                      ;
    const bool      gray      = usb_recv.byte(MODE_NDX ) != 0  ,
                    timed     = usb_recv.byte(TIMED_NDX) != 0  ;

    usb_recv.flush(CMD_LEN);

    count = increment > 0 ? low : high;

    if (rate == 0) {  // unlimited, halt via timer/USB and longjump
        in_progress = InProgress::COUNTING;

        if (timed)
            duration_timer(dura_low, true);

        while (true) {
            if (gray) {
                gpioa->odr = count ^ (count >> 1);
                ++count;
            }
            else
                gpioa->odr = count;

            count = counter_increment(count, increment, low, high);
        }
    }

    // else rate limited
    //
    uint64_t            duration =   (static_cast<uint64_t>(dura_hgh) << 32)
                                   |                        dura_low        ;
    arm::SysTickTimer   duratimer(64)                                       ;

    halt_code = HaltCode::DURATION;

    while (duratimer.elapsed64() <  duration) {
        if (gray) {
            gpioa->odr = count ^ (count >> 1);
            ++count;
        }
        else
            gpioa->odr = count;

        count = counter_increment(count, increment, low, high);

        for (sys_tick_timer.begin32() ; sys_tick_timer.elapsed32() < rate ; ) {
            duratimer.elapsed64();   // keep up-to-date

            if (usb_recv.fill(0)) {
                halt_code = HaltCode::USB;
                goto exit_counter;
            }
        }
    }

    exit_counter:
    gpioa_disable();

    send_buf[0] = halt_code;
    usb_send(1);

}  // counter()



void serial_number()
{
    usb_recv.flush(1);
    send_buf[0] = usb_dev.serial_number_length();
    for (unsigned ndx = 0 ; ndx < send_buf[0] ; ++ndx)
        send_buf[ndx + 1] = usb_dev.serial_number_digit(ndx);
    usb_send(send_buf[0] + 1);
}



void blink_user_led()
{
    usb_recv.flush(1);

    while (true) {
        user_led_off();
        sys_tick_timer.begin32();
        while (sys_tick_timer.elapsed32() < (3 * (72000000 >> 3)))  // 3/8 sec
            ;

        user_led_on();
        sys_tick_timer.begin32();
        while (sys_tick_timer.elapsed32() < (     72000000 >> 3) )  // 1/8 sec
            ;

        if (usb_recv.fill(0))
            break;
    }
}

#endif  // #if 1 (host communication (commands))




int main()
{
    usb_dev.serial_number_init();  // do before mcu_init() clock speed breaks
                  usb_mcu_init();
                     gpio_init();

    user_led_off();

    if (!usb_dev.init())
        while (true)    // hang
            asm("nop");

    while (usb_dev.device_state() != UsbDev::DeviceState::CONFIGURED)
        asm("nop");

    // see CONNECT_SIGNATURE
    wait_connect_signature();

    __asm__ volatile (
    // setjmp
    "movw       r0, #:lower16:longjump_buf                      ;"
    "movt       r0, #:upper16:longjump_buf                      ;"
    "mov        ip, sp                                          ;"
    "adr        lr, longjmp_return                              ;"
    "stm        r0, {r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}    ;"
    "movs       r0, #0                                          ;"
    // see ".macro irq_handler_exit CODE" in buck50_asm.s
    // must be aligned 8 for longjump to work
    ".balign 8                                                  ;"
    "longjmp_return:                                            ;"
    // clear out any pending IT conditional (can cause skip of movw/movt??))
    "nop                                                        ;"
    "nop                                                        ;"
    "nop                                                        ;"
    "nop                                                        ;"
    "movw       r1, #:lower16:longjump_buf                      ;"
    "movt       r1, #:upper16:longjump_buf                      ;"
    "ldmia.w    r1, {r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}    ;"
    "mov        sp, ip                                          ;"
    "movw       r1, #:lower16:halt_code                         ;"
    "movt       r1, #:upper16:halt_code                         ;"
    "strb       r0, [r1]                                        ;"
    :
    :
    : "r0", "ip", "lr"
    );

    if (halt_code != HaltCode::SETJMP) {
        if (in_progress & InProgress::SAMPLING_ETC) {
            reset_timer_ganged_ports();
            if (in_progress & InProgress::ANALOG) {
                // Unlikely during sampling because max duration c. 2.1 seconds
                //   due to sample memory constraints
                // so only anticipated if analog signal never triggers
                //   (level/slope) and USB interrupt from host
                // can't happen from timer because interrupt not enabled
                // shouldn't happen by HardFault unless bug
                // So ... clean up and return null result because is
                //   abort or failure -- analog_sampling() normally finishes
                //   and returns results before returning
                adc_disable();
                if (in_progress & InProgress::TRIGGERING) {
                    // USB interrupt while not yet triggered
                    // set up for send_samples()
                    analog_sample_rate = 8          ;  // invalid code
                    num_analog_words   = 0          ;
                    samples            = samples_end;  // no samples
                }
                // send null result to host
                send_buf    [0] = halt_code = HaltCode::USB;
                send_buf    [1] = num_analog_channels      ;
                send_buf    [2] = analog_channels.byte     ;
                send_buf    [3] = analog_sample_rate       ;
                send_uint16s[2] = num_analog_words         ;
                send_uint16s[3] = in_progress              ;
                usb_send(8);
            }
            else {  // digital sampling only ends via interrupt (mem/tim/USB)
                // might have incremented one past if HardFault trap
                if (samples_end > &STORAGE_END)
                    samples_end = &STORAGE_END;
                send_buf    [0] = sampling_mode                         ;
                send_buf    [1] = halt_code                             ;
                send_uint16s[1] = in_progress & InProgress::SAMPLING_ETC;
                send_uint16s[2] = samples_end - samples                 ;
                usb_send(6);
            }
        }
        else if (in_progress & InProgress::COUNTING) {
            gpioa_disable()        ;
            send_buf[0] = halt_code;
            usb_send(1)            ;
        }

        in_progress = InProgress::IDLE;
        halt_code   = HaltCode::NONE  ;  // unnecessary
    }

    user_led_on();

    while (true) {
        usb_recv.fill(1);  // waits until at least 1 byte received from host

        switch (usb_recv.byte(0)) {
            case Command::DIGITAL_SAMPLING:
                digital_sampling();
                break;

            case Command::ANALOG_SAMPLING:
                analog_sampling();
                break;

            case Command::HALT:
                usb_recv.flush(1);  // if not caught ...
                break;              // ... by USB_LP_CAN1_RX0_IRQHandler()

            case Command::SEND_SAMPLES:
                send_samples();
                break;

            case Command::LIVE:
                live();
                break;

            case Command::PWM:
                pwm();
                break;

            case Command::PARALLEL_BRIDGE:
                parallel_bridge();
                break;

            case Command::USART_BRIDGE:
                usart_bridge();
                break;

            case Command::SPI_BRIDGE:
                spi_bridge();
                break;

            case Command::I2C_BRIDGE:
                i2c_bridge();
                break;

            case Command::COUNTER:
                counter();
                break;

            case Command::RESET:
                reset_ganged();
                break;

            case Command::FLASH_WAIT_PRE:
                flash_wait_pre();
                break;

            case Command::CONNECT_SIG:
                // host UI program connecting to already-running firmware
                wait_connect_signature();
                break;

            case Command::IDENTITY:
                usb_recv.flush(1);
                send_uint32s[0] = IDENTITY;
                usb_send(4);
                break;

            case Command::VERSION:
                usb_recv.flush(1);
                send_buf[0] = VERSION[0];
                send_buf[1] = VERSION[1];
                send_buf[2] = VERSION[2];
                usb_send(3);
                break;

            case Command::SERIAL_NUMBER:
                serial_number();
                break;

            case Command::BLINK_USER_LED:
                blink_user_led();
                break;

            default:  // shouldn't ever happen
                usb_recv.flush(1);
                break;
            }
    }  // main loop

}  // main()
