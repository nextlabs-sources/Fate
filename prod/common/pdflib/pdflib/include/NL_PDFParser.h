#ifndef _PDFPARSER_H__
#define _PDFPARSER_H__

#include <string>

#pragma warning(push)
#pragma warning(disable: 4512 6011)

#include <PdfDefines.h>
#include <podofo.h>
#include "NL_PDFXRefParserObject.h"

#pragma warning(pop)

#define NEXTLABS_TAG		"/NLRESATTR"
#define NEXTLABS_CHECKSUM	":EAB12DEHDEAFEFA39IJ34ADFIK34L3"
#define NEXTLABS_IU			"%%NEXTLABS_IU"

using namespace PoDoFo;
using namespace std;

enum TRAILERTYPE
{
	INFO,
	ROOT
};

namespace nextlabs
{
	typedef struct PDF_Trailer 
	{
		string strInfo;
		string strID;
		string strRoot;
		string strPrev;
		string strSize;
		string strEncrypt;


		PDF_Trailer()
		{
			ResetTrailer();
		}

		void ResetTrailer()
		{
			strInfo.clear();
			strID.clear();
			strRoot.clear();
			strPrev.clear();
			strSize.clear();
			strEncrypt.clear();
		}

		void CopyTrailer(PDF_Trailer* pTrailer)
		{
			if(pTrailer)
			{
				strInfo = pTrailer->strInfo;
				strID = pTrailer->strID;
				strRoot = pTrailer->strRoot;
				strPrev = pTrailer->strPrev;
				strSize = pTrailer->strSize;
				strEncrypt = pTrailer->strEncrypt;
			}
		}
	}PDFTRAILER, *LPPDFTRAILER;

	struct PDF_OBJECT
	{
		string strRawData;
		string strPlainStream;
	};


	class CPDFParser: public PdfParser, public PdfMemDocument
	{
	public:
		CPDFParser(const char* pszFileName);
		~CPDFParser();

		bool Init();

		bool IsPDFFile(){return IsPdfFile();}

		bool ReadObject(PDF_OBJECT* pParam, pdf_uint32 nObjNum);
		bool ReadObject(PDF_OBJECT* pParam, pdf_long nObjOffset, PdfParserObject** pObj);

		const PdfObject* GetPDFTrailer(){return PdfParser::GetTrailer();}
		int GetLastXRefPos(){return m_nLastXRefPos;}

		string ReadRawData(int lPos, int nLen);

		bool ReadTrailer(LPPDFTRAILER pTrailer, int nXRef);

		int GetInfoObjNum();
		
		LPPDFTRAILER GetLastTrailer();

		bool GetMetaDataObj(LPPDFTRAILER pTrailer,int nXRefPos,string& strObjData, string& strObjRef);
		string GetRootObj(int& nPos);
		string GetPieceInfoObj();
		
		/*******************************************
		strObjContent: store the content of the latest DID
		return value: our tag string.
		********************************************/
		string GetOurTagObj(string& strObjContent, bool& bLastIU, bool bIsReadTagCall,string &strMetaDataObjContent,bool &bFindMetaObjContent);

		bool IsLastXRefStream(){return m_bLastXrefStream;}

		bool CreateEncryptObj();

		int GetCurFileSize(){return m_nCurFileSize;}

		/**********************************************
		We have used "comment" to implement the tags before,
		we need to consider the compatabitity here.
		%%EOF
		%%NLTAGVER=1.0
		%%NLTAG=YQBiYgBpdGFyAHllcwA=
		%%NLTAGLEN=46 
		***********************************************/
		void GetCommentTags(map<string, string>& mapTags, int& nLen);

		string GetPdfVersion();
	protected:
		/*
		function: Try to get the member "TVecOffsets   m_offsets;" of PdfParser
		Remarks:
		"TVecOffsets   m_offsets;" is a private data member of PdfParser, so we 
		can't access this member directly. Here, we count the offset of this member
		in the memory, and then copy the value of this address.
		*/
		bool GetOffsetsMember(char** pOffsets);
		bool GetFirstObjectMember(long* pFirstObj);

		string GetObjContent(PdfObject* pObj);

		string GetTrailerContentStream(int nPos);
		string GetTrailerContent(int nPos);
		bool ParseTrailer(LPPDFTRAILER pTrailer, const string& strData);

		string ExtractObjBody(const string& strObj);

		const PdfString GetDocumentID();
		
		bool GetObjOffsets(int nXRefPos,TRAILERTYPE Type = INFO);//Get all the object offsets in the xref which was specified by parameter.
		bool ReadXRefStreamEntries(int nPos, TVecOffsets& vOffsets);//read all the entries in stream
		bool ReadXRefEntries(int nPos, const int& nObjectNum = -1);//read all the enties at nPos.

		bool IsEncrypted(int nXRefPos);

		bool IsXRefStream(int nXRefPos);

		XRefEntry* FindObjectIndex(int nObjNum);

		int FindLastToken(const char* pToken);

        bool GetObjectFromObjectStream(PDF_OBJECT* pParam, int nContainerObjectNum, int nTargetObjectNumber);
	protected:
		PdfVecObjects*	m_pVecObjects;
		char*			m_pFileName;
		int				m_nLastXRefPos;
		LPPDFTRAILER	m_pLastTrailer;
		bool			m_bLastXrefStream;
		PdfEncrypt*		m_pEncryptObj;
	
		vector<XRefEntry>	m_vOffsets;

		int				m_nCurFileSize;
		int				m_nPrevInfoNum;
		
	};

}

#endif