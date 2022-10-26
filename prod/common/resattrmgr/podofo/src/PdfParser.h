/***************************************************************************
 *   Copyright (C) 2005 by Dominik Seichter                                *
 *   domseichter@web.de                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _PDF_PARSER_H_
#define _PDF_PARSER_H_

#include "PdfDefines.h"
#include "PdfTokenizer.h"
#include "PdfVecObjects.h"

#define W_ARRAY_SIZE 3
#define W_MAX_BYTES  4

namespace PoDoFo {

typedef std::map<int,PdfObject*>    TMapObjects;
typedef TMapObjects::iterator       TIMapObjects;
typedef TMapObjects::const_iterator TCIMapObjects;

class PdfEncrypt;
class PdfString;

/**
 * PdfParser reads a PDF file into memory. 
 * The file can be modified in memory and written back using
 * the PdfWriter class.
 * Most PDF features are supported
 */
class PODOFO_API PdfParser : public PdfTokenizer {
    friend class PdfDocument;
    friend class PdfWriter;

 public:
    /** Create a new PdfParser object
     *  You have to open a PDF file using ParseFile later.
     *  \param pVecObjects vector to write the parsed PdfObjects to
     *
     *  \see ParseFile  
     */
    PdfParser( PdfVecObjects* pVecObjects );

    /** Create a new PdfParser object and open a PDF file and parse
     *  it into memory.
     *
     *  \param pVecObjects vector to write the parsed PdfObjects to
     *  \param pszFilename filename of the file which is going to be parsed
     *  \param bLoadOnDemand If true all objects will be read from the file at
     *                       the time they are accesed first.
     *                       If false all objects will be read immediately.
     *                       This is faster if you do not need the complete PDF 
     *                       file in memory.
     *
     *  This might throw a PdfError( ePdfError_InvalidPassword ) exception
     *  if a password is required to read this PDF.
     *  Call SetPassword with the correct password in this case.
     *  
     *  \see SetPassword
     */
    PdfParser( PdfVecObjects* pVecObjects, const char* pszFilename, bool bLoadOnDemand = true );

    /** Create a new PdfParser object and open a PDF file and parse
     *  it into memory.
     *
     *  \param pVecObjects vector to write the parsed PdfObjects to
     *  \param pBuffer buffer containing a PDF file in memory
     *  \param lLen length of the buffer containing the PDF file
     *  \param bLoadOnDemand If true all objects will be read from the file at
     *                       the time they are accesed first.
     *                       If false all objects will be read immediately.
     *                       This is faster if you do not need the complete PDF 
     *                       file in memory.
     *
     *  This might throw a PdfError( ePdfError_InvalidPassword ) exception
     *  if a password is required to read this PDF.
     *  Call SetPassword with the correct password in this case.
     *  
     *  \see SetPassword
     */
    PdfParser( PdfVecObjects* pVecObjects, const char* pBuffer, long lLen, bool bLoadOnDemand = true );

    /** Create a new PdfParser object and open a PDF file and parse
     *  it into memory.
     *
     *  \param pVecObjects vector to write the parsed PdfObjects to
     *  \param rDevice read from this PdfRefCountedInputDevice
     *  \param bLoadOnDemand If true all objects will be read from the file at
     *                       the time they are accesed first.
     *                       If false all objects will be read immediately.
     *                       This is faster if you do not need the complete PDF 
     *                       file in memory.
     *
     *  This might throw a PdfError( ePdfError_InvalidPassword ) exception
     *  if a password is required to read this PDF.
     *  Call SetPassword with the correct password in this case.
     *  
     *  \see SetPassword
     */
    PdfParser( PdfVecObjects* pVecObjects, const PdfRefCountedInputDevice & rDevice, 
               bool bLoadOnDemand = true );

    /** Delete the PdfParser and all PdfObjects
     */
    virtual ~PdfParser();

    /** Open a PDF file and parse it.
     *
     *  \param pszFilename filename of the file which is going to be parsed
     *  \param bLoadOnDemand If true all objects will be read from the file at
     *                       the time they are accesed first.
     *                       If false all objects will be read immediately.
     *                       This is faster if you do not need the complete PDF 
     *                       file in memory.
     *
     *
     *  This might throw a PdfError( ePdfError_InvalidPassword ) exception
     *  if a password is required to read this PDF.
     *  Call SetPassword with the correct password in this case.
     *  
     *  \see SetPassword
     */
    void ParseFile( const char* pszFilename, bool bLoadOnDemand = true );

