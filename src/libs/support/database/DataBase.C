/* $Id: DataBase.C,v 1.2 1997/03/11 14:36:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/file.h>
#include <assert.h>

#include <libs/support/misc/general.h>
#include <libs/support/database/newdatabase.h>

#include <libs/support/strings/rn_string.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/support/file/File.h>

/*
 * conversion from FILE * to DB_FP *
 */

DB_FP *
db_convert_fp_to_db_fp(FILE *fp)
{
  return (DB_FP *) fp;
}

/*
 * Attribute io - buffered:
 * Boolean      db_buffered_read (fp,buffer,len)
 * Boolean      db_buffered_write(fp,buffer,len)
 * void 	db_buffered_write_name(fp,nm)
 * char		*db_buffered_read_name(fp,string)
 */

Generic
db_buffered_close(DB_FP *fp)
{
  int             retval;

  retval = fp->Close();
  return retval;
}

int db_buffered_write (DB_FP *fp, char *object, Generic size)
{
  return (fp->Write(object, size) ? 0 : size);
}

int db_buffered_read (DB_FP *fp, char *object, Generic size)
{
  return (fp->Read(object, size) ? 0 : size);
}

char *db_buffered_read_name(DB_FP *fp, char *str)
{
  int    l;
  char   *nm;

  (void) db_buffered_read(fp, (char *) &l, sizeof (l));
  nm = (char *) get_mem(l+1, str);
  (void) db_buffered_read(fp, nm, l);
  nm[l] = '\0';
  return nm;
}

void db_buffered_write_name(DB_FP *fp, char *nm)
{
  int l = strlen(nm);
  (void) db_buffered_write (fp, (char *) &l, sizeof (l));
  (void) db_buffered_write (fp, nm, l);
}

