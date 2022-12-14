#ifndef _DSIFSIOCTL_H_
#define _DSIFSIOCTL_H_	1
#include "dstypes.h"

#ifdef WIN32

#ifndef _NTIFS_
#include <winioctl.h>
#endif	//#ifndef _NTIFS_

#define BJ_IFS_GET_VERSION		 (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x00, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define BJ_IFS_REGISTER_APP		 (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x01, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define BJ_IFS_UNREGISTER_APP	 (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x02, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define BJ_IFS_INIT_KERNEL_IPC	 (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x03, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define BJ_IFS_DEINIT_KERNEL_IPC (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x04, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define BJ_IFS_GET_FILE_INFO	 (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x05, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define BJ_IFS_REGISTER_RECOVERY (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x06, METHOD_BUFFERED, FILE_WRITE_ACCESS )	

#define IOCTL_POLICY_REQUEST \
    CTL_CODE (FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_POLICY_RESPONSE \
    CTL_CODE (FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SET_CM_READY \
    CTL_CODE (FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ADD_PROTECTED_PID \
    CTL_CODE (FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DEL_PROTECTED_PID \
    CTL_CODE (FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_PROTECTED_PIDS \
    CTL_CODE (FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CLR_PROTECTED_PIDS \
    CTL_CODE (FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_POLICY_QUERY \
    CTL_CODE (FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _IPC_SET_CM_READY
{
	WCHAR wzInstalledDir [512];
	ULONG ulInstalledDirLength;
	WCHAR wzShortInstalledDir[512];
	ULONG ulShortInstalledDirLength;
}IPC_SET_CM_READY,*PIPC_SET_CM_READY;

#elif LINUX

#include <linux/ioctl.h>
#define BJ_IFS_IO_MAGIC	'I'
#define BJ_IFS_GET_VERSION			_IO(BJ_IFS_IOC_MAGIC, 0x1)		 
#define BJ_IFS_REGISTER_APP			_IO(BJ_IFS_IOC_MAGIC, 0x2)
#define BJ_IFS_UNREGISTER_APP		_IO(BJ_IFS_IOC_MAGIC, 0x3)
#define BJ_IFS_INIT_KERNEL_IPC		_IO(BJ_IFS_IOC_MAGIC, 0x4)		 
#define BJ_IFS_DEINIT_KERNEL_IPC	_IO(BJ_IFS_IOC_MAGIC, 0x5)		 
#define BJ_IFS_GET_FILE_INFO		_IO(BJ_IFS_IOC_MAGIC, 0x6)		 
#define BJ_IFS_REGISTER_RECOVERY	_IO(BJ_IFS_IOC_MAGIC, 0x7)

#endif	//#ifdef WIN32 ... #elif LINUX ... #endif

#endif	_DSIFSIOCTL_H_
