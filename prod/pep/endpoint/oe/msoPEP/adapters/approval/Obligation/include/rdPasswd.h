#ifndef __RD_PASSWD_H__
#define __RD_PASSWD_H__

#include <string>
class CPasswdGenerator
{
public:
	CPasswdGenerator(){srand(::GetTickCount());};
	std::wstring Generator(int iLen=8,int iFlag=62)
	{
		std::wstring str;
		
		for(int i = 0;i<iLen;i++)
		{
			str += GetChar(iFlag);
		}
		return str;
	};

private:
	std::wstring GetChar(int Flag=62)
	{
		std::wstring s;
		std::wstring m_sArray = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
		std::wstring m_sArray1 = L"1234567890";

		switch (Flag)
		{
		case 10:
			{
				int k = rand();
				k = k % 10;
				s = m_sArray1.substr(k,1);
			}
			break;
		case 26:
			{
				int k = rand();
				k = k % 26;
				s = m_sArray.substr(k,1);
			}
			break;
		case 52:
			{
				int k = rand();
				k = k % 52;
				s = m_sArray.substr(k,1);
			}
			break;
		case 62:
			{
				int k = rand();
				k = k % 62;
				s = m_sArray.substr(k,1);
			}
			break;
		default:
			{
				int k = rand();
				k = k % 26;
				s = m_sArray.substr(k,1);
			}
			break;
		}
			return s;
	}

};

#endif //__RD_PASSWD_H__