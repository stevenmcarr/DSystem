/* $Id: ViewFilter.C,v 1.1 1997/06/25 13:51:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/ViewFilter.c						*/
/*									*/
/*	ViewFilter -- determines how text is displayed			*/
/*	Last edited: October 14, 1992 at 5:50 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>
#include <libs/graphicInterface/cmdProcs/newEditor/TextView.h>
#include <libs/graphicInterface/cmdProcs/newEditor/ViewFilter.h>

#include <libs/support/arrays/FlexibleArray.h>

#include <libs/support/database/context.h>




/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* ViewFilter object */

typedef struct
  {
    Generic		filterOb;
    vf_FilterFunc       filterProc;
    int			elision;

    Generic		contents;
    TV_Methods *	methods;

    Generic		notifyOb;	/* TEMPORARY -- only one notifyee */
    vf_NotifyFunc       notifyProc;

    Flex *		c2v;
    Flex *		v2c;
    int			c_frontier;
    int			v_frontier;

    int                 c_max_query;

  } vf_Repr;

#define	R(ob)		((vf_Repr *) ob)




/* Contents-to-view mapping */

typedef struct
  {
    Boolean		visible;
    int			first;
    int			last;

  } C2V_Entry;




/* View-to-contents mapping */

typedef struct
  {
    Boolean		visible;
    union
      {
        struct
          {
            int		lineNum;
            int		sublineNum;

          }		line;
        struct
          {
            int		first;
            int		last;

          }		elision;
      }			u;

  } V2C_Entry;






/************************/
/*  Miscellaneous	*/
/************************/




#define INFINITY	9999		/* must fit in a 'short' */


static TextString vf_elisions[3];


static int 	  vf_InitCount = 0;





/************************/
/* Forward declarations */
/************************/




STATIC(Boolean,		ensure,(ViewFilter vf, int lineNum, Boolean inContents));
STATIC(Boolean,		advance,(ViewFilter vf));
STATIC(void,		backtrack,(ViewFilter vf, int c_lineNum));
STATIC(void,		convert_c2v,(ViewFilter vf, int c_lineNum, Boolean last,
                                     int *v_lineNum, Boolean *visible));
STATIC(void,		convert_v2c,(ViewFilter vf, int v_lineNum, Boolean last,
                                     int *c_lineNum, int *sublineNum, Boolean *visible));






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Initialization	*/
/************************/




void vf_Init()
{
  char dashes[80];
  int k;

  if( vf_InitCount++ == 0 )
    { /* initialize the various elision line styles */
        for( k = 0;  k < 80;  k++ )  dashes[k] = '-';

        vf_elisions[vf_NOTHING]  = emptyTextString;

        vf_elisions[vf_ELLIPSIS] = makeTextString(" ...",STYLE_NORMAL,"vf_Init");
        vf_elisions[vf_ELLIPSIS].ephemeral = false;

        vf_elisions[vf_DIVLINE]  = makePartialTextString(dashes, STYLE_NORMAL | ATTR_HALF, 80, "vf_Init");
        vf_elisions[vf_DIVLINE].ephemeral = false;
    }
}




void vf_Fini()
{
  if( --vf_InitCount == 0 )
    { destroyTextString(vf_elisions[vf_ELLIPSIS]);
      destroyTextString(vf_elisions[vf_DIVLINE]);
    }
}




/*ARGSUSED*/

ViewFilter vf_Open(Context context, DB_FP *fp, Generic filterOb, vf_FilterFunc filterProc)
{
  ViewFilter vf;

  /* allocate a new instance */
    vf = (ViewFilter) get_mem(sizeof(vf_Repr),"FortEditor:ViewFilter");
    if( (Generic) vf == 0 ) return UNUSED;

  /* initialize the parts */
    /* set creation parameters */
      R(vf)->filterOb   = filterOb;
      R(vf)->filterProc = filterProc;

    /* initialize contents */
      R(vf)->contents = nil;

    /* initialize conversion arrays -- not saved for now */
      R(vf)->c2v = flex_create(sizeof(C2V_Entry));
      R(vf)->v2c = flex_create(sizeof(V2C_Entry));
      R(vf)->c_frontier = 0;
      R(vf)->v_frontier = 0;

      R(vf)->c_max_query = 0;

  /* initialize misc. status */
    R(vf)->elision = vf_ELLIPSIS;

  return (Generic) vf;
}




