/* $Id: Utility.C,v 1.1 1997/06/25 13:51:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/Utility.c						*/
/*									*/
/*	Utility -- Commonly useful stuff for New Fortran Editor		*/
/*	Last edited: September 29, 1992 at 6:17 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>
#include <libs/graphicInterface/cmdProcs/newEditor/Utility.h>





/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  mumble		*/
/************************/




/* ... */






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




Color black_color;
Color white_color;






/************************/
/*  Initialization	*/
/************************/




void ut_Init()
{
  /* colors */
    black_color = getColorFromName("black");
    white_color = getColorFromName("white");
}




void ut_Fini()
{
  /* nothing */
}






/************************/
/*  Messages		*/
/************************/




void notImplemented(char *what)
  // char * what;
{
  message("%s is not implemented.", what);
}




void CHECK(int what)
  // int what;
{
  /* TEMPORARY */
  if( what < 0 )  die_with_message("Fatal error: what = %d", what);
}






/************************/
/*  Rectangles		*/
/************************/




void setRect(Rectangle *r, int left, int top, int right, int bottom)
//   Rectangle *r;
//   int left, top, right, bottom;
{
  *r = makeRect(makePoint(left,top), makePoint(right,bottom));
}




void setRectSize(Rectangle *r, int left, int top, int width, int height)
//   Rectangle *r;
//   int left, top, width, height;
{
  *r = makeRectFromSize(makePoint(left,top), makePoint(width,height));
}







/************************/
/*  Mouse		*/
/************************/




Boolean stillDown(Point *mousePt)
  // Point *mousePt;
{
  if( readyEvent() )
    { getEvent();
      switch( mon_event.type )
        {
          case MOUSE_UP:
          case MOUSE_EXIT:
          case EVENT_KEYBOARD:
            return false;
          case MOUSE_DRAG:
            { *mousePt = mon_event.loc;
              return true;
            }
          default:
            die_with_message("Utility.stillDown: unexpected screen event.");
            /*NOTREACHED*/
        }
    }
  else
    return true;
}








/************************/
/*  Color		*/
/************************/




Color color(char *name)
  // char * name;
{
  return getColorFromName(name);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */


