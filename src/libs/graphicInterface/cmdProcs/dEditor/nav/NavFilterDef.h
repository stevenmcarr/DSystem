/* $Id: NavFilterDef.h,v 1.2 1997/03/11 14:30:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/nav/NavFilterDef.h					*/
/*									*/
/*	NavFilterDef -- View filter defs for Ded nav pane		*/
/*	Last edited: October 14, 1993 at 4:40 pm			*/
/*									*/
/************************************************************************/




#ifndef NavFilterDef_h
#define NavFilterDef_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/cmdProcs/dEditor/src/SrcFilterDef.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/*************************/
/* NavFilterDef class */
/*************************/




struct NavFilterDef_Repr_struct;




class NavFilterDef: public SrcFilterDef
{
public:

  NavFilterDef_Repr_struct * NavFilterDef_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(NavFilterDef)
				NavFilterDef(Context context, DB_FP * session_fd);
  virtual			~NavFilterDef(void);


public:		/* pretend this is 'protected' ! */

/* filter def compilation */
  virtual Boolean		function(char * predicate, int &numArgs, int &opcode);

/* filter def interpretation */
  virtual Boolean		execute(int opcode,
                                        int linenum,
                                        char * line,
                                        void * environment,
                                        int &result);

/* filtering */
  virtual Boolean		filterLine(UserFilter * uvf,
                                           Boolean countOnly,
                                           int line,
                                           int &subline,
                                           TextString &text,
                                           TextData &data,
                                           void * environment);

/* standard definitions */
  static void			AddStandardDefs(FilterDefSet * defs);

/* filter instances */
  virtual UserFilter *		MakeFilter(void * environment);

};




#endif /* __cplusplus */

#endif /* not NavFilterDef_h */