void vf_Close(ViewFilter vf)
{
  if( vf != UNUSED )
    { flex_destroy(R(vf)->c2v);
      flex_destroy(R(vf)->v2c);

      free_mem((void*) vf);
    }
}






/************************/
/*  Filter specs	*/
/************************/




void vf_SetSpecs(ViewFilter vf, Generic filterOb, vf_FilterFunc filterProc)
{
  R(vf)->filterOb   = filterOb;
  R(vf)->filterProc = filterProc;
  backtrack(vf,0);
}




void vf_SetElision(ViewFilter vf, int elision)
{
  R(vf)->elision = elision;
  backtrack(vf,0);
}




void vf_SetContents(ViewFilter vf, Generic contents, TV_Methods *methods)
{
  R(vf)->contents = contents;
  R(vf)->methods  = methods;
  backtrack(vf,0);
}






/************************/
/*  Change notification */
/************************/




void vf_Notify(ViewFilter vf, Generic ob, vf_NotifyFunc notifyProc)
{
  R(vf)->notifyOb   = ob;
  R(vf)->notifyProc = notifyProc;
}




void vf_NoteChange(ViewFilter vf, int kind, Boolean autoScroll, 
                   int c_first, int c_last, int c_delta)
{
  int old_v_first, old_v_last, old_v_lastPlus1;
  int v_first, v_last, v_lastPlus1, v_delta;
  Boolean punt, dummy;

  switch( kind )
    {
      case NOTIFY_SEL_CHANGED:
        convert_c2v(vf, c_first, false, &v_first, &dummy);
        convert_c2v(vf, c_last,  true,  &v_last,  &dummy);
        (R(vf)->notifyProc) (R(vf)->notifyOb, kind, autoScroll,
                                              v_first, v_last, 0);
        break;


      case NOTIFY_DOC_WILL_CHANGE:
        break;


      case NOTIFY_DOC_CHANGED:
         /* ??? will this work if 'R(vf)->elipsis == vf_NOTHING' ? */

        /* remember old facts to calculate 'v_delta' etc */
          convert_c2v(vf, c_first, false, &old_v_first, &dummy);
          punt = BOOL( c_last+1 > R(vf)->c_frontier );
          if( punt )
            old_v_last = INFINITY;
          else
            { convert_c2v(vf, c_last, true, &old_v_last, &dummy);
              convert_c2v(vf, c_last+1, true, &old_v_lastPlus1, &dummy);
            }

        backtrack(vf, c_first);

        /* calculate view delta */
          if( punt )
            v_delta = INFINITY;
          else
            { convert_c2v(vf, c_first, false, &v_first, &dummy);
              convert_c2v(vf, c_last+1, true, &v_lastPlus1, &dummy);
              v_delta = c_delta +
                        ( (v_lastPlus1 - v_first) - (old_v_lastPlus1 - old_v_first) );
            }

        (R(vf)->notifyProc) (R(vf)->notifyOb, kind, autoScroll,
                                              old_v_first, old_v_last, v_delta);

        break;
    }
}






/************************/
/*  Filtering		*/
/************************/




void vf_GetDocSize(ViewFilter vf, Point *size)
{
  float ratio;
# define PADDING    20

  (R(vf)->methods->getDocSize) (R(vf)->contents,size);

  if( R(vf)->c_frontier < size->y )
    (void) ensure(vf, R(vf)->c_max_query + PADDING, true);

  if( R(vf)->c_frontier < size->y )
    { /* we have not yet determined the last view line num */
      if( R(vf)->c_frontier == 0 )
        ratio = 1.0;
      else
        ratio = ((float) R(vf)->v_frontier) / ((float) R(vf)->c_frontier);
      size->y = max( (int) (size->y * ratio), R(vf)->v_frontier + PADDING );
    }
  else
    { /* we know the view size exactly */
      size->y = R(vf)->v_frontier;
    }
}




