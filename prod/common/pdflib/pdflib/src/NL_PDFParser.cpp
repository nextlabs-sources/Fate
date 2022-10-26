#include "stdafx.h"
#include "NL_PdfParser.h"
#include "NL_Utils.h"
#include "NL_PDFXRefStreamParserObject.h"
#include "NL_base64.h"

#define PDF_STARTXREF		"startxref"
#define PDF_XREF			"xref"
#define PDF_TRAILER			"trailer"
#define PDF_ENDOBJ          "endobj"

#define PDF_TRAILER_INFO	"/INFO"
#define PDF_TRAILER_ID		"/ID"
#define PDF_TRAILER_SIZE	"/SIZE"
#define PDF_TRAILER_PREV	"/PREV"
#define PDF_TRAILER_ROOT	"/ROOT"
#define PDF_TRAILER_ENCRYPT	"/ENCRYPT"

#define PDF_ROOT_METADATA	"/METADATA"
#define PDF_ROOT_PEICEINFO	"/PIECEINFO"

#define PDF_ENDER			"%%EOF"
#define PDF_COMMENT_TAG		"%%NLTAG"
#define PDF_COMMENT_TAG_VER  "%%NLTAGVER"

using namespace nextlabs;


CPDFParser::CPDFParser(const char* pszFileName): PdfParser( PdfDocument::GetObjects())
{
	PdfDocument::Clear();

	m_pVecObjects = PdfDocument::GetObjects();
	

	m_pFileName = (char*)pszFileName;

	m_device = PdfRefCountedInputDevice(m_pFileName, "rb" );

	m_pLastTrailer = new PDFTRAILER;

	m_bLastXrefStream = false;


	m_pEncryptObj = NULL;
	m_nPrevInfoNum = 0;
}

CPDFParser::~CPDFParser()
{
	if(m_pLastTrailer)
	{
		delete m_pLastTrailer;
		m_pLastTrailer = NULL;
	}

	if(m_pEncryptObj)
	{
		delete m_pEncryptObj;
		m_pEncryptObj = NULL;
	}
}


bool CPDFParser::ReadXRefStreamEntries(int nPos, TVecOffsets& vOffsets)
{
	m_device.Device()->Seek( nPos );

	CPDFXRefStreamParserObject xrefObject( m_pVecObjects, m_device, m_buffer, &vOffsets );
	xrefObject.Parse();

	std::vector<pdf_int64> vIndex;

	xrefObject.ReadXRefTable(vIndex);

	//try to set the object number
	
	if(vIndex.size() % 2 == 0 )//the numbers in /Index is always the times of 2
	{
		for(string::size_type i = 0; i < vIndex.size(); )
		{
			if( vIndex[i] + vIndex[i + 1] <= (pdf_int64)vOffsets.size())
			{
				for(int j = 0; j < vIndex[i + 1]; j++)
				{
					XRefEntry entry;
					int nIndex = (int)vIndex[i] + j;
					entry.nObjNum = nIndex;
					entry.lOffset = vOffsets[nIndex].lOffset;
					entry.lGeneration = vOffsets[nIndex].lGeneration;
					entry.cUsed = vOffsets[nIndex].cUsed;

					m_vOffsets.push_back(entry);
				}

			}
			i+= 2;
		}

	}

	return true;
}

bool CPDFParser::ReadXRefEntries(int nPos, const int& nObjectNum)
{
/*	string strData;
	int nIndex = 0;

	const int nReadSize = 512;//read 512 bytes every time

#pragma warning(push)
#pragma warning(disable: 4127)
	while(true)
	{
		string strTemp = ReadRawData(nPos + nIndex * nReadSize, nReadSize);//try to read 512 bytes from nXref

		strData.append(strTemp);

		nIndex++;

		if(strData.find(PDF_TRAILER) != string::npos || strTemp.length() < nReadSize)
			break;			
	}
#pragma warning(pop)

	string::size_type nEnd = strData.find(PDF_TRAILER);
	if( nEnd != string::npos)
	{
		strData = strData.substr(strlen(PDF_XREF), nEnd - strlen(PDF_XREF));

		CPDFXRefParserObject obj;
		bool bRet = obj.ParseEntries(strData, m_vOffsets, nObjectNum);
		return bRet;
	}
*/
	CPDFXRefParserObject obj;
	return obj.ParseEntries(this, nPos, m_vOffsets, nObjectNum);
}
bool CPDFParser::GetObjOffsets(int nXRefPos, TRAILERTYPE Type)
{
	PDFTRAILER trailer;
	int nObjNumber = 0;
	if(	ReadTrailer(&trailer, nXRefPos) )
	{
		nObjNumber = atoi(trailer.strSize.c_str());
	}

	TVecOffsets offsets;
	offsets.resize(nObjNumber);


	bool bRet = false;
	m_vOffsets.clear();

//	DWORD dwStart = GetTickCount();
	if(IsXRefStream(nXRefPos))
	{
		bRet = ReadXRefStreamEntries(nXRefPos, offsets);
	}
	else
	{
		static int nPrevInfoNum = 0;
		int nInfoNum = 0;
		switch(Type)
		{
		case INFO:
			{
				nInfoNum = atoi(trailer.strInfo.c_str());
			}
			break;
		case ROOT:
			{
				nInfoNum = atoi(trailer.strRoot.c_str());
			}
			break;
		default:
			break;
		}
		
		if(nInfoNum > 0)
		{
			nPrevInfoNum = nInfoNum;
		}else
		{
			nInfoNum = nPrevInfoNum;
		}

		bRet = ReadXRefEntries(nXRefPos, nInfoNum);
	}

//	printf("read xref entries used: %d ms\r\n", GetTickCount() - dwStart);
	return bRet;
}

