#pragma once
#include <string>
#include <map>
#include "nl_pdfparser.h"

using namespace std;
using namespace nextlabs;

typedef enum STORE_POSITION
{
	DID = 0,
	PIECEINFO
};

class CIncrementalUpdate
{
public:
	CIncrementalUpdate();
	~CIncrementalUpdate(void);

	bool AddIncrementalUpdate(char* pszFileName, map<string,string>* pTags, bool bAdd = true);

protected:
	//old style of xref table + trailer, PDF1.4 and earlier version
	string CreateTrailer();
	string CreateXRefTable();
	string CreateNewObject(bool bXStem);

	string CreateOldStyleIncrementalUpdate();

	//new style, PDF1.5 and later version
	string CreateXRefStrem();
	string CreateNewStyleIncrementalUpdate();

	//Create an IU base on the style of last xref.
	string CreateIncrementalUpdate();
	bool WriteIncrementalUpdate();

	//Determine if we need to use DID or PieceInfo object
	STORE_POSITION GetStorePosition();

	string GenerateTagContent();

	bool NeedKeep(const string& strTagName);

protected:
	char*	m_pFileName;
	size_t  m_nFileSize;
	string  m_strNewObj;
	string  m_strMetaObj;
	string  m_strMetaObjRef;
	LPPDFTRAILER	m_pLastTrailer;
	int		m_nLastXRefPos;

	string m_strExistingData;//store the existing content of DID or piece info object
	string m_strOurTags;
	string m_strRootData;//Store the object body of ROOT.

	int		m_nPos1;//the position of first new object
	int		m_nPos2;//the position of second new object
	int		m_nPos3;

	map<string,string>* m_pMapTags;
	bool				m_bAdd;
	map<string, string> m_mapCommentTags;

	bool				m_bLastIUIsUs;
	
};
