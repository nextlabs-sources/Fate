// All sources, binaries and HTML pages (C) copyright 2008 by Blue Jungle Inc.,
// Redwood City CA, Ownership remains with Blue Jungle Inc,
// All rights reserved worldwide.
// Author : Saumitra Das
// Date   : 07/10/2006
// Note   : cm core user-mode functionality to use syscall interception.
//
// $Id$
//

#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/user.h>
#include <linux/unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "ce_syslistener.h"
#include "bj_cmd.h"

_syscall0(pid_t,gettid)

static int	ready = 0;
static pid_t	ourpid;
static pthread_t	thids[CM_NUM_THREADS]; 
static pid_t		tids[CM_NUM_THREADS];

static struct buf {
	char		capath[PATH_MAX+1];	/* tmp buffer area */
	char		tmpbuf[PATH_MAX+1];	/* tmp buffer area */
} tmp_data[CM_NUM_THREADS];

preqbuf_t	preq_infos = NULL;

static pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
static int pthreads_started = 0;

static void *cm_thread(void *data)
{
	response_data_t	rd;
	int		rval;
	int		thid = (int)data;
	preqbuf_t	prbuf = &preq_infos[thid];
	preqinfo_t	preq_info = &(prbuf->rb_ri);
	psc_arg_t	pargs = &(preq_info->ri_args);
	pid_t		mytid;
	policy_entry_t	pe;
	pid_t           watcherpid;
	/* Save our task id */
	tids[thid] = mytid = gettid();

	/* Set cached policy to allow all our syscalls */
	pe.pe_pid = mytid;
	pe.pe_hits = 0;
	pe.pe_allow_sc_bits = SC_BITMASK_ALL_POLICIES;
	pe.pe_user_cm_bits = 0LL;

	if (ioctl(sl_fd, IOCTL_ADD_POLICY, (char *)&pe) < 0)
		printf("Thread-%d(tid-%d): could not add policy for itself\n",
			thid, mytid);

	/* Increment count to tell parent that we are alive */
	pthread_mutex_lock(&count_mutex);
	pthreads_started++;
	pthread_mutex_unlock(&count_mutex);

	/* Wait for go signal from parent before starting to serve policy requests */
	while (ready <= 0)
		sched_yield();

	while (1) {
	    set_len_params(pargs, sizeof(prbuf->rb_buf));

	    if ((rval = ioctl(sl_fd, IOCTL_GET_REQUEST, (char *)thid)) == 0) {
		int nargs = get_num_params(pargs);
		/* Add the uid to the list */
		bj_uids_add(preq_info->ri_uid);
#undef  DEBUG
#ifdef	DEBUG
{
		printf("%d(Th-%d): Received request: req_tsk=%d, req_th=%d, syscall=%d, req_type=%c, uid=%d, handle=0x%x, #args=%d: ",
			mytid, thid,
			preq_info->ri_req_task,
			preq_info->ri_req_thread,
			preq_info->ri_sc_id,
			(preq_info->ri_req_type == SC_AT_ENTRY) ? 'E' : 'X',
			preq_info->ri_uid,
			preq_info->ri_handle,
			nargs
		      );

		if (nargs > 0) {
			if (is_param_type_str(pargs, 1))
				printf("arg1(s) = %s, ", pargs->sca_buf[0]);
			else
				printf("arg1(d) = %d, ", pargs->sca_buf[0]);
		}

		if (nargs > 1) {
			if (is_param_type_str(pargs, 2))
				printf("arg2(s) = %s, ", pargs->sca_buf[1]);
			else
				printf("arg2(d) = %d, ", pargs->sca_buf[1]);
		}

		if (nargs > 2) {
			if (is_param_type_str(pargs, 3))
				printf("arg3(s) = %s, ", pargs->sca_buf[2]);
			else
				printf("arg3(d) = %d, ", pargs->sca_buf[2]);
		}

		if (nargs > 3) {
			if (is_param_type_str(pargs, 4))
				printf("arg4(s) = %s, ", pargs->sca_buf[3]);
			else
				printf("arg4(d) = %d, ", pargs->sca_buf[3]);
		}

		if (nargs > 4) {
			if (is_param_type_str(pargs, 5))
				printf("arg5(s) = %s, ", pargs->sca_buf[4]);
			else
				printf("arg5(d) = %d, ", pargs->sca_buf[4]);
		}

		printf("\n");
}
#endif // DEBUG

		rd.rd_response = SC_ALLOW; /* Default: allow all requests */

#ifndef REMOVE_RESTRICTION
		/* Don't allow anyone to open or unlink our files. */
		/* NOTE: policy cache allows our exes to open/creat/unlnk file*/
		if (((preq_info->ri_sc_id == __NR_open) ||
		     (preq_info->ri_sc_id == __NR_creat) ||
		     (preq_info->ri_sc_id == __NR_unlink)) &&
		    (preq_info->ri_req_type == SC_AT_ENTRY)) {
			char *capath = tmp_data[thid].capath;
			if (get_absolute_path((char *)pargs->sca_buf[0], 
					preq_info->ri_req_task, capath,
					sizeof(tmp_data[thid].capath),
					tmp_data[thid].tmpbuf,
					sizeof(tmp_data[thid].tmpbuf)) < 0)
				capath = (char *)pargs->sca_buf[0];
			if (bj_is_bj_file(capath))
				rd.rd_response = SC_DENY;
		}
#endif // REMOVE_RESTRICTION

		rd.rd_req_handle = preq_info->ri_handle;

		if (preq_info->ri_sc_id == __NR_kill) {
			int	i   = 0;
			pid_t	pid = (pid_t)pargs->sca_buf[0];

#ifndef	REMOVE_RESTRICTION
			/* Don't allow killing our main process */
			if (ourpid == pid)
				rd.rd_response = SC_DENY;

			//jzhang 022107
			/* Don't allow killing our bj_kthread process */
			char kthreadname[256];
			char name[]="bj_kthread";
			int len = strlen(name)+1;
			int match=1;
			if(bj_pid_to_exename(pid,kthreadname,sizeof(kthreadname))>0 && strlen(kthreadname)>11)
			  {
			    printf("in policy.cpp, checking process for kill :%s\n",kthreadname);
			    for(i=0;i<len;i++)
			      {
				if(kthreadname[strlen(kthreadname)-i]!=name[len-i-1])
				  {
				    match = 0;
				    break;
				  }
			      }
			    if(match)
			      rd.rd_response = SC_DENY;
			  }
			  

			//jzhang 031307
			// Don't allow our watcher process being killed
			if(is_watcher(pid))
			  rd.rd_response = SC_DENY;

			/* Don't allow killing any of our threads */
			for (i=0; i < CM_NUM_THREADS; i++) {
				if (tids[thid] == pid)
					rd.rd_response = SC_DENY;
			}
#endif // REMOVE_RESTRICTION
			/* Only exception: bj executables can kill us */
			if (bj_pid_to_exename(preq_info->ri_req_task,tmp_data[thid].capath,sizeof(tmp_data[thid].capath)) > 0) {
			  //DEBUGG
			  //char str[1024];
			  //bj_pid_to_exename(preq_info->ri_req_task,str,sizeof(str));
			  //printf("KILLER pid = %d, name=%s \n\n\n",preq_info->ri_req_task,str);			
			  //printf("%s\n\n",tmp_data[thid].capath);

			  if (bj_is_bj_file(tmp_data[thid].capath)) {
			    /* TODO: Validate that we want to exit*/
			    rd.rd_response = SC_ALLOW;
			  }
			}
		}

#ifdef	DEBUG
		printf("%d(Th-%d): sending response %d\n", mytid, thid, rd.rd_response);
#endif // DEBUG
		ioctl(sl_fd, IOCTL_SEND_RESPONSE, &rd);
	    } else {
		break;
	    }
	}
#ifdef	DEBUG
	printf("%d(Th-%d): exiting; ioctl result is %d\n", mytid, thid, rval);
#endif
	//bj_policy_stop();

	pthread_exit(NULL);
} 