bool CPDFParser::IsEncrypted(int nXRefPos)
{
	PDFTRAILER trailer;
	
	if(	ReadTrailer(&trailer, nXRefPos) )
	{
		if(!trailer.strEncrypt.empty())
			return true;
	}

	return false;
}

bool CPDFParser::IsXRefStream(int nXRefPos)
{
	char buf[10] = {0};
	memset(buf, 0, sizeof(buf));
	m_device.Device()->Seek(nXRefPos);
	m_device.Device()->Read(buf, 4);

	return !(_stricmp(buf, "xref") == 0);	
}

bool CPDFParser::Init()
{
	if(m_pVecObjects && m_pFileName )
	{
		if(!IsPdfFile())
		{
			return false;
		}

		//get file size
		m_device.Device()->Seek(0, std::ios_base::end);
		m_nCurFileSize = static_cast<int> (m_device.Device()->Tell());

		//Try to get the last XREF table position.
		int nPos = FindLastToken(PDF_STARTXREF);
		if( nPos > 0)
		{
			m_device.Device()->Seek(nPos, std::ios_base::beg);
		}
		else
		{
			printf("Can't find %s\r\n", PDF_STARTXREF);
			return false;
		}

		
		pdf_long lOffset = m_device.Device()->Tell();

		char buf[200] = {0};
		m_device.Device()->Seek( (int)(lOffset + strlen(PDF_STARTXREF)) );
		m_device.Device()->Read(buf, 200);
		
		string strTemp(buf);
		CUtils::RemoveInvalidCharAtFront(strTemp);
		m_nLastXRefPos = ::atoi(strTemp.c_str());


		//Read the last xref obj, check if it is old style(PDF1.4) or xref stream object (PDF 1.5 and later)
		m_bLastXrefStream = IsXRefStream(m_nLastXRefPos);
		

		return !IsEncrypted(m_nLastXRefPos);//don't support encryption for this version
	}

	return false;
}

const PdfString CPDFParser::GetDocumentID()
{
	const PdfObject* pTrailer = GetPDFTrailer();

	if(pTrailer)
	{
		if( !pTrailer->GetDictionary().HasKey( PdfName("ID") ) )
		{
			PODOFO_RAISE_ERROR_INFO( ePdfError_InvalidEncryptionDict, "No document ID found in trailer.");
		}

		return pTrailer->GetDictionary().GetKey( PdfName("ID") )->GetArray()[0].GetString();
	}

	return PdfString("");
}

//don't support encryption for this version, this function was reserved for future
bool CPDFParser::CreateEncryptObj()
{
	return true;
}
/*
function: Try to get the member "TVecOffsets   m_offsets;" of PdfParser
Remarks:
"TVecOffsets   m_offsets;" is a private data member of PdfParser, so we 
can't access this member directly. Here, we count the offset of this member
in the memory, and then copy the value of this address.
*/
bool CPDFParser::GetOffsetsMember(char** pOffsets)
{
	if(!pOffsets)
	{
		return false;
	}

	//      base class PdfTokenizer
	// 		EPdfVersion   m_ePdfVersion;
	// 
	// 		bool          m_bLoadOnDemand;
	// 
	// 		pdf_long      m_nXRefOffset;
	// 		long          m_nFirstObject;
	// 		long          m_nNumObjects;
	// 		pdf_long      m_nXRefLinearizedOffset;
	// 		size_t        m_nFileSize;
	// 
	// 		TVecOffsets   m_offsets;

	int nOffset = sizeof(PdfTokenizer);
	int ret = nOffset % sizeof(EPdfVersion);
	if(ret != 0)
		nOffset += sizeof(EPdfVersion) - ret;// the start address of m_ePdfVersion

	nOffset += sizeof(EPdfVersion);

	ret = nOffset % sizeof(bool);
	if(ret != 0)
		nOffset += sizeof(bool) - ret;// the start address of m_bLoadOnDemand

	nOffset += sizeof(bool);

	ret = nOffset % sizeof(pdf_long);
	if(ret != 0)
		nOffset += sizeof(pdf_long) - ret;// the start address of m_nXRefOffset

	nOffset += sizeof(pdf_long);

	ret = nOffset % sizeof(long);
	if(ret != 0)
		nOffset += sizeof(long) - ret;// the start address of m_nFirstObject

	nOffset += sizeof(long);

	ret = nOffset % sizeof(long);
	if(ret != 0)
		nOffset += sizeof(long) - ret;// the start address of m_nNumObjects;

	nOffset += sizeof(long);

	ret = nOffset % sizeof(pdf_long);
	if(ret != 0)
		nOffset += sizeof(pdf_long) - ret;// the start address of m_nXRefLinearizedOffset

	nOffset += sizeof(pdf_long);

	ret = nOffset % sizeof(size_t);
	if(ret != 0)
		nOffset += sizeof(size_t) - ret;// the start address of m_nFileSize;

	nOffset += sizeof(size_t);


//	memcpy(pOffsets,(char*)this + nOffset, sizeof(TVecOffsets));
	*pOffsets = (char*)this + nOffset;

	return true;
}

