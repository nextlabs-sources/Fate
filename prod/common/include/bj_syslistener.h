// All sources, binaries and HTML pages (C) copyright 2008 by Blue Jungle Inc.,
// Redwood City CA, Ownership remains with Blue Jungle Inc,
// All rights reserved worldwide.
// Author : Saumitra Das
// Date   : 07/10/2006
// Note   : Definitions for the cm core kernel and user-mode syscall functions.
//
// $Id$
//

#ifndef	__KERNEL__
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/user.h>
#include <unistd.h>
#include <fam.h>
#endif	// __KERNEL__

#define BJ_DEV_NAME		"bj_sysl"
#define BJ_DEV_FILE		"/dev/bj_sysl"
#define BJ_ROOT_DEFAULT		"/usr/local/bj"
#define BJ_STARTUP_SCRIPT	"/etc/rc.d/init.d/bjfse"
#define BJ_CM_EXEC		"bin/controlmodule"

#define	CM_NUM_THREADS		5

#define IOCTL_GET_REQUEST	1
#define IOCTL_SEND_RESPONSE	2
#define IOCTL_SET_CM_READY	3
#define IOCTL_ADD_POLICY	4
#define IOCTL_UPD_POLICY	5
#define IOCTL_DEL_POLICY	6
#define IOCTL_GET_PIDTREE	7


struct sc_arg {
	unsigned long	sca_len_buf     : 13;/* Max 8K request buffer */
	unsigned long	sca_typ_str_flag: 1; /* Set if any of 5 parms is str */
	unsigned long	sca_unused_flag : 1;
	unsigned long	sca_num_params  : 3;
	unsigned long	sca_typ_param1  : 3;
	unsigned long	sca_typ_param2  : 3;
	unsigned long	sca_typ_param3  : 3;
	unsigned long	sca_typ_param4  : 3;
	unsigned long	sca_typ_param5  : 3; /* Fit bits to one ulong(32-bit) */
	unsigned long	sca_buf[5];
};

typedef	struct sc_arg	sc_arg_t;
typedef	struct sc_arg	*psc_arg_t;

#define	MAX_NUM_PARAMS			5
#define	MAX_ARG_BUF_SZ			(PATH_MAX & 0x1FFF) /* Max 8K req buf */

#define	SCA_PTYPE_LONG			0x0
#define	SCA_PTYPE_STR			0x1
#define	SCA_PDATA_TRUNC			0x4

#define clear_bits(scap)		(scap)->sca_buf[-1] = 0l

#define	get_param_type(p, i)		(p)->sca_typ_param##i

#define set_param_type(p, i, n)		(p)->sca_typ_param##i = (n)
#define set_param_type_long(p, i)	(p)->sca_typ_param##i = SCA_PTYPE_LONG
#define set_param_type_str(p, i)	(p)->sca_typ_param##i = SCA_PTYPE_STR

#define is_param_type_long(p, i)	(p)->sca_typ_param##i == SCA_PTYPE_LONG
#define is_param_type_str(p, i)		(p)->sca_typ_param##i == SCA_PTYPE_STR

#define	set_param_data_trunc(p, i)	(p)->sca_typ_param##i |= SCA_PDATA_TRUNC

#define get_len_params(p)		(p)->sca_len_buf
#define set_len_params(p, n)		(p)->sca_len_buf = (n)
#define	get_len_from_bits(nul32)	((psc_arg_t)&nul32)->sca_len_buf

#define	get_max_num_params()		MAX_NUM_PARAMS
#define	get_max_len_params()		MAX_ARG_BUF_SZ

#define	is_params_have_str(p)		(p)->sca_typ_str_flag
#define	set_params_have_str(p, flbit)	(p)->sca_typ_str_flag = (flbit)

#define get_num_params(p)		(p)->sca_num_params
#define set_num_params(p, n)		(p)->sca_num_params = (n)
#define inc_num_params(p)		(p)->sca_num_params++

typedef	short		req_type_t;

struct request_info {
	pid_t		ri_req_task;
	pid_t		ri_req_thread;
	short		ri_sc_id;
	req_type_t	ri_req_type;
	long		ri_uid;
	void		*ri_handle;
	sc_arg_t	ri_args;	/* NOTE: must be the last member; data follows */
};

typedef	struct request_info	reqinfo_t;
typedef	struct request_info	*preqinfo_t;

#define	reqinfo_sethandle(pri, hndl)	(pri)->ri_handle = (void *)hndl
#define	reqinfo_gethandle(pri)		(pri)->ri_handle

/* Request type constants */

