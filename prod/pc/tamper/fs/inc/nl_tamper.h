#ifndef __NL_TAMPER_H__
#define __NL_TAMPER_H__

#define NLTAMPER_PORT_NAME L"\\NLTamperPort"

#define NLFILT_MAX_PATH 512

#define NL_TAMPER_FLAG_NONE       (0x0)        /* Empty */
#define NL_TAMPER_FLAG_DENY_ALL   (0x1 << 0)   /* Deny any action */
#define NL_TAMPER_FLAG_DENY_WRITE (0x1 << 1)   /* Deny any action but those that can modify the file */
#define NL_TAMPER_FLAG_NOTIFY     (0x1 << 2)   /* Notify user-mode on action */
#define NL_TAMPER_FLAG_EXEMPT     (0x1 << 3)   /* Ignore all other flags  */

#define NL_TAMPER_FLAG_DENY       (NL_TAMPER_FLAG_DENY_ALL | NL_TAMPER_FLAG_DENY_WRITE)

/** fs_msg_t
 *
 *  File system event.
 */
typedef struct
{
  WCHAR fname[NLFILT_MAX_PATH];  /* File name */
  WCHAR pname[NLFILT_MAX_PATH];  /* Process image path */
  ULONG pid;                     /* Process ID */
  UINT32 access;                  /* Action ID */
} fs_msg_t;

typedef struct 
{
  FILTER_MESSAGE_HEADER h;
  fs_msg_t fsm;
} recv_msg_t;

typedef struct
{
  int type;         /* Policy type */
  int flags;
  fs_msg_t fsm;
} NLFiltPolicy;

typedef struct
{
  int type;             /* Command type */
  int flags;
  NLFiltPolicy policy;  /* Policy */
} NLFiltCommand;

typedef struct
{
	int bStoppable;
}NLFltUnlaodControl;

/* Policy Type */
enum
{
  NL_FILT_POLICY_PROTECT_FILE,             /* Protect File */
  NL_FILT_POLICY_EXEMPT_PROCESS_NAME,      /* Exempt Process by Name */
  NL_FILT_POLICY_EXEMPT_PROCESS_ID,        /* Exempt Process by PID */
  NL_FILT_POLICY_FILE_EXEMPT_PROCESS_NAME  /* Protected FILE Exempt Process by Name */ 
};

/* Command Type */
enum
{
  NL_FILT_COMMAND_ADD_POLICY,
  NL_FILT_COMMAND_REMOVE_POLICY
};

#endif /* __NLTAMPER_H__ */
