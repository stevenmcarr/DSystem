/* $Id: newdatabase.h,v 1.4 1997/03/11 14:36:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef newdatabase_h
#define newdatabase_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#include <stdio.h>

#include <sys/types.h>
#include <sys/uio.h>

#ifdef __cplusplus
class File;
#define DB_FP File
#else
#define DB_FP struct File
#endif

#define DB_NULLFP       ((DB_FP *) 0)

EXTERN(DB_FP *, db_convert_fp_to_db_fp, (FILE *fp));
EXTERN(Generic, db_buffered_close, (DB_FP *fp));
EXTERN(int, db_buffered_close, (DB_FP *fp));
EXTERN(int, db_buffered_write,
       (DB_FP *fp, char *object, Generic size));
EXTERN(int, db_buffered_read,
       (DB_FP *fp, char *object, 	Generic size));
EXTERN(char *, db_buffered_read_name,
       (DB_FP *fp, char *str));
EXTERN(void,   db_buffered_write_name,
       (DB_FP *fp, char *nm));

EXTERN(int, io_fprintf,
       ( DB_FP *fp, char *format, ... ));
EXTERN(char *, io_fgets,
       ( char *buf, int size, DB_FP *fp ));
EXTERN(int, io_fputs,
       ( char *buf, DB_FP *fp ));
EXTERN(int, io_fgetc,
       (DB_FP *fp));
EXTERN(int, io_fungetc,
       (char c, DB_FP *fp));
EXTERN(int, io_fputc,
       (char c, DB_FP *fp));
EXTERN(int, io_fseek,
       ( DB_FP *fp, long o, int p ));
EXTERN(int, io_fread,
       ( char *buf, int nbytes, int nitems, DB_FP *fp));
EXTERN(int, io_fwrite,
       ( char *buf, int nbytes, int nitems, DB_FP *fp ));
EXTERN(int, io_fflush,
       ( DB_FP *fp ));

#define DB_PATH_LENGTH       1024

#endif



