#pragma once


class CLibDetach
{
public:

	//	used need to set true to this flag in dllmain de_attach event.
	//	if flag is true, means the lib is going to be de_attached.
	void SetDetachFlag(BOOL bFlag)
	{
		m_bDetach =	bFlag ;
	}

	BOOL GetDetachFlag()
	{
		return	  m_bDetach ;
	}

public:
	CLibDetach()
	{
		m_bDetach = FALSE;
	}

private:

	BOOL m_bDetach;
};
