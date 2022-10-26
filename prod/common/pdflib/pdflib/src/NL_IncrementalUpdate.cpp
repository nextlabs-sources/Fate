#include "stdafx.h"
#include "NL_IncrementalUpdate.h"
#include "NL_Utils.h"
#include <io.h>

#define TRAILER_ID		"/ID"
#define TRAILER_INFO	"/Info"
#define TRAILER_SIZE	"/Size"
#define TRAILER_ROOT	"/Root"
#define TRAILER_PREV	"/Prev"


#define XREFSTREAM_HEADER		"/Type /XRef\r\n/Index[%d %d]\r\n/W[1 4 1]\r\n/Filter /FlateDecode\r\n/Size %d\r\n/Prev %d\r\n/Root %s\r\n/Info %d 0 R\r\n/ID %s\r\n/Length %d \r\n"

#define PDF_ROOT_PEICEINFO1	"/PIECEINFO"
#define PDF_ROOT_PEICEINFO2	"/PieceInfo"

#define PDF_APPLICATION_KEY	"/Nextlabs"
#define PDF_APPLICATION_KEY_LASTMODIFIED	"/LastModified"
#define PDF_APPLICATION_KEY_PRIVATE			"/Private"

const bool METAOBJECT  = false;
#define TONNY_DEBUG    1

//Convert to the position format, like: 789 -> 0000000789
static string GeneratePosFormat(int nPos)
{
	char buf[1024] = {0};
	_snprintf_s(buf, 1024, _TRUNCATE, "%d", nPos);
	string strPos(buf);
	while(strPos.length() < 10)
	{
		strPos = "0" + strPos;
	}

	return strPos;
}

CIncrementalUpdate::CIncrementalUpdate()
{
	m_pLastTrailer = new PDFTRAILER;
	m_pFileName = NULL;
	m_nFileSize = 0;

	m_strMetaObj.clear();

}

CIncrementalUpdate::~CIncrementalUpdate(void)
{
	if(m_pLastTrailer)
	{
		delete m_pLastTrailer;
		m_pLastTrailer = NULL;
	}
}