    /** Open a PDF file and parse it.
     *
     *  \param pBuffer buffer containing a PDF file in memory
     *  \param lLen length of the buffer containing the PDF file
     *  \param bLoadOnDemand If true all objects will be read from the file at
     *                       the time they are accesed first.
     *                       If false all objects will be read immediately.
     *                       This is faster if you do not need the complete PDF 
     *                       file in memory.
     *
     *
     *  This might throw a PdfError( ePdfError_InvalidPassword ) exception
     *  if a password is required to read this PDF.
     *  Call SetPassword with the correct password in this case.
     *  
     *  \see SetPassword
     */
    void ParseFile( const  char* pBuffer, long lLen, bool bLoadOnDemand = true );

    /** Open a PDF file and parse it.
     *
     *  \param rDevice the input device to read from
     *  \param bLoadOnDemand If true all objects will be read from the file at
     *                       the time they are accesed first.
     *                       If false all objects will be read immediately.
     *                       This is faster if you do not need the complete PDF 
     *                       file in memory.
     *
     *
     *  This might throw a PdfError( ePdfError_InvalidPassword ) exception
     *  if a password is required to read this PDF.
     *  Call SetPassword with the correct password in this case.
     *  
     *  \see SetPassword
     */
    void ParseFile( const PdfRefCountedInputDevice & rDevice, bool bLoadOnDemand = true );

    /** Get a reference to the sorted internal objects vector.
     *  \returns the internal objects vector.
     */
    inline const PdfVecObjects* GetObjects() const;

    /** Get the file format version of the pdf
     *  \returns the file format version as enum
     */
    inline EPdfVersion GetPdfVersion() const;

    /** Get the file format version of the pdf
     *  \returns the file format version as string
     */
    const char* GetPdfVersionString() const;

    /** Get the trailer dictionary
     *  which can be written unmodified to a pdf file.
     */
    inline const PdfObject* GetTrailer() const;

    /** \returns true if this PdfParser loads all objects on demand at
     *                the time they are accessed for the first time.
     *                The default is to load all object immediately.
     *                In this case false is returned.
     */
    inline bool GetLoadOnDemand() const;

    /** \returns whether the parsed document contains linearization tables
     */
    bool IsLinearized() const { return m_pLinearization != NULL; }

    /** \returns the length of the file
     */
    size_t GetFileSize() const { return m_nFileSize; }

    /** 
     * \returns true if this PdfWriter creates an encrypted PDF file
     */
    bool GetEncrypted() const { return (m_pEncrypt != NULL); }

    /** 
     * \returns the parsers encryption object or NULL if the read PDF file was not encrypted
     */
    const PdfEncrypt* GetEncrypt() const { return m_pEncrypt; }

    /** 
     * Takes the encryption object fro mthe parser. The internal handle will be set
     * to NULL and the ownership of the object is given to the caller.
     *
     * Only call this if you need access to the encryption object
     * before deleting the parser.
     *
     * \returns the parsers encryption object or NULL if the read PDF file was not encrypted
     */
    inline PdfEncrypt* TakeEncrypt();
    

    /** If you try to open an encrypted PDF file, which requires
     *  a password to open, PoDoFo will throw a PdfError( ePdfError_InvalidPassword ) 
     *  exception. 
     *  
     *  If you got such an exception, you have to set a password
     *  which should be used for opening the PDF.
     *
     *  The usual way will be to ask the user for the password
     *  and set the password using this method.
     *
     *  PdfParser will immediately continue to read the PDF file.
     *
     *  \param sPassword a user or owner password which can be used to open an encrypted PDF file
     *                   If the password is invalid, a PdfError( ePdfError_InvalidPassword ) exception is thrown!
     */
    void SetPassword( const std::string & sPassword );

 protected:
    /** Searches backwards from the end of the file
     *  and tries to find a token.
     *  The current file is positioned right after the token.
     * 
     *  \param pszToken a token to find
     *  \param lRange range in bytes in which to search
     *                begining at the end of the file
     */
    void FindToken( const char* pszToken, const long lRange );

    /** Reads the xref sections and the trailers of the file
     *  in the correct order in the memory
     *  and takes care for linearized pdf files.
     */
    void ReadDocumentStructure();

    /** Checks wether this pdf is linearized or not.
     *  Initializes the linearization directory on sucess.
     */
    void HasLinearizationDict();

    /** Merge the information of this trailer object
     *  in the parsers main trailer object.
     *  \param pTrailer take the keys to merge from this dictionary.
     */
    void MergeTrailer( const PdfObject* pTrailer );

    /** Read the trailer directory at the end of the file.
     */
    void ReadTrailer();

    /** Looks for a startxref entry at the current file position
     *  and saves its byteoffset to pXRefOffset.
     *  \param pXRefOffset store the byte offset of the xref section into this variable.
     */
    void ReadXRef( long* pXRefOffset );

