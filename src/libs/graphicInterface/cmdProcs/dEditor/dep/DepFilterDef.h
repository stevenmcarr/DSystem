/* $Id: DepFilterDef.h,v 1.2 1997/03/11 14:30:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/dep/DepFilterDef.h					*/
/*									*/
/*	DepFilterDef -- View filter defs for Ded dependence pane	*/
/*	Last edited: November 10, 1993 at 11:42 pm			*/
/*									*/
/************************************************************************/




#ifndef DepFilterDef_h
#define DepFilterDef_h


#include <libs/graphicInterface/framework/framework.h>

#include <libs/graphicInterface/framework/UserFilterDef.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/**********************/
/* DepFilterDef class */
/**********************/




struct DepFilterDef_Repr_struct;
class  DepEditor;
class  FilterDefSet;




class DepFilterDef: public UserFilterDef
{
public:

  DepFilterDef_Repr_struct * DepFilterDef_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(DepFilterDef)
				DepFilterDef(Context context,
                                             DB_FP * session_fd,
                                             DepEditor * editor);
  virtual			~DepFilterDef(void);

/* filter instances */
  virtual UserFilter *		MakeFilter(void * environment);


public:			/* pretend this is 'protected' ! */

/* filter def compilation */
  virtual Boolean		function(char * predicate, int &numArgs, int &opcode);

/* filter def interpretation */
  virtual Boolean		execute(int opcode,
                                        int linenum,
                                        char * line,
                                        void * environment,
                                        int &result);

/* standard definitions */
  static void			AddStandardDefs(FilterDefSet * defs);
};




EXTERN (void, DepFilterDef_AddStandardDefs, (FilterDefSet * defs))




#endif /* __cplusplus */

#endif /* not DepFilterDef_h */
