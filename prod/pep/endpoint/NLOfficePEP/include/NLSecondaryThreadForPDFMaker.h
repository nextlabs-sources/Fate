#pragma once

/*
	this class is used to deny office file convert to PDF file by adobe PDFMaker.
	The word flow about this action is:
	1. user use PDFMaker to convert office file to PDF
	2. NLOffice PEP hook CreateWindowW to get and do this action
	3. set a flag to deny this action and this secondary thread will be activate
	4. this thread will find the windows that pop up by adobe PDFMaker add-in
	5. after success to get the window we will close it and end to deny this case
*/

#define NLSECONDARYTHREADFORPDFMAKERINSTANCE (CNLSecondaryThreadForPDFMaker::NLGetInstance())

class CNLSecondaryThreadForPDFMaker
{
private:
	CNLSecondaryThreadForPDFMaker(void);
	~CNLSecondaryThreadForPDFMaker(void);

public:
	static CNLSecondaryThreadForPDFMaker& NLGetInstance();

public:
	bool NLStartPDFMakerSecondaryThread();
	void NLExitPDFMakerSecondaryThread();

	void NLStartDenyPDFMakerConvert();
	
	bool NLGetThreadInitializeFlag();

private:
	void NLSetThreadInitializeFlag( _In_ BOOL bInitializeFlag );

	void NLSetThreadExitFlag( _In_ BOOL bNeedExitThread );
	bool NLGetThreadExitFlag();

	void NLActivateThreadEvent( _In_ const bool bActivateEvent );
	
private:
	// Secondary thread
	static unsigned __stdcall NLFindPDFMakerConvertWindow( void* pArguments );
	
	// Call back function to find the window that pop up by PDFMaker 
	static BOOL CALLBACK EnumTopProc( HWND hwnd,LPARAM lParam );

private:
	BOOL   m_bInitializeFlag;
	BOOL   m_bNeedExitThread;
	HANDLE m_hThread;
	HANDLE m_hPDFMakerConvertEvent;
};