string CIncrementalUpdate::CreateIncrementalUpdate()
{
	//Read the trailer of the current PDF file
	string strIU;
	CPDFParser* pParser = new CPDFParser(m_pFileName);
    bool bNeedRemoveSummeryTag = false;
	if(pParser && pParser->Init())
	{
		m_nLastXRefPos = pParser->GetLastXRefPos();

		m_nFileSize = pParser->GetCurFileSize();

		LPPDFTRAILER pTrailer =pParser->GetLastTrailer();
		m_pLastTrailer->CopyTrailer(pTrailer);

        if (m_pMapTags != NULL && !m_pMapTags->empty())  bNeedRemoveSummeryTag = true;

		bool bIsReadTagCall = true;
		string strMetaDataObjContent = "";
		bool  bFindMetaObjContent = false;
		if(GetStorePosition() == PIECEINFO)
		{
#if TONNY_DEBUG
            OutputDebugStringW(L"================================> most of time, i don't assume that PDF tag code will enter  79 line of NL_IncrementalUpdate.cpp....\n");
#endif
			m_strExistingData = pParser->GetPieceInfoObj();
			m_strRootData = pParser->GetRootObj(m_nLastXRefPos);
            if (bNeedRemoveSummeryTag)
            {
                string strData = "";
                bool bLastIU;
                pParser->GetOurTagObj(strData, bLastIU, bIsReadTagCall, strMetaDataObjContent, bFindMetaObjContent);
            }            
		}
		else
			m_strOurTags = pParser->GetOurTagObj(m_strExistingData, m_bLastIUIsUs,bIsReadTagCall,strMetaDataObjContent,bFindMetaObjContent);

#if 1   //add by Tonny at 4/29/2016
        if (bNeedRemoveSummeryTag && !strMetaDataObjContent.empty())
        {
            map<string, string> mapSummeryTags;
            string strVersion = pParser->GetPdfVersion();
            mapSummeryTags["PDFVersion"] = strVersion;
            if (bFindMetaObjContent)
            {
                CUtils::AnalyzeMetaDataInfo(strMetaDataObjContent, mapSummeryTags);
            }

#if TONNY_DEBUG
            char g_szLog[1025] = { 0 };
#endif
            // remove summery tag from m_pMapTags
            std::map<string, string>::iterator it = mapSummeryTags.begin();
            for (; it != mapSummeryTags.end(); it++)
            {
                const string& strName = it->first;
                const string& strValue = it->second;
                std::map<string, string>::iterator init = m_pMapTags->begin();
                for (; init != m_pMapTags->end(); init++)
                {
                    const string& strOName = init->first;
                    const string& strOValue = init->second;
                    if (_stricmp(strName.c_str(), strOName.c_str()) == 0 &&
                        _stricmp(strValue.c_str(), strOValue.c_str()) == 0)
                    {
#if TONNY_DEBUG
                        sprintf_s(g_szLog, 1024,"--------> Remove PDF summery tag, [%s]=[%s].\n", strOName.c_str(), strOValue.c_str());
                        OutputDebugStringA(g_szLog);
#endif
                        m_pMapTags->erase(init);
                        break;
                    }
                }
            }
        }
#endif

		//try to get the file size before our IU
		if(m_bLastIUIsUs)
		{//the last UD belongs to us, we need to overwrite the IU. so that we can reduce the file size.
			string::size_type nStart = m_strOurTags.find(0x02);
			string::size_type nEnd = string::npos;
			if(nStart != string::npos)
			{
				nEnd = m_strOurTags.find(0x02, nStart + 1);
			}

			if(nEnd != string::npos && nStart != string::npos && nEnd > nStart)
			{
				m_nFileSize = atoi(m_strOurTags.substr(nStart + 1, nEnd - nStart - 1).c_str());//use the file size before our last IU.
			}
		}

		//try to get the comment tags.
		int nLen = 0;
		pParser->GetCommentTags(m_mapCommentTags, nLen);
//		if(!m_mapCommentTags.empty())
//		{//it means the current PDF file has the "comment tags", we need to overwrite the "comment tags" with new IU
//			m_nFileSize = nLen;
//		}

		//Generate the IU
		if(pParser->IsLastXRefStream())
		{
			strIU = CreateNewStyleIncrementalUpdate();
		}
		else
			strIU =  CreateOldStyleIncrementalUpdate();

		
		delete pParser;
		pParser = NULL;
	}

	return strIU;
}

string CIncrementalUpdate::CreateOldStyleIncrementalUpdate()
{
	string strNewObj = CreateNewObject(false);

	if(strNewObj.empty())
	{
		return "";
	}

	string strTrailer = CreateTrailer();
	string strXRefTable = CreateXRefTable();
	

	string strIncrementalUpdate;


	strIncrementalUpdate.append(strNewObj);//new object

	strIncrementalUpdate.append(strXRefTable);//xref table
	strIncrementalUpdate.append(strTrailer);//trailer
	strIncrementalUpdate.append("startxref\r\n");
	char buf[100] = {0};
	_snprintf_s(buf, 100, _TRUNCATE, "%d\r\n%%%%EOF\r\n", m_nFileSize + strNewObj.length() + m_strMetaObj.length());
	strIncrementalUpdate.append(buf);

	return strIncrementalUpdate;
}

string CIncrementalUpdate::CreateNewStyleIncrementalUpdate()
{
	string strNewObj = CreateNewObject(true);

	if(strNewObj.empty())
	{
		return "";
	}

	string strXRefObjStream = CreateXRefStrem();

	string strIncrementalUpdate;


	strIncrementalUpdate.append(strNewObj);//new object

	strIncrementalUpdate.append(strXRefObjStream);//xref stream
	
	strIncrementalUpdate.append("startxref\r\n");
	char buf[100] = {0};
	_snprintf_s(buf, 100, _TRUNCATE, "%d\r\n%%%%EOF\r\n", m_nFileSize + strNewObj.length());
	strIncrementalUpdate.append(buf);

	return strIncrementalUpdate;
}

