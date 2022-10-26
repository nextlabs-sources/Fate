#include "stdafx.h"
#include "NL_PDFXRefStreamParserObject.h"
#define XREF_KEYTYPE		"Type"
#define XREF_KEYSIZE		"Size"

using namespace nextlabs;

CPDFXRefStreamParserObject::CPDFXRefStreamParserObject(PdfVecObjects* pCreator, const PdfRefCountedInputDevice & rDevice, 
													 const PdfRefCountedBuffer & rBuffer, PdfParser::TVecOffsets* pOffsets )
													 : PdfParserObject( pCreator, rDevice, rBuffer ), m_lNextOffset(-1L), m_pOffsets( pOffsets )
{

}

CPDFXRefStreamParserObject::~CPDFXRefStreamParserObject() 
{

}

void CPDFXRefStreamParserObject::Parse()
{
	// Ignore the encryption in the XREF as the XREF stream must no be encrypted (see PDF Reference 3.4.7)
	this->ParseFile( NULL );

	// Do some very basic error checking
	if( !this->GetDictionary().HasKey( XREF_KEYTYPE ) )
	{
		PODOFO_RAISE_ERROR( ePdfError_NoXRef );
	} 

	PdfObject* pObj = this->GetDictionary().GetKey( XREF_KEYTYPE );
	if( !pObj->IsName() || ( pObj->GetName() != "XRef" ) )
	{
		PODOFO_RAISE_ERROR( ePdfError_NoXRef );
	}

	if( !this->GetDictionary().HasKey( XREF_KEYSIZE ) 
		|| !this->GetDictionary().HasKey( "W" ) )
	{
		PODOFO_RAISE_ERROR( ePdfError_NoXRef );
	} 

	if( !this->HasStreamToParse() )
	{
		PODOFO_RAISE_ERROR( ePdfError_NoXRef );
	}

	if( this->GetDictionary().HasKey("Prev") )
	{
		m_lNextOffset = static_cast<pdf_long>(this->GetDictionary().GetKeyAsLong( "Prev", 0 ));
	}
}

void CPDFXRefStreamParserObject::ReadXRefTable(std::vector<pdf_int64>& vIndex) 
{
	long long  lSize   = this->GetDictionary().GetKeyAsLong( XREF_KEYSIZE, 0 );
	PdfVariant vWArray = *(this->GetDictionary().GetKey( "W" ));

	// The pdf reference states that W is always an array with 3 entries
	// all of them have to be integers
	if( !vWArray.IsArray() || vWArray.GetArray().size() != 3 )
	{
		PODOFO_RAISE_ERROR( ePdfError_NoXRef );
	}


	pdf_int64 nW[W_ARRAY_SIZE] = { 0, 0, 0 };
	for( int i=0;i<W_ARRAY_SIZE;i++ )
	{
		if( !vWArray.GetArray()[i].IsNumber() )
		{
			PODOFO_RAISE_ERROR( ePdfError_NoXRef );
		}

		nW[i] = static_cast<pdf_int64>(vWArray.GetArray()[i].GetNumber());
	}

	std::vector<pdf_int64> vecIndeces;
	GetIndeces( vecIndeces, static_cast<pdf_int64>(lSize) );

	ParseStream( nW, vecIndeces );

	vIndex = vecIndeces;
}

void CPDFXRefStreamParserObject::ParseStream( const pdf_int64 nW[W_ARRAY_SIZE], const std::vector<pdf_int64> & rvecIndeces )
{
	char*        pBuffer;
	pdf_long     lBufferLen;
	const size_t entryLen  = static_cast<size_t>(nW[0] + nW[1] + nW[2]);

	this->GetStream()->GetFilteredCopy( &pBuffer, &lBufferLen );

	std::vector<pdf_int64>::const_iterator it = rvecIndeces.begin();

	bool bCheckWrongEntry = false;
	pdf_int64 nObjCount = 0;
	for(; it != rvecIndeces.end(); )
	{
		it++;
		nObjCount += *it;
		it++;
	}

	if(lBufferLen / entryLen >static_cast<ULONGLONG> (nObjCount))
	{
		printf("Exception, the entries count is bigger than the value in /Index\r\nEntry count: %d, the value in /Index: %d\r\n", lBufferLen / entryLen, (int)nObjCount);
		bCheckWrongEntry = true;
	}

	it = rvecIndeces.begin();
	char* const pStart = pBuffer;
	while( it != rvecIndeces.end() )
	{
		pdf_int64 nFirstObj = *it; ++it;
		pdf_int64 nCount    = *it; ++it;

//		pdf_int64 nFirstObjOrg = nFirstObj;
//		pdf_int64 nCountOrg = nCount;
		

		//printf("\n");
		//printf("nFirstObj=%i\n", static_cast<int>(nFirstObj));
		//printf("nCount=%i\n", static_cast<int>(nCount));
		while( nCount > 0 )
		{
			if( (pBuffer - pStart) >= lBufferLen ) 
			{
				PODOFO_RAISE_ERROR_INFO( ePdfError_NoXRef, "Invalid count in XRef stream" );
				break;
			}
			

			//printf("nCount=%i ", static_cast<int>(nCount));
			//printf("pBuffer=%li ", (long)(pBuffer - pStart));
			//printf("pEnd=%li ", lBufferLen);
			ReadXRefStreamEntry( pBuffer, lBufferLen, nW, (int)nFirstObj );

// 			if(bCheckWrongEntry && (*m_pOffsets)[nFirstObj].cUsed == 'n')
// 			{
// 				m_device.Device()->Seek((*m_pOffsets)[nFirstObj].lOffset);
// 				char buf[1] = {0};
// 				m_device.Device()->Read(buf, 1);
// 				if(buf[0] != '\0' && !(buf[0] >= '0' && buf[0] <= '9'))
// 				{
// 					pBuffer += entryLen;
// 					continue;
// 				}
// 			}

			pBuffer += entryLen;
			--nCount;
			nFirstObj++;
		}
		//printf("Exp: nFirstObj=%i nFirstObjOrg + nCount=%i\n", nFirstObj - 1, nFirstObjOrg + nCountOrg - 1 );
		//printf("===\n");
	}
	podofo_free( pStart );

}

