/* $Id: LineNumDecoration.C,v 1.2 1997/03/11 14:32:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/LineNumDecoration.C					*/
/*									*/
/*	LineNumDecoration -- Addition to a CTextView's left margin	*/
/*	Last edited: October 13, 1993 at 6:20 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/LineNumDecoration.h>

#include <libs/graphicInterface/framework/CTextView.h>
#include <libs/graphicInterface/framework/LineEditor.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* LineNumDecoration object */

typedef struct LineNumDecoration_Repr_struct
  {
    /* appearance */
    int			digits;
    int			extra;
    unsigned char	style;

  } LineNumDecoration_Repr;


#define R(ob)		(ob->LineNumDecoration_repr)

#define INHERITED	MarginDecoration






/*************************/
/*  Miscellaneous	 */
/*************************/




/* appearance */

#define DEFAULT_DIGITS	4
#define EXTRA_WIDTH	2







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




void LineNumDecoration::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(MarginDecoration);
    REQUIRE_INIT(CTextView);
}




void LineNumDecoration::FiniClass(void)
{
  /* nothing */
}






/**********************/
/*  Instance creation */
/**********************/




LineNumDecoration * LineNumDecoration::Create(void)
{
  LineNumDecoration * dec;
  
  dec= new LineNumDecoration;
  dec->LineNumDecoration::Init();
  
  return dec;
}




/****************************/
/*  Instance initialization */
/****************************/




META_IMP(LineNumDecoration)




LineNumDecoration::LineNumDecoration(void)
          : MarginDecoration()
{
  /* allocate instance's private data */
    this->LineNumDecoration_repr = (LineNumDecoration_Repr *) get_mem(sizeof(LineNumDecoration_Repr), "LineNumDecoration instance");
}




void LineNumDecoration::Init(void)
{
  this->INHERITED::Init();
  
  R(this)->digits = DEFAULT_DIGITS;
  R(this)->extra  = EXTRA_WIDTH;
  R(this)->style  = STYLE_ITALIC;
}




LineNumDecoration::~LineNumDecoration()
{
  free_mem((void*) this->LineNumDecoration_repr);
}






/***********/
/* Options */
/***********/



      
void LineNumDecoration::SetOptions(int digits, int extra, unsigned char style)
{
  R(this)->digits = digits;
  R(this)->extra  = extra;
  R(this)->style  = style;
}




void LineNumDecoration::GetOptions(int &digits, int &extra, unsigned char &style)
{
  digits = R(this)->digits;
  extra  = R(this)->extra;
  style  = R(this)->style;
}






/***************/
/* Margin text */
/***************/



      
int LineNumDecoration::Width(void)
{
  return R(this)->digits + R(this)->extra;
}



      
void LineNumDecoration::GetMarginText(int c_linenum,
                                      TextChar * textstring_tc_ptr,
                                      ColorPair * textdata_chars)
{
  char buffer[100];
  int k;
  
  /* insert the digits */
    sprintf(buffer, "%*d", R(this)->digits, c_linenum+1);	/* 'c_linenum' is 0-based */
    for( k = 0;  k < R(this)->digits;  k++ )
      textstring_tc_ptr[k] = makeTextChar(buffer[k], R(this)->style);

  /* insert the extra space */
    for( k = R(this)->digits;  k < R(this)->digits + R(this)->extra;  k++ )
      textstring_tc_ptr[k] = makeTextChar(' ', STYLE_NORMAL);
}






/*********/
/* Input */
/********/



      
void LineNumDecoration::Clicked(int c_linenum, int char1)
{
  this->GetEditor()->SetSelection(c_linenum, 0, c_linenum, INFINITY);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