#define	SC_AT_ENTRY	(req_type_t)0
#define	SC_AT_EXIT	(req_type_t)1

struct response_data {
	void		*rd_req_handle;
	int		rd_response;
};

typedef struct response_data	response_data_t;
typedef struct response_data	*presponse_data_t;

/* Response values */

#define	SC_DENY		0
#define	SC_ALLOW	1
#define	SC_INTR		2
#define	SC_CM		3
#define	SC_ABORTED	-1

#define	INVALID_PID	-1

struct reqbuf {
	reqinfo_t	rb_ri;
	char		rb_buf[PAGE_SIZE - sizeof(reqinfo_t)];
};

typedef struct reqbuf	reqbuf_t;
typedef struct reqbuf	*preqbuf_t;

struct pid_tree_hdr {
	int		pth_bufsz;
	pid_t		pth_rootpid;
	pid_t		pth_children[0];
};

typedef struct pid_tree_hdr	pid_tree_hdr_t;
typedef struct pid_tree_hdr	*ppid_tree_hdr_t;

#define	get_pidtree_bufsz(ppth)		(ppth)->pth_bufsz
#define	set_pidtree_bufsz(ppth, sz)	(ppth)->pth_bufsz = (sz)
#define	get_pidtree_rootpid(ppth)	(ppth)->pth_rootpid
#define	set_pidtree_rootpid(ppth, pid)	(ppth)->pth_rootpid = (pid)

struct policy_entry {
	pid_t		pe_pid;
	unsigned long	pe_hits;
	long long	pe_allow_sc_bits;
	long long	pe_user_cm_bits;
};

typedef	struct policy_entry	policy_entry_t;
typedef	struct policy_entry	*ppolicy_entry_t;

/* Define below the syscall mask bits to be (re)set in pid_data mask fields */
/* The bit range below is 0-63, based on size of long long in pid_data above */
/* NOTE: If any of the bits below is changed, make the appropriate change */
/*       to initialization of bj_global_policy_check_bits in policy.c */
#define	SC_BIT_KILL	0LL	/* Syscall 37  */
#define	SC_BIT_EXECVE	1LL	/* Syscall 11  */
#define	SC_BIT_FORK	2LL	/* Syscall 2   */
#define	SC_BIT_CLONE	3LL	/* Syscall 120 */
#define	SC_BIT_VFORK	4LL	/* Syscall 190 */
#define	SC_BIT_OPEN	5LL	/* Syscall 5   */
#define	SC_BIT_UNLINK	6LL	/* Syscall 10  */
#define	SC_BIT_CREAT	7LL	/* Syscall 8   */
#define	SC_BIT_EXIT	8LL	/* Syscall 1   */

#define	SC_BITMASK_ALL_POLICIES	((1LL << SC_BIT_KILL)   | \
				 (1LL << SC_BIT_EXECVE) | \
				 (1LL << SC_BIT_FORK)   | \
				 (1LL << SC_BIT_CLONE)  | \
				 (1LL << SC_BIT_VFORK)  | \
				 (1LL << SC_BIT_OPEN)   | \
				 (1LL << SC_BIT_UNLINK) | \
				 (1LL << SC_BIT_CREAT))

#ifdef __KERNEL__

#include <linux/wait.h>
#include <linux/sched.h>
#include <asm/ptrace.h>

#define	TASKS_MAX	500

struct dlist_header {
	struct dlist_header	*dh_next;
	struct dlist_header	*dh_prev;
};
typedef struct dlist_header	dl_hdr_t;
typedef struct dlist_header	*pdl_hdr_t;

static inline int dlist_is_empty(pdl_hdr_t hdr)
{
	return hdr->dh_next == hdr;
}

static inline pdl_hdr_t dlist_get_first(pdl_hdr_t hdr)
{
	return dlist_is_empty(hdr) ? NULL : hdr->dh_next;
}

static inline pdl_hdr_t dlist_get_next(pdl_hdr_t hdr, pdl_hdr_t cur)
{
	return (cur->dh_next == hdr) ? NULL : cur->dh_next;
}

static inline void dlist_add_tail(pdl_hdr_t hdr, pdl_hdr_t new)
{
	new->dh_next = hdr;
	new->dh_prev = hdr->dh_prev;
	hdr->dh_prev = new;
	new->dh_prev->dh_next = new;
}

static inline void dlist_remove_entry(pdl_hdr_t old)
{
	old->dh_next->dh_prev = old->dh_prev;
	old->dh_prev->dh_next = old->dh_next;
	old->dh_next = old->dh_prev = NULL;
}

