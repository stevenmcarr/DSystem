/* $Id: SrcFilterDef.h,v 1.3 1997/06/24 17:55:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/src/SrcFilterDef.h					*/
/*									*/
/*	SrcFilterDef -- View filter defs for Ded source pane		*/
/*	Last edited: October 14, 1993 at 4:40 pm			*/
/*									*/
/************************************************************************/




#ifndef SrcFilterDef_h
#define SrcFilterDef_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/FortFilterDef.h>




/************************************************************************/
/*	Procedural Interface						*/
/************************************************************************/


class FilterDefSet;


EXTERN (void, SrcFilterDef_Init, (void));
EXTERN (void, SrcFilterDef_Fini, (void));
EXTERN (void, SrcFilterDef_AddStandardDefs, (FilterDefSet * defs));




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/**********************/
/* SrcFilterDef class */
/**********************/




struct SrcFilterDef_Repr_struct;




class SrcFilterDef: public FortFilterDef
{
public:

  SrcFilterDef_Repr_struct * SrcFilterDef_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(SrcFilterDef)
				SrcFilterDef(Context context, DB_FP * session_fd);
  virtual			~SrcFilterDef(void);


public:		/* pretend this is 'protected' ! */

/* filter def compilation */
  virtual Boolean		function(char * predicate, int &numArgs, int &opcode);

/* filter def interpretation */
  virtual Boolean		execute(int opcode, int linenum, char * line,
                                        void * environment, int &result);
  virtual Boolean		getConcealed(int linenum, void * environment);
  virtual Boolean		getErroneous(int linenum, void * environment);
  virtual char *		getErrorMessage(int linenum, void * environment);

/* standard definitions */
  static void			AddStandardDefs(FilterDefSet * defs);
  
};




#endif /* __cplusplus */

#endif /* not SrcFilterDef_h */
