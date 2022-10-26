#pragma once

#pragma warning(push)
#pragma warning(disable: 4512 6011)
#include <PdfDefines.h>
#include <podofo.h>
#include <vector>
#pragma warning(pop)

using namespace std;
using namespace PoDoFo;


namespace nextlabs
{
	struct XRefEntry {
		inline XRefEntry() : lOffset(0), lGeneration(0), cUsed('\x00'), nObjNum(-1) { }
		pdf_long lOffset;
		long lGeneration;
		char cUsed;
		int  nObjNum;
	};

	class CPDFParser;
	class CPDFXRefParserObject
	{
	public:
		CPDFXRefParserObject(void);
		~CPDFXRefParserObject(void);

		int ComputeXRefLen(const int& nXRefPos, CPDFParser* pParser);
		bool ParseEntries(const string& strXRefData, vector<XRefEntry>& vEntries, const int& nObjectNum = -1);
		bool ParseEntries(CPDFParser* pParser, const int& nXRef, vector<XRefEntry>& vEntries, const int& nObjectNum);
	};
}

