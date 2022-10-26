/****************************************************************************************
 *
 * NextLabs Uninstall Cleanup Utility Tool
 *
 * Cleanup Base Class
 * January 2012 
 ***************************************************************************************/

#include "stdafx.h"
#include "nl_baseCleanup.h"

// virtual functions
int CleanupBase::createItems(itemList& items)
{
	return 0;
};

int CleanupBase::deleteItems(itemList& items)
{
	return 0;
};

int CleanupBase::createItem(wstring str)
{
	return 0;
};

int CleanupBase::deleteItem(wstring str)
{
	return 0;
};
void CleanupBase::getLastErrorStr(DWORD lastErr, WCHAR *errStr)
{
  //DWORD lastErr = GetLastError();
  //WCHAR errStr[1024];

  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                NULL, lastErr, 0, errStr, 1024, NULL);
} /* printLastError */

void CleanupBase::fprintfLastError(DWORD lastErr)
{
  //DWORD lastErr = GetLastError();
  WCHAR errStr[1024];

  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                NULL, lastErr, 0, errStr, 1024, NULL);
  fprintf(m_File, "%ws\n", errStr);
} /* printLastError */