string CIncrementalUpdate::CreateXRefStrem()
{
	string strSize = m_pLastTrailer->strSize;
	string strRoot = m_pLastTrailer->strRoot;
	string strID = m_pLastTrailer->strID;
	string strInfo = m_pLastTrailer->strInfo;
	string strPrev = m_pLastTrailer->strPrev;

	CUtils::RemoveCharAtFrontAndBack(strSize, ' ');
	CUtils::RemoveCharAtFrontAndBack(strRoot, ' ');
	CUtils::RemoveCharAtFrontAndBack(strID, ' ');
	CUtils::RemoveCharAtFrontAndBack(strInfo, ' ');

	int nSize = atoi(strSize.c_str());

	if(m_bLastIUIsUs)
		nSize -= 2;
	//try to create new xref steam object


	//xref stream object body
	char body[6] = {0};
	body[0] = 1;
	int nInfoObjNum = (int)m_nFileSize;
	char pos[4] = {0};
	memcpy_s(pos, 4, &nInfoObjNum, 4);
	body[1] = pos[3];
	body[2] = pos[2];
	body[3] = pos[1];
	body[4] = pos[0];
	body[5] = 0;

	std::auto_ptr<PdfFilter> pFilter = PdfFilterFactory::Create( ePdfFilter_FlateDecode );

	char* pOut = NULL;
	pdf_long nOut = 0;
	pFilter->Encode(body, 6, &pOut, &nOut);

	if(!pOut)
		return "";

	//xref stream object header
	char header[1024] = {0};
	if(m_bLastIUIsUs)
	{
		_snprintf_s(header, 1024, _TRUNCATE, XREFSTREAM_HEADER, nSize, 1, nSize + 2, atoi(strPrev.c_str()), strRoot.c_str(), nSize, strID.c_str(), nOut);
	}
	else
		_snprintf_s(header, 1024, _TRUNCATE, XREFSTREAM_HEADER, nSize, 1, nSize + 2, m_nLastXRefPos, strRoot.c_str(), nSize, strID.c_str(), nOut);

	string strXRefObj;
	char buf[100] = {0};
	_snprintf_s(buf, 100, _TRUNCATE, "%d 0 obj\r\n", nSize + 1 );
	strXRefObj.append(buf);
	strXRefObj.append("<<");
	strXRefObj.append(header);
	strXRefObj.append(">>\r\nstream\r\n");
	strXRefObj.append(pOut, nOut);
	strXRefObj.append("\r\nendstream\r\nendobj\r\n");

	if(pOut)
	{
		free(pOut);
		pOut = NULL;
	}
	return strXRefObj;
}

bool CIncrementalUpdate::WriteIncrementalUpdate()
{
	string strIncrementalUpdate = CreateIncrementalUpdate();

	if(strIncrementalUpdate.empty())
	{
		printf("failed to create incremental update\r\n");
		return false;
	}

	strIncrementalUpdate += NEXTLABS_IU;

	FILE* pFile = NULL;
	errno_t err = fopen_s(&pFile, m_pFileName, "rb+");
	bool bRet = false;

	if(pFile && err == 0)
	{
		setvbuf(pFile, 0, _IONBF, 0);
		int rt = -1;
		bool bTruncate = false;
		if(m_bLastIUIsUs)
		{//we need to overwrite our Incremental Update
			rt = fseek(pFile, (long)m_nFileSize, SEEK_SET);
			bTruncate = true;
		}
		else
		{
			rt = fseek(pFile, 0, SEEK_END);
		}

		if( 0 == rt )
		{
			size_t nWrite = 0;
			const char* pBuf = strIncrementalUpdate.c_str();
			while(nWrite < strIncrementalUpdate.length())
			{
				size_t nRet = fwrite(pBuf + nWrite, sizeof(char), strIncrementalUpdate.length() - nWrite, pFile);
				nWrite += nRet;
			}
			bRet = true;
		}

		if(bTruncate)
		{
			long	lLength = ftell(pFile);
			int		nFile = _fileno(pFile);

			_chsize_s(nFile, lLength);
		}

		fclose(pFile);
	}
	else{
		printf("Failed to open file %s with \"ab+\", err: %d\r\n", m_pFileName, err) ;
	}

	return bRet;
}