    /** Reads the xref table from a pdf file.
     *  If there is no xref table, ReadXRefStreamContents() is called.
     *  \param lOffset read the table from this offset
     *  \param bPositionAtEnd if true the xref table is not read, but the 
     *                        file stream is positioned directly 
     *                        after the table, which allows reading
     *                        a following trailer dictionary.
     */
    void ReadXRefContents( long lOffset, bool bPositionAtEnd = false );

    /** Read a xref subsection
     *  
     *  Throws ePdfError_NoXref if the number of objects read was not
     *  the number specified by the subsection header (as passed in
     *  `nNumObjects').
     *
     *  \param nFirstObject object number of the first object
     *  \param nNumObjects  how many objects should be read from this section
     */
    void ReadXRefSubsection( long & nFirstObject, long & nNumObjects );

    /** Reads a xref stream contens object
     *  \param lOffset read the stream from this offset
     *  \param bReadOnlyTrailer only the trailer is skipped over, the contents
     *         of the xref stream are not parsed
     */
    void ReadXRefStreamContents( long lOffset, bool bReadOnlyTrailer );

    void ReadXRefStreamEntry( char* pBuffer, long lLen, long lW[W_ARRAY_SIZE], int nObjNo );

    /** Reads all objects from the pdf into memory
     *  from the offsets listed in m_vecOffsets.
     *
     *  If required an encryption object is setup first.
     *
     *  The actual reading happens in ReadObjectsInternal()
     *  either if no encryption is required or a correct
     *  encryption object was initialized from SetPassword.
     */
    void ReadObjects();

    /** Reads all objects from the pdf into memory
     *  from the offsets listed in m_vecOffsets.
     *
     *  Requires a correctly setup PdfEncrypt object
     *  with correct password.
     *
     *  This method is called from ReadObjects
     *  or SetPassword.
     *
     *  \see ReadObjects
     *  \see SetPassword
     */
    void ReadObjectsInternal();

    /** Read the object with index nIndex from the object stream nObjNo
     *  and push it on the objects vector m_vecOffsets.
     *
     *  All objects are read from this stream and the stream object
     *  is free'd from memory. Further calls who try to read from the
     *  same stream simply do nothing.
     *
     *  \param nObjNo object number of the stream object
     *  \param nIndex index of the object which should be parsed
     *
     */
    void ReadObjectFromStream( int nObjNo, int nIndex );

    /** Checks the magic number at the start of the pdf file
     *  and sets the m_ePdfVersion member to the correct version
     *  of the pdf file.
     *
     *  \returns true if this is a pdf file, otherwise false
     */
    bool    IsPdfFile();

    void ReadNextTrailer();

 private:
    /** Free all internal data structures
     */
    void         Clear();

    /** Initializes all private members
     *  with their initial values.
     */
    void         Init();

    /** Small helper method to retrieve the document id from the trailer
     *
     *  \returns the document id of this PDF document
     */
    const PdfString & GetDocumentId();

 private:

    struct TXRefEntry {
        inline TXRefEntry() : lOffset(0), lGeneration(0), cUsed('\x00'), bParsed(false) { }
        long lOffset;
        long lGeneration;
        char cUsed;
        bool bParsed;
    };

    typedef std::vector<TXRefEntry>      TVecOffsets;
    typedef TVecOffsets::iterator        TIVecOffsets;
    typedef TVecOffsets::const_iterator  TCIVecOffsets;

    EPdfVersion   m_ePdfVersion;

    bool          m_bLoadOnDemand;

    long          m_nXRefOffset;
    long          m_nFirstObject;
    long          m_nNumObjects;
    long          m_nXRefLinearizedOffset;
    size_t        m_nFileSize;

    TVecOffsets m_offsets;
    PdfVecObjects* m_vecObjects;

    PdfObject*    m_pTrailer;
    PdfObject*    m_pLinearization;
    PdfEncrypt*   m_pEncrypt;

    bool          m_xrefSizeUnknown;

    std::set<int> m_setObjectStreams;
};

// -----------------------------------------------------
// 
// -----------------------------------------------------
bool PdfParser::GetLoadOnDemand() const
{
    return m_bLoadOnDemand;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
EPdfVersion PdfParser::GetPdfVersion() const
{
    return m_ePdfVersion;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
const PdfVecObjects* PdfParser::GetObjects() const
{
    return m_vecObjects;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
const PdfObject* PdfParser::GetTrailer() const
{
    return m_pTrailer;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
PdfEncrypt* PdfParser::TakeEncrypt() 
{ 
    PdfEncrypt* pEncrypt = m_pEncrypt;
    m_pEncrypt = NULL; 
    return pEncrypt; 
}

};

#endif // _PDF_PARSER_H_

