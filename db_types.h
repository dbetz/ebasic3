/* db_types.h - type definitions for a simple virtual machine
 *
 * Copyright (c) 2009 by David Michael Betz.  All rights reserved.
 *
 */

#ifndef __DB_TYPES_H__
#define __DB_TYPES_H__

/**********/
/* Common */
/**********/

#define VMTRUE      1
#define VMFALSE     0

/* system heap size (includes compiler heap and image buffer) */
#ifndef HEAPSIZE
#define HEAPSIZE            5000
#endif

/* size of image buffer (allocated from system heap) */
#ifndef IMAGESIZE
#define IMAGESIZE           2500
#endif

/* edit buffer size (separate from the system heap) */
#ifndef EDITBUFSIZE
#define EDITBUFSIZE         1500
#endif

/*********/
/* WIN32 */
/*********/

#ifdef WIN32

#include "db_inttypes.h"

#include <stdio.h>
#include <string.h>

typedef int32_t VMVALUE;
typedef uint32_t VMUVALUE;

#define ALIGN_MASK              3

#define FLASH_SPACE
#define DATA_SPACE

#define VMCODEBYTE(p)           *(uint8_t *)(p)
#define VMINTRINSIC(i)          Intrinsics[i]

#define ANSI_FILE_IO

#endif  // WIN32

/*****************/
/* MAC and LINUX */
/*****************/

#if defined(MAC) || defined(LINUX)

#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef int64_t VMVALUE;
typedef uint64_t VMUVALUE;

#define ALIGN_MASK              3

#define FLASH_SPACE
#define DATA_SPACE

#define VMCODEBYTE(p)           *(uint8_t *)(p)
#define VMINTRINSIC(i)          Intrinsics[i]

#define ANSI_FILE_IO

/*************/
/* PROPELLER */
/*************/

#elif defined(PROPELLER)

#include <string.h>
#include <stdint.h>

int strcasecmp(const char *s1, const char *s2);

#define FLASH_SPACE
#define DATA_SPACE

#define VMCODEBYTE(p)           *(uint8_t *)(p)
#define VMINTRINSIC(i)          Intrinsics[i]

typedef int32_t VMVALUE;
typedef uint32_t VMUVALUE;

#define ALIGN_MASK              3

//#define ANSI_FILE_IO

#else

#error Must define MAC, LINUX, or PROPELLER

#endif

/****************/
/* ANSI_FILE_IO */
/****************/

#ifdef ANSI_FILE_IO

#include <stdio.h>
#include <dirent.h>

typedef FILE VMFILE;

#define VM_fopen	fopen
#define VM_fclose	fclose
#define VM_fgets	fgets
#define VM_fputs	fputs

struct VMDIR {
    DIR *dirp;
};

struct VMDIRENT {
    char name[FILENAME_MAX];
};

#endif // ANSI_FILE_IO

#endif
