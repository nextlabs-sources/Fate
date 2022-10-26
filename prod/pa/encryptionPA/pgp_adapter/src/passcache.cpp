/* passcache.c - passphrase cache for GPGol
 *	Copyright (C) 2005 g10 Code GmbH
 *
 * This file is part of GPGol.
 * 
 * GPGol is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * GPGol is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/* We use a global passphrase cache.  The cache time is set at the
   time the passphrase gets stored.  */

#include "stdafx.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "util.h"
#include "log.h"
#include "passcache.h"


/* An item to hold a cached passphrase. */
struct cache_item
{
  /* We love linked lists; there are only a few passwords and access
     to them is not in any way time critical. */
  struct cache_item *next;  

  /* The Time to Live for this entry. */
  unsigned long ttl;

  /* The timestamp is updated with each access to the item and used
     along with TTL to exire this item. */
  time_t timestamp;

  /* The value of this item.  Malloced C String.  If this one is NULL
     this item may be deleted. */
  TCHAR *value;         

  /* The key for this item. C String. */
  TCHAR key[1];
};
typedef struct cache_item *cache_item_t;


/* The actual cache is a simple list anchord at this global
   variable. */
static cache_item_t the_cache = NULL;

/* Mutex used to serialize access to the cache. */
static LockHandle cache_mutex = NULL;


/* Initialize this mode.  Called at a very early stage. Returns 0 on
   success. */
int
passcache_initialize (void)
{
	if (the_cache)
	{
		return 0;
	}
	
	the_cache = NULL;

	cache_mutex = GsmLockAlloc();

	return cache_mutex? 0 : -1;
}

/* This is routine is used to remove all deleted entries from the
   linked list.  Deleted entries are marked by a value of NULL.  Note,
   that this routine must be called in a locked state. */
static void remove_deleted_items (void)
{
	cache_item_t item, prev;

again:
	for (item = the_cache; item; item = item->next)
		if (!item->value)
		{
			if (item == the_cache)
			{
				the_cache = item->next;
				free (item);
			}
			else
			{
				for (prev=the_cache; prev->next; prev = prev->next)
					if (prev->next == item)
					{
						prev->next = item->next;
						free (item);
						item = NULL;
						break;
					}
				assert (!item);
			}
			goto again; /* Yes, we use this pretty dumb algorithm ;-) */
		}
}



/* Flush all entries from the cache. */
void passcache_flushall (void)
{
	cache_item_t item;

	if (GsmLock(cache_mutex))
		return; /* FIXME: Should we pop up a message box? */ 

	for (item = the_cache; item; item = item->next)
		if (item->value)
		{
			free (item->value);
			item->value = NULL;
		}
	remove_deleted_items ();

	GsmUnlock(cache_mutex);
	GsmLockFree(cache_mutex);
	cache_mutex = NULL;
}


/* Store the passphrase in VALUE under KEY in out cache. Assign TTL
   seconds as maximum caching time.  If it already exists, merely
   updates the TTL. If the TTL is 0 or VALUE is NULL or empty, flush a
   possible entry. */
void
passcache_put (LPCTSTR key, LPCTSTR value, int ttl)
{
	cache_item_t item;

	if (!key || !*key)
	{
		DPA (("%s: no key given", __FUNCTION__));
		return;
    }

	if (GsmLock(cache_mutex))
		return; /* FIXME: Should we pop up a message box if a flush was
               requested? */
  
	for (item = the_cache; item; item = item->next)
		if (item->value && !wcscmp (item->key, key))
			break;

	if (item && (!ttl || !value || !*value))
    {
		/* Delete this entry. */
		free (item->value);
		item->value = NULL;
		/* Actual delete will happen before we allocate a new entry. */
    }
	else if (item)
    {
		/* Update this entry. */
		if (item->value)
        {
			free (item->value);
        }
		item->value = _wcsdup (value);
		item->ttl = ttl;
		item->timestamp = time (NULL);
    }
	else if (!ttl || !value || !*value)
    {
		DP ((_T("ignoring attempt to add empty entry `%s'"),
                key));
    }
	else 
    {
		/* Create new cache entry. */
		remove_deleted_items ();
		item = (cache_item_t)calloc (1, sizeof *item + wcslen(key)*sizeof(TCHAR));
		wcsncpy_s (item->key, wcslen(key)+1, key, _TRUNCATE);
		item->ttl = ttl;
		item->value = _wcsdup (value);
		item->timestamp = time (NULL);

		item->next = the_cache;
		the_cache = item;
    }

	GsmUnlock(cache_mutex);
}


/* Return the passphrase stored under KEY as a newly malloced string.
   Caller must release that string using xfree.  Using this function
   won't update the TTL.  If no passphrase is available under this
   key, the function returns NULL.  Calling thsi function with KEY set
   to NULL will only expire old entries. */
LPCTSTR
passcache_get (LPCTSTR key)
{
	cache_item_t item;
	LPTSTR result = NULL;
	time_t now = time (NULL);

	if (GsmLock(cache_mutex))
		return NULL;
  
	/* Expire entries. */
	for (item = the_cache; item; item = item->next)
		if (item->value && (unsigned long)(now - item->timestamp) > item->ttl)
		{
			free (item->value);
			item->value = NULL;
		}

	/* Look for the entry. */
	if (key && *key)
    {
		for (item = the_cache; item; item = item->next)
			if (item->value && !wcscmp (item->key, key))
			{
				result = _wcsdup (item->value);
				item->timestamp = time (NULL);
				break;
			}
    }

	GsmUnlock(cache_mutex);

	return result;
}