int bj_policy_init(void)
{
	int		i;
	policy_entry_t	pe;

	ourpid = getpid();

	/* Set cached policy to allow all our syscalls */
	pe.pe_pid = ourpid;
	pe.pe_hits = 0;
	pe.pe_allow_sc_bits = SC_BITMASK_ALL_POLICIES;
	pe.pe_user_cm_bits = 0LL;

	if (ioctl(sl_fd, IOCTL_ADD_POLICY, (char *)&pe) < 0) {
		printf("Policy addition for ourselves failed\n");
		ready = -1;
		return -1;
	}

	/* Get the request buffer mmap */
	preq_infos = (struct reqbuf *)mmap(0, CM_NUM_THREADS * PAGE_SIZE,
						PROT_READ | PROT_WRITE,
						MAP_FILE | MAP_SHARED,
						sl_fd,
						0);
        if(preq_infos == MAP_FAILED) {
                printf("mmap() failed; %s\n", strerror(errno));
                return -1;
        }

	/* Create children policy server threads */
	for (i=0; i < CM_NUM_THREADS; i++) {
		if (pthread_create(&thids[i], NULL, cm_thread, (void *)i) < 0) {
			printf("Can't create all the threads; exiting\n");
			ready = -1;
			return -1;
		}
	}

	/* Wait for all the children policy server threads to be started */
	while (pthreads_started != CM_NUM_THREADS)
		sched_yield();

	return 0;
}

int bj_policy_start()
{
	/* Activate policy checking by syslistener module */
	ioctl(sl_fd, IOCTL_SET_CM_READY, 1);

	/* Inform the children threads to start serving policy requests */
	ready = 1;

	return 0;
}

int bj_policy_stop()
{
	/* Deactivate policy checking by syslistener module */
	return ioctl(sl_fd, IOCTL_SET_CM_READY, 0);
}

void bj_policy_fini()
{
	int	i;

	/* Wait for all children threads to exit */
	for (i=0; i < CM_NUM_THREADS; i++) {
		pthread_join(thids[i], NULL);
	}

	if (preq_infos != NULL) {
		munmap((char *)preq_infos, CM_NUM_THREADS * PAGE_SIZE);
		preq_infos = NULL;
	}
}
