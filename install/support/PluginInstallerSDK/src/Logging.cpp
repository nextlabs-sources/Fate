#include "stdafx.h"
#include "PluginInstallerSDK.h"
#include <msiquery.h>

void writeToInstallerLog(MSIHANDLE hInstall, wchar_t* message)
{
	PMSIHANDLE messageRecord;
	wchar_t* fullMessage;
	wchar_t* formattedMessage;
	DWORD formattedMessageLength;

	fullMessage = new wchar_t[LOGGING_PREFIX_MESSAGE_SIZE + wcslen(message) + 1];
	wcscpy_s(fullMessage, LOGGING_PREFIX_MESSAGE_SIZE + wcslen(message) + 1, LOGGING_PREFIX_MESSAGE);
	wcscat_s(fullMessage, LOGGING_PREFIX_MESSAGE_SIZE + wcslen(message) + 1, message);

	messageRecord = MsiCreateRecord(1);                  
	if (!messageRecord)
	{
		// What to do here?  Can't log it?
		return;
	}

	MsiRecordSetString(messageRecord, 1, fullMessage);        
	MsiRecordSetString(messageRecord, 0, L"[1]");

	formattedMessageLength = 0;
	MsiFormatRecordW(hInstall, messageRecord, NULL, &formattedMessageLength); 
	formattedMessage = new wchar_t[formattedMessageLength + 1];
	MsiFormatRecordW(hInstall, messageRecord, formattedMessage, &formattedMessageLength); 
	MsiProcessMessage(hInstall,INSTALLMESSAGE_INFO, messageRecord);

	MsiCloseHandle(messageRecord);
	delete []fullMessage;
	delete []formattedMessage;
}