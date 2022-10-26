/*
 * pgpLists.h -- Interfaces for Lists of Objects
 *
 * $Id$
 */

#ifndef Included_PGPCmdlnLib_pgpLists_h
#define Included_PGPCmdlnLib_pgpLists_h

#include "pgpBase.h"
#include <stdio.h>  /* For NULL */

PGP_BEGIN_C_DECLARATIONS

/**
 * Generic List Objects.   This file contains macros to help
 * define the basic function prototypes for each list type.
 */

/** A Macro to declare the basic data type and common
 *  functions of a particular List object.
 */
#define PGPLIST_DECL(tname,c,dtype)											\
/** A list structure for tname. */											\
typedef struct {															\
  dtype *	elements; /**< The actual list contents */						\
  PGPUInt32	length;   /**< The number of elements in the list */			\
  PGPUInt32	allocSize; /**< The allocated size of the list (>= length) */	\
  /** Users can set the following items to control list operation */		\
  PGPBoolean	bSecure; /**< Whether to use a secure allocator */			\
  void	(*freeFunc)(dtype); /**< Function to free "Pointer" members */		\
} PGP##tname##List;															\
																		\
/** Returns TRUE if the list pointer is NULL or empty (length == 0) */	\
static PGP_INLINE_FUNC PGPBoolean PGP##tname##ListIsEmpty(				\
	PGP##tname##List *		pList) 										\
  {																		\
	return ((pList == NULL) || (pList->length == 0));					\
  }																		\
/** Resize the list (and preallocate space)								\
 *  note that this could leak space if you resize pointer lists smaller */ \
PGPError PGP##tname##ListResize(										\
	PGP##tname##List *		pList,										\
	PGPUInt32			size);											\
/** Add an element to the list (at the end) */							\
PGPError PGP##tname##ListAdd(											\
	PGP##tname##List *		pList,										\
	c dtype				element);										\
/** Insert an element into the list at 'offset' (which must be < length) */ \
PGPError PGP##tname##ListInsert(										\
	PGP##tname##List *		pList,										\
	PGPUInt32			offset,											\
	c dtype				element);										\
/** Combine the source list into the pList. */							\
PGPError PGP##tname##ListCombine(										\
	PGP##tname##List *		pList,										\
	const PGP##tname##List *	source);								\
/** Remove the element from the list (using the value of element) */	\
PGPError PGP##tname##ListRemove(										\
	PGP##tname##List *		pList,										\
	c dtype				element);										\
/** Remove the element at 'offset' in the list */						\
PGPError PGP##tname##ListRemoveAt(										\
	PGP##tname##List *		pList,										\
	PGPUInt32			offset);										\
/** Clear all the elements out of this list */							\
PGPError PGP##tname##ListClear(											\
	PGP##tname##List *		pList);

/** A Macro to declare a basic initialization function */
#define PGPLIST_DECL_INIT(tname,dtype)			\
/** Initialize a tname list */					\
PGPError PGP##tname##ListInitialize(			\
	PGP##tname##List *	pList,					\
	PGPBoolean		bSecure);

/** A Macro to declare a basic free function that destroys the list */
#define PGPLIST_DECL_FREE(tname,dtype)			\
/** Free a tname list */						\
PGPError PGP##tname##ListFree(					\
	PGP##tname##List *		pList);

/** A Macro to declare the basic type and all the basic functions,
 *  initialization, and free functions.
 */
#define PGPLIST_DECL_FULL(tname,dtype)			\
PGPLIST_DECL(tname, const, dtype)				\
PGPLIST_DECL_INIT(tname, dtype)					\
PGPLIST_DECL_FREE(tname, dtype)

/*==================================================================*/
/** LIST Declarations start here */

/** String Lists 
 *  The Add and Insert functions will copy the strings into the
 *  list.  The Remove functions will NOT free the strings.  The
 *  Free() function, however, WILL free all the strings.
 */
PGPLIST_DECL_FULL(String, PGPChar*);

/** This removes a string from the list by comparing at the actual
 *  string contents rather that looking at the string pointers.
 */
PGPError PGPStringListRemoveCompare(
	PGPStringList*	pList,
	const PGPChar*	pElement,
	PGPBoolean	bIgnoreCase);

/** Comma Lists */

/** Convert a StringList to a Comma List.
 *
 *      Parameters:
 *              pList      - string list to convert (can be empty but not NULL)
 *              pCommaList - pointer to receive the new comma list
 *                      This must be freed by the caller with PGPFreeData
 *
 *      Return Values:
 *              PGPError - unexpected error
 */
PGPError PGPCommaListFromStringList(
	PGPStringList*	pList,
	PGPChar**	pCommaList);

/** Fill in a StringList from the contents of a Comma List 
 *
 *      Notes:
 *              Whitespace around delimiters is stripped
 *              Valid lists - "", "a", "a,b", "a, b, c"
 *
 *      Parameters:
 *              commaList - string containing a list of comma separated elements *                      List may contain zero, one, or more than one elements
 *              pList     - pointer to receive the new string list
 *                      Must be freed by the caller with PGPStringListFree()
 *
 *      Return Value:
 *              PGPError - unexpected error
 */
PGPError PGPCommaListToStringList(
	PGPChar*	commaList,
	PGPStringList*	pList);

/** Pointer Lists. */
PGPLIST_DECL(Pointer, , void*);
PGPLIST_DECL_FREE(Pointer, void*);

/** Initialize a Pointer List.  Supply the function to free the pointers  */
PGPError PGPPointerListInitialize(
	PGPPointerList*	pList,
	void (*freeFunc)(void*));

/** Integer Lists */
PGPLIST_DECL_FULL(Int16, PGPInt16);
PGPLIST_DECL_FULL(UInt16, PGPUInt16);
PGPLIST_DECL_FULL(Int32, PGPInt32);
PGPLIST_DECL_FULL(UInt32, PGPUInt32);
PGPLIST_DECL_FULL(Int64, PGPInt64);
PGPLIST_DECL_FULL(UInt64, PGPUInt64);

PGP_END_C_DECLARATIONS

#endif /* Included_PGPCmdlnLib_pgpLists_h */
 
/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