bool CPDFParser::GetFirstObjectMember(long* pFirstObj)
{
	if(!pFirstObj)
	{
		return false;
	}

	//      base class PdfTokenizer
	// 		EPdfVersion   m_ePdfVersion;
	// 
	// 		bool          m_bLoadOnDemand;
	// 
	// 		pdf_long      m_nXRefOffset;
	// 		long          m_nFirstObject;
	// 		long          m_nNumObjects;
	// 		pdf_long      m_nXRefLinearizedOffset;
	// 		size_t        m_nFileSize;
	// 
	// 		TVecOffsets   m_offsets;

	int nOffset = sizeof(PdfTokenizer);
	int ret = nOffset % sizeof(EPdfVersion);
	if(ret != 0)
		nOffset += sizeof(EPdfVersion) - ret;// the start address of m_ePdfVersion

	nOffset += sizeof(EPdfVersion);

	ret = nOffset % sizeof(bool);
	if(ret != 0)
		nOffset += sizeof(bool) - ret;// the start address of m_bLoadOnDemand

	nOffset += sizeof(bool);

	ret = nOffset % sizeof(pdf_long);
	if(ret != 0)
		nOffset += sizeof(pdf_long) - ret;// the start address of m_nXRefOffset

	nOffset += sizeof(pdf_long);

	ret = nOffset % sizeof(long);
	if(ret != 0)
		nOffset += sizeof(long) - ret;// the start address of m_nFirstObject

	memcpy(pFirstObj, (char*)this + nOffset, sizeof(long));

	return true;
}

bool CPDFParser::ReadObject(PDF_OBJECT* pParam, pdf_long nObjOffset, PdfParserObject** pObj)
{
    if(!pParam)
        return false;

    PdfParserObject* pObject = new PdfParserObject( m_pVecObjects, m_device, m_buffer, nObjOffset );
    if(!pObject)
    {
        return false;
    }

    bool bRet = false;
	
    try
    {
	pObject->ParseFile(m_pEncryptObj);

	pParam->strRawData = GetObjContent(pObject);

	bRet = !(pParam->strRawData.empty());

	if(pObject->HasStream())//decompress stream
	{
            PdfParserObject * const pStream = dynamic_cast<PdfParserObject*>(pObject );

            char * pBuffer = NULL;
            pdf_long lBufferLen(0);

            pStream->GetStream()->GetFilteredCopy( &pBuffer, &lBufferLen );

            if(lBufferLen > 0 && pBuffer)
            {
                pParam->strPlainStream = string(pBuffer, lBufferLen);
                bRet = true;
            }

	}

	if(pObj)
	{
            *pObj = pObject;//delete outside of this function
	}
	else
	{
            delete pObject;
	}
	
	pObject = NULL;
    } catch (const PoDoFo::PdfError &) {
        delete pObject;
        bRet = false;
    }
	
    return bRet;
}

XRefEntry* CPDFParser::FindObjectIndex(int nObjNum)
{
	for(int i = (int)m_vOffsets.size() - 1; i >= 0; i--)//try to findthe offset for current object specified by nObjNum
	{
		if( nObjNum == m_vOffsets[i].nObjNum)
		{
			return &(m_vOffsets[i]);
		}
	}
	return NULL;
}

bool CPDFParser::GetObjectFromObjectStream(PDF_OBJECT* pParam, int nContainerObjectNum, int nTargetObjectNumber)
{
    XRefEntry* pEntry2 = FindObjectIndex(nContainerObjectNum);
    if (!pEntry2)
    {
        return false;  // maybe the parent object is in previous xref
    }

    pdf_long nObjOffset = pEntry2->lOffset;//the offset of "container" object

    PdfParserObject* pInfoObj = NULL;
    if(ReadObject(pParam, nObjOffset, &pInfoObj) && pInfoObj)  // read container object
    {
        //Try to find the related objects in stream
        long long lNum   = pInfoObj->GetDictionary().GetKeyAsLong( "N", 0 );
        long long lFirst = pInfoObj->GetDictionary().GetKeyAsLong( "First", 0 );


        PdfRefCountedInputDevice device( pParam->strPlainStream.c_str(), pParam->strPlainStream.length() );
        PdfTokenizer             tokenizer( device, m_buffer );
        PdfVariant               var;
        int                      i = 0;

        bool bFound = false;
        while( static_cast<long long>(i) < lNum )
        {
            long long lObj     = -1;
            long long lOff     = -1;

            try{
                lObj     = tokenizer.GetNextNumber();
                lOff     = tokenizer.GetNextNumber();
            }catch( const PoDoFo::PdfError & eCode )
            {
                printf("Exception, failed to call tokenizer.GetNextNumber. error: %d\r\n", eCode.GetError());
                delete pInfoObj;  // need to delete here, otherwise file handle can't be freed. fix bug 37922
                throw eCode;
            }

            
            const std::streamoff pos = device.Device()->Tell();

            // move to the position of the object in the stream
            device.Device()->Seek( static_cast<std::streamoff>(lFirst + lOff) );

            tokenizer.GetNextVariant( var, NULL );
            PdfObject* pSubObj = new PdfObject( PdfReference( static_cast<int>(lObj), 0LL ), var );

            if(pSubObj)
            {
                if(lObj == nTargetObjectNumber)  // nice, found the correct object for tags.
                {
                    pParam->strRawData = GetObjContent(pSubObj);
                    bFound = true;
                }
                delete pSubObj;

            }

            // move back to the position inside of the table of contents
            device.Device()->Seek( pos );

            ++i;

            if(bFound)
            {
                delete pInfoObj;
                return true;
            }
        }

        delete pInfoObj;
    }

    return false;
}