static inline pdl_hdr_t dlist_remove_head(pdl_hdr_t hdr)
{
	pdl_hdr_t old = hdr->dh_next;
	dlist_remove_entry(old);
	return old;
}

static inline int dlist_is_linked(pdl_hdr_t entry)
{
	return entry->dh_prev != NULL;
}
static inline pdl_hdr_t dlist_peek_next(pdl_hdr_t hdr)
{
	return hdr->dh_next;
}

static inline int dlist_is_last(pdl_hdr_t hdr, pdl_hdr_t cur)
{
	return cur->dh_next == hdr;
}

struct svc_queue {
	dl_hdr_t	sq_dlhdr;
        spinlock_t	sq_lock;
};

typedef struct svc_queue	svc_queue_t;

struct req_entry {
	dl_hdr_t	re_dlhdr;	/* Must be the first entry */
	task_t		*re_waiter;
	int		re_response;
};

typedef struct req_entry	req_entry_t;
typedef struct req_entry	*preq_entry_t;

struct svr_entry {
	dl_hdr_t	se_dlhdr;	/* Must be the first entry */
	task_t		*se_waiter;
	preqinfo_t	se_pri;
};

typedef struct svr_entry	svr_entry_t;
typedef struct svr_entry	*psvr_entry_t;

static inline void bj_mark_sleep(void)
{
	current->state = TASK_INTERRUPTIBLE;
}

static inline void bj_sleep_after_mark(void)
{
	schedule();
}

static inline void bj_resume(task_t *t)
{
	if (wake_up_process(t) != 1)
	  {
#ifdef DEBUG
		printk (KERN_ERR "%s(pid %d): Could not wake up process %s(pid %d)\n", current->comm, current->pid, t->comm, t->pid);
#endif
	  }
}

typedef int  (*fptr_initfunc)(void);
typedef void (*fptr_finifunc)(void);
typedef int  (*fptr_startfunc)(void);
typedef int  (*fptr_stopfunc)(void);

extern int  bj_start(void);
extern int  bj_stop(void);

extern int  bj_syscall_init(void);
extern long bj_replace_syscall(int, long, long *);
extern long bj_get_syscall(int);

extern int  bj_dev_init(void);
extern void bj_dev_fini(void);

extern int  bj_getppid_init(void);
extern void bj_getppid_fini(void);

extern int  bj_kill_init(void);
extern void bj_kill_fini(void);

extern int  bj_execve_init(void);
extern void bj_execve_fini(void);

extern int  bj_clone_init(void);
extern void bj_clone_fini(void);

extern int  bj_fork_init(void);
extern void bj_fork_fini(void);

extern int  bj_vfork_init(void);
extern void bj_vfork_fini(void);

extern int  bj_open_init(void);
extern void bj_open_fini(void);

extern int  bj_creat_init(void);
extern void bj_creat_fini(void);

extern int  bj_unlink_init(void);
extern void bj_unlink_fini(void);

extern int  bj_exit_init(void);
extern void bj_exit_fini(void);

extern int  bj_policy_init(void);
extern int  bj_policy_stop(void);
extern void bj_policy_set_sc_bit(int, char);
extern int  bj_policy_cache_check(pid_t, int, req_type_t);
extern int  bj_policy_cache_add(ppolicy_entry_t);
extern int  bj_policy_cache_update(ppolicy_entry_t);
extern int  bj_policy_cache_delete(pid_t);
extern int  bj_policy_cache_purge(void);
extern int  bj_policy_check(preqinfo_t);

extern int  bj_svcmgr_init(void);
extern void bj_svcmgr_fini(void);
extern int  bj_svcmgr_start(void);
extern int  bj_svcmgr_stop(void);
extern int  bj_svcmgr_up(void);
extern int  bj_request_service(preqinfo_t);
extern int  bj_request_by_server(preqinfo_t);
extern int  bj_response_from_server(presponse_data_t);

extern int  clone_ret_routine(int);
extern int  fork_ret_routine(int);
extern int  vfork_ret_routine(int);
extern int  execve_ret_routine(int);

extern int  bj_is_bj_file(char *);
extern int  bj_get_exename(char *, int);
extern int  get_exec_args(struct pt_regs *, char *, int);
extern int  bj_copy_reqinfo_special(preqinfo_t, preqinfo_t);
extern int  bj_get_pidtree(ppid_tree_hdr_t __user);
extern int  bj_pid_to_procname(pid_t pid, char *buf, int len);

extern char *BJ_ROOT;
extern preqbuf_t pkshared_buf;
extern preqbuf_t __user pushared_buf;

#endif // __KERNEL__
