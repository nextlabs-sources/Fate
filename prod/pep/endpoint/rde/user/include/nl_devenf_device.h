
#ifndef __NL_DEVENF_DEVICE_HPP__
#define __NL_DEVENF_DEVICE_HPP__

#include "nl_devenf.h"

/** is_firewire
 *
 *  \brief Determine if the given device is a FireWire (IEEE 1394) device.
 *  \return true if the deivce is a FireWire (IEEE 1394) device.
 */
bool is_firewire( _In_ const DeviceInfo* device );

/** is_usb
 *
 *  \brief Determine if the given device is a USB device.
 *  \return true if the device is a USB device.
 */
bool is_usb( _In_ const DeviceInfo* device );

/** is_pcmcia
 *
 *  \brief Determine if the given device is a PCMCIA (PC Card) device.
 *  \return true if the deivce is a PCMCIA (PC Card) device.
 */
bool is_pcmcia( _In_ const DeviceInfo* device );

/** is_composite_device
 *
 *  \brief Determine if the given device is a composite device.
 *  \return true if the device is a composite device.
 */
bool is_composite_device( _In_ const DeviceInfo* device );

/** read_device
 *
 *  \brief Extract parameters from the DeviceInfo structure.
 *
 *  \param device (in)                  Device.
 *  \param in_vendor_id (out)           Vendor ID.
 *  \param in_product_id (out)          Product ID.
 *  \param in_serial_number (out)       Serial Number.
 *  \param in_serial_number_count (in)  Serial Number size in characters.
 *
 *  \return true on successful parsing of device attributes.
 */
bool read_device( _In_ const DeviceInfo* device ,
		  _Out_ int* in_vendor_id ,
		  _Out_ int* in_product_id ,
		  _Out_cap_(in_serial_number_count) WCHAR* in_serial_number ,
		  _In_ size_t in_serial_number_count );

#endif /* __NL_DEVENF_DEVICE_HPP__ */
