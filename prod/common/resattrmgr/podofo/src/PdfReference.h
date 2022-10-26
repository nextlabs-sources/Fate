/***************************************************************************
 *   Copyright (C) 2006 by Dominik Seichter                                *
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

#ifndef _PDF_REFERENCE_H_
#define _PDF_REFERENCE_H_

#include "PdfDefines.h"

#include "PdfDataType.h"

namespace PoDoFo {

class PdfOutputDevice;

/**
 * A reference is a pointer to a object in the PDF file of the form
 * "4 0 R", where 4 is the object number and 0 is the generation number.
 * Every object in the PDF file can be indetified this way.
 *
 * This class is a indirect reference in a PDF file.
 */
class PODOFO_API PdfReference : public PdfDataType {
 public:
    /**
     * Create a PdfReference with object number and generation number
     * initialized to 0.
     */
    PdfReference()
        : m_nObjectNo( 0 ), m_nGenerationNo( 0 )
    {
    }

    /**
     * Create a PdfReference to an object with a given object and generation number.
     *
     * \param nObjectNo the object number
     * \param nGenerationNo the generation number
     */
    PdfReference( const unsigned long nObjectNo, const pdf_uint16 nGenerationNo )
        : m_nObjectNo( nObjectNo ), m_nGenerationNo( nGenerationNo ) 
    {
    }

    /**
     * Create a copy of an existing PdfReference.
     * 
     * \param rhs the object to copy
     */
    PdfReference( const PdfReference & rhs ) : PdfDataType()
    {
        this->operator=( rhs );
    }

    PODOFO_NOTHROW virtual ~PdfReference() { }

    /** Convert the reference to a string.
     *  \returns a string representation of the object.
     *
     *  \see PdfVariant::ToString
     */
    const std::string ToString() const;

   /**
     * Assigne the value of another object to this PdfReference.
     *
     * \param rhs the object to copy
     */
    PODOFO_NOTHROW inline const PdfReference & operator=( const PdfReference & rhs );

    /** Write the complete variant to an output device.
     *  This is an overloaded member function.
     *
     *  \param pDevice write the object to this device
     */
    void Write( PdfOutputDevice* pDevice, const PdfEncrypt* pEncrypt = NULL) const;

    /** 
     * Compare to PdfReference objects.
     * \returns true if both reference the same object
     */
    PODOFO_NOTHROW inline bool operator==( const PdfReference & rhs ) const;

    /** 
     * Compare to PdfReference objects.
     * \returns false if both reference the same object
     */
    PODOFO_NOTHROW inline bool operator!=( const PdfReference & rhs ) const;

    /** 
     * Compare to PdfReference objects.
     * \returns true if this reference has a smaller object and generation number
     */
    PODOFO_NOTHROW inline bool operator<( const PdfReference & rhs ) const;

    /** Set the object number of this object
     *  \param o the new object number
     */
    PODOFO_NOTHROW inline void SetObjectNumber( unsigned long o );

    /** Get the object number.
     *  \returns the object number of this PdfReference
     */
    PODOFO_NOTHROW inline unsigned long ObjectNumber() const;

    /** Set the generation number of this object
     *  \param g the new generation number
     */
    PODOFO_NOTHROW inline void SetGenerationNumber( const pdf_uint16 g );

    /** Get the generation number.
     *  \returns the generation number of this PdfReference
     */
    PODOFO_NOTHROW inline pdf_uint16 GenerationNumber() const;

    /** Allows to check if a reference points to an indirect
     *  object.
     *
     *  A reference is indirect if object number and generation
     *  number are both not equal 0.
     *
     *  \returns true if this reference is the reference of
     *           an indirect object.
     */
    PODOFO_NOTHROW inline bool IsIndirect() const;

 private:
    unsigned long m_nObjectNo;
    pdf_uint16    m_nGenerationNo;
};

// -----------------------------------------------------
// 
// -----------------------------------------------------
const PdfReference & PdfReference::operator=( const PdfReference & rhs )
{
    m_nObjectNo     = rhs.m_nObjectNo;
    m_nGenerationNo = rhs.m_nGenerationNo;
    return *this;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
bool PdfReference::operator<( const PdfReference & rhs ) const
{
    return m_nObjectNo == rhs.m_nObjectNo ? m_nGenerationNo < rhs.m_nGenerationNo : m_nObjectNo < rhs.m_nObjectNo;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
bool PdfReference::operator==( const PdfReference & rhs ) const
{
    return ( m_nObjectNo == rhs.m_nObjectNo && m_nGenerationNo == rhs.m_nGenerationNo);
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
inline bool PdfReference::operator!=( const PdfReference & rhs ) const
{
    return !this->operator==( rhs );
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
void PdfReference::SetObjectNumber( unsigned long o )
{
    m_nObjectNo = o;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
unsigned long PdfReference::ObjectNumber() const
{
    return m_nObjectNo;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
void PdfReference::SetGenerationNumber( pdf_uint16 g )
{
    m_nGenerationNo = g;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
pdf_uint16 PdfReference::GenerationNumber() const
{
    return m_nGenerationNo;
}

// -----------------------------------------------------
// 
// -----------------------------------------------------
bool PdfReference::IsIndirect() const
{
    return !( !m_nObjectNo && !m_nGenerationNo );
}

};

#endif // _PDF_REFERENCE_H_


