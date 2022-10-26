/****************************************************************************
 *
 * nl_devenf.h
 *
 * NextLabs Device Enforcer - Driver interface
 *
 ***************************************************************************/
#ifndef __NL_DEVENF_H__
#define __NL_DEVENF_H__

/* Device Names */
#define NL_DEVENF_DEVICE_NAME        L"\\Device\\NLDevEnf"
#define NL_DEVENF_SYMBOLIC_LINK_NAME L"\\DosDevices\\NLDevEnf"
#define NL_DEVENF_DEVICE             L"\\\\.\\NLDevEnf"

/** Device States
 *
 *  Bits that may be set in the 'state' member of DeviceInfo.
 */
#define NL_DEVENF_DEVICE_STATE_ENABLED  (0x1 << 0) /* Device is enabled */

/** DeviceInfoContext
 *
 *  Storage for user-mode context.  User-mode may store any content here.
 */
typedef struct
{
  unsigned char context[256];
} DeviceInfoContext;

/** DeviceInfo
 *
 *  \brief Information that can uniquely identify a device.
 */
typedef struct
{
  LONG device_number;              /* Device Number
				    *
				    *   Unique for each device.  Indicates this was the
				    *   Nth device to be attached.
				    */

  LARGE_INTEGER sys_time_arrival;  /* System time of arrival */

  LARGE_INTEGER sys_time_changed;  /* System time of change */

  WCHAR InterfaceName[512];    /* Device Interface
			        *   The symbolic link uniquely identifies a device
			        *   on the system.  User-mode must not modify this
			        *   member.
			        */

  WCHAR SetupClass[128];       /* Device Setup Class (GUID)
			        *   Setup class GUID of the device.  User-mode must
			        *   not modify this member.
				*   MSDN: DevicePropertyClassGuid
			        */

  WCHAR ClassName[32];         /* Class name (e.g. USB, Net, etc.)
				*   MSDN: DevicePropertyClassName
				*/

  WCHAR CompatibleIDs[512];    /* Compatible IDs
				*   MSDN: DevicePropertyCompatibleIDs
				*
				*   Strings separated by NULL termination.  Also known
				*   as REG_MULTI_SZ format.
				*/

  GUID BusTypeGuid;            /* BusTypeGuid
				*   MSDN: DevicePropertyBusTypeGuid
				*   Example: GUID_BUS_TYPE_1394
				*            GUID_BUS_TYPE_USB
				*/

  WCHAR EnumeratorName[32];    /* Enumerator name (e.g. USB, 1394, etc.)
				*   MSDN: DevicePropertyEnumeratorName
				*/

  int state;                   /* Device State
			        *   NL_DEVENF_DEVICE_STATE_XXX
			        */

  DeviceInfoContext context;   /* User-mode context
			        *   Can be used for storage of any data of size
			        *   sizeof(context).
			        */
} DeviceInfo;

/** Device Operations
 *
 *  IOCTLs for NLDevEnf
 */
enum
{
  NL_DEVENF_IOCTL_BASE = 0x800,
  NL_DEVENF_IOCTL_GET_ALL_DEVICES = NL_DEVENF_IOCTL_BASE,
  NL_DEVENF_IOCTL_TEST_DEVICE_ARRIVE,
  NL_DEVENF_IOCTL_WAIT_DEVICE_ARRIVE,
  NL_DEVENF_IOCTL_SET_DEVICE_STATE
};

/** IOCTL_NL_DEVENF_GET_ALL_DEVICES
 *
 *  Read all devices.
 *
 *  Input      : User supplied space for storage of an 'DeviceInfo' array.
 *  Output     : N/A
 *  Output Size: Number of bytes written to the user supplied space.  When that space is
 *               not sufficient the size indicates the required user supplied space for a
 *               successful call.  This occurs only when the user supplied buffer is NULL,
 *               otherwise, STATUS_MORE_ENTRIES is returned to indicate that are is to read
 *               than there is space supplied.
 */
#define IOCTL_NL_DEVENF_GET_ALL_DEVICES CTL_CODE(FILE_DEVICE_UNKNOWN,                 \
                                                 NL_DEVENF_IOCTL_GET_ALL_DEVICES,     \
					         METHOD_BUFFERED,                     \
					         FILE_READ_DATA | FILE_WRITE_DATA)

/** IOCTL_NL_DEVENF_TEST_DEVICE_ARRIVE
 *
 *  Determine if a device has arrived.  More specifically, it determines if a device
 *  has arrived since the last check by any process.  Does not block.  Does not change
 *  signal value.  If the arrival is signaled it remains signaled.
 *
 *  Input      : N/A
 *  Output     : int
 *               Non-zero value indicates that a device has arrived.
 *  Output Size: N/A
 */
#define IOCTL_NL_DEVENF_TEST_DEVICE_ARRIVE CTL_CODE(FILE_DEVICE_UNKNOWN,                \
                                                    NL_DEVENF_IOCTL_TEST_DEVICE_ARRIVE, \
			                            METHOD_BUFFERED,                    \
					            FILE_READ_DATA | FILE_WRITE_DATA)

/** IOCTL_NL_DEVENF_WAIT_DEVICE_ARRIVE
 *
 *  Determine if a device has arrived.  Block until a device arrives.
 *
 *  Input      : N/A
 *  Output     : N/A
 *  Output Size: N/A
 */
#define IOCTL_NL_DEVENF_WAIT_DEVICE_ARRIVE CTL_CODE(FILE_DEVICE_UNKNOWN,                 \
                                                    NL_DEVENF_IOCTL_WAIT_DEVICE_ARRIVE,  \
					            METHOD_BUFFERED,                     \
					            FILE_READ_DATA | FILE_WRITE_DATA)

/** IOCTL_NL_DEVENF_SET_DEVICE_STATE
 *
 *  Diable a device.  Once a device has been disabled
 *
 *  Input      : DeviceInfo*
 *  Output     : N/A
 *  Output Size: N/A
 */
#define IOCTL_NL_DEVENF_SET_DEVICE_STATE CTL_CODE(FILE_DEVICE_UNKNOWN,                 \
                                                  NL_DEVENF_IOCTL_SET_DEVICE_STATE,    \
				                  METHOD_BUFFERED,                     \
				                  FILE_READ_DATA | FILE_WRITE_DATA)

#endif /* __NL_DEVENF_H__ */
