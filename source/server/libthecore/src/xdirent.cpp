#include "stdafx.h"

#ifdef _WIN32

/*

    Implementation of POSIX directory browsing functions and types for Win32.

    Author:  Kevlin Henney (kevlin@acm.org, kevlin@curbralan.com)
    History: Created March 1997. Updated June 2003.
    Rights:  See end of file.

*/

#include "xdirent.h"

#include <errno.h>
#include <io.h> /* _findfirst and _findnext set errno iff they return -1 */
#include <stdlib.h>
#include <string.h>

/* Entries missing from MSVC 6.0 */
#if !defined(FILE_ATTRIBUTE_DEVICE)
# define FILE_ATTRIBUTE_DEVICE 0x40
#endif

/* File type and permission flags for stat() */
#if defined(_MSC_VER)  &&  !defined(S_IREAD)
# define S_IFMT   _S_IFMT                      /* file type mask */
# define S_IFDIR  _S_IFDIR                     /* directory */
# define S_IFCHR  _S_IFCHR                     /* character device */
# define S_IFFIFO _S_IFFIFO                    /* pipe */
# define S_IFREG  _S_IFREG                     /* regular file */
# define S_IREAD  _S_IREAD                     /* read permission */
# define S_IWRITE _S_IWRITE                    /* write permission */
# define S_IEXEC  _S_IEXEC                     /* execute permission */
#endif
#define S_IFBLK   0                            /* block device */
#define S_IFLNK   0                            /* link */
#define S_IFSOCK  0                            /* socket */

#if defined(_MSC_VER)
# define S_IRUSR  S_IREAD                      /* read, user */
# define S_IWUSR  S_IWRITE                     /* write, user */
# define S_IXUSR  0                            /* execute, user */
# define S_IRGRP  0                            /* read, group */
# define S_IWGRP  0                            /* write, group */
# define S_IXGRP  0                            /* execute, group */
# define S_IROTH  0                            /* read, others */
# define S_IWOTH  0                            /* write, others */
# define S_IXOTH  0                            /* execute, others */
#endif

/* Indicates that d_type field is available in dirent structure */
#define _DIRENT_HAVE_D_TYPE

/* File type flags for d_type */
#define DT_UNKNOWN  0
#define DT_REG      S_IFREG
#define DT_DIR      S_IFDIR
#define DT_FIFO     S_IFFIFO
#define DT_SOCK     S_IFSOCK
#define DT_CHR      S_IFCHR
#define DT_BLK      S_IFBLK

/* Macros for converting between st_mode and d_type */
#define IFTODT(mode) ((mode) & S_IFMT)
#define DTTOIF(type) (type)

/*
* File type macros.  Note that block devices, sockets and links cannot be
* distinguished on Windows and the macros S_ISBLK, S_ISSOCK and S_ISLNK are
* only defined for compatibility.  These macros should always return false
* on Windows.
*/
#define    S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#define    S_ISLNK(mode)  (((mode) & S_IFMT) == S_IFLNK)
#define    S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)
#define    S_ISCHR(mode)  (((mode) & S_IFMT) == S_IFCHR)
#define    S_ISBLK(mode)  (((mode) & S_IFMT) == S_IFBLK)


#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct DIR
	{
		dirent           curentry;                  /* Current directory entry */
		WIN32_FIND_DATAA find_data;                 /* Private file data */
		int              cached;                    /* True if data is valid */
		HANDLE           search_handle;             /* Win32 search handle */
		char             patt[MAX_PATH + 3];        /* Initial directory name */
	} DIR;

#if defined(_MSC_VER) && _MSC_VER >= 1400
# define DIRENT_STRNCPY(dest,src,size) strncpy_s((dest),(size),(src),_TRUNCATE)
#else
# define DIRENT_STRNCPY(dest,src,size) strncpy((dest),(src),(size))
#endif

	/* Set errno variable */
#if defined(_MSC_VER)
#define DIRENT_SET_ERRNO(x) _set_errno (x)
#else
#define DIRENT_SET_ERRNO(x) (errno = (x))
#endif

