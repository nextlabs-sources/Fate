#include <sys/types.h>
#include <sys/socket.h>
#include <pwd.h>

#define	INVALID_UID	999999999
#define	UIDS_MAX	20

struct uid_entry {
	uid_t		ue_uid;
	char		*ue_name;
	time_t		ue_ctime;
};

typedef	struct uid_entry	uid_entry_t;
typedef	struct uid_entry	*puid_entry_t;

enum exe_id {
	EID_KDE_TERM = 0,
	EID_GNOME_TERM,
	EID_XTERM,
	EID_VIM,
	EID_EMACS,
	EID_XEMACS,
	EID_NEDIT,
	EID_FIREFOX,
	EID_OOFFICE,
	EID_CAT,
	EID_MORE,
	EID_LESS,
	EID_TAR,
	EID_CPIO,
	EID_COMPRESS,
	EID_UNCOMPRESS,
	EID_GZIP,
	EID_GUNZIP,
	EID_BZIP2,
	EID_BUNZIP2,
	EID_ENSCRIPT,
	EID_UNKNOWN
};

#define	KDE_TERM_EXENAME	"kdeinit"
#define	GNOME_TERM_EXENAME	"gnome-terminal"
#define	XTERM_EXENAME		"xterm"

#define	VIM_EXENAME		"vim"
#define	EMACS_EXENAME		"emacs"
#define	XEMACS_EXENAME		"xemacs"
#define	NEDIT_EXENAME		"nedit"

#define	FIREFOX_EXENAME		"firefox-bin"

#define	OOFFICE_EXENAME		"soffice.bin"

#define	CAT_EXENAME		"cat"
#define	MORE_EXENAME		"more"
#define	LESS_EXENAME		"less"

#define	TAR_EXENAME		"tar"
#define	CPIO_EXENAME		"cpio"
#define	COMPRESS_EXENAME	"compress"
#define	UNCOMPRESS_EXENAME	"uncompress"
#define	GZIP_EXENAME		"gzip"
#define	GUNZIP_EXENAME		"gunzip"
#define	BZIP2_EXENAME		"bzip2"
#define	BUNZIP2_EXENAME		"bunzip2"

#define	ENSCRIPT_EXENAME	"enscript"

typedef int (*fptr_exetype_check)(char *path, struct ucred *);

extern int bj_uids_init(void);
extern int bj_uids_add(uid_t uid);
extern void bj_uids_fini(void);

extern int bj_sel_init(void);
extern void bj_sel_fini(void);

extern int bj_policy_init(void);
extern void bj_policy_fini(void);
extern int bj_policy_start(void);
extern int bj_policy_stop(void);

extern int bj_is_bj_file(char *);
extern int get_absolute_path(char *, pid_t, char *, int, char *, int);
extern int bj_pid_to_exename(pid_t, char *, int);

extern enum exe_id bj_get_exe_id(struct ucred *);
extern const char *get_eid_string(enum exe_id);

extern int bj_get_konsole_pid(struct ucred *);

extern int is_watcher(pid_t);

extern int sl_fd;
