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


#ifndef USB_DEV_HXX
#define USB_DEV_HXX

#define USB_DEV_MAJOR_VERSION   1
#define USB_DEV_MINOR_VERSION   2
#define USB_DEV_MICRO_VERSION   1

#include <stm32f103xb.hxx>

#if STM32F103XB_MAJOR_VERSION == 1
#if STM32F103XB_MINOR_VERSION  < 2
#warning STM32F103XB_MINOR_VERSION >= 2 with required STM32F103XB_MAJOR_VERSION == 1
#endif
#else
#error STM32F103XB_MAJOR_VERSION != 1
#endif



namespace stm32f10_12357_xx {

class UsbDev {
  public:

    // client application code optinally uses device_state() accessor to
    // test if "CONFIGURED", indicating that USB device has been fully
    // enumerated by host and peripheral is ready to send and receive data
    enum class DeviceState {
        CONSTRUCTED = 0,
        INITIALIZED    ,
        RESET          ,
        ADDRESSED      ,
        CONFIGURED
    };

    // As defined by USB standards.
    // Must be public for use in derived class static descriptors.
    enum class DescriptorType {
        DEVICE        = 0x1,
        CONFIGURATION = 0x2,
        STRING        = 0x3,
        INTERFACE     = 0x4,
        ENDPOINT      = 0x5,
    };

    // As defined by USB standards.
    // Must be public for use in derived class static descriptors.
    enum class Descriptor {
        DEVICE           = 1,
        CONFIGURATION    = 2,
        STRING           = 3,
        INTERFACE        = 4,
        ENDPOINT         = 5,
        DEVICE_QUALIFIER = 6,
        OTHER_SPEED      = 7,
        INTERFACE_POWER  = 8,
        OTG              = 9,
    };

    // As defined by USB standards.
    // Must be public for use in derived class static descriptors.
    enum class EndpointType {
        CONTROL      = 0,
        ISYNCHRONOUS = 1,
        BULK         = 2,
        INTERRUPT    = 3,
    };

    // USB standard: offset into configuration descriptor to length field
    static const uint8_t    CONFIG_DESC_SIZE_NDX = 2;

    // USB standard: Bit in endpoint descriptor address byte. If set, endpoint
    //   is IN; if clear is OUT (host-centric nomenclature)
    // Endpoint numbers aka addresses limited to 4 bits. Other bits reserverd,
    //   should be set to zero
    static const uint8_t    ENDPOINT_DIR_IN      = 0x80,
                            ENDPOINT_ADDR_MASK   = 0x0F;



    constexpr
    UsbDev()
    :   _endpoints            {                         },
#ifdef USB_DEV_ENDPOINT_CALLBACKS
        _recv_callbacks       {{0, 0}                   },
        _send_callbacks       {{0, 0}                   },
#endif
        _epaddr2eprn          {0                        },
        _eprn2epaddr          {0                        },
        _send_info            (                         ),
        _recv_info            (                         ),
        _setup_packet         (0                        ),
        _device_state         (DeviceState ::CONSTRUCTED),
    //  _status               (0                        ),
        _recv_readys          (0x0000                   ),
        _send_readys          (0x0000                   ),
        _send_readys_pending  (0x0000                   ),
        _last_send_size       (0                        ),
        _num_eprns            (1                        ), // parse descriptor,
                                                           // always endpoint 0
        _current_configuration(0                        ),
        _current_interface    (0                        ),
        _pending_set_addr     (IMPOSSIBLE_DEV_ADDR      )
    {}


    // need public accessors for static initialization of _STRING_DESCS
    //
    static constexpr const uint8_t* language_id_string_desc()
    {
        return _LANGUAGE_ID_STRING_DESC;
    }
    static constexpr const uint8_t* vendor_string_desc()
    {
        return _VENDOR_STRING_DESC;
    }
    static constexpr const uint8_t* serial_number_string_desc()
    {
        return _SERIAL_NUMBER_STRING_DESC;
    }