string CIncrementalUpdate::CreateTrailer()
{
	string strTrailer;
	
	if(m_pLastTrailer)
	{
		if(m_pLastTrailer && m_pLastTrailer->strRoot.length() > 0 && m_pLastTrailer->strSize.length() > 0)
		{
			strTrailer.append("trailer\r\n<<\r\n");

			if(m_pLastTrailer->strID.length() > 0 )
			{
				strTrailer.append(TRAILER_ID);
				if(m_pLastTrailer->strID[0] != ' ')
					strTrailer.append(" ");

				strTrailer.append(m_pLastTrailer->strID);
			}

			char buf[512] = {0};

			if(GetStorePosition() == DID)
			{
				strTrailer.append(TRAILER_ROOT);
				if(m_pLastTrailer->strRoot[0] != ' ')
					strTrailer.append(" ");

				strTrailer.append(m_pLastTrailer->strRoot);
			}
			else if (GetStorePosition() == PIECEINFO)
			{
				strTrailer.append(TRAILER_ROOT);

				memset(buf, 0, sizeof(buf));
				_snprintf_s(buf, 512, _TRUNCATE, " %d 0 R\r\n", atoi(m_pLastTrailer->strSize.c_str()) + 2);

				strTrailer.append(buf);
			}


			strTrailer.append(TRAILER_SIZE);
			
			if(GetStorePosition() == DID)
			{
				if(m_bLastIUIsUs)
					_snprintf_s(buf, 512, _TRUNCATE, " %d\r\n", atoi(m_pLastTrailer->strSize.c_str()));
				else
					_snprintf_s(buf, 512, _TRUNCATE, " %d\r\n", atoi(m_pLastTrailer->strSize.c_str()) + 1);
			}				

			strTrailer.append(buf);

			//info
			if(GetStorePosition() == DID)
			{
				strTrailer.append(TRAILER_INFO);

				memset(buf, 0, sizeof(buf));
				if(m_bLastIUIsUs)
					_snprintf_s(buf, 512, _TRUNCATE, " %d 0 R\r\n", atoi(m_pLastTrailer->strSize.c_str()) - 1);
				else
					_snprintf_s(buf, 512, _TRUNCATE, " %d 0 R\r\n", atoi(m_pLastTrailer->strSize.c_str()));

				strTrailer.append(buf);
			}
			else if(GetStorePosition() == PIECEINFO)
			{
				if(!m_pLastTrailer->strInfo.empty())
				{
					strTrailer.append(TRAILER_INFO);
					strTrailer.append(" ");
					strTrailer.append(m_pLastTrailer->strInfo);
					strTrailer.append("\r\n");
				}
			}
			//Prev
			strTrailer.append(TRAILER_PREV);
			
			memset(buf, 0, sizeof(buf));

			if(m_bLastIUIsUs)
				_snprintf_s(buf, 512, _TRUNCATE, " %s", m_pLastTrailer->strPrev.c_str());
			else
				_snprintf_s(buf, 512, _TRUNCATE, " %d", m_nLastXRefPos);
			strTrailer.append(buf);

			strTrailer.append(">>\r\n");

		}

	}

	return strTrailer;
}

