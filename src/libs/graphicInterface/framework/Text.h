/* $Id: Text.h,v 1.4 1997/03/11 14:32:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/Text.h						*/
/*									*/
/*	Text -- Abstract class for all lines-of-text objects		*/
/*	Last edited: October 16, 1993 at 4:11 pm			*/
/*									*/
/************************************************************************/




#ifndef Text_h
#define Text_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/DBObject.h>




/************************************************************************/
/*	Procedural Interface						*/
/************************************************************************/




/* must agree with 'TV_Data' in "ned_cp/TextView.h" */

#define MAX_TEXT_CHARS    500

typedef struct
  {
    Color	foreground;
    Color	background;

  } ColorPair;


typedef struct
  {
    Boolean	multiColored;
    ColorPair	all;
    ColorPair	chars[MAX_TEXT_CHARS];

  } TextData;


extern ColorPair Text_DefaultColorPair;

EXTERN (void, Text_DefaultData, (TextData &data))
EXTERN (void, Text_MultiColoredData, (TextData &data))





/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/**************/
/* Text class */
/**************/




struct Text_Repr_struct;




class Text: public DBObject
{
public:

  Text_Repr_struct * Text_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(Text)
				Text(Context context, DB_FP * session_fp);
  virtual			~Text(void);

/* access to lines */
  virtual int			NumLines(void);
  virtual int			MaxLineWidth(void);
  virtual void			GetLine(int k, TextString &ts, TextData &td);
  virtual void			SetLine(int k, TextString line);
  virtual void			InsertLine(int k, TextString line);
  virtual void			DeleteLine(int k);

};




#endif /* __cplusplus */

#endif /* not Text_h */