    // serial number convenience accessors
    //
    static constexpr unsigned serial_number_length()
    {
        return _SERIAL_NUMBER_STRING_LEN;
    }
    static uint8_t serial_number_digit(
    const uint8_t   digit)
    {
        // pull char out of uint16_t[] USB protocol-defined descriptor
        // no checking
        return _SERIAL_NUMBER_STRING_DESC[2 + (digit << 1)];
    }


    // Optional use by client application.
    // Will copy/format ST "Unique Device ID" value into USB descriptor,
    // Otherwise value will be  default string "000..."
    // Hardware bug: must be done while main CPU clock at default 8 MHz
    //   before mandatory increase to 48 or 72 MHz for USB peripheral.
    void    serial_number_init();

    // Not done in constexpr constructor because must be done after
    //   MCU peripheral, clock, etc. configuration/initialization.
    bool    init();

#ifdef USB_DEV_FORCE_RESET_CAPABILITY
    // Experimental
    // Not useful, doesn't reset USB bus (pull D+ line low) to indicate
    //   reset to host and cause re-enumeration
    void    force_reset();
#endif

    // see "enum class DeviceState", above
    DeviceState     device_state() const { return _device_state ; }

    void interrupt_handler();


    // STM32F10xx/UsbDev endpoint semantics allow endpoint N to be
    // bidirectional, i.e. both receive (USB standard host-centric "OUT",
    // endpoint number N in USB endpoint descriptor) and send (USB standard
    // host-centric "IN", endpoint number 0x80|N in USB endpoint descriptor)
    //

#ifndef USB_DEV_INTERRUPT_DRIVEN
    // Client application must call at high frequency  during USB enumeration
    //   process, less so afterwards
    // Returns bits set corresponding to  endpoints with received data or
    //   ready to send.
    uint32_t  poll()
    {
        if (stm32f103xb::usb->istr.any(  stm32f103xb::Usb::Istr::CTR
                                       | stm32f103xb::Usb::Istr::RESET))
            interrupt_handler();

        return (_send_readys << 16) | _recv_readys;
    }

    // convenience routine for parsing poll() return value
    static constexpr uint32_t poll_recv_ready(
    const uint8_t   endpoint)
    {
        return 1 << endpoint;
    }

    // convenience routine for parsing poll() return value
    static constexpr uint32_t poll_send_ready(
    const uint8_t   endpoint)
    {
        return 1 << (endpoint + 16);
    }
#endif


#ifdef USB_DEV_ENDPOINT_CALLBACKS
    // callback will be called when endpoint receives data from host
    void register_recv_callback(
    void        (*callback)(const uint8_t,
                            void*         ),
    uint8_t       endpoint                 ,   // not checked
    void         *user_data                 )
    {
        _recv_callbacks[endpoint]._callback  = callback ;
        _recv_callbacks[endpoint]._user_data = user_data;
        if (_recv_readys & (1 << endpoint))
            callback(endpoint, user_data);
    }

    // callback will be if called endpoint is ready to send data to host
    void register_send_callback(
    void        (*callback)(const uint8_t,
                            void*         ),
    uint8_t       endpoint                 ,   // not checked
    void         *user_data                 )
    {
        _send_callbacks[endpoint]._callback  = callback ;
        _send_callbacks[endpoint]._user_data = user_data;
        if (_send_readys & (1 << endpoint))
            callback(endpoint, user_data);
    }
#endif


    // accessor for information parsed from USB endpoint descriptor
    uint16_t endpoint_recv_bufsize(
    const uint8_t   endpoint)
    const {
        return _endpoints[_epaddr2eprn[endpoint]].max_recv_packet;
    }

    // accessor for information parsed from USB endpoint descriptor
    uint16_t endpoint_send_bufsize(
    const uint8_t   endpoint)
    const {
        return _endpoints[_epaddr2eprn[endpoint]].max_send_packet;
    }


    // Accessors for endpoint states
    //
    // must be volatile for #ifdef USB_DEV_INTERRUPT_DRIVEN
    uint16_t    recv_readys() const volatile { return _recv_readys; }
    uint16_t    send_readys() const volatile { return _send_readys; }