void vf_GetLine(ViewFilter vf, int v_lineNum, TextString *text, TV_Data *data)
{
  int c_lineNum,sublineNum;
  Boolean visible,exists;

  convert_v2c(vf,v_lineNum,false,&c_lineNum,&sublineNum,&visible);

  if( visible )
    { exists = (R(vf)->methods->getLine) (R(vf)->contents, c_lineNum, text, (Generic)data);
      if( exists )
        (R(vf)->filterProc) (R(vf)->filterOb, false, R(vf)->contents, c_lineNum, 
                             &sublineNum, text, (VF_MaxData*)data);
      else
        { *text = emptyTextString;
          tv_DefaultData(data);
        }
    }
  else
    { *text = copyTextString( vf_elisions[R(vf)->elision] );
      tv_DefaultData(data);
    }
}




void vf_GetSelection(ViewFilter vf, int *v_lineNum, int *v_sel1, int *v_sel2)
{
  int c_lineNum,c_sel1,c_sel2;
  Boolean visible;

  (R(vf)->methods->getSelection) (R(vf)->contents,&c_lineNum,&c_sel1,&c_sel2);

  if( c_lineNum != UNUSED )
    { convert_c2v(vf,c_lineNum,false,v_lineNum,&visible);
      if( visible )
        { *v_sel1 = c_sel1;
          *v_sel2 = c_sel2;
        }
      else
        { *v_sel1 = 1;
          *v_sel2 = 0;
        }
    }
  else
    { *v_lineNum = UNUSED;
      convert_c2v(vf,c_sel1,false,v_sel1,&visible);
      convert_c2v(vf,c_sel2,true, v_sel2,&visible);
    }
}




void vf_SetSelection(ViewFilter vf, int v_lineNum, int v_sel1, int v_sel2)
{
  int c_lineNum1,c_lineNum2,sublineNum,c_sel1,c_sel2,idummy;
  Boolean visible,bdummy;

  if( v_lineNum != UNUSED )
    { convert_v2c(vf,v_lineNum,false,&c_lineNum1,&sublineNum,&visible);
      if( visible && sublineNum == 0 )
        { c_sel1 = v_sel1;
          c_sel2 = v_sel2;
        }
      else
        { convert_v2c(vf,v_lineNum,true,&c_lineNum2,&idummy,&bdummy);
          c_sel1 = c_lineNum1;
          c_sel2 = c_lineNum2;
          c_lineNum1 = UNUSED;
        }
    }
  else
    { convert_v2c(vf,v_sel1,false,&c_sel1,&idummy,&bdummy);
      convert_v2c(vf,v_sel2,true, &c_sel2,&idummy,&bdummy);
      c_lineNum1 = UNUSED;
    }

  (R(vf)->methods->setSelection) (R(vf)->contents,c_lineNum1,c_sel1,c_sel2);
}




Boolean vf_ContentsToView(ViewFilter vf, int c_lineNum, int *v_lineNum)
{
  Boolean visible;

  convert_c2v(vf,c_lineNum,false,v_lineNum,&visible);
  return visible;
}




