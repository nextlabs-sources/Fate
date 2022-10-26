/*********************************************************************************
 *
 * NextLabs Device Enforcer (NLDevEnf)
 *
 * User-mode API
 *
 ********************************************************************************/

#ifndef __NL_DEVENF_LIB_H__
#define __NL_DEVENF_LIB_H__

#include <windows.h>
#include "nl_devenf.h"

/** Device_GetDevices
 *
 *  \brief Retrieve the devices on the system.
 *
 *  \param devices (in/out)     User supplied space for storage.
 *  \param num_devices (in/out) Number of devices.
 *
 *  \return Non-zero on success.  Zero on failure.
 *
 *  \notes  When the supplied space is not large enough
 */
extern "C" int Device_GetDevices( DeviceInfo* devices ,
				  int* num_devices );

/** Device_TestForArrival
 *
 *  \brief Determine if there is a device which has arrived.  If a device has
 *         arrived, a call to Device_WaitForArrival will not block.  The
 *         arrival state is unchanged by this call.
 *
 *  \param device_arrived (out) Indicates whether if a device has arrived.  A
 *                              non-zero value indicates a device has arrived.
 *
 *  \return Non-zero on success.  Zero on failure.
 */
extern "C" int Device_TestForArrival( int* device_arrived );

/** Device_WaitForArrival
 *
 *  \brief Wait (block) for a device to arrive.
 *
 *  \param cancel_event (in) An event which can be signaled to abort waiting for
 *                           a device to arrive.  It must be created by calling
 *                           CreateEvent and must be created in an unsignaled
 *                           state.
 *
 *  \return Non-zero on success.  Zero on failure.
 */
extern "C" int Device_WaitForArrival( HANDLE cancel_event );

/** Device_Disable
 *
 *  \brief Disable device.  Block IO to the device.
 *
 *  \return Non-zero on success.  Zero on failure.
 */
extern "C" int Device_Disable( DeviceInfo* device );

/** Device_GetContext
 *
 *  \brief Return a pointer to the context space of the DeviceInfo structure.
 *         When the device entry is created the context the entire context
 *         space is initialized to zero.
 *
 *  \param device (in) Device to retreive context of.
 *
 *  \return Pointer to context space.
 *  \sa Device_GetContextSize.
 */
extern "C" void* Device_GetContext( const DeviceInfo* device );

/** Device_GetContextSize
 *
 *  \brief Return the size in bytes of the context space of DeviceInfo.  The
 *         use of Device_SetContext can be used to write upto this many bytes
 *         into the context space.
 *
 *  \return Number of bytes available in the device context space.
 */
extern "C" size_t Device_GetContextSize(void);

/** Device_SetContext
 *
 *  \brief Set user-defined device context.
 *
 *  \param device (in)      Device.
 *  \param context (in)     User-defined context pointer.
 *  \param contex_size (in) Size in bytes of user-defined context.
 *
 *  \return Non-zero on success.  Zero on failure.
 *  \notes  The call will fail if the context size exceeds the context space
 *          available.
 */
extern "C" int Device_SetContext( DeviceInfo* device ,
				  const void* context,
				  size_t context_size );

#endif /* __NL_DEVENF_LIB_H__ */
