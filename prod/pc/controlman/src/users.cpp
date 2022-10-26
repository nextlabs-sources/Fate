// All sources, binaries and HTML pages (C) copyright 2008 by Blue Jungle Inc.,
// Redwood City CA, Ownership remains with Blue Jungle Inc,
// All rights reserved worldwide.
// Author : Saumitra Das
// Date   : 07/10/2006
// Note   : cm core user-mode functionality to use syscall interception.
//
// $Id$
//

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <syscall.h>
#include <sys/ioctl.h>
#include <sched.h>
#include <linux/unistd.h>
#include <errno.h>
#include <time.h>
#include <pwd.h>
#include "ce_syslistener.h"
#include "bj_cmd.h"

static uid_entry_t	uids_list[UIDS_MAX] = { { INVALID_UID, NULL, 0}, };
static int		uids_total = 0;

static pthread_rwlock_t uids_lock;

static int uids_find_index(uid_t uid)
{
	int	left = 0;
	int	right = uids_total - 1;

	while (left <= right) {
		int	mid      = ((right-left) >> 1) + left;
		uid_t	this_uid = uids_list[mid].ue_uid;

		if (this_uid == uid)
			return mid;

		if (uid < this_uid)
			right = mid - 1;
		else
			left = mid + 1;
	}

	return -1;
}

static int uids_check(uid_t uid)
{
	int	index;

        pthread_rwlock_rdlock(&uids_lock);
	index = uids_find_index(uid);
        pthread_rwlock_unlock(&uids_lock);
	return index;
}

static int uids_delete(uid_t uid)
{
	int	rc = 0;
	int	index;

        pthread_rwlock_wrlock(&uids_lock);
	index = uids_find_index(uid);
	if (index < 0) {
		rc = -1;
	} else {
		--uids_total;
		if (uids_list[index].ue_name)
			free(uids_list[index].ue_name);
		memmove(&uids_list[index+1], &uids_list[index],
			(uids_total-index) * sizeof(uid_entry_t));
	}
        pthread_rwlock_unlock(&uids_lock);
	return rc;
}

char *getuname(uid_t uid)
{
	struct passwd	*pwent;
	char		*name = NULL;

	if (pwent = getpwuid(uid))
		name = strdup(pwent->pw_name);

	return name;
}

int bj_uids_add(uid_t uid)
{
	int	left = 0;
	int	right;
	int	pos;

	/* NOTE: This routine is expected to be called for EVERY syscall policy  */
	/*       check. Thus, most of the times, the uid is expected to be found */
	/*       in the cache and relatively rarely a user will be added/deleted */
	/*       from uids list. Hence we optimize the check for presence of uid */
	/*       in the list, by performing the check under read lock first.     */
        pthread_rwlock_rdlock(&uids_lock);
	pos = uids_find_index(uid);
        pthread_rwlock_unlock(&uids_lock);
	if (pos >= 0) {
		/* Update the last usage/syscall time atomically */
		uids_list[pos].ue_ctime = time(NULL);
		return -1;
	}

	/* NOTE: pos is -1 from above */

        pthread_rwlock_wrlock(&uids_lock);

	right = uids_total - 1;
	while (left <= right) {
		uid_t	this_uid;

		pos = ((right-left) >> 1) + left;
		this_uid = uids_list[pos].ue_uid;

		if (this_uid == uid) {
			/* Attempt to add uid; update the timestamp */
			uids_list[pos].ue_ctime = time(NULL);
        		pthread_rwlock_unlock(&uids_lock);
			return -1;
		}

		if (uid < this_uid)
			right = pos - 1;
		else
			left = pos + 1;
	}

	if (pos < 0) {
		/* We are about to add the first element */
		pos = 0;
	} else {
		if (uid > uids_list[pos].ue_uid) {
			/* the new entry is added to position pos+1 */
			++pos;
		}
		/* move all the entries from pos onwards, one space down */
		memmove(&uids_list[pos+1], &uids_list[pos],
			(uids_total-pos) * sizeof(uid_entry_t));
	}
	++uids_total;
	uids_list[pos].ue_uid = uid;
	uids_list[pos].ue_name = getuname(uid);
	uids_list[pos].ue_ctime = time(NULL);
        pthread_rwlock_unlock(&uids_lock);
#ifdef	DEBUG2
	printf("Added user %s(id:%d) at time %d\n", uids_list[pos].ue_name, uid, uids_list[pos].ue_ctime);
#endif

	return 0;
}

static int uids_purge()
{
	int	i;

        pthread_rwlock_wrlock(&uids_lock);
	for (i=0; i < uids_total; i++)
		if (uids_list[i].ue_name)
			free(uids_list[i].ue_name);
	memset(&uids_list[0], uids_total * sizeof(uid_entry_t), 0);
	uids_total = 0;
        pthread_rwlock_unlock(&uids_lock);
	return 0;
}

int bj_uids_init()
{
  pthread_rwlock_init(&uids_lock, NULL);
}

void bj_uids_fini()
{
        pthread_rwlock_destroy(&uids_lock);
}
