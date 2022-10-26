#include "chromecontextt.h"

namespace nextlabs
{

/************************************************************************/
/* CChromeContext                                                */
/************************************************************************/

void CChromeContext::OnResponseInitHookTable(bool(&table)[kHM_MaxItems])
{
	table[kHMDeleteFileW] = true;
	table[kHMCreateFileMappintW] = true;
	table[kHMFindFirstFileExW] = true;
	table[kHMMoveFileW] = true;
	table[kHMMoveFileExW] = true;
	table[kHMMoveFileWithProgressW] = true;
	table[kHMKernelBaseMoveFileExW] = true;
	table[kHMKernelBaseMoveFileWithProgressW] = true;
	table[kHMCopyFileW] = true;  
	table[kHMCopyFileExW] = true; 
	table[kHMPrivCopyFileExW] = true; 
	table[kHMKernelBasePrivCopyFileExW] = true;
	table[kHMMoveFileW] = true;
	table[kHMMoveFileExW] = true;
	table[kHMMoveFileWithProgressW] = true;
	table[kHMSetFileAttributesW] = true;
	table[kHMGetFileAttributesW] = true;
	table[kHMDeviceIoControl] = true;
	table[kHMEncryptFileW] = true;
	table[kHMCreateFileW] = true;
	table[kHMCreateDirectoryW] = true;
	table[kHMCloseHandle] = true;
	table[kHMDecryptFileW] = true;
	table[kHMCreateProcessW] = true;
	table[kHMAddUsersToEncryptedFile] = true;
	table[kHMSetNamedSecurityInfoW] = true;

	table[kHMWriteFile] = true;
	table[kHMWriteFileEx] = true;

	table[kHMSetClipboardData] = true;
	table[kHMGetClipboardData] = true;
	table[kHMOleSetClipboard] = true;
	table[kHMOleGetClipboard] = true;

	table[kHMDoDragDrop] = true;
	table[kHMRegisterDragDrop] = true;
	table[kHMRevokeDragDrop] = true;

	table[kHMSetFileInformationByHandle] = true;

	table[kHMCoCreateInstance] = true;
	table[kHMCOMCopyItems] = true;

	table[kHMCOMShow] = true;

	table[kHMInternetConnectW] = true;
	table[kHMInternetCloseHandle] = true;
	table[kHMHttpOpenRequestW] = true;

	table[kHMGetSaveFileNameW] = true;

	table[kHMBitBlt] = true;
	table[kHMMaskBlt] = true;
	table[kHMPlgBlt] = true;
	table[kHMStretchBlt] = true;
	table[kHMPrintWindow] = true;
	table[kHMCreateDCA] = true;
	table[kHMCreateDCW] = true;
	table[kHMDeleteDC] = true;
	table[kHMGetDC] = true;
	table[kHMGetDCEx] = true;
	table[kHMGetWindowDC] = true;
	table[kHMReleaseDC] = true;

	table[kHMNtSetSecurityObject] = true;
}

}  // ns nextlabs