string CIncrementalUpdate::CreateXRefTable()
{
	string strXRefTable;
	strXRefTable.append("xref\r\n0 0\r\n");

	if(m_pLastTrailer)
	{
		int nNewObjNum = atoi(m_pLastTrailer->strSize.c_str());
		if(m_bLastIUIsUs)
			nNewObjNum--;

		int nNewObjPos = (int)(m_nFileSize + m_strMetaObj.length());
		string strPos = GeneratePosFormat(nNewObjPos);
		
		char buf[1024] = {0};
		
		if(GetStorePosition() == DID)
			_snprintf_s(buf, 1024, _TRUNCATE, "xref\r\n0 1\r\n0000000000 65535 f\r\n%d 1\r\n%s 00000 n\r\n", nNewObjNum, strPos.c_str());
		else if(GetStorePosition() == PIECEINFO)
		{
			string strPos2 = GeneratePosFormat(m_nPos2);
			string strPos3 = GeneratePosFormat(m_nPos3);
			_snprintf_s(buf, 1024, _TRUNCATE, "xref\r\n0 1\r\n0000000000 65535 f\r\n%d 1\r\n%s 00000 n\r\n%s 00000 n \r\n%s 00000 n\r\n", nNewObjNum, strPos.c_str(), strPos2.c_str(), strPos3.c_str());
		}

		strXRefTable = string(buf);
	}

	return strXRefTable;
}

bool CIncrementalUpdate::AddIncrementalUpdate(char* pszFileName, map<string,string>* pTags, bool bAdd)
{
	if(!pszFileName)
	{
		return false;
	}

	m_pFileName = pszFileName;
	m_pMapTags = pTags;
	m_bAdd = bAdd;

	return WriteIncrementalUpdate();
}

string CIncrementalUpdate::CreateNewObject(bool bXStem)
{
	if(!m_pLastTrailer)
	{
		return "";
	}

	string strObjContent = GenerateTagContent();
	if(strObjContent.empty())
	{
		return "";
	}

	string strObject;
	
	string strSize = m_pLastTrailer->strSize;
	CUtils::RemoveCharAtFrontAndBack(strSize, ' ');
	CUtils::RemoveCharAtFrontAndBack(strSize, '\r');
	CUtils::RemoveCharAtFrontAndBack(strSize, '\n');

	int nNewObjNum = atoi(strSize.c_str());

	if(m_bLastIUIsUs)
	{
		if(bXStem)
			nNewObjNum -= 2;
		else
			nNewObjNum -= 1;
	}

	char temp[1024] = {0};
	_snprintf_s(temp, 1024, _TRUNCATE, "%d 0 obj\r\n<<", nNewObjNum);
	strObject.append(temp);

	
	strObject.append(strObjContent);

	strObject.append(">>\r\nendobj\r\n");

	CUtils::RemoveCharAtFrontAndBack(strObject, ' ');

	m_nPos1 = (int)m_nFileSize;

	if (GetStorePosition() == PIECEINFO)
	{
		//Create the application data dictionary object
		char buf[1024] = {0};
		_snprintf_s(buf, 1024, _TRUNCATE, "%d 0 obj\r\n<<\r\n%s (D:20110303144537+08'00')\r\n%s %s\r\n>>endobj\r\n", nNewObjNum + 1, PDF_APPLICATION_KEY_LASTMODIFIED, PDF_APPLICATION_KEY_PRIVATE, "(abcdefg)");
		m_nPos2 = (int)(m_nFileSize + strObject.length());
		strObject.append(buf);

		//Create a new object for ROOT
		string strTemp = m_strRootData;
		transform(strTemp.begin(), strTemp.end(), strTemp.begin(), toupper);

		string::size_type nStart = strTemp.find(PDF_ROOT_PEICEINFO1);

		

		memset(buf, 0, sizeof(buf));
		_snprintf_s(buf, 100, _TRUNCATE, " %d 0 ", nNewObjNum);
		if(nStart != string::npos)
		{
			string::size_type nEnd = strTemp.find("R", nStart + strlen(PDF_ROOT_PEICEINFO1));
			if(nEnd != string::npos)
			{
				m_strRootData.replace(nStart + strlen(PDF_ROOT_PEICEINFO1), nEnd - nStart - strlen(PDF_ROOT_PEICEINFO1), buf);//Update the /PieceInfo, point to new PieceInfo object
			}
		}
		else//Add a pieceinfo entry in the ROOT object
		{
			memset(buf, 0, sizeof(buf));
			_snprintf_s(buf, 100, _TRUNCATE, "%s %d 0 R", PDF_ROOT_PEICEINFO2, nNewObjNum);
			m_strRootData.append(buf);
		}

		memset(buf, 0, sizeof(buf));
		_snprintf_s(buf, 100, _TRUNCATE, "%d 0 obj\r\n", nNewObjNum + 2);

		m_nPos3 = (int)(m_nFileSize + strObject.length());

		m_strRootData = string(buf) + string("<<") + m_strRootData + string(">>endobj\r\n");

		strObject.append(m_strRootData);

	}

	return strObject;

}

