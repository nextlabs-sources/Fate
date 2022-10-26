
#ifndef __NL_DEVICE_H__
#define __NL_DEVICE_H__

typedef struct
{
  int vendor_id;
  wchar_t vendor_string[512];

  int product_id;
  wchar_t product_string[512];

  char sn[512];
} USBDevice;

/** nl_device_from_letter
 *
 *  \brief Map a USB device to a drive letter.
 *
 *  \param root_drive (in) Drive letter such as c:
 *  \param udev (in-out)   USB device information
 *
 *  \return true on success, otherwise false.
 */
bool nl_device_from_letter( _In_ wchar_t root_drive ,
			    _Out_ USBDevice* udev );

/** nl_device_get_info
 *
 *  \brief Map vendor and product IDs to the string values from the USB
 *         device descriptor.  The device for <idVendor,idProduct> must
 *         be present on a the system.
 *
 *  \param idVendor               Vendor ID
 *  \param idProduct              Product ID
 *  \param idVendorString         Vendor name
 *  \param idVendorString_count   Count of chars for vendor name
 *  \param idProductString        Product name
 *  \param idProductString_count  Count of chars for product name
 *
 *  \return true if the <idVendor,idProduct> is mapped to their string
 *          values.  Otherwise, the return value is false.
 */
bool nl_device_get_info(_In_ int idVendor , _In_ int idProduct ,
			 _Out_z_capcount_(idVendorString_count)wchar_t* idVendorString , _In_ size_t idVendorString_count ,
			 _Out_z_capcount_(idProductString_count)wchar_t* idProductString , _In_ size_t idProductString_count  );

#endif /* __NL_DEVICE_H__ */