    // must be volatile for #ifdef USB_DEV_INTERRUPT_DRIVEN
    bool    recv_ready(const uint16_t   endpoints) const volatile
            { return _recv_readys & endpoints; }
    bool    send_ready(const uint16_t   endpoints) const volatile
            { return _send_readys & endpoints; }


    // Endpoint data transfers
    //
    //
    // STM32F10xx USB data is arranged internally in memory according to the
    // following layout:
    //
    // A USB data packet containing N bytes:
    //    b0, b1, b2, b3, b4, ... bN-1
    //
    // is arranged in STM32F10xx USB starting at "addr" as:
    //    addr      b0 b1 xx xx
    //    addr+4    b2 b3 xx xx
    //    addr+8    b4 b5 xx xx
    //    ...       .. .. .. ..
    // (where "xx" are unused/"don't care" values).
    //
    // As a consequence, data must be written to USB memory using the following
    // (or equivalent) methods:
    //    uint8_t    data    = {b0, b1, b2, b3, b4};
    //    uint16_t*  source  = reinterpret_cast<uint16_t*>(data + 2);
    //    uint32_t*  address = base_address + 4;
    //    *address = *data;
    // and similar for reading data from USB memory. See writ_pma_data() and
    // read_pma_data() in the usb_dev.cxx implementation file for examples
    // using both CPU and DMA.
    //
    // UsbDev's send(uint8_t, const uint8_t* const, const uint16_t)
    // and recv(uint8_t, uint8_t* const) methods, along with
    // read(const uint8_t, const uint8_t) and
    // writ(const uint8_t, const uint16_t, const uint8_t)
    // handle this correctly. Note that the data_ndx argument to both read()
    // and writ() must be specified in terms of uint16_t offsets, i.e. the
    // above example could be done via:
    //    uint16_t  data = (b2 << 8) | b3;   // little-endian
    //    usb_dev.writ(endpoint, data, 1);   // address known from endpoint
    //
    // If using raw buffer addresses obtained via send_buf() and recv_buf(),
    // client code must do the above correctly. All of these UsbDev methods,
    // with the exception of the the send() and recv() versions which take
    // uint8_t* arguments, are intended for client applications which wish
    // to avoid data copying by writing into, or reading data from, the above
    // internal memory layout. A particular use case would be DMA directly
    // from or to another hardware peripheral, such as USART for a
    // canonical USB CDC-ACM class USB-to-serial bridge. See the writ_pma_data()
    // and read_pma_data() implementations for DMA setup requirements.
    //
    // If not using send() and recv() versions which copy client data,
    // the USB_DEV_NO_BUFFER_RECV_SEND pre-processor macro can be defined
    // to eliminate their compilation and reduce binary code size.
    //

#ifndef USB_DEV_NO_BUFFER_RECV_SEND
    // no checking of params -- caller must guarantee valid
    //   endpoint and buffer
    uint16_t recv(const uint8_t         endpoint,
                        uint8_t* const  buffer  );

    // no checking of params -- caller must guarantee valid
    //   endpoint_number, data, and length
    bool send(const uint8_t         endpoint,
              const uint8_t* const  data    ,
              const uint16_t        length  );
#endif

    // for use with direct access to hardware USB buffers, below
    //
    uint16_t recv_lnth(
    const uint8_t   endpoint)   // no check for valid endpoint
    {
        if (!(_recv_readys & (1 << endpoint)))
            return 0;

        return  _pma_descs
               .eprn(_epaddr2eprn[endpoint])
               .count_rx.shifted(  stm32f103xb
                                 ::UsbBufDesc
                                 ::CountRx
                                 ::COUNT_0_SHFT)          ;
    }

    bool recv_done(
    const uint8_t   endpoint)   // no check for valid endpoint
    {
        if (!(_recv_readys & (1 << endpoint)))
            return false;

        _recv_readys &= ~(1 << endpoint);

          stm32f103xb
        ::usb
        ->eprn(_epaddr2eprn[endpoint])
         .stat_rx(stm32f103xb::Usb::Epr::STAT_RX_VALID);

        return true;
    }

