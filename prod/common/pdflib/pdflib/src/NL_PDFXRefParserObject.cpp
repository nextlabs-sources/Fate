#include "stdafx.h"
#include "NL_PDFXRefParserObject.h"
#include "nl_utils.h"
#include "NL_PDFParser.h"

#define ENTRY_LEN		20

using namespace nextlabs;


CPDFXRefParserObject::CPDFXRefParserObject(void)
{
}

CPDFXRefParserObject::~CPDFXRefParserObject(void)
{
}

/***************************************************
Format
0 1
0000000000 65535 f
13 1
0000579912 00000 n
***************************************************/
bool CPDFXRefParserObject::ParseEntries(const string& strXRefData, vector<XRefEntry>& vEntries, const int& nObjectNum)
{
	string strTemp = strXRefData;

	const int nEntrySize = 20;//the length of every entry line is always 20

	string::size_type nIndex = 0;
	while(!strTemp.empty())
	{
		CUtils::RemoveInvalidCharAtFront(strTemp);		


		string::size_type nSpaceIndex = strTemp.find(" ");//the fist space
		if(nSpaceIndex == string::npos)
		{
			printf("wrong xref table, maybe this file was damaged!\r\n");
			break;//error happens
		}
		
		int nFirstNum = atoi(strTemp.substr(0, nSpaceIndex).c_str());

		string::size_type i1 = strTemp.find("\n", nSpaceIndex + 1);
		string::size_type i2 = strTemp.find(" ", nSpaceIndex + 1);
		string::size_type i3 = strTemp.find("\r", nSpaceIndex + 1);
		nIndex = i1 < i2? i1: i2;//sometime, there aren't \r\n after "header", it can use blank space. crazy.
		nIndex = nIndex < i3? nIndex: i3;

		string strHeader = strTemp.substr(0, nIndex);

		CUtils::RemoveCharAtFrontAndBack(strHeader, '\r');

		int nObjCount = atoi(strHeader.substr(nSpaceIndex + 1, strHeader.length() - nSpaceIndex - 1).c_str());

		strTemp = strTemp.substr(nIndex + 1, strTemp.length() - nIndex - 1);

		//try to find the start position of entries
		CUtils::RemoveInvalidCharAtFront(strTemp);		


		DWORD dwStart = GetTickCount();
		printf("Start to parse %d entries\r\n", nObjCount);

		int i = 0;
		for(; i < nObjCount; i++)//try to get all the objects
		{
			if(strTemp.length() < nEntrySize)
			{
				break;//errors, the object count in the "header" is bigger than xref table
			}

			string entry = strTemp.substr(0, nEntrySize);
			strTemp = strTemp.substr(nEntrySize, strTemp.length() - nEntrySize);

			//Parse entry
			XRefEntry xentry;
			xentry.nObjNum = nFirstNum + i;
			nSpaceIndex = entry.find(" ");
			if(nSpaceIndex != string::npos)
			{
				xentry.lOffset = atoi(entry.substr(0, nSpaceIndex).c_str());//offset
				entry = entry.substr(nSpaceIndex + 1, entry.length() - 1);
				nSpaceIndex = entry.find(" ");
				if(nSpaceIndex != string::npos)//try to get generation number
				{
					xentry.lGeneration = atoi(entry.substr(0, nSpaceIndex).c_str());

					entry = entry.substr(nSpaceIndex + 1, entry.length() - 1);

					if(!entry.empty())
					{
						xentry.cUsed = entry[0];
						vEntries.push_back(xentry);

						if(nObjectNum >= 0 && xentry.nObjNum == nObjectNum)
						{//found the specified object already, we can break hear, this can save a lot of time if there are lots of objects in xref table.
							strTemp.clear();
							break;
						}
					}
				}
			}
		}
		printf("Parsed %d entries, used: %d ms\r\n", i, GetTickCount() - dwStart);

	}

	return true;
}

