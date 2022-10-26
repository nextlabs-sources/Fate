#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fam.h>
#include <stdio.h>

#include "ce_syslistener.h"
#include "bj_cmd.h"

/*
 * Determine if filename is a BJ specific file. 
 * Returns 1 if true; else 0 
 * Assumes, filename is a canonical path
 * Depends on BJ_ROOT environment variable
 */
int bj_is_bj_file(char *filename)
{
	char	*BJ_ROOT;
	int	len;

	//[jzhang 102606] env variable is BJROOT not BJ_ROOT
	if ((BJ_ROOT = getenv("RUNTIMEROOT")) == NULL)
		BJ_ROOT = BJ_ROOT_DEFAULT;

	len = strlen(BJ_ROOT);

	while ((len > 0) && (BJ_ROOT[len-1] == '/'))
		len--;

	if ((strncmp(filename, BJ_ROOT, len) == 0) &&
		((filename[len] == '/') || (filename[len] == '\0')))
		return 1;

	if (strcmp(filename, BJ_STARTUP_SCRIPT) == 0 || strcmp(filename, BJ_GUARDIAN_SCRIPT)==0)
		return 1;

	return 0;
}


/*
 * Determing if the process id is our watcher process id
 */
int is_watcher(pid_t targetid)
{
  int ret = 0;
  char *BJ_ROOT;

  if ((BJ_ROOT = getenv("RUNTIMEROOT")) == NULL)
    BJ_ROOT = BJ_ROOT_DEFAULT;
  
  char watcherpath[256];
  _snprintf_s(watcherpath,256, _TRUNCATE, "%s/%s",BJ_ROOT,BJ_WATCHER_PID_FILE);
  FILE *fp = fopen(watcherpath, "r");
  if(fp)
    {
      pid_t watcherpid;
      fscanf(fp,"%d",&watcherpid);
      if(watcherpid==targetid)
	ret = 1;
    }
  fclose(fp);

  return ret;
}


/*
 * Get absolute canonical path for an input filename for process pid
 * Returns 0 on success; -1 on failure.
 * Uses two input buffers (should be of len PATH_MAX) for path manipulation
 * Depends on /proc/<pid>/cwd file
 */
int get_absolute_path(char *path, pid_t pid, char *obuf, int obufsz,
		      char *tbuf, int tbufsz)
{
	char *fpath;
	char *rc;
	int  len;

	if (path[0] != '/') {
		/* First concatanate 'pwd' to make path absolute */
		_snprintf_s(obuf, obufsz, _TRUNCATE, "/proc/%d/cwd", pid);
		if ((len = readlink(obuf, tbuf, tbufsz-1)) < 0)
			return -1;
		tbuf[len] = '/';
		tbuf[++len] = '\0';
		strncat_s(tbuf, tbufsz, path, _TRUNCATE);
		fpath = tbuf;
	} else
		fpath = path;

	rc = realpath(fpath, obuf);

	if (rc == NULL) {
		/* We could not find the file; it is either a file to be created or bad name */
		/* We try to traverse the path backwards till we find a valid 'basename' */
		int i = strlen(fpath);

		/* First skip all trailing '/'s */
		while ((i > 0) && (fpath[i-1] == '/'))
			--i;

		/* Try repeatatively to break the complete pathname, into the */
		/*   longest basename for which we can get realpath and then */
		/*   append the retmaining filename to the realpath'ed basename */
		while (1) {
			while ((--i > 0) && (fpath[i] != '/'));

			/* We are guaranteed, fpath[0] = '/'; see above */
			if (i <= 0)
				break;
			/* fpath[i] = '/' and separates the basename and filename */
			fpath[i] = '\0';
			rc = realpath(fpath, obuf);
			fpath[i] = '/';

			if (rc != NULL) {
				len = strlen(obuf);
				strncat_s(obuf, obufsz, &fpath[i], _TRUNCATE);
				return 0;
			}
		}
		/* We can't get realname for the full pathname; pass it as it is */
		/* TODO: Can we do anything better? */
		strncpy_s(obuf, obufsz, fpath, _TRUNCATE);
	}

	return 0;
}


int bj_pid_to_exename(pid_t pid, char *buf, int len)
{
	char tbuf[32];
	int  rc;

	_snprintf_s(tbuf, 32, _TRUNCATE, "/proc/%d/exe", pid);
	rc = readlink(tbuf, buf, len-1);
	if (rc >= 0)
		buf[rc] = '\0';
	else
		buf[0] = '\0';

	return rc;
}
