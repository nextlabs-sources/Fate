#ifndef _DSIFSINTERFACE_H_
#define _DSIFSINTERFACE_H_	1

#include "dstypes.h"

#define DSIFS_DRIVER_NAME      L"DSIFSFLT.SYS"
#define DSIFS_DEVICE_NAME      L"dsifsflt"
#define DSIFS_W32_DEVICE_NAME  L"\\\\.\\dsifsflt"
#define DSIFS_DOSDEVICE_NAME   L"\\DosDevices\\dsifsflt"
#define DSIFS_FULLDEVICE_NAME1 L"\\FileSystem\\Filters\\dsifsflt"
#define DSIFS_FULLDEVICE_NAME2 L"\\FileSystem\\dsifsfltCDO"

//Common structure for IFS Filter Driver and Control Module
typedef struct _IPCSLOT 
{
	PVOID 			pSendEvent;	//Note that Kernel and User event are opposite to each other.
	PVOID 			pRecvEvent;	//Example:Send in kernel is Recv in user and via versa
	ULONG			ulSlotNumber;	
	DS_LIST_ENTRY	dsObjectEntry;	//Link to the dsFreeIPCQ
}IPC_SLOT, *PIPC_SLOT;

//Common structure for IFS Filter Driver and Control Module
typedef struct _IPCSETUPINFO
{
	PVOID			pIPCSlot;
	DS_LIST_ENTRY	dsFreeIPCQ;		//List of FreeIPCSlot, kernel use only
	ULONG			ulNumSlot;
	PVOID 			pShareMem;
	PVOID			pShareMemMdl;	//Mdl for ShareMem, Windows kernel use only
	ULONG			ulMemSize;
	ULONG			ulSlotSize;
	ULONG			ulProcessID;
	WCHAR			wzInstalledDir[MAX_NAME_LENGTH];
	ULONG			ulInstalledDirLength;
	WCHAR			wzShortInstalledDir[MAX_NAME_LENGTH];
	ULONG			ulShortInstalledDirLength;
	ULONG			ulReserve1;
	ULONG			ulReserve2;
}IPC_SETUP_INFO, *PIPC_SETUP_INFO;

//For use with BJ_IFS_REGISTER_APPLICATION ioctl for the driver to handle 
//application crash log, recovery.  The same structure must send down  
//for ther BJ_IFS_UNREGISTER_APPLICATION ioctl.
typedef struct _USER_REGISTRATION
{
	IN ULONG 	ulProcessID;
	IN char 	szAppEXE[MAX_NAME_LENGTH];
	IN char		szAppPath[MAX_NAME_LENGTH];
	IN char		szAppDescription[MAX_NAME_LENGTH];
}USER_REGISTRATION, *PUSER_REGISTRATION;

#endif	#ifndef _DSIFSINTERFACE_H_