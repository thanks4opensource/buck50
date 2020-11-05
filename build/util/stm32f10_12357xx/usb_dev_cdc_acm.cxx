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


#include <usb_dev_cdc_acm.hxx>

namespace stm32f10_12357_xx {

const uint8_t UsbDev::_DEVICE_DESC[] = {
    0x12,   // bLength
    static_cast<uint8_t>(UsbDev::DescriptorType::DEVICE),
    0x00,
    0x02,   // bcdUSB = 2.00
    0x02,   // bDeviceClass: CDC
    0x00,   // bDeviceSubClass
    0x00,   // bDeviceProtocol
    0x40,   // bMaxPacketSize0
    0x83,   // idVendor = 0x0483
    0x04,   //    "     = MSB of uint16_t
    0x40,   // idProduct = 0x5740
    0x57,   //     "     = MSB of uint16_t
    0x00,   // bcdDevice = 2.00
    0x02,   //     "     = MSB of uint16_t
    1,      // Index of string descriptor describing manufacturer
    2,      // Index of string descriptor describing product
    3,      // Index of string descriptor describing device serial number
    0x01    // bNumConfigurations
};

uint8_t UsbDev::_CONFIG_DESC[] = {  // not const because set total size entry
    // Configuration Descriptor
    0x09,   // bLength: Configuration Descriptor size
    static_cast<uint8_t>(UsbDev::DescriptorType::CONFIGURATION),
    0,      // wTotalLength: including sub-descriptors; will be set a runtime
    0x00,   //      "      : MSB of uint16_t
    0x02,   // bNumInterfaces: 2 interface
    0x01,   // bConfigurationValue: Configuration value
    0x00,   // iConfiguration: Index of string descriptor for configuration
    0xC0,   // bmAttributes: self powered
    0x32,   // MaxPower 0 mA

    // Interface Descriptor
    0x09,   // bLength: Interface Descriptor size
    static_cast<uint8_t>(UsbDev::DescriptorType::INTERFACE),
    // Interface descriptor type
    0x00,   // bInterfaceNumber: Number of Interface
    0x00,   // bAlternateSetting: Alternate setting
    0x01,   // bNumEndpoints: One endpoints used
    0x02,   // bInterfaceClass: Communication Interface Class
    0x02,   // bInterfaceSubClass: Abstract Control Model
    0x01,   // bInterfaceProtocol: Common AT commands
    0x00,   // iInterface:

    // Header Functional Descriptor
    0x05,   // bFunctionLength
    0x24,   // bDescriptorType: CS_INTERFACE
    0x00,   // bDescriptorSubtype: Header Func Desc
    0x10,   // bcdCDC: spec release number
    0x01,

    // Call Management Functional Descriptor
    0x05,   // bFunctionLength
    0x24,   // bDescriptorType: CS_INTERFACE
    0x01,   // bDescriptorSubtype: Call Management Func Desc
    0x00,   // bmCapabilities: D0+D1
    0x01,   // bDataInterface: 1

    // ACM Functional Descriptor
    0x04,   // bFunctionLength
    0x24,   // bDescriptorType: CS_INTERFACE
    0x02,   // bDescriptorSubtype: Abstract Control Management desc
    0x02,   // bmCapabilities

    // Union Functional Descriptor
    0x05,   // bFunctionLength
    0x24,   // bDescriptorType: CS_INTERFACE
    0x06,   // bDescriptorSubtype: Union func desc
    0x00,   // bMasterInterface: Communication class interface
    0x01,   // bSlaveInterface0: Data Class Interface

    // Endpoint 2 Descriptor
    0x07,   // bLength: Endpoint Descriptor size
    static_cast<uint8_t>(UsbDev::DescriptorType::ENDPOINT),
    UsbDevCdcAcm::ACM_ENDPOINT | UsbDev::ENDPOINT_DIR_IN,   // bEndpointAddress
    static_cast<uint8_t>(UsbDev::EndpointType::INTERRUPT),  // bmAttributes
    UsbDevCdcAcm::ACM_DATA_SIZE,                            // wMaxPacketSize:
    0x00,
    0xFF,   // bInterval:


    // Data class interface descriptor
    0x09,   // bLength: Interface Descriptor size
    static_cast<uint8_t>(UsbDev::DescriptorType::INTERFACE),
    0x01,   // bInterfaceNumber: Number of Interface
    0x00,   // bAlternateSetting: Alternate setting
    0x02,   // bNumEndpoints: Two endpoints used
    0x0A,   // bInterfaceClass: CDC
    0x00,   // bInterfaceSubClass:
    0x00,   // bInterfaceProtocol:
    0x00,   // iInterface:

    // Endpoint 3 Descriptor
    0x07,   // bLength: Endpoint Descriptor size
    static_cast<uint8_t>(UsbDev::DescriptorType::ENDPOINT),
    UsbDevCdcAcm::CDC_ENDPOINT_OUT, // bEndpointAddress: (OUT3)
    static_cast<uint8_t>(UsbDev::EndpointType::BULK),   // bmAttributes: Bulk
    UsbDevCdcAcm::CDC_OUT_DATA_SIZE,                    // wMaxPacketSize: 64
    0x00,                                               //    MSB of uint16_t
    0x00,   // bInterval: ignore for Bulk transfer

    // Endpoint 1 Descriptor
    0x07,                               // bLength: Endpoint Descriptor size
    static_cast<uint8_t>(UsbDev::DescriptorType::ENDPOINT),
    UsbDevCdcAcm::CDC_ENDPOINT_IN | UsbDev::ENDPOINT_DIR_IN,//bEndpointAddress
    static_cast<uint8_t>(UsbDev::EndpointType::BULK),  // bmAttributes: Bulk
    UsbDevCdcAcm::CDC_IN_DATA_SIZE,     // wMaxPacketSize:
    0x00,
    0x00                                // bInterval
};

const uint8_t   UsbDevCdcAcm::_device_string_desc[] = {
                46,
                static_cast<uint8_t>(UsbDev::DescriptorType::STRING),
                'S', 0, 'T', 0, 'M', 0, '3', 0,
                '2', 0, ' ', 0, 'V', 0, 'i', 0,
                'r', 0, 't', 0, 'u', 0, 'a', 0,
                'l', 0, ' ', 0, 'C', 0, 'O', 0,
                'M', 0, ' ', 0, 'P', 0, 'o', 0,
                'r', 0, 't', 0                };     // "STM32 Virtual COM Port"

const uint8_t   *UsbDev::_STRING_DESCS[] = {
    UsbDev      ::  language_id_string_desc(),
    UsbDev      ::       vendor_string_desc(),
    UsbDevCdcAcm::       device_string_desc(),
    UsbDev      ::serial_number_string_desc(),
};

UsbDevCdcAcm::LineCoding UsbDevCdcAcm::_line_coding = {9600, 0, 0, 8} ;



bool UsbDevCdcAcm::init()
{
    _CONFIG_DESC[UsbDev::CONFIG_DESC_SIZE_NDX]  = sizeof(_CONFIG_DESC);

    return UsbDev::init();
}



bool UsbDev::device_class_setup()
{
    if (!  _setup_packet
         ->request_type
         . all(  SetupPacket::RequestType::TYPE_CLASS
               | SetupPacket::RequestType::RECIPIENT_INTERFACE))
        return false;

    uint8_t     *data;
    uint16_t     size;

    switch (_setup_packet->request) {
        case UsbDevCdcAcm::_SET_LINE_CODING:
        case UsbDevCdcAcm::_GET_LINE_CODING:
            data =  reinterpret_cast<uint8_t*>(&UsbDevCdcAcm::_line_coding);
            size =                      sizeof( UsbDevCdcAcm::_line_coding);
            break;

        case UsbDevCdcAcm::_SET_CONTROL_LINE_STATE:
            data = 0;
            size = 0;
            break;

        default:
            return false;
    }

    if (_setup_packet->request_type.any(   SetupPacket
                                        ::RequestType
                                        ::DIR_DEV_TO_HOST))
        _send_info.set(data, size);
    else
        _recv_info.set(data, size);

    return true;
}


void UsbDev::set_configuration() {}
void UsbDev::set_interface    () {}


}  // namespace stm32f10_12357_xx {