int CPDFXRefParserObject::ComputeXRefLen(const int& nXRefPos, CPDFParser* pParser)
{
	if(!pParser)
	{
		return 0;
	}

	int nPos = nXRefPos;

	string strTemp = pParser->ReadRawData(nPos, 100);

#pragma warning(push)
#pragma warning(disable: 4127)
	while(1)
	{
		//try to locate the position of xref table content,
		int nCount = 0;
		string::size_type i = 0;
		for(; i< strTemp.length(); i++)
		{
			if(strTemp[i] >= '0' && strTemp[i] <= '9')
			{
				break;
			}

			nCount++;
		}
		if(nCount >= (int)strTemp.length() - 1)
			break;
		
		strTemp = strTemp.substr(nCount, strTemp.length() - nCount);//try to locate the first number position

		string::size_type nSpaceIndex = strTemp.find(" ");//the fist space
		if(nSpaceIndex == string::npos)
		{
			printf("wrong xref table header, maybe this file was damaged!\r\n");
			break;
		}

	//	int nFirstNum = atoi(strTemp.substr(0, nSpaceIndex).c_str());

		string::size_type i1 = strTemp.find("\n", nSpaceIndex + 1);
		string::size_type i2 = strTemp.find(" ", nSpaceIndex + 1);
		string::size_type i3 = strTemp.find("\r", nSpaceIndex + 1);
		string::size_type nIndex = i1 < i2? i1: i2;//sometime, there aren't \r\n after "header", it can use blank space. crazy.
		nIndex = nIndex < i3? nIndex: i3;

		string strHeader = strTemp.substr(0, nIndex);

		int nObjCount = atoi(strHeader.substr(nSpaceIndex + 1, strHeader.length() - nSpaceIndex - 1).c_str());

		//compute the characters between header and entries, for example, there are \r\n 
		int nCount1 = 0;
		for(i = strHeader.length(); i< strTemp.length(); i++)
		{
			if( (strTemp[i] >= '0' && strTemp[i] <= '9') || (strTemp[i] >= 'a' && strTemp[i] <= 'z') )
			{
				break;
			}

			nCount1++;
		}

		nPos += nCount + (int)strHeader.length() + nCount1 + nObjCount * ENTRY_LEN;

		strTemp = pParser->ReadRawData(nPos, 100);
		if(strTemp.length() < 1 || strTemp.find("trailer") == 0 || !(strTemp[0] >= '0' && strTemp[0] <= '9') )
			break;
	}
#pragma warning(pop)

	return nPos - nXRefPos;
}

bool CPDFXRefParserObject::ParseEntries(nextlabs::CPDFParser *pParser, const int& nXRef, std::vector<XRefEntry> &vEntries, const int &nObjectNum)
{
	if(!pParser)
		return false;

	int nPos = nXRef;

	string strTemp = pParser->ReadRawData(nPos, 100);

	bool bRet = false;

#pragma warning(push)
#pragma warning(disable: 4127)
	while(1)
	{
		//try to locate the position of xref table content,
		int nCount = 0;
		string::size_type i = 0;
		for(; i< strTemp.length(); i++)
		{
			if(strTemp[i] >= '0' && strTemp[i] <= '9')
			{
				break;
			}

			nCount++;
		}

		if(nCount >= (int)strTemp.length() - 1)
			break;


		strTemp = strTemp.substr(nCount, strTemp.length() - nCount);//try to locate the first number position

		string::size_type nSpaceIndex = strTemp.find(" ");//the fist space
		if(nSpaceIndex == string::npos)
		{
			printf("ParseEntries() wrong xref table header, maybe this file was damaged!\r\n");
			return 0;
		}

		int nFirstNum = atoi(strTemp.substr(0, nSpaceIndex).c_str());

		string::size_type i1 = strTemp.find("\n", nSpaceIndex + 1);
		string::size_type i2 = strTemp.find(" ", nSpaceIndex + 1);
		string::size_type i3 = strTemp.find("\r", nSpaceIndex + 1);
		string::size_type nIndex = i1 < i2? i1: i2;//sometime, there aren't \r\n after "header", it can use blank space. crazy.
		nIndex = nIndex < i3? nIndex: i3;

		string strHeader = strTemp.substr(0, nIndex);

		int nObjCount = atoi(strHeader.substr(nSpaceIndex + 1, strHeader.length() - nSpaceIndex - 1).c_str());

		//compute the characters between header and entries, for example, there are \r\n 
		int nCount1 = 0;
		for(i = strHeader.length(); i< strTemp.length(); i++)
		{
			if(strTemp[i] >= '0' && strTemp[i] <= '9')
			{
				break;
			}

			nCount1++;
		}

		if(nObjectNum >= nFirstNum && nObjectNum < nFirstNum + nObjCount)
		{
			int nEntryPos = nPos + nCount + (int) strHeader.length() + nCount1 + (nObjectNum - nFirstNum) * ENTRY_LEN;
			string entry = pParser->ReadRawData(nEntryPos, ENTRY_LEN);

			//Parse entry
			XRefEntry xentry;
			xentry.nObjNum = nObjectNum;
			nSpaceIndex = entry.find(" ");
			if(nSpaceIndex != string::npos)
			{
				xentry.lOffset = atoi(entry.substr(0, nSpaceIndex).c_str());//offset
				entry = entry.substr(nSpaceIndex + 1, entry.length() - 1);
				nSpaceIndex = entry.find(" ");
				if(nSpaceIndex != string::npos)//try to get generation number
				{
					xentry.lGeneration = atoi(entry.substr(0, nSpaceIndex).c_str());

					entry = entry.substr(nSpaceIndex + 1, entry.length() - 1);

					if(!entry.empty())
					{
						xentry.cUsed = entry[0];
						vEntries.push_back(xentry);
						bRet = true;
					}
				}
			}

			break;
		}

		nPos += nCount + (int)strHeader.length() + nCount1 + nObjCount * ENTRY_LEN;

		strTemp = pParser->ReadRawData(nPos, 100);

		if(strTemp.length() < 1 || strTemp.find("trailer") == 0 || !(strTemp[0] >= '0' && strTemp[0] <= '9') )
			break;
	}
#pragma warning(pop)

	return bRet;
}