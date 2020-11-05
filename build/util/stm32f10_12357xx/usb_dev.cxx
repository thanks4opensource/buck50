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


#include <bin_to_hex.hxx>

#include "usb_dev.hxx"


namespace stm32f10_12357_xx {

using namespace stm32f103xb;



// public

void UsbDev::serial_number_init()  // do before mcu_init() clock speed breaks
{
    char    serial_number[_SERIAL_NUMBER_STRING_LEN];

    bitops::BinToHex::uint32(elec_sig->u_id_95_64, &serial_number[ 0]);
    bitops::BinToHex::uint32(elec_sig->u_id_63_32, &serial_number[ 8]);
    bitops::BinToHex::uint16(elec_sig->u_id_31_16, &serial_number[16]);
    bitops::BinToHex::uint16(elec_sig->u_id_15_0,  &serial_number[20]);

    for (uint8_t ndx = 0 ; ndx < _SERIAL_NUMBER_STRING_LEN ; ++ndx)
        // little-endian uint16_t
          reinterpret_cast<uint16_t*>(&_SERIAL_NUMBER_STRING_DESC[2])[ndx]
        = serial_number[ndx];
}



bool UsbDev::init()
{
    // PMA buffer descriptor table grows up from zero
    // TX and RX buffers grow down from end
    uint16_t    pma_addr = USB_PMASIZE;

    // set up control endpoint
    //

    // USB 2.0 limit is 64 bytes (and device descriptor IN and OUT
    // endponts are always both 64?)
    uint16_t    max_packet_size = _DEVICE_DESC[_DEVICE_DESC_MAX_PACKET_SIZE_NDX];

    _endpoints[0].max_recv_packet  = max_packet_size ;
    _endpoints[0].max_send_packet  = max_packet_size ;
    _send_info.maxpkt               (max_packet_size);
    _recv_info.maxpkt               (max_packet_size);

    // use raw value
    _pma_descs.EPRN<0>().count_tx =   UsbBufDesc
                                    ::CountTx
                                    ::count_0(max_packet_size);

    // converts to on of allowed modulo 2 (2, 4, ... 62)
    // or modulo 32 values (32, 64, 96, ... 512)
    _pma_descs.EPRN<0>().count_rx.set_num_blocks_0(max_packet_size);

    // retreive adjusted size
    uint16_t    rx_buffer_size = _pma_descs.EPRN<0>().count_rx.num_bytes_0();

    // keep PMA memory addresses aligned on 32-bit boundaries
    rx_buffer_size = (rx_buffer_size + 3) & ~0x3;

    // CPU vs peripheral memory addressing
    _pma_descs.EPRN<0>().addr_rx = (pma_addr -= rx_buffer_size);
    _endpoints[0].recv_pma       = reinterpret_cast<uint32_t*>(
                                                      USB_PMAADDR
                                                    + _BTABLE_OFFSET
                                                    + (pma_addr << 1));
    _setup_packet                = reinterpret_cast<SetupPacket*>(
                                                      USB_PMAADDR
                                                    + _BTABLE_OFFSET
                                                    + (pma_addr << 1));

    // use non-adjusted size, and keep PMA memory addresses aligned
    // on 32-bit boundaries
    max_packet_size = (max_packet_size + 3) & ~0x3;

    // CPU vs peripheral memory addressing
    _pma_descs.EPRN<0>().addr_tx = (pma_addr -= max_packet_size);
    _endpoints[0].send_pma       = reinterpret_cast<uint32_t*>(
                                                      USB_PMAADDR
                                                    + _BTABLE_OFFSET
                                                    + (pma_addr << 1));


    bool    success = true;   // return value

    // Parse configuration descriptor to find endpoint descriptors.
    // Assumes descriptor size ("bLength" value, first byte of descrriptor)
    //   is correct.
    // Note  _num_eprns is initialized to 1 in constructor (always have
    // control endpoint)
    //
    for (const uint8_t*     desc_data =   _CONFIG_DESC                       ;
                            desc_data <   _CONFIG_DESC
                                        + _CONFIG_DESC[CONFIG_DESC_SIZE_NDX] ;
                                                                              ){
        if (   *(desc_data + 1)
            != static_cast<uint8_t>(DescriptorType::ENDPOINT)) {
            desc_data += *desc_data;  // is length -- step to next descriptor
            continue;
        }

        // save endpoint info
        //

        // handle possible endpoint descriptor max packet size > 255 (shouldn't
        // happen with USB 2.0 low or full speed except full speed isonchronous
          max_packet_size
        =   *(desc_data + _ENDPOINT_DESC_PACKET_SIZE_NDX    )
          + *(desc_data + _ENDPOINT_DESC_PACKET_SIZE_NDX + 1) * 256;

        uint8_t     address = *(desc_data + _ENDPOINT_DESC_ADDRESS_NDX);

        // extract in/out bit and mask in case illegal bits 4,5,6 set
        uint8_t     endpoint_dir  = address & ENDPOINT_DIR_IN   ,
                    endpoint_addr = address & ENDPOINT_ADDR_MASK;

        if (endpoint_addr == 0) {
            // illegal endpoint (can't redefine control endpoint 0)
            success = false;
            break;
        }

        // check if already have this endpoint_addr, possibly with different
        // endpoint_dir (okay, will just add new dir to old) or pathologically
        // (mistake in configuration descriptor, repeated endpoint+dir)
        uint8_t     eprn_ndx = _epaddr2eprn[endpoint_addr];
        if (eprn_ndx == 0)            // is new, not-seen_before endpoint address
            eprn_ndx = _num_eprns++;  // assign to next empty Usb::Eprn
        // else is either malformed redefinition of a prior endpoint
        // (accept without error) or is defining OUT for already define IN
        // or vice-versa (allowed by USB spec and ST USB peripheral)

        if (eprn_ndx == Usb::NUM_ENDPOINT_REGS) {    // have run out of EPRNS
            success = false;
            break;
        }

        // mappings
        _epaddr2eprn[endpoint_addr] = eprn_ndx   ;
        _eprn2epaddr[eprn_ndx     ] = endpoint_addr;

        uint16_t    adjusted_packet_size;

        if (endpoint_dir)  // IN / send / tx
            // keep 32-bit alignment
            adjusted_packet_size = (max_packet_size + 3) & ~0x3;
        else { // OUT / recv / rx
            // converts to allowed modulo 2 or modulo 32 values
             _pma_descs
            .eprn(eprn_ndx)
            .count_rx
            .set_num_blocks_0(max_packet_size);

            // retrieve converted value
            adjusted_packet_size =  _pma_descs.eprn(eprn_ndx)
                                   .count_rx
                                   .num_bytes_0();

            // but still ensure 32-bit alignment (might be only modulo-2)
            adjusted_packet_size = (max_packet_size + 3) & ~0x3;
        }

        // check for memory collision, up-growing buffer descriptors
        // vs. down-growing packet buffer memory
        if (   _BTABLE_OFFSET + (eprn_ndx * sizeof(UsbBufDesc))
            >= (pma_addr -= adjusted_packet_size)                    ) {
            // ignore this and any further endpoint descriptors
            success = false;
            break;
        }

        _endpoints[eprn_ndx].type = static_cast<EndpointType>(
                                              *(  desc_data
                                            + _ENDPOINT_DESC_ATTRIBUTES_NDX));

        if (endpoint_dir) {  // IN / send / tx
            // use original value
            _endpoints[eprn_ndx].max_send_packet = max_packet_size;

            // already decremented above
            _pma_descs.eprn(eprn_ndx).addr_tx = pma_addr;

            // CPU memory addressing
              _endpoints[eprn_ndx].send_pma
            = reinterpret_cast<uint32_t*>(  USB_PMAADDR
                                          + _BTABLE_OFFSET
                                          + (pma_addr << 1));
        }
        else {
            // use original value
            _endpoints[eprn_ndx].max_recv_packet = max_packet_size;

            // already decremented above
            _pma_descs.eprn(eprn_ndx).addr_rx = pma_addr;

            // count_rx already set abov

            // CPU vs peripheral memory addressing
              _endpoints[eprn_ndx].recv_pma
            = reinterpret_cast<uint32_t*>(  USB_PMAADDR
                                          + _BTABLE_OFFSET
                                          + (pma_addr << 1));
        }

        // is length -- step to next descriptor
        desc_data += *desc_data;
    }


    // clear any pending interrupts (particularly reset)
    usb->istr.clr(  Usb::Istr::PMAOVR
                  | Usb::Istr::ERR
                  | Usb::Istr::WKUP
                  | Usb::Istr::SUSP
                  | Usb::Istr::RESET
                  | Usb::Istr::SOF
                  | Usb::Istr::ESOF );

    // enable notification flags (and interrupts if USB_DEV_INTERRUPT_DRIVEN)
    usb->cntr = Usb::Cntr ::CTRM | Usb::Cntr::RESETM;

    // listen for configuration requests on default pipe 0
    set_address(0);

    _device_state = DeviceState::INITIALIZED;

    return success;   // false if aborted endpoint search/parsing/initialization

}  // init()



#ifdef USB_DEV_FORCE_RESET_CAPABILITY  // see usb_dev.hxx
void UsbDev::force_reset()
{
    usb->cntr = Usb::Cntr::FRES | Usb::Cntr ::CTRM | Usb::Cntr::RESETM;
    for (uint32_t ndx = 0 ; ndx < 7200000 ; ++ndx) asm("nop");  // 100 ms
    usb->cntr =                   Usb::Cntr ::CTRM | Usb::Cntr::RESETM;
}
#endif



#ifndef USB_DEV_NO_BUFFER_RECV_SEND
uint16_t UsbDev::recv(
const uint8_t           endpoint,   // trust caller for OUT endpoint
      uint8_t* const    buffer  )   // trust caller for valid buffer,size
{
    if (!(_recv_readys & (1 << endpoint)))
        return 0;

    uint8_t     eprn_ndx = _epaddr2eprn[endpoint];
    uint16_t    recv_len =  _pma_descs.eprn(eprn_ndx)
                           .count_rx
                           .shifted(UsbBufDesc::CountRx::COUNT_0_SHFT);

    // shouldn't ever happen
    if (recv_len > _endpoints[eprn_ndx].max_recv_packet)
        recv_len = _endpoints[eprn_ndx].max_recv_packet;

    read_pma_data(buffer, _endpoints[eprn_ndx].recv_pma, recv_len);

    _recv_readys &= ~(1 << endpoint);

    usb->eprn(eprn_ndx).stat_rx(Usb::Epr::STAT_RX_VALID);

    return recv_len;

}  // recv()



bool UsbDev::send(
const uint8_t           endpoint   ,  // trust caller for OUT endpoint
const uint8_t* const    data       ,  // trust caller for valid buffer
const uint16_t          data_length)  // trust caller <= max_send_packet
{
    if (!(_send_readys & (1 << endpoint)))
        return false;

    uint8_t     eprn_ndx = _epaddr2eprn[endpoint];

    writ_pma_data(data, _endpoints[eprn_ndx].send_pma, data_length);

    _pma_descs.eprn(eprn_ndx).count_tx =   UsbBufDesc
                                         ::CountTx
                                         ::count_0(data_length);

    usb->eprn(eprn_ndx).stat_tx(Usb::Epr::STAT_TX_VALID);

    _send_readys &= ~(1 << endpoint);

    return true;

}  // send()
#endif   // ifndef USB_DEV_NO_BUFFER_RECV_SEND



void UsbDev::interrupt_handler()
{

    if (stm32f103xb::usb->istr.any(Usb::Istr::RESET))
        reset();

    while (stm32f103xb::usb->istr.any(Usb::Istr::CTR))
        ctr();

    // clear all interrupt bits
    usb->istr.clr(  Usb::Istr::PMAOVR
                  | Usb::Istr::ERR
                  | Usb::Istr::WKUP
                  | Usb::Istr::SUSP
                  | Usb::Istr::RESET
                  | Usb::Istr::SOF
                  | Usb::Istr::ESOF );

}  // interrupt_handler()




// protected:

const Usb::Epr::mskd_t  UsbDev::_DESC_EP_TYPE_TO_EPR_EP_TYPE[] = {
    Usb::Epr::EP_TYPE_CONTROL,   // 0b01   EndpointType::CONTROL      = 0
    Usb::Epr::EP_TYPE_ISO,       // 0b10   EndpointType::ISYNCHRONOUS = 1
    Usb::Epr::EP_TYPE_BULK,      // 0b00   EndpointType::BULK         = 2
    Usb::Epr::EP_TYPE_INTERRUPT, // 0b11   EndpointType::INTERRUPT    = 3
};

const uint8_t   UsbDev::_LANGUAGE_ID_STRING_DESC[] = {
                4,
                static_cast<uint8_t>(UsbDev::DescriptorType::STRING),
                0x09, 0x04},         // language codes