//We should call GetObjOffsets() to get the offsets before calling ReadObject()
bool CPDFParser::ReadObject(PDF_OBJECT* pParam, pdf_uint32 nObjNum)
{	
	int nInfoOffset = 0;
	
	XRefEntry* pEntry = FindObjectIndex(nObjNum);
	if(!pEntry)
		return false;//it means we can't find the offset in this xref
	
	nInfoOffset = (int)pEntry->lOffset;

	if(pEntry->cUsed != 's')
	{
		if(ReadObject(pParam, nInfoOffset, NULL))//Read the "info" object directly 
		{
			return true;
		}
	}
	else
	{	
		//It means "info" object was in a stream of another object
		
		pdf_long nObj = pEntry->lGeneration;//the object number of "container" object

        
        if (GetObjectFromObjectStream(pParam, nObj, nObjNum))
        {
            return true;
        }
        else
        {
            const int searchRange = 10;

            // try search next objects
            for (int i = 1; i <= searchRange; ++i)
            {
                if (GetObjectFromObjectStream(pParam, nObj + i, nObjNum))
                {
                    return true;
                }
                
            }
            
            // try search previous objects
            for (int j = 1; j <= searchRange; ++j)
            {
                if (GetObjectFromObjectStream(pParam, nObj - j, nObjNum))
                {
                    return true;
                }
                
            }

        }
       

	}
	

	return false;

}

/*******************************************
strObjContent: store the content of latest DID
return value: our tag string.
********************************************/
string CPDFParser::GetOurTagObj(string& strObjContent, bool& bLastIU, bool bIsReadTagCall,string &strMetaDataObjContent, bool &bFindMetaObjContent)
{
	int nXRefPos = m_nLastXRefPos;

	string strLastDID;
	int nIndex = 0;
	bLastIU = false;
	//try to find our tag
#pragma warning(push)
#pragma warning(disable: 4127)

	
	while(1)
	{
		PDFTRAILER trailer;
		int nInfoObjNum = 0;

		nIndex++;

	//	dwStart = GetTickCount();
		if(	ReadTrailer(&trailer, nXRefPos) )
		{
	//		printf("ReadTrailer() used: %d ms\r\n", GetTickCount() - dwStart);

			nInfoObjNum = atoi(trailer.strInfo.c_str());//the objet number of DID

			if(nInfoObjNum > 0)
			{
				m_nPrevInfoNum = nInfoObjNum;
			}
			else//Use the previous info object number if we can't find the /Info in the current Trailer
			{
				nInfoObjNum = m_nPrevInfoNum;
			}

		//	dwStart = GetTickCount();
			if(nInfoObjNum > 0 && GetObjOffsets(nXRefPos))
			{
				if (bIsReadTagCall&&GetObjOffsets(nXRefPos,ROOT))
				{
					string strObjRef;
					bFindMetaObjContent = GetMetaDataObj(&trailer,nXRefPos,strMetaDataObjContent,strObjRef);
				}
		//		printf("GetObjOffsets() used: %d ms\r\n", GetTickCount() - dwStart);

				PDF_OBJECT obj;
				GetObjOffsets(nXRefPos);

				bool bRet = ReadObject(&obj, nInfoObjNum);
				
				if(bRet)
				{
					if(strLastDID.empty())
						strLastDID = obj.strRawData;

					if(obj.strRawData.find(NEXTLABS_TAG) != string::npos && obj.strRawData.find(NEXTLABS_CHECKSUM, obj.strRawData.find(NEXTLABS_TAG)) != string::npos)
					{
						if(nIndex == 1)
						{//it means the last Incremental Update is ours.
							//try to read the comment
							int nFlagLen =static_cast<int> (strlen(NEXTLABS_IU));
							m_device.Device()->Seek(-nFlagLen, std::ios_base::end);
							char* buf = new char[nFlagLen + 1];
							memset(buf, 0, (nFlagLen + 1) * sizeof(char));
							m_device.Device()->Read(buf, nFlagLen);

							if(_memicmp(buf, NEXTLABS_IU, nFlagLen * sizeof(char)) == 0)
								bLastIU = true;

							delete []buf;

						}

						strObjContent = ExtractObjBody(strLastDID);

						//try to get our tag string
						string::size_type nStart = obj.strRawData.find(NEXTLABS_TAG);
						string::size_type i1 = obj.strRawData.find('/', nStart + strlen(NEXTLABS_TAG));
						string::size_type i2 = obj.strRawData.find(">>", nStart + strlen(NEXTLABS_TAG));
						string::size_type nEnd = i1 < i2? i1: i2;

						if(nEnd != string::npos)
							return obj.strRawData.substr(nStart + strlen(NEXTLABS_TAG), nEnd - nStart - strlen(NEXTLABS_TAG));
					}
				}
			}

			nXRefPos = atoi(trailer.strPrev.c_str());//try to search in previous xref

			if(nXRefPos > 0)
				continue;

		}

		break;
	
	}
#pragma warning(pop)

	strObjContent = ExtractObjBody(strLastDID);
	
	return "";
}




string CPDFParser::GetObjContent(PdfObject* pObj)
{
	if(!pObj)
		return "";

	pdf_long len = pObj->GetObjectLength();

	char* pBuffer = static_cast<char*>(malloc( sizeof(char) * len ));
	if( !pBuffer )
	{
		return "";
	}

	PdfOutputDevice device( pBuffer, len);
	pObj->WriteObject( &device, NULL, PdfName("") );//Write to buffer

	string strContent = string(pBuffer, len);

	free(pBuffer);
	pBuffer = NULL;

	return strContent;
}

string CPDFParser::ExtractObjBody(const string& strObj)
{
	string::size_type nStart = strObj.find("<<");
	string::size_type nEnd = strObj.rfind(">>");

	if(nStart != string::npos && nEnd != string::npos && nEnd > nStart)
	{
		return strObj.substr(nStart + 2, nEnd - nStart - 2);
	}

	return "";
}

string CPDFParser::ReadRawData(int lPos, int nLen)
{
	m_device.Device()->Seek(lPos);
	char* buf = new char[nLen];
	m_device.Device()->Read(buf, nLen);

	string data(buf, nLen);

	delete []buf;
	buf = NULL;

	return data;
}


/*******************************************************************
1. PDF1.4 and earlier version
it will use xref table + trailer independently.
like:
xref
48 1
0000019131 00000 n
49 1
0000019743 26587 n
trailer
<<
/ID [<D97542D41B46F3BC9B2071CE55EF8D5A><52EC979E80B5A847855DC511ACADE9A2>]
/Info 48 0 R
/Size 50
/Root 25 0 R
/Prev 19319
>>
startxref
32444
%%EOF

2. PDF 1.5 and later
after PDF1.5, it's possilbe that the PDF file uses "object stream". it means the "xref table and trailer" are stored in an object,
like:
31 0 obj
<</DecodeParms<</Columns 4/Predictor 12>>/Filter/FlateDecode/ID[<CF8C8FBF09EE7A4FB431B9225705946A><D50C13F3448C654A92E6082FCE27072F>]/Index[24 16]/Info 23 0 R/Length 55/Prev 9736/Root 25 0 R/Size 40/Type/XRef/W[1 2 1]>>stream...endstreamendobj
********************************************************************/
bool CPDFParser::ReadTrailer(LPPDFTRAILER pTrailer, int nXRef)
{
	if(!pTrailer)
		return false;

	bool bRet = false;

	string strData;
	if(!IsXRefStream(nXRef))//xref table
	{
		strData = GetTrailerContent(nXRef);
	}
	else//xref stream
	{
		strData = GetTrailerContentStream(nXRef);
	}

	if(!strData.empty())
	{
		bRet = ParseTrailer(pTrailer, strData);
	}

	return bRet;
}

string CPDFParser::GetTrailerContent(int nPos)
{
	CPDFXRefParserObject obj;
	int nLen = obj.ComputeXRefLen(nPos, this);//get the length of xref table content
	if(nLen <= 0)
		return "";

	string strData = ReadRawData( nPos + nLen, 1024);//locate the trailer position
	string strTemp = strData;
	transform(strTemp.begin(), strTemp.end(), strTemp.begin(), tolower);

	string::size_type nStart = strTemp.find(PDF_TRAILER);
	string::size_type nEnd = strTemp.find(PDF_STARTXREF);

	if(nEnd > nStart && nEnd != string::npos && nStart != string::npos)
	{
		strData = strData.substr(nStart + strlen(PDF_TRAILER), nEnd - nStart - strlen(PDF_TRAILER));

		/*
		This is an interesting case, see below format, there are xref table, but no "%%EOF" between these 2 xref tables.
		this PDF file was generated by Acrobat7.0 to convert a 1.4 PDF to 1.6 (named: xfig_ref_en.pdF)
		in order to support this case, we need to find the first ">>", and truncate the string.
		xref
		...
		trailer
		<</Size 4764/Root 4643 0 R>>
		xref
		0 0
		trailer
		<</Size 4764/Prev 1510868/XRefStm 2753/Root 4643 0 R/Info 3667 0 R/ID[<FB656E42EADC3580F0850DFACD3F5380><2B640CDFA0F815458D810C3729ECC576>]>>
		startxref
		1606201
		%%EOF
		*/
		nStart = strData.find("xref");
		nEnd = string::npos;
		if(nStart != string::npos)
			nEnd = strData.rfind(">>", nStart);
		
		if(nStart != string::npos && nEnd != string::npos)
			strData = strData.substr(0, nEnd + 2);
		return strData;
	}

	

	return "";
}

