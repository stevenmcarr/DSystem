/* $Id: FortFilterDef.h,v 1.4 1997/03/11 14:32:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/FortFilterDef.h					*/
/*									*/
/*	FortFilterDef -- View filter defs for Fortran source code	*/
/*	Last edited: October 13, 1993 at 6:08 pm			*/
/*									*/
/************************************************************************/




#ifndef FortFilterDef_h
#define FortFilterDef_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/UserFilterDef.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/***********************/
/* FortFilterDef class */
/***********************/




struct FortFilterDef_Repr_struct;
class  UserFilter;
class  FilterDefSet;




class FortFilterDef: public UserFilterDef
{
public:

  FortFilterDef_Repr_struct * FortFilterDef_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(FortFilterDef)
				FortFilterDef(Context context, DB_FP * session_fp);
  virtual			~FortFilterDef(void);


public:			/* pretend this is 'protected' ! */

/* filter def compilation */
  virtual Boolean		function(char * predicate, int &numArgs, int &opcode);

/* filter def interpretation */
  virtual Boolean		execute(int opcode,
                                        int linenum,
                                        char * line,
                                        void * environment,
                                        int &result);
  virtual Boolean		getConcealed(int linenum, void * environment);
  virtual Boolean		getErroneous(int linenum, void * environment);
  virtual char *		getErrorMessage(int linenum, void * environment);
  
/* standard definitions */
  static void			AddStandardDefs(FilterDefSet * defs);

};




#endif /* __cplusplus */

#endif /* not FortFilterDef_h */
