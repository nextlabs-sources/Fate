// All sources, binaries and HTML pages (C) copyright 2008 by Blue Jungle Inc.,
// Redwood City CA, Ownership remains with Blue Jungle Inc,
// All rights reserved worldwide.
// Author : Saumitra Das
// Date   : 07/10/2006
// Note   : cm core user-mode functionality to use syscall interception.
//
// Migrated from $Id$
// Migrated on 08/23/2006 by Jack Zhang
// $Id$
//

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
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
#include <string.h>
#include "ce_syslistener.h"
#include "bj_cmd.h"

int	sl_fd = -1;

/*
_syscall0(pid_t,gettid)

static int	ready = 0;
static pid_t	ourpid;
pthread_t	thids[CM_NUM_THREADS]; 
pid_t		tids[CM_NUM_THREADS];

struct reqbuf {
	reqinfo_t	ri;
	char		buf[MAX_ARG_BUF_SZ];
	char		capath[PATH_MAX+1];	// tmp buffer area 
	char		tmpbuf[PATH_MAX+1];	// tmp buffer area 
} req_infos[CM_NUM_THREADS];

pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
int pthreads_started = 0;
*/


//JZhang 091306, all util functions have been moved to util.cpp
/*
 * Determine if filename is a BJ specific file. 
 * Returns 1 if true; else 0 
 * Assumes, filename is a canonical path
 * Depends on BJ_ROOT environment variable
 */
/*
int bj_is_bj_file(char *filename)
{
	char	*BJ_ROOT;
	int	len;

	if ((BJ_ROOT = getenv("BJROOT")) == NULL)
		BJ_ROOT = BJ_ROOT_DEFAULT;

	len = strlen(BJ_ROOT);

	while ((len > 0) && (BJ_ROOT[len-1] == '/'))
		len--;

	if ((strncmp(filename, BJ_ROOT, len) == 0) &&
		((filename[len] == '/') || (filename[len] == '\0')))
		return 1;

	if (strcmp(filename, BJ_STARTUP_SCRIPT) == 0)
		return 1;

	return 0;
}

*/

/*
 * Get absolute canonical path for an input filename for process pid
 * Returns 0 on success; -1 on failure.
 * Uses two input buffers (should be of len PATH_MAX) for path manipulation
 * Depends on /proc/<pid>/cwd file
 */
/*
int get_absolute_path(char *path, pid_t pid, char *obuf, int obufsz,
		      char *tbuf, int tbufsz)
{
	char *fpath;
	char *rc;
	int  len;

	if (path[0] != '/') {
		// First concatanate 'pwd' to make path absolute 
		sprintf(obuf, "/proc/%d/cwd", pid);
		if ((len = readlink(obuf, tbuf, tbufsz-1)) < 0)
			return -1;
		tbuf[len] = '/';
		tbuf[++len] = '\0';
		strncat(tbuf, path, tbufsz - len);
		tbuf[tbufsz-1] = '\0';
		fpath = tbuf;
	} else
		fpath = path;

	rc = realpath(fpath, obuf);

	if (rc == NULL) {
	  // We could not find the file; it is either a file to be created or bad name 
	  // We try to traverse the path backwards till we find a valid 'basename' 
		int i = strlen(fpath);

		// First skip all trailing '/'s
		while ((i > 0) && (fpath[i-1] == '/'))
			--i;

		// Try repeatatively to break the complete pathname, into the
		//   longest basename for which we can get realpath and then 
		//   append the retmaining filename to the realpath'ed basename 
		while (1) {
			while ((--i > 0) && (fpath[i] != '/'));

			// We are guaranteed, fpath[0] = '/'; see above 
			if (i <= 0)
				break;
			// fpath[i] = '/' and separates the basename and filename 
			fpath[i] = '\0';
			rc = realpath(fpath, obuf);
			fpath[i] = '/';

			if (rc != NULL) {
				len = strlen(obuf);
				strncat(obuf, &fpath[i], (obufsz - len));
				obuf[obufsz-1] = '\0';
				return 0;
			}
		}
		// We can't get realname for the full pathname; pass it as it is 
		// TODO: Can we do anything better? 
		strcpy(obuf, fpath);
	}

	return 0;
}

int bj_pid_to_exename(pid_t pid, char *buf, int len)
{
	char tbuf[32];
	int  rc;

	sprintf(tbuf, "/proc/%d/exe", pid);
	rc = readlink(tbuf, buf, len-1);
	if (rc >= 0)
		buf[rc] = '\0';
	else
		buf[0] = '\0';

	return rc;
}
*/