string CPDFParser::GetTrailerContentStream(int nPos)
{
	string strData;

	const int nReadSize = 512;//read 512 bytes every time

	strData = ReadRawData(nPos, nReadSize);//try to read 512 bytes from nXref
	
	string strTemp = strData;
	transform(strTemp.begin(), strTemp.end(), strTemp.begin(), tolower);
	string::size_type nEndObj = strTemp.find(PDF_ENDOBJ);
	if (nEndObj != string::npos)
	{
		strData = strData.substr(0, nEndObj + strlen(PDF_ENDOBJ));
	}
	
	return strData;
}

bool CPDFParser::ParseTrailer(LPPDFTRAILER pTrailer, const string& strData)
{
	if(!pTrailer)
		return false;

	string strTemp = strData;
	transform(strTemp.begin(), strTemp.end(), strTemp.begin(), toupper);

	string::size_type nStart, nEnd;
	if( (nStart = strTemp.find(PDF_TRAILER_ID) ) != string::npos)
	{
		nEnd = strTemp.find("/", nStart + strlen(PDF_TRAILER_ID));
		if(nEnd == string::npos)
			nEnd = strTemp.find(">>", nStart + strlen(PDF_TRAILER_ID));

		if(nEnd != string::npos)
		{
			pTrailer->strID = strTemp.substr(nStart + strlen(PDF_TRAILER_ID), nEnd - nStart - strlen(PDF_TRAILER_ID));

			CUtils::RemoveCharAtFrontAndBack(pTrailer->strID, ' ');
			CUtils::RemoveCharAtFrontAndBack(pTrailer->strID, '\n');
			CUtils::RemoveCharAtFrontAndBack(pTrailer->strID, '\r');
		}
	}

	if( (nStart = strTemp.find(PDF_TRAILER_INFO) ) != string::npos)
	{
		nEnd = strTemp.find("/", nStart + strlen(PDF_TRAILER_INFO));
		if(nEnd == string::npos)
			nEnd = strTemp.find(">>", nStart + strlen(PDF_TRAILER_INFO));
		if(nEnd != string::npos)
		{
			pTrailer->strInfo = strTemp.substr(nStart + strlen(PDF_TRAILER_INFO), nEnd - nStart - strlen(PDF_TRAILER_INFO));

			CUtils::RemoveCharAtFrontAndBack(pTrailer->strInfo, ' ');
			CUtils::RemoveCharAtFrontAndBack(pTrailer->strInfo, '\n');
			CUtils::RemoveCharAtFrontAndBack(pTrailer->strInfo, '\r');
		}
	}

	if( (nStart = strTemp.find(PDF_TRAILER_SIZE) ) != string::npos)
	{
		nEnd = strTemp.find("/", nStart + strlen(PDF_TRAILER_SIZE));
		if(nEnd == string::npos)
			nEnd = strTemp.find(">>", nStart + strlen(PDF_TRAILER_SIZE));
		if(nEnd != string::npos)
		{
			pTrailer->strSize = strTemp.substr(nStart + strlen(PDF_TRAILER_SIZE), nEnd - nStart - strlen(PDF_TRAILER_SIZE));

			CUtils::RemoveCharAtFrontAndBack(pTrailer->strSize, ' ');
			CUtils::RemoveCharAtFrontAndBack(pTrailer->strSize, '\n');
			CUtils::RemoveCharAtFrontAndBack(pTrailer->strSize, '\r');
		}
	}

	if( (nStart = strTemp.find(PDF_TRAILER_PREV) ) != string::npos)
	{
		nEnd = strTemp.find("/", nStart + strlen(PDF_TRAILER_PREV));
		if(nEnd == string::npos)
			nEnd = strTemp.find(">>", nStart + strlen(PDF_TRAILER_PREV));
		if(nEnd != string::npos)
		{
			pTrailer->strPrev = strTemp.substr(nStart + strlen(PDF_TRAILER_PREV), nEnd - nStart - strlen(PDF_TRAILER_PREV));

			CUtils::RemoveCharAtFrontAndBack(pTrailer->strPrev, ' ');
			CUtils::RemoveCharAtFrontAndBack(pTrailer->strPrev, '\n');
			CUtils::RemoveCharAtFrontAndBack(pTrailer->strPrev, '\r');
		}
	}

	if( (nStart = strTemp.find(PDF_TRAILER_ROOT) ) != string::npos)
	{
		nEnd = strTemp.find("/", nStart + strlen(PDF_TRAILER_ROOT));
		if(nEnd == string::npos)
			nEnd = strTemp.find(">>", nStart + strlen(PDF_TRAILER_ROOT));
		if(nEnd != string::npos)
		{
			pTrailer->strRoot = strTemp.substr(nStart + strlen(PDF_TRAILER_ROOT), nEnd - nStart - strlen(PDF_TRAILER_ROOT));

			CUtils::RemoveCharAtFrontAndBack(pTrailer->strRoot, ' ');
			CUtils::RemoveCharAtFrontAndBack(pTrailer->strRoot, '\n');
			CUtils::RemoveCharAtFrontAndBack(pTrailer->strRoot, '\r');
		}
	}

	if( (nStart = strTemp.find(PDF_TRAILER_ENCRYPT) ) != string::npos)
	{
		nEnd = strTemp.find("/", nStart + strlen(PDF_TRAILER_ENCRYPT));
		if(nEnd == string::npos)
			nEnd = strTemp.find(">>", nStart + strlen(PDF_TRAILER_ENCRYPT));
		if(nEnd != string::npos)
		{
			pTrailer->strEncrypt = strTemp.substr(nStart + strlen(PDF_TRAILER_ENCRYPT), nEnd - nStart - strlen(PDF_TRAILER_ENCRYPT));

			CUtils::RemoveCharAtFrontAndBack(pTrailer->strEncrypt, ' ');
			CUtils::RemoveCharAtFrontAndBack(pTrailer->strEncrypt, '\n');
			CUtils::RemoveCharAtFrontAndBack(pTrailer->strEncrypt, '\r');
		}
	}

	return true;
}

