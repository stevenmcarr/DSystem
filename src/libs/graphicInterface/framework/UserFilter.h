/* $Id: UserFilter.h,v 1.4 1997/03/11 14:32:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/UserFilter.h						*/
/*									*/
/*	UserFilter -- ViewFilter programmed in user notation		*/
/*	Last edited: October 13, 1993 at 11:00 pm			*/
/*									*/
/************************************************************************/




#ifndef UserFilter_h
#define UserFilter_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/CViewFilter.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/*********************/
/* UserFilter class */
/*********************/




struct UserFilter_Repr_struct;
class  UserFilterDef;




class UserFilter: public CViewFilter
{
public:

  UserFilter_Repr_struct * UserFilter_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(UserFilter)
				UserFilter(Context context,
				           DB_FP * session_fp,
                                           UserFilterDef * def,
                                           void * environment);
  virtual			~UserFilter(void);

/* access to filter settings */
  virtual char *		GetName(Boolean withError);
  virtual void			SetShowErrors(Boolean show);
  virtual void			GetShowErrors(Boolean &show);


public:		/* pretend this is 'protected' */

/* filtering */
  virtual Boolean		filterLine(Boolean countOnly,
                                           int line,
                                           int &subline,
                                           TextString &text,
                                           TextData &data);

};




#endif /* __cplusplus */

#endif /* not UserFilter_h */