STORE_POSITION CIncrementalUpdate::GetStorePosition()
{
//	return PIECEINFO;
	return DID;
}

/*************************************************
/CreationDate (D:20110301183602+08'00')
/Creator (Acrobat 4.0 Import Plug-in for Windows )
/ModDate (D:20110301183639+08'00')
/Producer (Acrobat 4.0 Import Plug-in for Windows )
**************************************************/
bool CIncrementalUpdate::NeedKeep(const string& strTagName)
{
	if(_stricmp(strTagName.c_str(), "CreationDate") == 0 ||
		_stricmp(strTagName.c_str(), "Creator") == 0 ||
		_stricmp(strTagName.c_str(), "ModDate") == 0 ||
		_stricmp(strTagName.c_str(), "Producer") == 0 ||
		_stricmp(strTagName.c_str(), "Author") == 0 ||
		_stricmp(strTagName.c_str(), "Title") == 0 ||
		_stricmp(strTagName.c_str(), "Keywords") == 0 ||
		_stricmp(strTagName.c_str(), "Subject") == 0 
		)
	{
		return true;
	}
	
	return false;
}

string CIncrementalUpdate::GenerateTagContent()
{
	if(!m_pMapTags)
		return "";

	map<string, string> mapExistingTags;
	map<string, string> mapOurTags;
	CUtils::SplitTags(m_strExistingData, m_strOurTags, mapExistingTags, mapOurTags);

	//// Bug 34135 - ALPO: Unable to tag for some PDF files on email
	//// PDF support to add tags into Info entry in the file trailer dictionary by incremental updating.Unfortunately,
	//// here if there is no Info entry in some PDF file, while it won't append a new Info entry to store new tags.
	//if(m_strExistingData.empty())
	//{//can't get the DID, don't add new tags
	//	return "";
	//}

	string strTags;
	map<string, string>::iterator itr;

	if(mapOurTags.empty())//don't have our tags, we need to handle the "compatibility problem"
	{
		//Read the tags in DID
		for( itr = mapExistingTags.begin(); itr != mapExistingTags.end(); itr++)
		{
				mapOurTags[(*itr).first] = (*itr).second;
			}

		//read the "comment tags"
		for( itr = m_mapCommentTags.begin(); itr != m_mapCommentTags.end(); itr++)
		{
			mapOurTags[(*itr).first] = (*itr).second;
		}
	}
	else
	{
		for( itr = mapExistingTags.begin(); itr != mapExistingTags.end(); itr++)
		{
			mapOurTags[(*itr).first] = (*itr).second;
		}
	}
	
	if(m_bAdd)
	{
		for( itr = m_pMapTags->begin(); itr != m_pMapTags->end(); itr++)
		{
			mapOurTags[(*itr).first] = (*itr).second;
		}
	}
	else
	{
		for( itr = m_pMapTags->begin(); itr != m_pMapTags->end(); itr++)
		{
			map<string, string>::iterator itr2 = mapOurTags.find((*itr).first);
			if(itr2 != mapOurTags.end())
			{
				mapOurTags.erase(itr2);
			}
		}
	}

	//Generate the new string
	for(itr = mapOurTags.begin(); itr != mapOurTags.end(); itr++)
	{
		strTags.append("\r\n/");
		strTags.append((*itr).first);
		strTags.append(" (");
		strTags.append((*itr).second);
		strTags.append(")");
	}


	string strText;
	strText.append(strTags);
	strText.append("\r\n");

	return strText;
}