/*____________________________________________________________________________
	pgpLeaks.h
	
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	Leak tracking module for memory allocation routines

	$Id$
____________________________________________________________________________*/
#ifndef Included_pgpLeaks_h	/* [ */
#define Included_pgpLeaks_h

#include "pgpTypes.h"
#include "pgpDebug.h"

#ifndef USE_PGP_LEAKS
#define USE_PGP_LEAKS		PGP_DEBUG
#endif

PGP_BEGIN_C_DECLARATIONS

#if USE_PGP_LEAKS	/* [ */

/*
 * WARNING: The leaks library is currently not thread-safe
 */

#define kLeakDeallocTypeMagic	0xD073A915
#define kLeakAllocTypeMagic		0xE19C834B
#ifndef kLeakItemMagic
#define kLeakItemMagic			0xB74D951F
#endif

typedef struct LeakDeallocType_ const	LeakDeallocType;
struct LeakDeallocType_
{
	PGPUInt32			magic;
	PFLChar const *		name;
};

typedef struct LeakAllocType_ const	LeakAllocType;
struct LeakAllocType_
{
	PGPUInt32			magic;
	PFLChar const *		name;
	LeakDeallocType *	deallocType;
};

#define LeakStructName(name)	name ## __struct

#define DECLARE_LEAKDEALLOCSTRUCT(varName, nameStr)				\
		LeakDeallocType LeakStructName(varName) = {				\
							kLeakDeallocTypeMagic,				\
							nameStr }

#define DECLARE_LEAKDEALLOCTYPE(varName, nameStr)				\
		DECLARE_LEAKDEALLOCSTRUCT(	varName, nameStr),			\
						*varName = &LeakStructName(varName)

#define DECLARE_LEAKALLOCTYPE(varName, nameStr, deallocType)	\
		LeakAllocType LeakStructName(varName) = {				\
							kLeakAllocTypeMagic,				\
							nameStr,							\
							&LeakStructName(deallocType) },		\
						*varName = &LeakStructName(varName)

/*
 * The LeakItems form a circularly-linked list.
 * Each session has its own list, each with its own dummy element.
 */
typedef struct LeakItem_		LeakItem;
struct LeakItem_
{
	PGPUInt32		magic;
	LeakAllocType *	allocType;
	PGPSize			size;
	PGPBoolean		ignore;
	PGPBoolean		inSessionList;
	PGPBoolean		pad1;
	PGPBoolean		pad2;
	LeakItem *		prev;
	LeakItem *		next;
	PFLChar const *	fileName;
	long			lineNumber;
	void const *	memory;
	PGPUInt32		allocationNumber;
};

#define pgpaLeakDeallocTypeValid(type)										\
		pgpaFailIf((type) == NULL || (type)->magic != kLeakDeallocTypeMagic,\
					(pgpaFmtPrefix, PFLTXT_DEBUG("Invalid LeakDeallocType magic")))
#define pgpaLeakAllocTypeValid(type)										\
		pgpaFailIf((type) == NULL || (type)->magic != kLeakAllocTypeMagic,	\
					(pgpaFmtPrefix, PFLTXT_DEBUG("Invalid LeakAllocType magic")))
#define pgpaLeakItemValid(item)												\
		pgpaFailIf((item) == NULL || (item)->magic != kLeakItemMagic,		\
					(pgpaFmtPrefix, PFLTXT_DEBUG("Invalid LeakInfo magic")))


void 	pgpLeaksBeginSession(PFLChar const *sessionName);
void 	pgpLeaksEndSession(void);

void 	pgpLeaksRememberItem(LeakItem *item, LeakAllocType *type,
									void const *memory, PGPSize size,
									PFLChar const *fileName,
									long lineNumber);
void 	pgpLeaksIgnoreItem(LeakItem *item);
void 	pgpLeaksIgnoreMemory( const void * memory);
void 	pgpLeaksForgetItem(LeakItem *item, LeakDeallocType *type);

LeakItem  *	pgpLeaksFindItem(void const *memory);

/*
 * Call this whenever a LeakItem moves in memory.  <oldItem> will not be
 * referenced and need not contain any valid data.  It need only indicate
 * where the item used to be in memory.
 */
void 	pgpLeaksNotifyItemMoved(LeakItem const *oldItem,
										LeakItem *newItem);

void 	pgpLeaksSuspend(void);
void 	pgpLeaksResume(void);

#else /* ] USE_PGP_LEAKS [ */

#define pgpLeaksBeginSession(name)
#define pgpLeaksEndSession()
#define pgpLeaksRememberItem(item, type, memory, fileName, lineNumber)
#define pgpLeaksIgnoreItem(item)
#define pgpLeaksIgnoreMemory(memory)
#define pgpLeaksForgetItem(item, type)
#define pgpLeaksSuspend()
#define pgpLeaksResume()

#endif /* ] USE_PGP_LEAKS */

PGP_END_C_DECLARATIONS


#endif /* ] Included_pgpLeaks_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