    bool send(  // no check for valid endpoint or length
    const uint8_t   endpoint,
    const uint16_t  length  )
    {
        if (!(_send_readys & (1 << endpoint)))
            return false;

          _pma_descs.eprn(_epaddr2eprn[endpoint]).count_tx
        = stm32f103xb::UsbBufDesc::CountTx::count_0(length);

          stm32f103xb
        ::usb
        ->eprn(_epaddr2eprn[endpoint])
         .stat_tx(stm32f103xb::Usb::Epr::STAT_TX_VALID);

        _send_readys &= ~(1 << endpoint);

        return true;
    }

    // direct access to hardware USB buffers
    //
    uint16_t read(              // no checking of parameters
    const uint8_t   endpoint,
    const uint8_t   data_ndx)   // uint16_t index, i.e. byte index divided by 2
    {
        return *(_endpoints[_epaddr2eprn[endpoint]].recv_pma + data_ndx);
    }

    void writ(                  // no checking of parameters
    const uint8_t   endpoint,
    const uint16_t  data    ,
    const uint8_t   data_ndx)   // uint16_t index, i.e. byte index divided by 2
    {
        *(_endpoints[_epaddr2eprn[endpoint]].send_pma + data_ndx) = data;
    }

    // e.g. for DMA from/to peripheral
    //
    volatile uint32_t* recv_buf(
    const uint8_t   endpoint)
    {
        return _endpoints[_epaddr2eprn[endpoint]].recv_pma;
    }

    volatile uint32_t* send_buf(
    const uint8_t   endpoint)
    {
        return _endpoints[_epaddr2eprn[endpoint]].send_pma;
    }


  protected:

    // Information parsed from USB endpoint descriptors contained inside
    // configuration descriptor(s). Must be saved for subsequent execution
    // of reset() via USB request from host.
    struct Endpoint {
        uint32_t        *recv_pma       , // buffer, CPU addressing
                        *send_pma       ; //   "   ,  "      "
        uint16_t         max_recv_packet, // maximum USB tranfer size
                         max_send_packet; //    "     "     "      "
        EndpointType     type          ;  // convert to Usb::Epr::mskd_t with
                                          // _DESC_EP_TYPE_TO_EPR_EP_TYPE[]
    };


    // for handling multiple transfers to host via USB control endpoint pipe
    template <typename CONST_OR_NON> class DataInfo {
      public:
        constexpr
        DataInfo()
        :   _buffer(0),
            _length(0),
            _offset(0),
            _maxpkt(0)
        {}

        void    maxpkt(uint16_t size) { _maxpkt = size; }

        void set(
        CONST_OR_NON const  buffer,
        const uint16_t      length)
        {
            _buffer = buffer;
            _length = length;
            _offset = 0     ;
        }

        uint16_t remaining_size() const { return _length - _offset; }

        uint16_t transfer_size()
        const
        {
            return remaining_size() > _maxpkt ? _maxpkt : remaining_size();
        }

        CONST_OR_NON remaining_data() const { return _buffer + _offset; }

        void update(const uint16_t  xferred) { _offset += xferred; }

        void reset() { _offset = _length = 0; }

      protected:
        CONST_OR_NON    _buffer;
        uint16_t        _length,
                        _offset,
                        _maxpkt;
    };



    // To respond to host query.
    // Derived class can modify _status member variable.
    struct Status {
        constexpr Status() : status_type(0) {}

        struct StatusType {
            using pos_t = regbits::Pos<uint8_t, StatusType>;
            static constexpr pos_t   SELF_POWERED_POS = pos_t(0),
                                    REMOTE_WAKEUP_POS = pos_t(1);

            using bits_t = regbits::Bits<uint8_t, StatusType>;
            static constexpr bits_t
                    SELF_POWERED  = bits_t(1,  SELF_POWERED_POS),
                    REMOTE_WAKEUP = bits_t(1, REMOTE_WAKEUP_POS);
        };  // struct StatusType
        using status_type_t = regbits::Reg<uint16_t, StatusType>;
              status_type_t   status_type;
    };  // struct Status