/*
void *cm_thread(void *data)
{
	response_data_t	rd;
	int		rval;
	int		thid = (int)data;
	struct reqbuf	*prbuf = &req_infos[thid];
	preqinfo_t	preq_info = &(prbuf->ri);
	psc_arg_t	pargs = &(preq_info->ri_args);
	pid_t		mytid;
	policy_entry_t	pe;

	// Save our task id 
	tids[thid] = mytid = gettid();

	// Set cached policy to allow all our syscalls 
	pe.pe_pid = mytid;
	pe.pe_hits = 0;
	pe.pe_allow_sc_bits = SC_BITMASK_ALL;
	pe.pe_user_cm_bits = 0LL;

	if (ioctl(fd, IOCTL_ADD_POLICY, (char *)&pe) < 0)
		printf("Thread-%d(tid-%d): could not add policy for itself\n",
			thid, mytid);

	// Increment count to tell parent that we are alive 
	pthread_mutex_lock(&count_mutex);
	pthreads_started++;
	pthread_mutex_unlock(&count_mutex);

	// Wait for go signal from parent before starting to serve policy requests 
	while (ready <= 0)
		sched_yield();

	while (1) {
	    set_len_params(pargs, sizeof(prbuf->buf));

	    if ((rval = ioctl(fd, IOCTL_GET_REQUEST, (char *)preq_info)) == 0) {
		int nargs = get_num_params(pargs);

#ifdef	DEBUG
		printf("%d(Th-%d): Received request: requester=%d, syscall=%d, req_type=%c, uid=%d, handle=0x%x, #args=%d: ",
			mytid, thid,
			preq_info->ri_requester,
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
#endif // DEBUG

		rd.rd_response = SC_ALLOW; // Default: allow all requests 

		// Don't allow anyone to open or unlink our files. 
		// NOTE: policy cache allows our exes to open/creat/unlnk file
		if ((((preq_info->ri_sc_id == __NR_open) &&
		      (pargs->sca_buf[1] & (O_WRONLY|O_RDWR|O_CREAT|O_APPEND))) ||
		     (preq_info->ri_sc_id == __NR_creat) ||
		     (preq_info->ri_sc_id == __NR_unlink)) &&
		    (preq_info->ri_req_type == SC_AT_ENTRY)) {
			char *capath = req_infos[thid].capath;
			if (get_absolute_path((char *)pargs->sca_buf[0], 
					      preq_info->ri_requester, capath,
					      sizeof(req_infos[thid].capath),
					      req_infos[thid].tmpbuf,
					      sizeof(req_infos[thid].tmpbuf)) < 0)
			  capath = (char *)pargs->sca_buf[0];
			if (bj_is_bj_file(capath))
			  rd.rd_response = SC_DENY;
		}

		rd.rd_req_handle = preq_info->ri_handle;

		if (preq_info->ri_sc_id == __NR_kill) {
			int	i   = 0;
			pid_t	pid = (pid_t)pargs->sca_buf[0];

			// Don't allow killing our main process 
			if (ourpid == pid)
				rd.rd_response = SC_DENY;
			// Don't allow killing any of our threads
			for (i=0; i < CM_NUM_THREADS; i++) {
				if (tids[thid] == pid)
					rd.rd_response = SC_DENY;
			}
			
			// Only exception: bj executables can kill us 
			if (bj_pid_to_exename(preq_info->ri_requester,
					req_infos[thid].capath,
					sizeof(req_infos[thid].capath)) > 0) {
				if (bj_is_bj_file(req_infos[thid].capath)) {
				  // TODO: Validate that we want to exit
					rd.rd_response = SC_ALLOW;
				}
			}
		}

#ifdef	DEBUG
		printf("%d(Th-%d): sending response %d\n", mytid, thid, rd.rd_response);
#endif // DEBUG
		ioctl(fd, IOCTL_SEND_RESPONSE, &rd);
	    } else {
		break;
	    }
	}

	pthread_exit(NULL);
} 
*/

