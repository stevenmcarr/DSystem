/* $Id: DistFilterDef.h,v 1.2 1997/03/11 14:30:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/dist/DistFilterDef.h					*/
/*									*/
/*	DistFilterDef -- View filter defs for Ded distribution pane	*/
/*	Last edited: October 17, 1993 at 10:55 pm			*/
/*									*/
/************************************************************************/




#ifndef DistFilterDef_h
#define DistFilterDef_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/UserFilterDef.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/*************************/
/* DistFilterDef class */
/*************************/




struct DistFilterDef_Repr_struct;
class  DistEditor;
class  FilterDefSet;




class DistFilterDef: public UserFilterDef
{
public:

  DistFilterDef_Repr_struct * DistFilterDef_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(DistFilterDef)
				DistFilterDef(Context context,
                                                DB_FP * session_fd,
                                                DistEditor * editor);
  virtual			~DistFilterDef(void);

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

};




EXTERN (void, DistFilterDef_AddStandardDefs, (FilterDefSet * defs))




#endif /* __cplusplus */

#endif /* not DistFilterDef_h */
