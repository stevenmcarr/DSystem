/* $Id: DBObject.h,v 1.6 1997/03/11 14:32:35 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/DBObject.h						*/
/*									*/
/*	DBObject -- Abstract class for all persistent objects		*/
/*	Last edited: October 13, 1993 at 5:40 pm      			*/
/*									*/
/************************************************************************/




#ifndef DBObject_h
#define DBObject_h


#include <libs/graphicInterface/framework/framework.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/******************/
/* DBObject class */
/******************/




struct DBObject_Repr_struct;

class File;




class DBObject: public Object
{
public:

  DBObject_Repr_struct * DBObject_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(DBObject)
				DBObject(Context context, File *session_fp);
  virtual			~DBObject(void);

/* database */
  virtual void			Open(Context context,
		                     Context mod_in_pgm_context,
		                     Context pgm_context,
		                     File * session_fp);
  virtual void			Close(void);
  virtual void			Save(Context context, File * session_fp);
  virtual void			GetAttribute(char * &attr);


protected:

/* database */
  virtual void			isnew(Context context);
  virtual void			read(File * fp, File * session_fp);
  virtual void			write(File * fp, File * session_fp);

};




#endif /* __cplusplus */

#endif /* not DBObject_h */
