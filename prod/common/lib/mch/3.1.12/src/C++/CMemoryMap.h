// ***************************************************************
//  CMemoryMap.h              version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  getting an overview over the current memory range
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _CMEMORYMAP_H
#define _CMEMORYMAP_H

#ifndef _CCOLLECTION_H
	#include "CCollection.h"
#endif

typedef struct tagMemoryArea
{
	LPVOID AreaBegin;
	LPVOID AreaEnd;
	BOOL Readable;
} MEMORY_AREA;

class SYSTEMS_API CMemoryMap
{
   public:
      // Constructs a memory map of creating process
      CMemoryMap();
      // Destructs memory map
      ~CMemoryMap();
      // Returns a count of memory areas
      int GetCount() const;
      // Returns a Memory Area structure
	   const MEMORY_AREA& operator[] (int index) const;
      // Returns TRUE of given memory pointer is readable
      BOOL CheckTryRead( LPCVOID pMem, int length ) const;

      static BOOL TryRead( LPCVOID source, LPVOID destination, DWORD count, CMemoryMap *pMemoryMap=NULL );
      static BOOL TryWrite( LPCVOID source, LPVOID destination, DWORD count, CMemoryMap *pMemoryMap=NULL );

   protected:
      CCollection< MEMORY_AREA, CStructureEqualHelper<MEMORY_AREA>  > mMemoryAreas;
};

#endif