void CPDFXRefStreamParserObject::GetIndeces( std::vector<pdf_int64> & rvecIndeces, pdf_int64 size ) 
{
	// get the first object number in this crossref stream.
	// it is not required to have an index key though.
	if( this->GetDictionary().HasKey( "Index" ) )
	{
		PdfVariant array = *(this->GetDictionary().GetKey( "Index" ));
		if( !array.IsArray() )
		{
			PODOFO_RAISE_ERROR( ePdfError_NoXRef );
		}

		TCIVariantList it = array.GetArray().begin();
		while ( it != array.GetArray().end() )
		{
			rvecIndeces.push_back( (*it).GetNumber() );
			++it;
		}
	}
	else
	{
		// Default
		rvecIndeces.push_back( static_cast<pdf_int64>(0) );
		rvecIndeces.push_back( size );
	}

	// vecIndeces must be a multiple of 2
	if( rvecIndeces.size() % 2 != 0)
	{
		PODOFO_RAISE_ERROR( ePdfError_NoXRef );
	}
}

void CPDFXRefStreamParserObject::ReadXRefStreamEntry( char* pBuffer, pdf_long, const pdf_int64 lW[W_ARRAY_SIZE], int nObjNo )
{
	int              i, z;
	unsigned long    nData[W_ARRAY_SIZE];

	for( i=0;i<W_ARRAY_SIZE;i++ )
	{
		if( lW[i] > W_MAX_BYTES )
		{
			PdfError::LogMessage( eLogSeverity_Error, 
				"The XRef stream dictionary has an entry in /W of size %i.\nThe maximum supported value is %i.\n", 
				lW[i], W_MAX_BYTES );

			PODOFO_RAISE_ERROR( ePdfError_InvalidXRefStream );
		}

		nData[i] = 0;
		for( z=W_MAX_BYTES-(int)lW[i];z<W_MAX_BYTES;z++ )
		{
			nData[i] = (nData[i] << 8) + static_cast<unsigned char>(*pBuffer);
			++pBuffer;
		}
	}

	if ((*m_pOffsets).size() == 0)
	{
		return;
	}

	//printf("OBJ=%i nData = [ %i %i %i ]\n", nObjNo, static_cast<int>(nData[0]), static_cast<int>(nData[1]), static_cast<int>(nData[2]) );
	(*m_pOffsets)[nObjNo].bParsed = true;
	switch( nData[0] ) // nData[0] contains the type information of this entry
	{
	case 0:
		// a free object
		(*m_pOffsets)[nObjNo].lOffset     = nData[1];
		(*m_pOffsets)[nObjNo].lGeneration = nData[2];
		(*m_pOffsets)[nObjNo].cUsed       = 'f';
		break;
	case 1:
		// normal uncompressed object
		(*m_pOffsets)[nObjNo].lOffset     = nData[1];
		(*m_pOffsets)[nObjNo].lGeneration = nData[2];
		(*m_pOffsets)[nObjNo].cUsed       = 'n';
		break;
	case 2:
		// object that is part of an object stream
		(*m_pOffsets)[nObjNo].lOffset     = nData[2]; // index in the object stream
		(*m_pOffsets)[nObjNo].lGeneration = nData[1]; // object number of the stream
		(*m_pOffsets)[nObjNo].cUsed       = 's';      // mark as stream
		break;
	default:
		{
			PODOFO_RAISE_ERROR( ePdfError_InvalidXRefType );
		}
	}
	//printf("m_offsets = [ %i %i %c ]\n", (*m_pOffsets)[nObjNo].lOffset, (*m_pOffsets)[nObjNo].lGeneration, (*m_pOffsets)[nObjNo].cUsed );
}