/* $Id: CommFilterDef.h,v 1.2 1997/03/11 14:30:13 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/comm/CommFilterDef.h					*/
/*									*/
/*	CommFilterDef -- View filter defs for Ded communication pane	*/
/*	Last edited: October 14, 1993 at 3:43 pm			*/
/*									*/
/************************************************************************/




#ifndef CommFilterDef_h
#define CommFilterDef_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/UserFilterDef.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/*************************/
/* CommFilterDef class */
/*************************/




struct CommFilterDef_Repr_struct;
class  CommEditor;
class  FilterDefSet;




class CommFilterDef: public UserFilterDef
{
public:

  CommFilterDef_Repr_struct * CommFilterDef_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(CommFilterDef)
				CommFilterDef(Context context,
                                              DB_FP * session_fd,
                                              CommEditor * editor);
  virtual			~CommFilterDef(void);

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




EXTERN (void, CommFilterDef_AddStandardDefs, (FilterDefSet * defs))




#endif /* __cplusplus */

#endif /* not CommFilterDef_h */