void sighandler(int sig)
{
  //ioctl(fd, IOCTL_SET_CM_READY, 0);
  bj_policy_stop();

  char		*BJ_ROOT;
  if ((BJ_ROOT = getenv("RUNTIMEROOT")) == NULL)
    BJ_ROOT = BJ_ROOT_DEFAULT;

  //jzhang 031307  Failover feature
  // Normally, this lock file will be removed by enforcerStopper, here we do it for debug mode
  char lckfile[256];
  _snprintf_s(lckfile,256, _TRUNCATE, "%s/%s",BJ_ROOT,BJ_LCK_FILE);
  FILE *fp = fopen(lckfile,"r");
  if(fp)
    {
      //pid file exists, remove it
      fclose(fp);
      unlink(lckfile);
    }
  
#ifdef DEBUG
  printf("Exiting\n");
#endif // DEBUG
  exit(0);
}

void startShield()
{
  /* Change current working directory */
  char	*BJ_ROOT;
  
  if ((BJ_ROOT = getenv("BJROOT")) == NULL)
    BJ_ROOT = BJ_ROOT_DEFAULT;
  
  if ((chdir(BJ_ROOT)) < 0)
    exit(EXIT_FAILURE);
  
  //ourpid = getpid();
  sl_fd = open(BJ_DEV_FILE,  O_RDWR | O_SYNC);
  if (sl_fd < 0) {
    printf("Can't open device file: %s\n", BJ_DEV_NAME);
    //ready = -1;
    exit(EXIT_FAILURE);
  }
  
  /* Set handler for shutting down CM on reciving SIGINT or SIGTERM */
  signal(SIGINT, sighandler);
  signal(SIGTERM, sighandler);
  
  if (bj_policy_init() ||
      bj_uids_init() )
    exit(EXIT_FAILURE);
  
  bj_policy_start();
  
  /* NOTE: Currently, the exit is only from the signal handler */
  /*       bj_policy_fini() keeps waiting for policy threads to exit */
  bj_policy_fini();
  bj_uids_fini();
  
  close(sl_fd);
  exit(EXIT_SUCCESS);
  

  //JZhang 091306, comment out for Saumitra's updated code
  
  /* Set cached policy to allow all our syscalls */
  //pe.pe_pid = ourpid;
  //pe.pe_hits = 0;
  //pe.pe_allow_sc_bits = SC_BITMASK_ALL;
  //pe.pe_user_cm_bits = 0LL;
  
  //if (ioctl(fd, IOCTL_ADD_POLICY, (char *)&pe) < 0) {
  //	printf("Policy addition for ourselves failed\n");
  //	ready = -1;
  //	exit(EXIT_FAILURE);
  //}
  
  /* Create children policy server threads */
  //for (i=0; i < CM_NUM_THREADS; i++) {
  //	if (pthread_create(&thids[i], NULL, cm_thread, (void *)i) < 0) {
  //		printf("Can't create all the threads; exiting\n");
  //		ready = -1;
  //		exit(EXIT_FAILURE);
  //	}
  //}
  
  /* Wait for all the children policy server threads to be started */
  //while (pthreads_started != CM_NUM_THREADS)
  //	sched_yield();
  
  /* Activate policy checking by syslistener module */
  //ioctl(fd, IOCTL_SET_CM_READY, 1);
  
  /* Inform the children threads to start serving policy requests */
  //ready = 1;
  
  /* Wait for all children threads to exit */
  //for (i=0; i < CM_NUM_THREADS; i++) {
  //	pthread_join(thids[i], NULL);
  //}
  
  //close(fd);
  //exit(0);
}

/*
int main(void)
{
	pid_t pid, sid;
        
	if ((pid = fork()) < 0)
		exit(EXIT_FAILURE);

        if (pid > 0) {
		// Parent 
                exit(EXIT_SUCCESS);
	} else {
		// Child 

		sched_yield();

		umask(022);
                
		// Create a new SID 
		if ((sid = setsid()) < 0)
			exit(EXIT_FAILURE);
        
        
		// Close out the standard file descriptors
		close(STDIN_FILENO);
		// close(STDOUT_FILENO);
		close(STDERR_FILENO);

		return start();
	}
	exit(EXIT_SUCCESS);
}
*/