DIR *opendir(const char *dirname)
{
	DIR *dirp;

	/* ensure that the resulting search pattern will be a valid file name */
	if (dirname == nullptr) {
		DIRENT_SET_ERRNO(ENOENT);
		return nullptr;
	}
	if (strlen(dirname) + 3 >= MAX_PATH) {
		DIRENT_SET_ERRNO(ENAMETOOLONG);
		return nullptr;
	}

	/* construct new DIR structure */
	dirp = (DIR*)malloc(sizeof(struct DIR));
	if (dirp != nullptr) {
		int error;

		/*
		* Convert relative directory name to an absolute one.  This
		* allows rewinddir() to function correctly when the current working
		* directory is changed between opendir() and rewinddir().
		*/
		if (GetFullPathNameA(dirname, MAX_PATH, dirp->patt, nullptr)) {
			char *p;

			/* append the search pattern "\\*\0" to the directory name */
			p = strchr(dirp->patt, '\0');
			if (dirp->patt < p  &&  *(p - 1) != '\\'  &&  *(p - 1) != ':') {
				*p++ = '\\';
			}
			*p++ = '*';
			*p = '\0';

			/* open directory stream and retrieve the first entry */
			dirp->search_handle = FindFirstFileA(dirp->patt, &dirp->find_data);
			if (dirp->search_handle != INVALID_HANDLE_VALUE) {
				/* a directory entry is now waiting in memory */
				dirp->cached = 1;
				error = 0;
			}
			else {
				/* search pattern is not a directory name? */
				DIRENT_SET_ERRNO(ENOENT);
				error = 1;
			}
		}
		else {
			/* buffer too small */
			DIRENT_SET_ERRNO(ENOMEM);
			error = 1;
		}

		if (error) {
			free(dirp);
			dirp = nullptr;
		}
	}

	return dirp;
}

int closedir(DIR *dirp)
{
	if (dirp == nullptr) {
		/* invalid directory stream */
		DIRENT_SET_ERRNO(EBADF);
		return -1;
	}

	/* release search handle */
	if (dirp->search_handle != INVALID_HANDLE_VALUE) {
		FindClose(dirp->search_handle);
		dirp->search_handle = INVALID_HANDLE_VALUE;
	}

	/* release directory structure */
	free(dirp);
	return 0;
}

struct dirent *readdir(DIR *dirp)
{
	DWORD attr;
	if (dirp == nullptr) {
		/* directory stream did not open */
		DIRENT_SET_ERRNO(EBADF);
		return nullptr;
	}

	/* get next directory entry */
	if (dirp->cached != 0) {
		/* a valid directory entry already in memory */
		dirp->cached = 0;
	}
	else {
		/* get the next directory entry from stream */
		if (dirp->search_handle == INVALID_HANDLE_VALUE) {
			return nullptr;
		}
		if (FindNextFileA(dirp->search_handle, &dirp->find_data) == FALSE) {
			/* the very last entry has been processed or an error occured */
			FindClose(dirp->search_handle);
			dirp->search_handle = INVALID_HANDLE_VALUE;
			return nullptr;
		}
	}

	/* copy as a multibyte character string */
	DIRENT_STRNCPY(dirp->curentry.d_name,
		dirp->find_data.cFileName,
		sizeof(dirp->curentry.d_name));
	dirp->curentry.d_name[MAX_PATH] = '\0';

	/* compute the length of name */
	dirp->curentry.d_namlen = strlen(dirp->curentry.d_name);

	/* determine file type */
	attr = dirp->find_data.dwFileAttributes;
	if ((attr & FILE_ATTRIBUTE_DEVICE) != 0) {
		dirp->curentry.d_type = DT_CHR;
	}
	else if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
		dirp->curentry.d_type = DT_DIR;
	}
	else {
		dirp->curentry.d_type = DT_REG;
	}
	return &dirp->curentry;
}

void rewinddir(DIR* dirp)
{
	if (dirp != nullptr) {
		/* release search handle */
		if (dirp->search_handle != INVALID_HANDLE_VALUE) {
			FindClose(dirp->search_handle);
		}

		/* open new search handle and retrieve the first entry */
		dirp->search_handle = FindFirstFileA(dirp->patt, &dirp->find_data);
		if (dirp->search_handle != INVALID_HANDLE_VALUE) {
			/* a directory entry is now waiting in memory */
			dirp->cached = 1;
		}
		else {
			/* failed to re-open directory: no directory entry in memory */
			dirp->cached = 0;
		}
	}
}

#ifdef __cplusplus
}
#endif

/*

    Copyright Kevlin Henney, 1997, 2003. All rights reserved.

    Permission to use, copy, modify, and distribute this software and its
    documentation for any purpose is hereby granted without fee, provided
    that this copyright and permissions notice appear in all copies and
    derivatives.
    
    This software is supplied "as is" without express or implied warranty.

    But that said, if there are any problems please get in touch.

*/

#endif // #ifdef _WIN32
