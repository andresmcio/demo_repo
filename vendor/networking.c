/*
 * list_single from BusyBox ls.c
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */

#include "libbb.h"
#include <sys/stat.h>

/* Esto es exactamente lo que SCANOSS detectará como un Snippet Match */
static uint32_t list_single(const char *name, struct stat *statbuf, 
                            unsigned long opt, uint16_t mode)
{
	uint32_t column_width = 0;
	char *fpath;

	if (opt & OPT_l) {
		column_width = display_single(name, statbuf, opt, mode);
	} else {
		if (opt & OPT_i)
			printf("%7u ", (unsigned int)statbuf->st_ino);
		if (opt & OPT_s)
			printf("%4u ", (unsigned int)(statbuf->st_blocks >> 1));
		
		fpath = (char*)name;
		if (opt & OPT_F) {
			if (S_ISDIR(mode)) fpath = xasprintf("%s/", name);
			else if (S_ISLNK(mode)) fpath = xasprintf("%s@", name);
			else if (S_ISSOCK(mode)) fpath = xasprintf("%s=", name);
			else if (S_ISFIFO(mode)) fpath = xasprintf("%s|", name);
			else if (mode & S_IXUGO) fpath = xasprintf("%s*", name);
		}
		printf("%s", fpath);
		if (fpath != name) free(fpath);
	}
	return column_width;
}
