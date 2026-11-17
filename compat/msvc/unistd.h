/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 ***************************************************************************/

/* Replacement for the POSIX header, which MSVC does not have. */

#ifndef AQBANKING_COMPAT_MSVC_UNISTD_H
#define AQBANKING_COMPAT_MSVC_UNISTD_H

#include <io.h>      /* access, close, dup, isatty, lseek, read, unlink, write */
#include <process.h> /* getpid */
#include <direct.h>  /* chdir, getcwd, rmdir */

#define F_OK 0
#define W_OK 2
#define R_OK 4

#ifdef __cplusplus
extern "C" {
#endif

/* Same declaration as in <windows.h>, which this header must not include
   because of its side effects. */
__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);

static __inline unsigned int sleep(unsigned int seconds)
{
  Sleep(seconds * 1000);
  return 0;
}

#ifdef __cplusplus
}
#endif

#endif
