/* $Id: UserFilterDef.h,v 1.4 1997/03/11 14:32:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/UserFilterDef.h					*/
/*									*/
/*	UserFilterDef -- User-notation definition of view filters	*/
/*	Last edited: October 13, 1993 at 11:00 pm			*/
/*									*/
/************************************************************************/




#ifndef UserFilterDef_h
#define UserFilterDef_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/DBObject.h>
#include <libs/graphicInterface/framework/Text.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/***********************/
/* UserFilterDef class */
/***********************/




struct UserFilterDef_Repr_struct;
class  UserFilter;




class UserFilterDef: public DBObject
{
  friend UserFilter;

public:

  UserFilterDef_Repr_struct * UserFilterDef_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(UserFilterDef)
				UserFilterDef(Context context, DB_FP * session_fp);
  virtual			~UserFilterDef(void);

/* access to definition */
  virtual void			GetName(char * &name);
  virtual void			SetName(char * name);
  virtual void			GetDefinition(char * &def,
                                              Boolean &concealed,
                                              Boolean &errors);
  virtual Boolean		SetDefinition(char * def,
                                              Boolean concealed,
                                              Boolean errors,
                                              char * &msg);
  virtual Boolean		InUse(void);

/* filter instances */
  virtual UserFilter *		MakeFilter(void * environment);


protected:

/* database */
  virtual void			isnew(Context context);
  virtual void			read(DB_FP * fp, DB_FP * session_fp);
  virtual void			write(DB_FP * fp, DB_FP * session_fp);


public:			/* pretend this is 'protected' */

/* filter def compilation */
  virtual Boolean		function(char * predicate, int &numArgs, int &opcode);

/* filter def interpretation */
  virtual Boolean		execute(int opcode,
                                        int linenum,
                                        char * line,
                                        void * environment,
                                        int &result);
  virtual int			operand(void);
  virtual Boolean		getConcealed(int linenum, void * environment);
  virtual Boolean		getErroneous(int linenum, void * environment);
  virtual char *		getErrorMessage(int linenum, void * environment);

/* filter census */
  virtual void			addFilter(UserFilter * uvf);
  virtual void			removeFilter(UserFilter * uvf);

/* filtering */
  virtual Boolean		filterLine(UserFilter * uvf,
                                           Boolean countOnly,
                                           int line,
                                           int &subline,
                                           TextString &text,
                                           TextData &data,
                                           void * environment);

};




#endif /* __cplusplus */

#endif /* not UserFilterDef_h */