                UsbDev::_VENDOR_STRING_DESC[] = {
                38,
                static_cast<uint8_t>(UsbDev::DescriptorType::STRING),
                'S', 0, 'T', 0, 'M', 0, 'i', 0,
                'c', 0, 'r', 0, 'o', 0, 'e', 0,
                'l', 0, 'e', 0, 'c', 0, 't', 0,
                'r', 0, 'o', 0, 'n', 0, 'i', 0,
                'c', 0, 's', 0                }; // "STMicroelectronics"

      uint8_t   UsbDev::_SERIAL_NUMBER_STRING_DESC[] = {
                _SERIAL_NUMBER_STRING_LEN * 2 + 4,
                static_cast<uint8_t>(UsbDev::DescriptorType::STRING),
                '0', 0, '0', 0, '0', 0, '0', 0,
                '0', 0, '0', 0, '0', 0, '0', 0,
                '0', 0, '0', 0, '0', 0, '0', 0,
                '0', 0, '0', 0, '0', 0, '0', 0,
                '0', 0, '0', 0, '0', 0, '0', 0,
                '0', 0, '0', 0, '0', 0, '0', 0,
                 0,  0                        };    // terminating null




void UsbDev::reset()
{
    usb->btable = _BTABLE_OFFSET;

    // must reset all endpoint info because reset sets back to default
    //

    // endpoint 0 (control) is special case
    //
    // can just write toggle bits because known to be currently all 0
    usb->EPRN<0>() =   Usb::Epr::STAT_RX_VALID
                     | Usb::Epr::EP_TYPE_CONTROL
                     | Usb::Epr::STAT_TX_STALL
                     | Usb::Epr::EA<0>()        ;

    _recv_readys         = 0x0000;
    _send_readys         = 0x0001;  // control endpoint, not ever used
    _send_readys_pending = 0x0001;  //    "       "    ,  "   "    "

    // rest of endpoints
    for (uint8_t eprn_ndx = 1 ; eprn_ndx < _num_eprns ; ++eprn_ndx) {
        uint8_t             endpoint_addr = _eprn2epaddr[eprn_ndx        ];
        Usb::Epr::mskd_t    endpoint_type = _DESC_EP_TYPE_TO_EPR_EP_TYPE[
                                             static_cast<unsigned>(
                                             _endpoints[eprn_ndx].type)  ];

        // can just write toggle bits because known to be currently all 0
        //
        if (   _endpoints[eprn_ndx].max_send_packet
            && _endpoints[eprn_ndx].max_recv_packet) {
            // can't do separately below because toggle-only bits
            // this is more efficient than epr_t::write() even with extra
            //   if -- else if -- else
            usb->eprn(eprn_ndx) =   Usb::Epr::STAT_RX_VALID
                                  | endpoint_type
                                  | Usb::Epr::STAT_TX_NAK
                                  | Usb::Epr::ea(endpoint_addr);
            _send_readys_pending |= 1 << endpoint_addr;
        }

        else if (_endpoints[eprn_ndx].max_send_packet) {
            usb->eprn(eprn_ndx) =   Usb::Epr::STAT_TX_NAK
                                  | endpoint_type
                                  | Usb::Epr::ea(endpoint_addr);
            _send_readys_pending |= 1 << endpoint_addr;
        }

        else if (_endpoints[eprn_ndx].max_recv_packet)
            usb->eprn(eprn_ndx) =   Usb::Epr::STAT_RX_VALID
                                  | endpoint_type
                                  | Usb::Epr::ea(endpoint_addr);
    }

    if (   _device_state == DeviceState::ADDRESSED
        || _device_state == DeviceState::CONFIGURED)
        // real re-reset while running, so reset to receive setup packets
        set_address(0);
    else
        // either first reset and DADDR already 0 from init()
        //   or prophylactic reset sent during enumeration so *don't* change
        //   away from address already set
        // just set endpoint numbers in registers while leaving DADDR alone
        set_address(IMPOSSIBLE_DEV_ADDR);

    _device_state = DeviceState::RESET;

}  // reset()



void UsbDev::ctr()  // CTR_LP()
{
    Usb::istr_t istr = usb->istr;  // need to save copy as-is on entry

    uint8_t     eprn_ndx = istr >> Usb::Istr::EP_ID_SHFT;

    if (eprn_ndx == 0) {  // is control endpoint
        // ctr_tx can clear spontaneously (observerved) and also
        // possibly due to setting STAT_TX/STAT_RX
        bool    ctr_tx  = usb->EPRN<0>().any(Usb::Epr::CTR_TX),
                ctr_stp = usb->EPRN<0>().any(Usb::Epr::SETUP ); // can change

        if (!usb->EPRN<0>().any    (  Usb::Epr::CTR_RX | Usb::Epr::CTR_TX)) {
             usb->EPRN<0>().stat_tx(  Usb::Epr::STAT_TX_STALL
                                    | Usb::Epr::STAT_RX_STALL);
            return;
        }

        // ignore ISTR DIR flag because if set (DIR==OUT) can still
        // have EPRN<0> CTR_TX in addition to CTR_RX, as happens normally
        // when ISTR DIR flag is clear
        if (usb->EPRN<0>().any(Usb::Epr::CTR_RX)) {
            usb->EPRN<0>().clear(Usb::Epr::CTR_RX);
            if (ctr_stp /*usb->EPRN<0>().any(Usb::Epr::SETUP*/)
                setup();
            else
                control_out();
        }

        // might have been cleared (spontaneously/side-effect/intentinally)
        // in setup() and/or control_out(), and also might have been
        // set by hardware during their execution
        if (ctr_tx || usb->EPRN<0>().any(Usb::Epr::CTR_TX)) {
            usb->EPRN<0>().clear(Usb::Epr::CTR_TX);
            control_in();
        }
    }

    else {  // normal endpoint
        uint8_t     epaddr = _eprn2epaddr[eprn_ndx];

        if (usb->eprn(eprn_ndx).any(Usb::Epr::CTR_RX)) {
            _recv_readys |= 1 << epaddr;

            usb->eprn(eprn_ndx).clear(Usb::Epr::CTR_RX);

#ifdef USB_DEV_ENDPOINT_CALLBACKS
            if (_recv_callbacks[epaddr]._callback)
                _recv_callbacks[epaddr]._callback(epaddr                  ,
                                                   _recv_callbacks[epaddr]
                                                  ._user_data             );
#endif
        }

        if (usb->eprn(eprn_ndx).any(Usb::Epr::CTR_TX)) {
            _send_readys |= 1 << epaddr;

            usb->eprn(eprn_ndx).clear(Usb::Epr::CTR_TX);

#ifdef USB_DEV_ENDPOINT_CALLBACKS
            if (_send_callbacks[epaddr]._callback)
                _send_callbacks[epaddr]._callback(epaddr                  ,
                                                   _send_callbacks[epaddr]
                                                  ._user_data             );
#endif
        }
    }

}  // ctr()



void UsbDev::setup()
{
    bool    standard_handled = false;

    if (  _setup_packet
        ->request_type
        .all(SetupPacket::RequestType::TYPE_STANDARD))
        standard_handled = standard_request();

    // insane USB protocol: e.g. HID descriptor requests are TYPE_STANDARD,
    // not TYPE_CLASS
    if (     _setup_packet
           ->request_type
           .all(SetupPacket::RequestType::TYPE_CLASS)
        || !standard_handled                         )
        device_class_setup();

    // always call, either to send real data or zero-length status packet
    data_stage_in();

     _pma_descs
    .EPRN<0>()
    .count_rx
    .set_num_blocks_0(_endpoints[0].max_recv_packet);

}  // setup()



bool UsbDev::standard_request()
{
    if (  _setup_packet
        ->request_type
        . all(SetupPacket::RequestType::RECIPIENT_DEVICE))
        return device_request();

    else if (  _setup_packet
             ->request_type
             . all(SetupPacket::RequestType::RECIPIENT_INTERFACE))
        return interface_request();

#if 0  // would be implemented in derived class if necessary
    else if (  _setup_packet
             ->request_type
             . all(SetupPacket::RequestType::RECIPIENT_ENDPOINT))
        return endpoint_request();
#endif

    else
        return false;
}



bool UsbDev::device_request()
{

    switch (static_cast<SetupPacket::Request>(_setup_packet->request)) {
        case SetupPacket::Request::GET_DESCRIPTOR:
            return descriptor_request();

        case SetupPacket::Request::SET_ADDRESS:
            // Can *not* immediately set address. Must wait until next
            //     IN packet (zero-length status packet) has been sent.
            _pending_set_addr = _setup_packet->value.bytes.byte0;
            return true;

        case SetupPacket::Request::GET_STATUS:
            _send_info.set(reinterpret_cast<uint8_t*>(&_status), 2);
            return true;

        case SetupPacket::Request::GET_CONFIGURATION:
            _send_info.set(&_current_configuration, 1);
            return true;

        case SetupPacket::Request::SET_CONFIGURATION:
            _current_configuration = _setup_packet->value.bytes.byte0;
            _send_readys           = _send_readys_pending            ;
            _device_state          = DeviceState::CONFIGURED         ;

            set_configuration();  // notify derived class if interested
            _send_info.reset ();  // just in case
            return true;

        default:
            return false;
    }

    return false;
}



bool UsbDev::interface_request()
{
    switch (static_cast<SetupPacket::Request>(_setup_packet->request)) {
        case SetupPacket::Request::GET_INTERFACE:
            _send_info.set(&_current_interface, 1);
            return true;

        case SetupPacket::Request::SET_INTERFACE:
            _current_interface = _setup_packet->value.bytes.byte0;
            set_interface   ();  // notify derived class if interested
            _send_info.reset();  // just in case
            return true;

        default:
            return false;
    }

    return false;
}



bool UsbDev::descriptor_request()
{

    switch (static_cast<Descriptor>(_setup_packet->value.bytes.byte1)) {
        case Descriptor::DEVICE:
            _send_info.set(_DEVICE_DESC                      ,
                           _DEVICE_DESC[_DESCRIPTOR_SIZE_NDX]);
            return true;

        case Descriptor::CONFIGURATION:
            _send_info.set(_CONFIG_DESC, _setup_packet->length);
            return true;

        case Descriptor::STRING:
            _send_info.set(_STRING_DESCS[_setup_packet->value.bytes.byte0],
                           _STRING_DESCS[_setup_packet->value.bytes.byte0]
                                        [_DESCRIPTOR_SIZE_NDX            ]);
            return true;

        default:
            usb->EPRN<0>().stat_rx(Usb::Epr::STAT_TX_STALL);
            return false;    // no further action needed
    }

    return false;
}



void UsbDev::control_out()
{

    if (_recv_info.remaining_size()) {
        uint16_t    recv_size = _recv_info.transfer_size();

        read_pma_data(_recv_info.remaining_data() ,
                      _endpoints[0].recv_pma      ,
                      recv_size                   );

        _recv_info.update(recv_size);

        // needed to set TX_VALID next transaction after getting CDC/ACM
        // SET_LINE_CODING request (type: 0x21  rqst: 0x20)
        // why when no data being sent back?
        _pma_descs. EPRN<0>().count_tx = 0                         ;
               usb->EPRN<0>().stat_tx_rx(  Usb::Epr::STAT_TX_VALID
                                         | Usb::Epr::STAT_RX_VALID);
        _last_send_size = 0;
    }

    if (_recv_info.remaining_size())
        // Don't see why this should happen (more than control/endpt0 size)
        // In fact, don't see why getting any endpt0 non-setup OUT at all
        //   except as "status stage" of multiple data stages.
        usb->EPRN<0>().stat_rx(Usb::Epr::STAT_RX_STALL);
    else
        usb->EPRN<0>().stat_rx(Usb::Epr::STAT_RX_VALID);

}  // control_out()



void UsbDev::control_in()
{
    // ludicrous USB standards mandated delayed set of new device address
    if (_pending_set_addr != IMPOSSIBLE_DEV_ADDR) {
        set_address(_pending_set_addr);
        _pending_set_addr = IMPOSSIBLE_DEV_ADDR;
    }

    if (_send_info.remaining_size() || _last_send_size > 0) {
        // either real data to send or zero-length status handshake
        data_stage_in();
        return;
    }

    usb->EPRN<0>().stat_tx(Usb::Epr::STAT_TX_STALL | Usb::Epr::STAT_RX_STALL);

}  // control_in()



void UsbDev::data_stage_in()
{

    _last_send_size = _send_info.transfer_size();

    if (_last_send_size) {
        writ_pma_data(_send_info.remaining_data() ,
                      _endpoints[0].send_pma      ,
                      _last_send_size             );

        _send_info.update(_last_send_size);
    }

    _pma_descs. EPRN<0>().count_tx = _last_send_size       ;
           usb->EPRN<0>().stat_tx(+Usb::Epr::STAT_TX_VALID);

    if (_last_send_size == 0) {
        // reenable RX for next transfer
        usb->EPRN<0>().stat_rx(Usb::Epr::STAT_RX_VALID);
    }

}  // data_stage_in()



#ifdef USB_DEV_DMA_PMA
#ifndef USB_DEV_DMA_CHANNEL
#error Must define USB_DEV_DMA_CHANNEL with USB_DEV_DMA_PMA
#endif

#if USB_DEV_DMA_CHANNEL < 1 || USB_DEV_DMA_CHANNEL > 7
#error USB_DEV_DMA_CHANNEL must be in range 1..7
#endif

#ifndef USB_DEV_DMA_PRIORITY
#error Must define USB_DEV_DMA_PRIORITY with USB_DEV_DMA_PMA
#endif

#if USB_DEV_DMA_PRIORITY < 0 || USB_DEV_DMA_PRIORITY > 3
#error USB_DEV_DMA_PRIORITY must be in range 0..3
#endif

#define DMA_CHAN_CONCAT(CHAN)       dma1_channel##CHAN
#define DMA_CHAN_PRE_CONCAT(CHAN)   DMA_CHAN_CONCAT(CHAN)
#define DMA_CHANNEL                 DMA_CHAN_PRE_CONCAT(USB_DEV_DMA_CHANNEL)

#define DMA_TCIF_CONCAT(PRIOR)      TCIF##PRIOR
#define DMA_TCIF_PRE_CONCAT(PRIOR)  DMA_TCIF_CONCAT(PRIOR)
#define DMA_TCIF                    DMA_TCIF_PRE_CONCAT(USB_DEV_DMA_PRIORITY)

#define DMA_CTCIF_CONCAT(PRIOR)     CTCIF##PRIOR
#define DMA_CTCIF_PRE_CONCAT(PRIOR) DMA_CTCIF_CONCAT(PRIOR)
#define DMA_CTCIF                   DMA_CTCIF_PRE_CONCAT(USB_DEV_DMA_PRIORITY)

#endif  // #ifdef USB_DEV_DMA_PMA




void UsbDev::writ_pma_data(
const uint8_t*  const   data,
      uint32_t* const   addr,
const uint16_t          size)
{
#ifdef USB_DEV_DMA_PMA
    DMA_CHANNEL->ccr = 0;

    DMA_CHANNEL->pa  = reinterpret_cast<uint32_t>(addr);
    DMA_CHANNEL->ma  = reinterpret_cast<uint32_t>(data);
    DMA_CHANNEL->ndt = (size + 1) >> 1                 ;

    DMA_CHANNEL->ccr =    DmaChannel::Ccr::MEM2MEM
                        | DmaChannel::Ccr::mskd_t(DmaChannel::Ccr::PL_MASK,
                                                  USB_DEV_DMA_PRIORITY    ,
                                                  DmaChannel::Ccr::PL_POS )
                        | DmaChannel::Ccr::MSIZE_16_BITS
                        | DmaChannel::Ccr::PSIZE_32_BITS
                        | DmaChannel::Ccr::MINC
                        | DmaChannel::Ccr::PINC
                        | DmaChannel::Ccr::DIR_MEM2PERIPH                     ;

    DMA_CHANNEL->ccr |= DmaChannel::Ccr::EN;

    while (!dma1->isr.any(Dma::Isr::DMA_TCIF))   // wait for DMA to finish
        ;

    DMA_CHANNEL->ccr -= DmaChannel::Ccr ::EN       ;
    dma1->ifcr       |= Dma       ::Ifcr::DMA_CTCIF;
#else
    const uint16_t*     dat = reinterpret_cast<const uint16_t*>(data);
          uint32_t*     pma = addr                                   ;

    for (uint16_t   count = (size + 1) >> 1; count ; --count)
        *pma++ = *dat++;
#endif
}



void UsbDev::read_pma_data(
      uint8_t* const    data,
const uint32_t*         addr,
const uint16_t          size)
{
#ifdef USB_DEV_DMA_PMA
    DMA_CHANNEL->ccr = 0;

    DMA_CHANNEL->pa  = reinterpret_cast<uint32_t>(addr);
    DMA_CHANNEL->ma  = reinterpret_cast<uint32_t>(data);
    DMA_CHANNEL->ndt = (size + 1) >> 1                 ;

    DMA_CHANNEL->ccr =    DmaChannel::Ccr::MEM2MEM
                        | DmaChannel::Ccr::mskd_t(DmaChannel::Ccr::PL_MASK,
                                                  USB_DEV_DMA_PRIORITY    ,
                                                  DmaChannel::Ccr::PL_POS )
                        | DmaChannel::Ccr::MSIZE_16_BITS
                        | DmaChannel::Ccr::PSIZE_32_BITS
                        | DmaChannel::Ccr::MINC
                        | DmaChannel::Ccr::PINC
                        | DmaChannel::Ccr::DIR_PERIPH2MEM                     ;

    DMA_CHANNEL->ccr |=   DmaChannel::Ccr::EN;

    while (!dma1->isr.any(Dma::Isr::DMA_TCIF))   // wait for DMA to finish
        ;

    DMA_CHANNEL->ccr -= DmaChannel::Ccr ::EN       ;
    dma1->ifcr       |= Dma       ::Ifcr::DMA_CTCIF;
#else
          uint16_t*     dat = reinterpret_cast<uint16_t*>(data);
    const uint32_t*     pma = addr                             ;

    for (uint16_t   count = (size + 1) >> 1; count ; --count)
        *dat++ = *pma++;
#endif
}



void UsbDev::set_address(
const uint8_t   address)
{

    for (uint8_t ndx = 0 ; ndx < _num_eprns ; ++ndx)
        usb->eprn(ndx).write(Usb::Epr::mskd_t(Usb::Epr::EA_MASK,
                                              _eprn2epaddr[ndx],
                                              Usb::Epr::EA_POS ));

    if (address == IMPOSSIBLE_DEV_ADDR)
        return;

    usb->daddr = Usb::Daddr::add(address) | Usb::Daddr::EF;

    _device_state = DeviceState::ADDRESSED;

}  // set_address()

} // namespace stm32f10_12357_xx
