#pragma once

#define FTPEDLL __declspec( dllexport )

extern "C"
{
	/****************************************************************************
	* Plug-in Entry Points
	***************************************************************************/
	int FTPEDLL WINAPI	 InstallSPI() ;
	int FTPEDLL WINAPI	 UninstallSPI()	 ;
}