Boolean	vf_ViewToContents(ViewFilter vf, int v_lineNum, int *c_lineNum)
{
  int dummy;
  Boolean visible;

  convert_v2c(vf,v_lineNum,false,c_lineNum,&dummy,&visible);
  return visible;
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
Boolean ensure(ViewFilter vf, int lineNum, Boolean inContents)
{
  Boolean more;

  more = true;
  while( more &&
         lineNum >= (inContents ? R(vf)->c_frontier : R(vf)->v_frontier) )
    more = advance(vf);

  return more;
}




static
Boolean advance(ViewFilter vf)
{
  int c = R(vf)->c_frontier;
  int v = R(vf)->v_frontier;
  TextString text;
  TV_Data data;
  int numSublines,k;
  C2V_Entry c2vEntry;
  V2C_Entry v2cEntry;
  Boolean exists,needNewElision;

  /* filter the next contents line */
    exists = (R(vf)->methods->getLine) (R(vf)->contents, c, &text, (Generic)&data);
    if( ! exists )  return false;
    (R(vf)->filterProc) (R(vf)->filterOb, true, R(vf)->contents, c, &numSublines,
                         &text, (VF_MaxData*)&data);
    destroyTextString(text);

  if( numSublines > 0 )
    { /* advance the c2v array */
        c2vEntry.visible = true;
        c2vEntry.first   = v;
        c2vEntry.last    = v + numSublines - 1;
        flex_set_one(R(vf)->c2v,c,(char *)&c2vEntry);
        R(vf)->c_frontier += 1;

      /* advance the v2c array */
        for( k = 0;  k < numSublines;  k++ )
          { v2cEntry.visible           = true;
            v2cEntry.u.line.lineNum    = c;
            v2cEntry.u.line.sublineNum = k;
            flex_set_one(R(vf)->v2c,v+k,(char *)&v2cEntry);
          }
        R(vf)->v_frontier += numSublines;
    }
  else
    { /* set 'v' to the elision line for 'c' if any */
        if( R(vf)->elision != vf_NOTHING )
          { if( v == 0 )
              needNewElision = true;
            else
              { (void) flex_get_buffer(R(vf)->v2c,v-1,1,(char *)&v2cEntry);
                needNewElision = v2cEntry.visible;
              }

            if( needNewElision )
              { v2cEntry.visible         = false;
                v2cEntry.u.elision.first = c;
                v2cEntry.u.elision.last  = c;
                R(vf)->v_frontier += 1;
              }
            else
              v = v - 1;
          }

      /* advance the c2v array */
        c2vEntry.visible = false;
        c2vEntry.first   = v;
        c2vEntry.last    = v;
        flex_set_one(R(vf)->c2v, c, (char *) &c2vEntry);
        R(vf)->c_frontier += 1;

      /* advance the v2c array if appropriate */
        if( R(vf)->elision != vf_NOTHING )
          { /* ASSERT: 'v2cEntry' has the entry for the elision line */
            v2cEntry.u.elision.last = c;
            flex_set_one(R(vf)->v2c,v,(char *)&v2cEntry);
            /* ASSERT: 'v_frontier' is already increased if necessary */
          }
    }

  return true;
}




static
void backtrack(ViewFilter vf, int c_lineNum)
{
  int v_lineNum,elisionStart,idummy;
  Boolean c_visible, v_visible, discard_this_v;

  if( c_lineNum < R(vf)->c_frontier )
    { convert_c2v(vf, c_lineNum, false, &v_lineNum, &c_visible);
      if( c_visible )
        { R(vf)->c_frontier = c_lineNum;
          R(vf)->v_frontier = v_lineNum;
        }
      else
        { convert_v2c(vf, v_lineNum, false, &elisionStart, &idummy, &v_visible);
          R(vf)->c_frontier = c_lineNum;
          discard_this_v = (v_visible ? true : c_lineNum == elisionStart);
          R(vf)->v_frontier = v_lineNum + (discard_this_v ? 0 : 1);
        }
    }

  R(vf)->c_max_query = min(R(vf)->c_max_query, c_lineNum);
}




static
void convert_c2v(ViewFilter vf, int c_lineNum, Boolean last, 
                 int *v_lineNum, Boolean *visible)
{
  C2V_Entry entry;

  if( c_lineNum == INFINITY )
    { *v_lineNum = INFINITY;
      *visible = true;
    }
  else if( ensure(vf,c_lineNum,true) )
    { (void) flex_get_buffer(R(vf)->c2v,c_lineNum,1,(char*)&entry);

      *v_lineNum = (last ? entry.last : entry.first);
      *visible   = entry.visible;
    }
  else
    { *v_lineNum = R(vf)->v_frontier + (c_lineNum - R(vf)->c_frontier);
      *visible = true;
    }

  R(vf)->c_max_query = max(R(vf)->c_max_query, c_lineNum);
}




static
void convert_v2c(ViewFilter vf, int v_lineNum, Boolean last, int *c_lineNum, 
                 int *sublineNum, Boolean *visible)
{
  V2C_Entry entry;

  if( v_lineNum == INFINITY )
    { *c_lineNum = INFINITY;
      *sublineNum = 0;
      *visible = true;
    }
  else if( ensure(vf,v_lineNum,false) )
    { (void) flex_get_buffer(R(vf)->v2c,v_lineNum,1,(char *)&entry);

      if( entry.visible )
        { *c_lineNum  = entry.u.line.lineNum;
          *sublineNum = entry.u.line.sublineNum;
        }
      else
        { *c_lineNum  = (last ? entry.u.elision.last : entry.u.elision.first);
          *sublineNum = 0;
        }
      *visible = entry.visible;
    }
  else
    { *c_lineNum  = R(vf)->c_frontier + (v_lineNum - R(vf)->v_frontier);
      *sublineNum = 0;
      *visible = true;
    }

  R(vf)->c_max_query = max(R(vf)->c_max_query, *c_lineNum);
}




