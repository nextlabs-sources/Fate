/*____________________________________________________________________________
	Copyright 1991 Lloyd Chambers
	Rights granted to PGP to use this source code freely and
	without restriction.

	This file should be #included, and the following symbols defined:
	
	HeapSortName
	HeapSortItem
	
	For example:
	#define HeapSortName	MySortName
	#define HeapSortItem	MyStructName
	#include "pgpHeapSort.h"
	#undef HeapSortName
	#undef HeapSortItem
	
	$Id$
____________________________________________________________________________*/


/*____________________________________________________________________________
	This function is declared static on purpose. Place it in a C file and
	write a wrapper if you want to make it visible.
____________________________________________________________________________*/
	static void
HeapSortName(
	HeapSortItem *		start,
	size_t				numItems,
	int 				(*compareFunc)( HeapSortItem *item1,
							HeapSortItem *item2, void *userValue ),
	void *				userValue
	)
{
	HeapSortItem	*i;
	HeapSortItem	*j;
	HeapSortItem	*right;
	HeapSortItem	*left;
	HeapSortItem	key;
	HeapSortItem *	stop;

	if ( numItems == 0 )
		return;
	stop	= start + (numItems - 1);
	if ( stop - start  < 1)
		return;
		
H1:
	right = stop;
	left =  start + (stop-start)/2 + 1;

H2:
	if ( left > start )
	{
		--left;
		key = *left;
	}
	else
	{
		key = *right;
		*right	= *start;
		--right;
		if ( right == start )
		{
			*start	= key;
			return;	/* all done */
		}
	}

H3:
	j = left;
H4:
	i = j;
	j += j - start + 1;	/* j = 2*j */
	
	if ( j < right )
		goto H5;
	if ( j == right )
		goto H6;
	if ( j > right )
		goto H8;
	
H5:	if ( compareFunc( j, j + 1, userValue) < 0 )
		++j;
H6:
	if ( compareFunc( &key, j, userValue) >= 0)
		goto H8;
	
H7:
	*i	= *j;
	goto H4;

H8:
	*i	= key;
	goto H2;
}



