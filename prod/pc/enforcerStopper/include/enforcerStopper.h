#ifndef ENFORCERSTOPPER
#define ENFORCERSTOPPER

#include "Resource.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL             InitInstance(HINSTANCE hInstance, int nCmdShow);
BOOL             GetPasswordFromCmdLine (LPWSTR pszCmdLine, LPWSTR pszPassword);


CEResult_t __stdcall stopAgentService (TCHAR * lpszPassword);
CEResult_t __stdcall stopAgentServiceWithoutPassword (TCHAR * lpszPassword);


#endif /* ENFORCERSTOPPER */