    // Information sent in requests from host during USB enumation (and after).
    // Available to MCU at fixed/configurable address in peripheral
    //   PMA buffer memory
    struct SetupPacket {
        struct RequestType {
            using pos_t = regbits::Pos<uint8_t, RequestType>;
            static constexpr pos_t        DIR_POS = pos_t(7),
                                         TYPE_POS = pos_t(5),
                                    RECIPIENT_POS = pos_t(0);
            using bits_t = regbits::Bits<uint8_t, RequestType>;
            static constexpr bits_t     DIR_DEV_TO_HOST = bits_t(1, DIR_POS);

            static const uint8_t         TYPE_MASK =    0b11,
                                    RECIPIENT_MASK = 0b11111;


            using mskd_t = regbits::Mskd<uint8_t, RequestType>;
            static constexpr mskd_t
            TYPE_STANDARD       = mskd_t(     TYPE_MASK, 0,      TYPE_POS),
            TYPE_CLASS          = mskd_t(     TYPE_MASK, 1,      TYPE_POS),
            TYPE_VENDOR         = mskd_t(     TYPE_MASK, 2,      TYPE_POS),
            RECIPIENT_DEVICE    = mskd_t(RECIPIENT_MASK, 0, RECIPIENT_POS),
            RECIPIENT_INTERFACE = mskd_t(RECIPIENT_MASK, 1, RECIPIENT_POS),
            RECIPIENT_ENDPOINT  = mskd_t(RECIPIENT_MASK, 2, RECIPIENT_POS),
            RECIPIENT_OTHER     = mskd_t(RECIPIENT_MASK, 3, RECIPIENT_POS);
        };  // struct RequestType
        using request_type_t = regbits::Reg<uint8_t, RequestType>;
              request_type_t   request_type;

        enum class Request {
            GET_STATUS        =  0,
            CLR_FEATURE       =  1,
            _RESERVED_2       =  2,
            SET_FEATURE       =  3,
            _RESERVED_4       =  4,
            SET_ADDRESS       =  5,
            GET_DESCRIPTOR    =  6,
            SET_DESCRIPTOR    =  7,
            GET_CONFIGURATION =  8,
            SET_CONFIGURATION =  9,
            GET_INTERFACE     = 10,
            SET_INTERFACE     = 11,
            SYNCH_FRAME       = 12,
        };

        uint8_t     request;

        uint16_t    _alignment_0;
        union {
            uint16_t                    word;
            struct {
                uint8_t     byte0;   // is byte-reversed
                uint8_t     byte1;   // "   "      "
            }                           bytes;
        } value;

        uint16_t    _alignment_1,
                    index,
                    _alignment_2,
                    length,
                    _alignment_3;
    };  // struct SetupPacket


#ifdef USB_DEV_ENDPOINT_CALLBACKS
    struct EndpointCallback {
        void    (*_callback)(const uint8_t,
                             void*        );
        void     *_user_data               ;
    };
#endif


    static const uint32_t   _BTABLE_OFFSET = 0;

    static const uint8_t    _DESCRIPTOR_SIZE_NDX             =  0,
                            _DEVICE_DESC_MAX_PACKET_SIZE_NDX =  7,
                            _DEVICE_DESC_NUM_CONFIGS_NDX     = 17,
                            _SERIAL_NUMBER_STRING_NDX        =  3,
                            _SERIAL_NUMBER_STRING_LEN        = 24,
                            _ENDPOINT_DESC_ADDRESS_NDX       =  2,
                            _ENDPOINT_DESC_ATTRIBUTES_NDX    =  3,
                            _ENDPOINT_DESC_PACKET_SIZE_NDX   =  4;

    static const uint8_t    _ENDPOINT_ATTRS_TYPE_MASK = 0x0f;

    static const uint8_t    IMPOSSIBLE_DEV_ADDR = 0xff;