int CPDFParser::GetInfoObjNum()
{
	PDFTRAILER trailer;
	if(ReadTrailer(&trailer, m_nLastXRefPos))
	{
		if(m_pLastTrailer)
		{
			m_pLastTrailer->CopyTrailer(&trailer);
		}

		while(trailer.strInfo.empty() && !trailer.strPrev.empty())
		{
			trailer.ResetTrailer();
			if(!ReadTrailer(&trailer, atoi(trailer.strPrev.c_str())))
				break;
		}

		return atoi(trailer.strInfo.c_str());
	}

	return -1;
}

LPPDFTRAILER CPDFParser::GetLastTrailer()
{
	if(m_pLastTrailer)
	{
		if(m_pLastTrailer->strRoot.empty() || m_pLastTrailer->strSize.empty())
		{
			ReadTrailer(m_pLastTrailer, m_nLastXRefPos);
		}

		return m_pLastTrailer;
	}
	return NULL;
}

string CPDFParser::GetRootObj(int& nPos)
{
	PDFTRAILER trailer;
	if(ReadTrailer(&trailer, nPos))
	{
		int nRootObj = atoi(trailer.strRoot.c_str());//Get the ROOT obj number
		PDF_OBJECT obj;
		if(ReadObject(&obj, nRootObj))//Get the content of Root object.
		{
			return ExtractObjBody(obj.strRawData);
		}
	}

	return "";
}

string CPDFParser::GetPieceInfoObj()
{
	string strRoot = GetRootObj(m_nLastXRefPos);
	if(!strRoot.empty())
	{
		string strTemp = strRoot;
		transform(strTemp.begin(), strTemp.end(), strTemp.begin(), toupper);

		string::size_type nStart = strTemp.find(PDF_ROOT_PEICEINFO);
		if( nStart != string::npos)
		{
			string::size_type nEnd = strTemp.find("R", nStart + strlen(PDF_ROOT_PEICEINFO));
			if(nEnd != string::npos)
			{
				string PieceInfoObj = strTemp.substr(nStart + strlen(PDF_ROOT_PEICEINFO), nEnd - nStart);
				CUtils::RemoveCharAtFrontAndBack(PieceInfoObj, ' ');

				int nPieceInfoObjNum = atoi(PieceInfoObj.c_str());

				PDF_OBJECT obj;
				if(ReadObject(&obj, nPieceInfoObjNum))
				{
					string strBody = ExtractObjBody(obj.strRawData);
					return strBody;
				}
			}
		}

	}

	return "";
}

bool CPDFParser::GetMetaDataObj(LPPDFTRAILER pTrailer,int nXRefPos,string& strObjData, string& strObjRef)
{
	if(!pTrailer)
	{
		return false;
	}

	int nRootObj = atoi(pTrailer->strRoot.c_str());
	PDF_OBJECT obj;
	string strData;
	bool bRet = false;
	if(ReadObject(&obj, nRootObj))//Get the content of Root object.
	{
		string strRoot = obj.strRawData;
		transform(strRoot.begin(), strRoot.end(), strRoot.begin(), toupper);

		string::size_type nStart = strRoot.find(PDF_ROOT_METADATA);
		if(nStart != string::npos)
		{
			string::size_type nEnd = strRoot.find("R", nStart);
			if(nEnd != nStart)
			{
				string strMetaObjNum = strRoot.substr(nStart + strlen(PDF_ROOT_METADATA), nEnd - nStart - strlen(PDF_ROOT_METADATA));
				CUtils::RemoveCharAtFrontAndBack(strMetaObjNum, ' ');

				int nMetaObjNum = atoi(strMetaObjNum.c_str());
				if (nMetaObjNum < 0)
				{
					nMetaObjNum = 0;
				}
				bRet = ReadXRefEntries(nXRefPos, nMetaObjNum);
				if (!bRet)
				{
					return bRet;
				}
				if(ReadObject(&obj, nMetaObjNum))
				{
					strData = obj.strRawData;

					strObjData = strData;
					strObjRef = strMetaObjNum;

					bRet = true;
				}
			}
		}
	}

	return bRet;
}

