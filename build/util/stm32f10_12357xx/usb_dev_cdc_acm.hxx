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


#ifndef USB_DEV_CDC_ACM_HXX
#define USB_DEV_CDC_ACM_HXX


#ifndef CDC_OUT_EP_SIZE
#define CDC_OUT_EP_SIZE 64  // must be modulo 4, max 64)
#endif


#include <usb_dev.hxx>

#if USB_DEV_MAJOR_VERSION == 1
#if USB_DEV_MINOR_VERSION  < 0
#warning USB_DEV_MINOR_VERSION < 0 with required USB_DEV_MAJOR_VERSION == 1
#endif
#else
#error USB_DEV_MAJOR_VERSION != 1
#endif


namespace stm32f10_12357_xx {

class UsbDevCdcAcm : public UsbDev
{
  public:
    static const uint8_t    // have to be public for clients, static descriptors
                            ACM_ENDPOINT             =  2,
                            CDC_ENDPOINT_IN          =  1,
                            CDC_ENDPOINT_OUT         =  3,
                            // have to be public for extern static definition
                            CDC_IN_DATA_SIZE         = 64,
                            CDC_OUT_DATA_SIZE        = CDC_OUT_EP_SIZE,
                            ACM_DATA_SIZE            =  8;

    constexpr UsbDevCdcAcm()
    :   UsbDev()
    {}

    bool init();


    // need public accessor for static initialization of _NEW_STRING_DESCS
    //
    static constexpr const uint8_t* device_string_desc()
    {
        return _device_string_desc;
    }




  protected:
    friend class UsbDev;

    struct LineCoding {
        uint32_t    baud       ;
        uint8_t     stop_bits  ,
                    parity_code,
                    bits       ;
    };

    static const uint8_t    _NUM_ENDPOINTS          = 4   ,
                            _SET_LINE_CODING        = 0x20,
                            _GET_LINE_CODING        = 0x21,
                            _SET_CONTROL_LINE_STATE = 0x22;

    static const uint8_t        _device_string_desc[];
    static       LineCoding     _line_coding         ;

};  // class UsbDevCdcAcm

}  // namespace stm32f10_12357_xx

#endif  // ifndef USB_DEV_CDC_ACM_HXX
