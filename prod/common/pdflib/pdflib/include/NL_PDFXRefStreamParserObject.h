#pragma once

#pragma warning(push)
#pragma warning(disable: 4512 6011)
#include <PdfDefines.h>
#include <podofo.h>
#pragma warning(pop)


using namespace PoDoFo;
using namespace std;

namespace nextlabs
{
	
	class CPDFXRefStreamParserObject: public PdfParserObject
	{
	public:
		CPDFXRefStreamParserObject(PdfVecObjects* pCreator, const PdfRefCountedInputDevice & rDevice, 
			const PdfRefCountedBuffer & rBuffer, PdfParser::TVecOffsets* pOffsets );

		~CPDFXRefStreamParserObject();

		void Parse();

		void ReadXRefTable(std::vector<pdf_int64>& vIndex);

		/**
		* \returns true if there is a previous XRefStream
		*/
		inline bool HasPrevious();

		/**
		* \returns the offset of the previous XRef table
		*/
		inline pdf_long GetPreviousOffset();

	private:
		/**
		* Read the /Index key from the current dictionary
		* and write uit to a vector.
		*
		* \param rvecIndeces store the indeces hare
		* \param size default value from /Size key
		*/
		void GetIndeces( std::vector<pdf_int64> & rvecIndeces, pdf_int64 size );

		/**
		* Parse the stream contents
		*
		* \param nW /W key
		* \param rvecIndeces indeces as filled by GetIndeces
		*
		* \see GetIndeces
		*/
		void ParseStream( const pdf_int64 nW[W_ARRAY_SIZE], const std::vector<pdf_int64> & rvecIndeces );

		void ReadXRefStreamEntry( char* pBuffer, pdf_long, const pdf_int64 lW[W_ARRAY_SIZE], int nObjNo );
	private:
		pdf_long m_lNextOffset;

		PdfParser::TVecOffsets* m_pOffsets;

	};

}