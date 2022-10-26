#pragma once

class CRegUtility
{
public:
	CRegUtility(void);
	~CRegUtility(void);

	static BOOL OpenEDPMKey(HKEY* pKey);

	static BOOL CloseEDPMKey(HKEY* pKey);
};