int CPDFParser::FindLastToken(const char* pToken)
{
	if(!pToken)
		return -1;

	m_device.Device()->Seek( 0, std::ios_base::end );

	std::streamoff nFileSize = m_device.Device()->Tell();
	if (nFileSize == -1)
	{
		PODOFO_RAISE_ERROR_INFO(
			ePdfError_NoXRef,
			"Failed to seek to EOF when looking for xref");
	}

	const pdf_long lRange = 100;
	
	pdf_long lLen = 0;
	for(int i = 1; i < 100; i++)
	{
		lLen  = PDF_MIN( static_cast<pdf_long>(nFileSize), static_cast<pdf_long>(lRange) * i);

		char* buf = new char[lLen];
		if(buf)
		{
            memset(buf, 0, sizeof(char) * lLen);

//            std::streamoff nCurrOffset = m_device.Device()->Tell();

			m_device.Device()->Seek( -lLen, std::ios_base::end/*std::ios_base::cur*/ );

//            std::streamoff nSeekOffset = m_device.Device()->Tell();

			pdf_long lRead = 0;
			if( (lRead = m_device.Device()->Read( buf, lLen )) != lLen )
			{
				delete []buf;
				buf = NULL;
				break;
			}
			else
			{
				string strTemp(buf, lRead);
				string::size_type nStart = strTemp.rfind(pToken);
				if(nStart != string::npos)
				{
					delete []buf;
					buf = NULL;
					return static_cast<int> (nFileSize - lRead + nStart);
				}
			}

			delete []buf;
			buf = NULL;
		}
	}
	return -1;
}

/*
Like:
%%EOF
%%NLTAGVER=1.0
%%NLTAG=YQBiYgBpdGFyAHllcwA=
%%NLTAGLEN=46   
*/
void CPDFParser::GetCommentTags(map<string, string>& mapTags, int& nLen)
{
	int nPos = FindLastToken(PDF_ENDER);
	if(nPos > 0)
	{
		m_device.Device()->Seek(nPos + strlen(PDF_ENDER), std::ios_base::beg);

		char* buf = new char[m_nCurFileSize - nPos];
		if(!buf)
			return;
		
		memset(buf, 0, sizeof(char) * (m_nCurFileSize - nPos));
		int nRead = (int)m_device.Device()->Read(buf, m_nCurFileSize - nPos);

		string strTemp(buf, nRead);

		bool bEOL = false;
		if(strTemp.length() > 2 && strTemp[0] == '\r' && strTemp[1] == '\n')
			bEOL = true;

		delete []buf;
		buf = NULL;

		string::size_type nIndex = strTemp.find(PDF_COMMENT_TAG_VER);
		if(nIndex == string::npos)
		{//that means we don't have "comment tag".
			return;
		}

		nIndex = strTemp.find(PDF_COMMENT_TAG, nIndex + strlen(PDF_COMMENT_TAG_VER));
		if(nIndex != string::npos)
		{
			strTemp = strTemp.substr(nIndex, strTemp.length() - nIndex);
			nLen = static_cast<int> (nPos + strlen(PDF_ENDER));
			if(bEOL) nLen += 2;

			string::size_type nStart = strTemp.find("=");
			string::size_type nEnd = strTemp.find("\r\n");
			if(nStart != string::npos && nEnd != string::npos && nEnd > nStart)
			{
				string tags = strTemp.substr(nStart + 1, nEnd - nStart - 1);
				tags = Base64::base64_decode(tags);

				while(!tags.empty())
				{
					nIndex = tags.find('\0');
					assert(nIndex != string::npos);
					string strTagName = tags.substr(0, nIndex);
					tags = tags.substr(nIndex + 1, tags.length() - nIndex - 1);
					nIndex = tags.find('\0');
					assert(nIndex != string::npos);
					string strTagValue = tags.substr(0, nIndex);

					mapTags[strTagName] = strTagValue;

					if(nIndex < tags.length())
						tags = tags.substr(nIndex + 1, tags.length() - nIndex);
					else
						break;

				}
			}
		}
		
	}

	return;
}


string CPDFParser::GetPdfVersion()
{
	string strVersion = "";
	EPdfVersion Version = ((PdfParser*)this)->GetPdfVersion();
	switch(Version)
	{
	case ePdfVersion_1_0:
		{
			strVersion = "1.0";
		}
		break;
	case ePdfVersion_1_1:
		{
			strVersion = "1.1";
		}
		break;
	case ePdfVersion_1_2:
		{
			strVersion = "1.2";
		}
		break;
	case ePdfVersion_1_3:
		{
			strVersion = "1.3";
		}
		break;
	case ePdfVersion_1_4:
		{
			strVersion = "1.4";
		}
		break;
	case ePdfVersion_1_5:
		{
			strVersion = "1.5";
		}
		break;
	case ePdfVersion_1_6:
		{
			strVersion = "1.6";
		}
		break;
	case ePdfVersion_1_7:
		{
			strVersion = "1.7";
		}
		break;
	default:
		break;
	}
	return strVersion;
}

