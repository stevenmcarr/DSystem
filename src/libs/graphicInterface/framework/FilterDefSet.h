/* $Id: FilterDefSet.h,v 1.4 1997/03/11 14:32:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/FilterDefSet.h					*/
/*									*/
/*	FilterDefSet -- Editable set of UserFilterDefs			*/
/*	Last edited: October 13, 1992 at 6:00 pm			*/
/*									*/
/************************************************************************/




#ifndef FilterDefSet_h
#define FilterDefSet_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/DBObject.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/**********************/
/* FilterDefSet class */
/**********************/




struct FilterDefSet_Repr_struct;
class  Editor;
class  UserFilterDef;
class  UserFilter;




class FilterDefSet: public DBObject
{
public:

  FilterDefSet_Repr_struct * FilterDefSet_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(FilterDefSet)
				FilterDefSet(Context context,
				             DB_FP * session_fp,
                                             Editor * editor);
  virtual			~FilterDefSet(void);
  virtual Editor *		GetEditor(void);

/* access to filter defs */
  virtual int			NumFilterDefs(void);
  virtual void			AddFilterDef(UserFilterDef * def);
  virtual void			GetFilterDef(int k, UserFilterDef * &def);
  virtual void			GetFilterDefByName(char * name, UserFilterDef * &def);

/* user interaction */
  virtual Boolean		DoDialog(void);
  virtual Boolean		DoMenu(UserFilter * &filter, void * environment);


protected:

/* database */
  virtual void			isnew(Context context);
  virtual void			read(DB_FP * fp, DB_FP * session_fp);
  virtual void			write(DB_FP * fp, DB_FP * session_fp);

};




#endif /* __cplusplus */

#endif /* not FilterDefSet_h */