    // mapping from USB descriptor bDescriptorType (at CONFIG_DESC_SIZE_NDX
    // offset in descriptor) to Usb::Epr::EP_TYPE_XXX
    static const stm32f103xb::Usb::Epr::mskd_t  _DESC_EP_TYPE_TO_EPR_EP_TYPE[];

    // derived class implements _DEVICE_DESC, _CONFIG_DESC, and _STRING_DESCS
    // _CONFIG_DESC parsed in init() -- minimal checking done, malformed
    //   descriptor (bad bLength fields, duplicate or 0==control
    //   bEndpointAddress values, etc) will cause HardFault exception or
    //   inoperative USB peripheral. (Note *can* have bEndpointAddress of
    //   0x8n and 0x0n -- IN and OUT endpoints with same numeric address.)
    static const uint8_t    _DEVICE_DESC              [],
                            _LANGUAGE_ID_STRING_DESC  [],
                            _VENDOR_STRING_DESC       [];
                            // must be non-const because runtime setting of ...
    static       uint8_t    _CONFIG_DESC              [],  // ... bLength field
                            _SERIAL_NUMBER_STRING_DESC[];  // ... all bytes
    static const uint8_t*   _STRING_DESCS[];

    void    reset(),
            ctr  ();

    void    setup          (),
            control_out    (),
            control_in     (),
            data_stage_in  ();

    bool    standard_request  (),
            device_request    (),
            interface_request (),
            endpoint_request  (),
            descriptor_request();

    bool    device_class_setup();  // derived class must provide
    void    set_configuration ();  //    "      "    "      "
    void    set_interface     ();  //    "      "    "      "

    void    set_address(const uint8_t   address);

    void    writ_pma_data(const uint8_t*  const     data,
                                uint32_t* const     addr,
                          const uint16_t            size),
            read_pma_data(      uint8_t*  const     data,
                          const uint32_t* const     addr,
                          const uint16_t            size);


    // fake endpoint count of 1 okay, only using  statically-checked EPRN<0>()
    stm32f103xb ::UsbPmaDescs<1, _BTABLE_OFFSET>    _pma_descs;

    // indexed by ST endpoint register (Usb::Epr, UsbBufDesc, UsbPmaDescs)
    Endpoint                    _endpoints     [  stm32f103xb
                                                ::Usb
                                                ::NUM_ENDPOINT_REGS];

#ifdef USB_DEV_ENDPOINT_CALLBACKS
      EndpointCallback          _recv_callbacks[  stm32f103xb
                                                ::Usb
                                                ::NUM_ENDPOINT_REGS],
                                _send_callbacks[  stm32f103xb
                                                ::Usb
                                                ::NUM_ENDPOINT_REGS];
#endif

      // mappings between endpoint address as per USB descriptor
      // and ST peripheral endpoint registers (Usb::Epr) and
      // pseudo-registers (UsbPmaDescs/UsbBufDesc in PMA memory)
      //
                                // USB endpoint descriptor numbers and ST regs'
                                // Istr::EP_ID and Epr::EA fields are 4 bits wide
      uint8_t                   _epaddr2eprn   [ENDPOINT_ADDR_MASK + 1];

                                // must be saved -- can't rely on on
                                // Usb::eprn[n].EA field because gets cleared
                                // on extra USB reset during enumeration
      uint8_t                   _eprn2epaddr   [  stm32f103xb
                                                ::Usb
                                                ::NUM_ENDPOINT_REGS   ];

      DataInfo<const uint8_t*>  _send_info            ;
      DataInfo<      uint8_t*>  _recv_info            ;
      SetupPacket*              _setup_packet         ;
      DeviceState               _device_state         ;
      Status                    _status               ;

                                // bit N indicates USB endpoint descriptor addr
      uint16_t                  _recv_readys          ,
                                _send_readys          ,
                                _send_readys_pending  ;

      uint16_t                  _last_send_size       ;
      uint8_t                   _num_eprns            ,
                                _current_configuration,
                                _current_interface    ,
                                _pending_set_addr     ;
};  // class UsbDev

} // namespace stm32f10_12357_xx

#endif  // ifndef USB_DEV_HXX
