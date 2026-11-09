/*
 * Warning, this file was automatically created by the TIFF configure script
 * VERSION:	 v3.4beta037
 * DATE:	 Tue Dec 21 10:08:29 1999
 * TARGET:	 i686-pc-mingw32
 * CCOMPILER:	 /gcc-2.95.2/msvcrt/bin/gcc-2.95.2 19991024 (release)

 */
#ifndef _PORT_
#define _PORT_ 1
#ifdef __cplusplus
extern "C" {
#endif
#include <sys/types.h>
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;
#define HOST_FILLORDER FILLORDER_MSB2LSB
#define HOST_BIGENDIAN	0
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
typedef double dblparam_t;
#ifdef __STRICT_ANSI__
#define	INLINE	__inline__
#else
#define	INLINE	inline
#endif
#define GLOBALDATA(TYPE,NAME)	extern TYPE NAME
extern void* malloc(size_t);
extern void* realloc(void*, size_t);
extern void free(void*);
#ifdef __cplusplus
}
#endif
#endif
