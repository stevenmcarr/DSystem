/* $Id: Text.C,v 1.6 1997/03/11 14:32:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/Text.C						*/
/*									*/
/*	Text -- Abstract class for all lines-of-text objects		*/
/*	Last edited: October 16, 1993 at 4:13 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/Text.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* Text object */

typedef struct Text_Repr_struct
  {
    /* not used */

  } Text_Repr;


#define R(ob)		(ob->Text_repr)

#define INHERITED	DBObject






/*************************/
/*  Forward declarations */
/*************************/




/* none */







/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




ColorPair Text_DefaultColorPair;




void Text::InitClass(void)
{
  /* initialize needed submodules */
    /* ... */

  /* initialize structured constants */
    Text_DefaultColorPair.foreground = black_color;
    Text_DefaultColorPair.background = white_color;
}




void Text::FiniClass(void)
{
  /* ... */
}




void Text_DefaultData(TextData &data)
{
  data.multiColored = false;
  data.all          = Text_DefaultColorPair;
}




void Text_MultiColoredData(TextData &data)
{
  int k;

  if( ! data.multiColored )
    { data.multiColored = true;
      for( k = 0;  k < MAX_TEXT_CHARS;  k++ )
        { data.chars[k].foreground = data.all.foreground;
          data.chars[k].background = data.all.background;
        }
    }
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(Text)




Text::Text(Context context, DB_FP * session_fp)
   : DBObject (context, session_fp)
{
  /* allocate instance's private data */
    this->Text_repr = (Text_Repr *) get_mem(sizeof(Text_Repr), "Text instance");

  /* save creation parameters */
    /* ... */
}




Text::~Text()
{
  free_mem((void*) this->Text_repr);
}






/********************/
/*  Access to lines */
/********************/




int Text::NumLines(void)
{
  SUBCLASS_RESPONSIBILITY("Text::NumLines");
  return 0;
}




int Text::MaxLineWidth(void)
{
  SUBCLASS_RESPONSIBILITY("Text::MaxLineWidth");
  return 0;
}





void Text::GetLine(int k, TextString &ts, TextData &td)
{
  SUBCLASS_RESPONSIBILITY("Text::GetLine");
}





void Text::SetLine(int k, TextString line)
{
  SUBCLASS_RESPONSIBILITY("Text::SetLine");
}





void Text::InsertLine(int k, TextString line)
{
  SUBCLASS_RESPONSIBILITY("Text::InsertLine");
}





void Text::DeleteLine(int k)
{
  SUBCLASS_RESPONSIBILITY("Text::DeleteLine");
}







/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
