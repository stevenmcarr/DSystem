/* $Id: FortEditor.C,v 1.2 1997/03/11 14:30:49 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditor/FortEditor.c					*/
/*									*/
/*	FortEditor -- abstract text/structure editor for Fortran	*/
/*	Last edited: July 3, 1990 at 12:18 pm				*/
/*									*/
/************************************************************************/



#include <include/bstring.h>
#include <libs/support/msgHandlers/Changes.h>

#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>
#include <libs/graphicInterface/cmdProcs/newEditor/NedScrap.h>

#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/FortEditor.i>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/FortEditorSave.h>

#include <libs/graphicInterface/oldMonitor/include/sms/scroll_sm.h>
#include <ctype.h>
#include <libs/support/arrays/FlexibleArray.h>

#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/FortScrap.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/EditorView.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/Expander.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/Searcher.h>

#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>

#if 0
#include <libs/fileAttrMgmt/attributedFile/AttributedFile.h>
#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/FortTextTreeModAttr.h>
#endif

EXTERN(void, vf_Init,(void));
EXTERN(void, vf_Fini,(void));


#if 0
extern Composition *checked_program_context;
#endif

/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/


typedef void (*NotifyMethod)(EditorView ev, int kind, Boolean autoScroll, 
			    FortTreeNode node, int first, int last, int delta) ;

typedef struct
  {
    Generic		ob;
    NotifyMethod	method;
  } NotifyRequest;




typedef struct
  {
    /* contents */
      FortTree		ft;
      FortTextTree	ftt;

      FortranModule       *fmodule;

    /* specialist subparts */
      Expander		ex;
      Searcher		srch;

    /* change notification */
      Flex *		notifiees;

    /* status */
      Selection		sel;
      int		curLine;
      TextString	curText;
      int		curTextSize;
      Boolean		curLineChanged;

    /* deferred updates */
      int		freeze;
      Boolean		changeDeferred;
      int		changeLine;
      Boolean		changeAutoscroll;
      Boolean		changeInitialLineSel;
      Selection		oldSel;
      Boolean		needCurLine;

  } ed_Repr;

#define	R(ob)		((ed_Repr *) ob)






/************************/
/*  Initialization	*/
/************************/




static int		ed_InitCount = 0;






/************************/
/*  Saving		*/
/************************/




char			*ed_SourceAttribute;






/************************/
/*  Miscellaneous	*/
/************************/




# undef INFINITY    // squash definition imported with framework.h

# define CONTINUE_COL        6-1      /* here cols are numbered from 0 */
# define INFINITY          99999






/************************/
/* Forward declarations	*/
/************************/




STATIC(void,			getSelection,(Selection sel, int *line1, int *char1,
                                              int *line2, int *char2));
STATIC(void,			setSelection,(FortEditor ed, int line, int sel1, 
                                              int sel2));
STATIC(void,                    getStructureSelection,(FortEditor ed));
STATIC(void,			getCurLine,(FortEditor ed, int line));
STATIC(void,			putCurLine,(FortEditor ed));
STATIC(void,			docChanged,(FortEditor ed, int oneLine, 
                                            Boolean autoscroll));
STATIC(Boolean,			insidePlaceholder,(FortEditor ed, int pos, 
                                                   int *first, int *last));
STATIC(Boolean,			placeholderChar,(FortEditor ed, int k, Boolean *first));
STATIC(void,                    notePlaceholderSelection,(FortEditor ed, 
                                                          Boolean always));
STATIC(void,			edcopy,(FortEditor ed, FortScrap *scrap));
STATIC(void,			edpaste,(FortEditor ed, FortScrap scrap));
STATIC(void,			edclear,(FortEditor ed));
STATIC(Boolean,			insertChar,(FortEditor ed, int start, char ch,
                                           Boolean underline));
STATIC(int,			insertChars,(FortEditor ed, int start, int count,
                                             TextString chars, Boolean respectField));
STATIC(int,			insertInField,(TextChar *buffer, int fieldStart, int
                                               fieldCount, int fieldMax, int start,
                                               int count, TextString chars));
STATIC(void,			deleteChars,(FortEditor ed, int start, int count));
STATIC(int,			deletePortion,(TextChar *buffer, int buffCount, int
                                               fieldStart, int fieldMax, int start,
                                               int count));
STATIC(int,			deleteInField,(TextChar *buffer, int fieldShart, 
                                               int fieldCount, int fieldMax, int start, 
                                               int count, Boolean fill));
STATIC(void,			insertNewLine,(FortEditor ed, int lineNum));
STATIC(void,			initialLineSelection,(FortEditor ed, int lineNum));
STATIC(Boolean,                 curLineIsComment,(FortEditor ed));
STATIC(void,			beginEdit,(FortEditor ed));
STATIC(void,			endEdit,(FortEditor ed));
STATIC(Boolean,			shouldInsertAfter,(FortEditor ed));
STATIC(void,			other,(FortEditor ed));
STATIC(void,			removeUnderlining,(FortEditor ed));
STATIC(void,			addNotifiee,(FortEditor ed, Generic ob, 
					     NotifyMethod method));
STATIC(void,			notify,(FortEditor ed, int kind, Boolean autoscroll,
                                        FortTreeNode node, int first, int last, 
                                        int delta));




/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Initialization	*/
/************************/




void ed_Init()
{
  if( ed_InitCount++ == 0 )
    { /* initialize submodules */
        ft_Init();
        ftt_Init();

        fsc_Init();
        vf_Init();
        ex_Init();
        srch_Init();

      /* initialize ourselves */
        ed_SourceAttribute = ft_SourceAttribute;
    }
}




void ed_Fini()
{
  if( --ed_InitCount == 0 )
    { /* finalize submodules */
        srch_Fini();
        ex_Fini();
        vf_Fini();
        fsc_Fini();

        ftt_Fini();
        ft_Fini();
    }
}




#if 0
FortEditor ed_Open(Context context, DB_FP *fp, FortTextTree ftt, FortTree ft) 
{
  R(ed)->fmodule->DetachAttribute(R(ed)->ftAttr);
  R(ed)->fmodule->DetachAttribute(R(ed)->fttAttr);
  
  if (checked_program_context) {
    delete checked_program_context;
  } else delete R(ed)->fmodule;

  return ed_OpenTree(context, fp, ftt, ft);
}
#endif




FortEditor ed_Open(Context context, DB_FP *fp, FortTextTree ftt, FortTree ft)
{
  FortEditor ed;
  Boolean savedEditState;
  int line, sel1, sel2;
  
  /* allocate a new instance */
  ed = (FortEditor) get_mem(sizeof(ed_Repr),"FortEditor");
  if( (Generic) ed == 0 ) return UNUSED;
  
  /* initialize the parts */
  /* set creation parameters */
  
  /* initialize change notification */
  R(ed)->notifiees = flex_create(sizeof(NotifyRequest));
  
  /* init current-line buffer */
  R(ed)->sel.line       = 0;
  R(ed)->sel.sel1       = 0;
  R(ed)->sel.sel2       = -1;
  R(ed)->curLine        = UNUSED;
  R(ed)->curTextSize    = 100;
  R(ed)->curText        = createTextString(R(ed)->curTextSize,
					   "FortEditor currentLine");
  R(ed)->curText.num_tc = 0;
  R(ed)->curLineChanged = false;
  
  R(ed)->freeze = 0;
  R(ed)->changeDeferred = false;
  R(ed)->oldSel = R(ed)->sel;
  
  R(ed)->fmodule = (FortranModule *) context;

  /* initialize the tree */
  R(ed)->ftt = ftt;
  R(ed)->ft = ft;
  
  /* open other subparts & initialize saved private information */
  if( fp == DB_NULLFP )
    savedEditState = false;
  else
    (void) db_buffered_read(fp, (char *) &savedEditState, sizeof(Boolean));
  
  if( savedEditState )
    { (void) db_buffered_read(fp, (char *) &line, sizeof(int));
      (void) db_buffered_read(fp, (char *) &sel1, sizeof(int));
      (void) db_buffered_read(fp, (char *) &sel2, sizeof(int));
      
      R(ed)->ex   = ex_Open(context,fp,ed,R(ed)->ftt);
      R(ed)->srch = srch_Open(context,fp,ed);
      
      getCurLine(ed, line);
      ed_SetSelection(ed, line, sel1, sel2);
    } else {
      R(ed)->ex   = ex_Open(context,DB_NULLFP,ed,R(ed)->ftt);
      R(ed)->srch = srch_Open(context,DB_NULLFP,ed);
      
      if( ftt_NumLines(R(ed)->ftt) == 0 )
	ftt_InsertLine(R(ed)->ftt, 0, emptyTextString);
      getCurLine(ed, 0);
      initialLineSelection(ed, 0);
    }
  
  return ed;
}




void ed_Close(FortEditor ed)
{
  /* destroy the notify list */
  flex_destroy(R(ed)->notifiees);

  /* destroy line buffer */
  destroyTextString(R(ed)->curText);

  /* destroy the subparts */
  srch_Close(R(ed)->srch);
  ex_Close(R(ed)->ex);

  free_mem((void*) ed);
}




void ed_Save(FortEditor ed, Context context, DB_FP *fp, Boolean saveEditState)
{
  /* select the fort tree */
    ft_AstSelect(R(ed)->ft);

  /* save contents */
    putCurLine(ed);
    (void) ft_Check(R(ed)->ft);
    ftt_TreeChanged(R(ed)->ftt, ft_Root(R(ed)->ft));  /* since root, no need to warn */

  /* export to the reference source file */
    ed_RefSrcSave((FortranModule *) context, message, yes_no);

    //----------------------------------------------------------------
    // enable attribute caching, save attributes to cache, then
    // disable saves again until next time user requests a save.
    //----------------------------------------------------------------
    context->EnableAttributeCaching();
  
    if( fp != DB_NULLFP )
      (void) db_buffered_write(fp, (char *) &saveEditState, sizeof(Boolean));

  /* save edit session state if asked */
    if( saveEditState )
      { if( fp != DB_NULLFP )
          { (void) db_buffered_write(fp, (char *) &R(ed)->sel.line, sizeof(int));
            (void) db_buffered_write(fp, (char *) &R(ed)->sel.sel1, sizeof(int));
            (void) db_buffered_write(fp, (char *) &R(ed)->sel.sel2, sizeof(int));
          }

        /* subparts */
          ex_Save(R(ed)->ex,context,fp);
          srch_Save(R(ed)->srch,context,fp);
      }
}




void ed_Notify(FortEditor ed, Generic ob, NotifyMethod method)
{
  addNotifiee(ed, ob, method);
}






/************************/
/*  Contents		*/
/************************/




Point ed_DocSize(FortEditor ed)
{
  Point size;

  size = ftt_GetDocSize(R(ed)->ftt);
  size.x = max(size.x, R(ed)->curText.num_tc);

  return size;
}




void ed_GetTextTree(FortEditor ed, FortTextTree *ftt)
{
  *ftt = R(ed)->ftt;
}




void ed_GetTree(FortEditor ed, FortTree *ft)
{
  *ft = R(ed)->ft;
}




void ed_GetRoot(FortEditor ed, FortTreeNode *node)
{
  *node = ft_Root(R(ed)->ft);
}




int ed_NumLines(FortEditor ed)
{
  return ftt_NumLines(R(ed)->ftt);
}




void ed_GetLine(FortEditor ed, int lineNum, TextString *text)
{
  if( lineNum == R(ed)->curLine )
    *text = copyTextString(R(ed)->curText);
  else
    ftt_GetLine(R(ed)->ftt,lineNum,text);
}




int ed_GetLineLength(FortEditor ed, int lineNum)
{
  if( lineNum == R(ed)->curLine )
    return R(ed)->curText.num_tc;
  else
    return ftt_GetLineLength(R(ed)->ftt,lineNum);
}




int ed_GetLineIndent(FortEditor ed, int lineNum)
{
    return ftt_GetLineIndent(R(ed)->ftt,lineNum);
}




char * ed_GetTextLine(FortEditor ed, int lineNum)
{
  TextString ts;
  char *text;
  int i;

  if( lineNum == R(ed)->curLine )
    { /* flatten the text-string (losing style information) */
        ts = R(ed)->curText;
        text = (char*)get_mem(ts.num_tc + 1, "ed_GetTextLine");
        for( i = 0; i < ts.num_tc; i++ )
          text[i] = ts.tc_ptr[i].ch;
        text[ts.num_tc] = '\0';

      return text;
    }
  else
    return ftt_GetTextLine(R(ed)->ftt, lineNum);
}




Boolean ed_NodeToText(FortEditor ed, FortTreeNode node, int *line1, 
                      int *char1, int *line2, int *char2)
{
  return ftt_NodeToText(R(ed)->ftt, node, line1, char1, line2, char2);
}




Boolean ed_TextToNode(FortEditor ed, int line1, int char1, int line2, 
                      int char2, FortTreeNode *node)
{
  return ftt_TextToNode(R(ed)->ftt, line1, char1, line2, char2, node);
}




void ed_TreeWillChange(FortEditor ed, FortTreeNode node)
{
  ftt_TreeWillChange(R(ed)->ftt, node);
}




void ed_TreeChanged(FortEditor ed, FortTreeNode node)
{
  other(ed);
  
  ftt_TreeChanged(R(ed)->ftt, node);
  getCurLine(ed,R(ed)->curLine);
  docChanged(ed,UNUSED,true);
}






/************************/
/*  Viewing		*/
/************************/




short ed_ViewScreenModuleIndex()
{
  return ev_ScreenModuleIndex();
}




Point ed_ViewSize(Point charSize, short font)
{
  return ev_ViewSize(charSize, font);
}




void ed_ViewInit(FortEditor ed, FortEditorView pane, FortVFilter filter, 
                 Point scrollPos, short font)
{
  ev_PaneInit(pane, ed, filter, scrollPos, font);
  addNotifiee(ed, (Generic) pane, ev_NoteChange);
}




/*ARGSUSED*/

void ed_ViewScrollBars(FortEditor ed, FortEditorView pane, 
                       ScrollBar hscroll, ScrollBar vscroll)
{
  ev_ScrollBars(pane,hscroll,vscroll);
}




/*ARGSUSED*/

Point ed_ViewGetSize(FortEditor ed, FortEditorView pane)
{
  return ev_GetViewSize(pane);
}




/*ARGSUSED*/

void ed_ViewGetFilter(FortEditor ed, FortEditorView pane, FortVFilter *filter)
{
  ev_GetFilter(pane,filter);
}




/*ARGSUSED*/

void ed_ViewSetFilter(FortEditor ed, FortEditorView pane, FortVFilter filter)
{
  Rectangle infinity;

  setRect(&infinity,0,0,999999,999999);
  ev_SetFilter(pane,filter,false,infinity);
}




/*ARGSUSED*/

void ed_ViewSetFilterFast(FortEditor ed, FortEditorView pane, 
                           FortVFilter filter, Rectangle changed)
{
  ev_SetFilter(pane,filter,true,changed);
}




/*ARGSUSED*/

void ed_ViewGetConceal(FortEditor ed, FortEditorView pane, int line, 
                       Boolean *conceal)
{
  ftt_GetConceal(R(ed)->ftt,line,conceal);
}




/*ARGSUSED*/

void ed_ViewSetConceal(FortEditor ed, FortEditorView pane, int line1, 
                       int line2, Boolean conceal)
{
  ftt_SetConceal(R(ed)->ftt,line1,line2,conceal);
  docChanged(ed,UNUSED,true);
}




/*ARGSUSED*/

void ed_ViewSetConcealNone(FortEditor ed, FortEditorView pane, int line1, 
                           int line2)
{
  ftt_SetConcealNone(R(ed)->ftt,line1,line2);
  docChanged(ed,UNUSED,true);
}




/*ARGSUSED*/

Point ed_ViewGetScroll(FortEditor ed, FortEditorView pane)
{
  return ev_GetScroll(pane);
}




/*ARGSUSED*/

void ed_ViewSetScroll(FortEditor ed, FortEditorView pane, Point scrollPos)
{
  ev_SetScroll(pane,scrollPos);
}




/*ARGSUSED*/

void ed_ViewScrollBy(FortEditor ed, FortEditorView pane, Point delta)
{
  ev_ScrollBy(pane,delta);
}




/*ARGSUSED*/

void ed_ViewEnsureVisible(FortEditor ed, FortEditorView pane, Point pt)
{
  ev_EnsureVisible(pane,pt,true);
}




/*ARGSUSED*/

void ed_ViewEnsureSelVisible(FortEditor ed, FortEditorView pane)
{
  Point pt;

  if( R(ed)->sel.line == UNUSED )
    pt = makePoint(0,R(ed)->sel.sel1);
  else
    pt = makePoint(R(ed)->sel.sel1,R(ed)->sel.line);

  ev_EnsureVisible(pane,pt,true);
}




/*ARGSUSED*/

void ed_ViewMouse(FortEditor ed, FortEditorView pane, Point mousePt)
{
  /* ... */
}




/*ARGSUSED*/

void ed_ViewGetSelectionBehavior(FortEditor ed, FortEditorView pane, 
                                 int *beh)
{
  ev_GetSelectionBehavior(pane,beh);
}




/*ARGSUSED*/

void ed_ViewSetSelectionBehavior(FortEditor ed, FortEditorView pane, 
                                 int beh)
{
  ev_SetSelectionBehavior(pane,beh);
}






/************************/
/*  Selecting		*/
/************************/




void ed_GetSelection(FortEditor ed, int *line, int *sel1, int *sel2)
{
  *line = R(ed)->sel.line;
  *sel1 = R(ed)->sel.sel1;
  *sel2 = R(ed)->sel.sel2;
}




void ed_SetSelection(FortEditor ed, int line, int sel1, int sel2)
{
  int oldLine = R(ed)->sel.line;
  int maxLine = ed_NumLines(ed) - 1;
  int maxWidth;

  /* enforce constraints on selection location */
    if( line != UNUSED )
      { line = max(0,      min(maxLine, line));
        maxWidth = ed_GetLineLength(ed, line);
        sel1 = max(0,      min(maxWidth,     sel1));
        sel2 = max(sel1-1, min(maxWidth - 1, sel2));
      }
    else
      { sel1 = max(0, min(maxLine,sel1));
        sel2 = max(0, min(maxLine,sel2));
      }

  if( line != oldLine )
    { putCurLine(ed);
      getCurLine(ed,line);
    }

  setSelection(ed,line,sel1,sel2);

  /* must notify Expander AFTER the selection has changed */
    notePlaceholderSelection(ed,true);
}




void ed_MoreSelection(FortEditor ed)
{
  Selection sel;

  putCurLine(ed);
  getStructureSelection(ed);
  sel = R(ed)->sel;

  if( ! sel.structure )
    ed_SetSelectedNode(ed,sel.node);
  else if( sel.node != ft_Root(R(ed)->ft) )
    ed_SetSelectedNode(ed, ftt_GetFather(R(ed)->ftt,sel.node));
}




void ed_GetSelectedNode(FortEditor ed, FortTreeNode *node)
{
  getStructureSelection(ed);
  *node = R(ed)->sel.node;
}




void ed_SetSelectedNode(FortEditor ed, FortTreeNode node)
{
  int line1,line2,char1,char2;
  Boolean res;

  res = ftt_NodeToText(R(ed)->ftt, node, &line1, &char1, &line2, &char2);

  if( res )
    ed_SetSelection(ed,UNUSED,line1,line2);
  else
    { getCurLine(ed,line1);  /* otherwise selection may get clipped */
      ed_SetSelection(ed,line1,char1,char2);
    }
}




void ed_ToggleTypingField(FortEditor ed)
{
  int line = R(ed)->sel.line;
  int sel1 = R(ed)->sel.sel1;
  int dummy;

  if( line != UNUSED )
    { if( sel1 > CONTINUE_COL )
        { sel1 = CONTINUE_COL;
          while( sel1-1 >= 0  &&  R(ed)->curText.tc_ptr[sel1-1].ch == ' ' )
            sel1 -= 1;
          if( insidePlaceholder(ed,sel1-1,&dummy,&dummy) )
            sel1 -= 1;   /* will cause whole placeholder to be selected */
        }
      else
        { sel1 = CONTINUE_COL + 1;
          while( sel1 < R(ed)->curText.num_tc  &&
                 R(ed)->curText.tc_ptr[sel1].ch == ' ' )
            sel1 += 1;
        }

      setSelection(ed,line,sel1,sel1-1);
    }
}










/************************/
/*  Editing		*/
/************************/




void ed_BeginEdit(FortEditor ed)
{
  beginEdit(ed);
}




void ed_EndEdit(FortEditor ed)
{
  /* kludge for 'srch_ReplaceText' when 'global = true' */
    R(ed)->changeAutoscroll = false;

  endEdit(ed);
}




void ed_Key(FortEditor ed, KbChar kb)
{
  int line1,char1,line2,char2,first,last,num;
  Boolean underline,wasIP;

  if( ! ( isprint((char) kb)  ||
          kb == KB_Backspace  ||  kb == KB_Delete  ||  kb == KB_Enter ) )
    return;

  if( kb == KB_Delete  ||  kb == KB_Enter )
    other(ed);
  else
    { /* note that 'KB_Backspace' WILL be passed to the expander */
      notePlaceholderSelection(ed,false);
      underline = ex_Key(R(ed)->ex, (char) kb);	/* must happen first */
    }


  getSelection(R(ed)->sel, &line1, &char1, &line2, &char2);
  wasIP = BOOL( (line1 == line2)  &&  (char1 > char2 ) );

  beginEdit(ed);
  edclear(ed);

  if( kb_swap_bs_del  &&  ( kb == KB_Backspace  ||  kb == KB_Delete ) )
    kb = ( kb == KB_Delete )  ?  KB_Backspace  :  KB_Delete;

  switch( kb )
    {
      case KB_Backspace:
        if( wasIP  &&  char1 > 0 )
          { if( insidePlaceholder(ed,char1-1,&first,&last) )
              num = last-first+1;
            else
              num = 1;

            deleteChars(ed,char1-num,num);
            docChanged(ed,line1,true);
            setSelection(ed,line1,char1-num,char1-num-1);
          }
        break;

      case KB_Delete:
        if( wasIP )
          { if( insidePlaceholder(ed,char2+1,&first,&last) )
              num = last-first+1;
            else
              num = 1;

            deleteChars(ed,char2+1,num);
            docChanged(ed,line1,true);
            setSelection(ed,line1,char2+1,char2);
          }
        break;

      case KB_Enter:
        if( shouldInsertAfter(ed) )
          insertNewLine(ed, line1+1);
        else
          insertNewLine(ed, line1);
        break;

      default:  
        if( insertChar(ed, char1, (char) kb, underline) )
          { docChanged(ed,line1,true);
            setSelection(ed,line1,char1+1,char1);
          }
        break;
    }

  endEdit(ed);
}




void ed_Expand(FortEditor ed, int choice)
{
  Expander ex = R(ed)->ex;
  FortTreeNode selected;
  Boolean wantExpand;

  /*** cyclic expansion not implemented ***/

  /* Expander is responsible for putting back current line etc */

  if( ex_Expandee(ex) )
    { if( choice == UNUSED )
        wantExpand = ex_AskChoice(ex, &choice);
      else
        wantExpand = true;

      if( wantExpand )
        { selected = ex_Expand(ex, choice);
          getCurLine(ed,R(ed)->curLine);
          docChanged(ed,UNUSED,true);
        }
    }
}




void ed_Copy(FortEditor ed, FortScrap *scrap)
{
  FortScrap my_scrap;

  edcopy(ed,&my_scrap);

  if( scrap != nil )
    *scrap = my_scrap;
  else
    sc_Set(fsc_Type,ed,my_scrap);
}




void ed_Cut(FortEditor ed, FortScrap *scrap)
{
  FortScrap my_scrap;

  other(ed);

  edcopy(ed,&my_scrap);
  edclear(ed);

  if( scrap != nil )
    *scrap = my_scrap;
  else
    sc_Set(fsc_Type,ed,my_scrap);
}




void ed_Paste(FortEditor ed, FortScrap scrap)
{
  Generic dummy;

  other(ed);

  if( scrap == nil )
    sc_Get(fsc_Type,&dummy,&scrap);

  if( scrap != nil )
    { beginEdit(ed);
      edclear(ed);
      edpaste(ed,scrap);
      endEdit(ed);
    }
}




void ed_PasteString(FortEditor ed, char *str)
{
  TextString text;
  FortScrap scrap;

  other(ed);

  text = makeTextString(str,STYLE_NORMAL,"ed_PasteString");
  scrap = fsc_CreateSmall(R(ed)->ft,text,0,text.num_tc);
  ed_Paste(ed,scrap);
  fsc_Destroy(scrap);
}




void ed_Clear(FortEditor ed)
{
  other(ed);

  edclear(ed);
}




void ed_ClearToEndOfLine(FortEditor ed)
{
  other(ed);

  if( R(ed)->sel.line != UNUSED )
    { /* trick */
      R(ed)->sel.sel2 = R(ed)->curText.num_tc;
      edclear(ed);
    }
}




void ed_CheckLine(FortEditor ed)
{
  other(ed);

  putCurLine(ed);
}




Boolean ed_CheckModule(FortEditor ed)
{
  ft_States ok;

  putCurLine(ed);

  ed_TreeWillChange(ed, ft_Root(R(ed)->ft));
  ok = ft_Check(R(ed)->ft);
  ed_TreeChanged(ed, ft_Root(R(ed)->ft));

  return BOOL(ok == ft_CORRECT);
}






/************************/
/*  Searching		*/
/************************/




void ed_SetPattern(FortEditor ed, FortVFilter filter)
{
  srch_SetPattern(R(ed)->srch,filter);
}




void ed_SetPatternText(FortEditor ed, char *str, Boolean fold)
{
  srch_SetPatternText(R(ed)->srch,str,fold);
}




Boolean ed_Find(FortEditor ed, Boolean dir)
{
  Boolean found;

  other(ed);

  found = srch_Find(R(ed)->srch,dir,true);
  if( found )  docChanged(ed,R(ed)->sel.line,true);

  return found;
}




Boolean ed_FindPlaceholder(FortEditor ed, Boolean dir)
{
  Boolean found;

  other(ed);

  found = srch_FindPlaceholder(R(ed)->srch,dir,true);
  if( found )  docChanged(ed,R(ed)->sel.line,true);

  return found;
}




int ed_ReplaceText(FortEditor ed, Boolean dir, Boolean global, 
                   Boolean all, char *newStr)
{
  other(ed);

  return srch_ReplaceText(R(ed)->srch,dir,global,all,newStr);
}




int ed_ReplaceTree(FortEditor ed, Boolean dir, Boolean global, 
                   Boolean all, FortTreeNode newNode)
{
  other(ed);

  return srch_ReplaceTree(R(ed)->srch,dir,global,all,newNode);
}






/************************************************************************/
/*	Operations for subparts only 					*/
/************************************************************************/




void ed__GetStatus(FortEditor ed, Selection *sel, int *curLine, 
                   TextString *curText, Boolean *dirty)
{
  *sel = R(ed)->sel;
  *curLine = R(ed)->curLine;
  *curText = copyTextString(R(ed)->curText);
  *dirty = R(ed)->curLineChanged;
}




void ed__SetStatus(FortEditor ed, Selection sel, int curLine, 
                   TextString curText, Boolean dirty)
{
  R(ed)->sel = sel;
  R(ed)->curLine = curLine;

  destroyTextString(R(ed)->curText);
  R(ed)->curText = copyTextString(curText);
  R(ed)->curTextSize = curText.num_tc;

  R(ed)->curLineChanged = dirty;
}




void ed__MakeClean(FortEditor ed, Boolean put, Boolean struc, 
                   Boolean update, Selection *sel)
{
  if( put )
    putCurLine(ed);
  if( struc )
    getStructureSelection(ed);
  if( update )
    { removeUnderlining(ed);
      docChanged(ed,R(ed)->curLine,false);
    }

  *sel = R(ed)->sel;
}







/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/*ARGSUSED*/

static
void getSelection(Selection sel, int *line1, int *char1, int *line2, 
                  int *char2)
{
  if( sel.line == UNUSED )
    { *line1 = sel.sel1;
      *char1 = 0;
      *line2 = sel.sel2;
      *char2 = INFINITY;
    }
  else
    { *line1 = sel.line;
      *char1 = sel.sel1;
      *line2 = sel.line;
      *char2 = sel.sel2;
    }
}




static
void setSelection(FortEditor ed, int line, int sel1, int sel2)
{
  int place1,place2;

  /* ensure placeholders are selected atomically */
    if( line != UNUSED )
      { if( insidePlaceholder(ed,sel1,&place1,&place2) )  sel1 = place1;
        if( insidePlaceholder(ed,sel2,&place1,&place2) )  sel2 = place2;
      }

  /* set text selection */
    R(ed)->sel.line = line;
    R(ed)->sel.sel1 = sel1;
    R(ed)->sel.sel2 = sel2;
    R(ed)->changeInitialLineSel = false;

  if( R(ed)->freeze == 0 )
    { /* notify all interested objects */
        notify(ed, NOTIFY_SEL_CHANGED, false, (FortTreeNode) UNUSED, UNUSED, 
	       UNUSED, UNUSED);

      /* remember this selection for next notification */
        R(ed)->oldSel = R(ed)->sel;
    }
}




static
void getStructureSelection(FortEditor ed)
{
  int line = R(ed)->sel.line;
  int sel1 = R(ed)->sel.sel1;
  int sel2 = R(ed)->sel.sel2;
  Boolean structure;
  FortTreeNode node;

  if( line != UNUSED )
    structure = ftt_TextToNode(R(ed)->ftt, line, sel1, line, sel2, &node);
  else
    structure = ftt_TextToNode(R(ed)->ftt, sel1, UNUSED, sel2, UNUSED, &node);

  R(ed)->sel.structure = structure;
  R(ed)->sel.node = node;
}




static
void getCurLine(FortEditor ed, int line)
{
  TextString text;

  R(ed)->curLine = line;

  if( line != UNUSED )
    { ftt_GetLine(R(ed)->ftt,line,&text);
      if( text.num_tc > R(ed)->curTextSize )
        { /* expand the current text string buffer */
            R(ed)->curTextSize = text.num_tc;
            R(ed)->curText.tc_ptr = (TextChar*)reget_mem((void*)R(ed)->curText.tc_ptr,
		R(ed)->curTextSize * sizeof(TextChar), "getCurLine");
        }
      moveTextString(text,&R(ed)->curText);
      destroyTextString(text);
    }

  R(ed)->curLineChanged = false;
  R(ed)->needCurLine = BOOL( R(ed)->freeze != 0 );
}




static
void putCurLine(FortEditor ed)
{
#define AT_OR_PAST(a, b) ( a.non_spaces > b.non_spaces  || \
		(a.non_spaces == b.non_spaces  &&  a.spaces >= b.spaces ) )
#define UNSET -2
  struct position {
      int   spaces;
      int   non_spaces; 
  } pos, pos1, pos2;
  int curLine  = R(ed)->curLine;
  int numLines = ed_NumLines(ed);
  int i, line, sel1, sel2;

  if( curLine != UNUSED  &&  R(ed)->curLineChanged )
    { ed_GetSelection(ed, &line, &sel1, &sel2);
      if( line == curLine  &&  R(ed)->freeze == 0 )
        { /* save the position of the selection in a stable representation */
            pos.non_spaces = 0;
            pos.spaces     = 0;
            for( i = 0; i < R(ed)->curText.num_tc; i++)
              { if( i == sel1 )  pos1 = pos;
                if( i == sel2 + 1 )  pos2 = pos;
                if( !isspace(R(ed)->curText.tc_ptr[i].ch) )
                  { pos.non_spaces++;
                    pos.spaces = 0;
                  }
                else
                  pos.spaces++;
              }
            if( i == sel1 )  pos1 = pos;
            if( i == sel2 + 1 )  pos2 = pos;
         }

      /* set and prettyprint the line */
        if( curLine >= numLines )
          ftt_InsertLine(R(ed)->ftt, curLine, R(ed)->curText);
        else
          ftt_SetLine(R(ed)->ftt, curLine, R(ed)->curText);


        docChanged(ed,UNUSED,false);
        R(ed)->curLineChanged = false;

      if( line == curLine  &&  R(ed)->freeze == 0 )
        { getCurLine(ed, curLine);      /* must happen before notifies */

          /* restore the position of the selection */
            pos.non_spaces = 0;
            pos.spaces     = 0;
            sel1 = sel2 = UNSET;
            for( i = 0; i < R(ed)->curText.num_tc; i++)
              { if( sel1 == UNSET  &&  AT_OR_PAST(pos, pos1) )
                  sel1 = i;
                if( sel2 == UNSET  &&  AT_OR_PAST(pos, pos2) )
                  sel2 = i - 1;

                if( !isspace(R(ed)->curText.tc_ptr[i].ch) )
                  { pos.non_spaces++;
                    pos.spaces = 0;
                  }
                else
                  pos.spaces++;
              }
            if( sel1 == UNSET )  sel1 = i;
            if( sel2 == UNSET )  sel2 = i - 1;

            setSelection(ed, line, sel1, sel2);
         }
    }
}




static
void docChanged(FortEditor ed, int oneLine, Boolean autoscroll)
{
  int curLine = R(ed)->curLine;
  int first,last,delta;
  FortTreeNode dummy;
  Boolean firstDefer;

  if( R(ed)->freeze == 0 )
    { /* compute affected area, always at least one line */
        if( oneLine != UNUSED )
          { first = oneLine;  last = oneLine;  delta = 0; }
        else
          { ftt_GetChanges(R(ed)->ftt,&first,&last,&delta,&dummy);
            if( last < first )
              { first = curLine;  last = curLine;  delta = 0; }
            if( first == UNUSED || last == UNUSED )  return;
          }

	R(ed)->fmodule->Changed(SrcLineChange, 0);	

      /* notify all interested objects */
        notify(ed, NOTIFY_DOC_CHANGED, autoscroll, (FortTreeNode) UNUSED, 
	       first, last, delta);

      R(ed)->changeDeferred = false;
    }
  else
    { firstDefer = NOT( R(ed)->changeDeferred );

      R(ed)->changeDeferred = true;

      if( firstDefer )
        { R(ed)->changeLine = oneLine;
          R(ed)->changeAutoscroll = autoscroll;
        }
      else
        { if( R(ed)->changeLine != oneLine )
            R(ed)->changeLine = UNUSED;

          R(ed)->changeAutoscroll = BOOL( autoscroll || R(ed)->changeAutoscroll );
        }
    }
}




static
Boolean insidePlaceholder(FortEditor ed, int pos, int *first, int *last)
{
  Boolean isFirst;

  if( placeholderChar(ed,pos,&isFirst) )
    { *first = pos;
      while( NOT(isFirst) && placeholderChar(ed,*first-1,&isFirst) )
        *first -= 1;

      *last = pos;
      while( placeholderChar(ed,*last+1,&isFirst) && NOT(isFirst) )
        *last += 1;

      return true;
    }
  else return false;
}




static
Boolean placeholderChar(FortEditor ed, int k, Boolean *first)
{
  TextChar * buffer = R(ed)->curText.tc_ptr;
  int buffCount = R(ed)->curText.num_tc;

  if( 0 <= k  &&  k < buffCount )
    { *first = BOOL( buffer[k].style & ftt_BEGIN_PLACEHOLDER_STYLE );
      return BOOL( buffer[k].style & ftt_PLACEHOLDER_STYLE );
    }
  else
    { *first = false;
      return false;
    }
}




static
void notePlaceholderSelection(FortEditor ed, Boolean always)
{
  Selection sel;
  TextChar * buffer = R(ed)->curText.tc_ptr;
  int buffCount = R(ed)->curText.num_tc;
  Boolean ph;
  int k;

  sel = R(ed)->sel;

  if( sel.line == UNUSED )
    ph = false;
  else if( sel.sel1 > sel.sel2 )
    { /* return true iff line is empty */
      ph = true;  k = 0;
      while( ph  &&  k < buffCount )
        if( buffer[k].ch == ' ')
          k++;
        else
          ph = false;
    }
  else
    { ph = true;  k = sel.sel1;
      while( ph  &&  k <= sel.sel2 )
        if( buffer[k].style & ftt_PLACEHOLDER_STYLE )
          k++;
        else
          ph = false;
    }

  if( always || ph )
    ex_Select(R(ed)->ex, ph);
}




static
void edcopy(FortEditor ed, FortScrap *scrap)
{
  FortTextTree ftt = R(ed)->ftt;
  FortTree ft = ftt_Tree(ftt);
  int line = R(ed)->sel.line;
  int sel1 = R(ed)->sel.sel1;
  int sel2 = R(ed)->sel.sel2;
  int k;
  TextString text;
  int iconceal;

  if( line != UNUSED )
    *scrap = fsc_CreateSmall(ft, R(ed)->curText, sel1, sel2-sel1+1);
  else
    { *scrap = fsc_CreateLarge(ft);
      for( k = sel1;  k <= sel2;  k++ )
        { ftt_GetLine(ftt,k,&text);
          ftt_GetConcealCount(ftt,k,&iconceal);
          fsc_AddLarge(*scrap,text,iconceal);
          destroyTextString(text);
        }
    }
}




static
void edpaste(FortEditor ed, FortScrap scrap)
{
  FortTextTree ftt = R(ed)->ftt;
  int line = R(ed)->sel.line;
  int sel1 = R(ed)->sel.sel1;
  int sel2 = R(ed)->sel.sel2;
  Boolean after;
  int k,len;
  TextString text;
  int iconceal;

  /* ASSERT: selection is an insertion point on R(ed)->curLine */

  if( fsc_IsSmall(scrap) )
    { text = fsc_GetSmall(scrap);
      len  = text.num_tc;
      len = insertChars(ed,sel1,len,text,false);
      docChanged(ed, line, true);
      setSelection(ed, line, sel1+len, sel1+len-1);
    }
  else
    { beginEdit(ed);

      after = shouldInsertAfter(ed);
      if( after )  line += 1;

      putCurLine(ed);
      len = fsc_GetLargeLength(scrap);
      for( k = len-1;  k >= 0;  k-- )
        { fsc_GetLarge(scrap,k,&text,&iconceal);
          ftt_InsertLine(ftt,line,text);	/* 'line', NOT 'k'! */
          ftt_SetConcealCount(ftt,line,iconceal);
        }
      docChanged(ed,UNUSED,true);
      R(ed)->curLine += len;
      getCurLine(ed, R(ed)->curLine);

      if( after )
        initialLineSelection(ed,R(ed)->curLine);
      else
        setSelection(ed,R(ed)->curLine,sel1,sel2);

      endEdit(ed);
    }
}




static
void edclear(FortEditor ed)
{
  FortTextTree ftt = R(ed)->ftt;
  int line = R(ed)->sel.line;
  int sel1 = R(ed)->sel.sel1;
  int sel2 = R(ed)->sel.sel2;
  static TextString text = { 0, (TextChar *) 0, false }; /* empty line */
  int k;

  if( line != UNUSED )
    { deleteChars(ed, sel1, sel2 - sel1 + 1);
      docChanged(ed, line, true);
      setSelection(ed,line,sel1,sel1-1);
    }
  else
    { beginEdit(ed);

      for( k = sel1;  k <= sel2;  k++ )
        ftt_DeleteLine(ftt,sel1);	/* 'sel1', NOT 'k'! */
      if( sel1 >= ftt_NumLines(ftt) )
        ftt_InsertLine(ftt, sel1, text);

      getCurLine(ed, sel1);
      docChanged(ed, UNUSED, true);
      initialLineSelection(ed, sel1);

      endEdit(ed);
    }
}




static
Boolean insertChar(FortEditor ed, int start, char ch, Boolean underline)
{
  static TextChar tc;
  static TextString ts = { 1, &tc, false };
  unsigned char style;
  int len;

  style = STYLE_NORMAL;
  if( underline )  style |= ATTR_UNDERLINE;

  tc = makeTextChar(ch,style);
  len = insertChars(ed,start,1,ts,true);

  return BOOL( len > 0 );
}




static
int insertChars(FortEditor ed, int start, int count, TextString chars, 
                Boolean respectField)
{
  int buffCount = R(ed)->curText.num_tc;
  int fieldStart,fieldCount,fieldMax,fieldWidth;
  int buffSize;
  int inserted;

  /* determine which field to insert in */
    if( curLineIsComment(ed)  ||  ! respectField )
      { fieldStart = 0;
        fieldMax   = INFINITY;
      }

    else if( start < CONTINUE_COL )
      { fieldStart = 0;
        fieldMax   = CONTINUE_COL;
      }

    else if( start == CONTINUE_COL )
      { fieldStart = CONTINUE_COL;
        fieldMax   = CONTINUE_COL+1;
      }

    else
      { fieldStart = CONTINUE_COL+1;
        fieldMax   = INFINITY;
      }

  /* if necessary, make sure there's room for whole insertion */
    if( fieldMax == INFINITY )
      { buffSize = max(start, buffCount) + count;
        if( R(ed)->curTextSize <= buffSize )
          { R(ed)->curTextSize = buffSize + 100;
            R(ed)->curText.tc_ptr =
                (TextChar *) reget_mem((void*)R(ed)->curText.tc_ptr,
                                        R(ed)->curTextSize * sizeof(TextChar),
                                        "FortEditor: insertChars"
                                      );
          }
      }

  /* insert into the chosen field */
    fieldCount = min(fieldMax,buffCount) - fieldStart;
    inserted = insertInField(R(ed)->curText.tc_ptr,
                             fieldStart,fieldCount,fieldMax,
                             start,count,chars);

    fieldWidth = fieldMax - fieldStart;
    R(ed)->curText.num_tc = max(buffCount, fieldStart + min(fieldWidth, fieldCount+inserted));

    R(ed)->curLineChanged = true;

  /* return the number of chars actually inserted */
    return inserted;
}



static
int insertInField(TextChar *buffer, int fieldStart, int fieldCount, int fieldMax, 
                  int start, int count, TextString chars)
{
  int fieldLast = fieldStart + fieldCount;
  TextChar * from;
  TextChar * to;
  int shiftCount;

  /* ASSERT: fieldStart <= start < fieldStart + fieldMax */

  /* add white space to the end of the field if necessary */
    while( fieldLast < start )
      { buffer[fieldLast] = makeTextChar(' ',STYLE_NORMAL);
        fieldLast += 1;
      }

  /* don't insert more chars than fit in field */
    count = min(count, fieldMax-start);

  /* shift existing chars right to make room */
    from = buffer + start;
    to   = buffer + start + count;
    shiftCount = min(fieldLast, fieldMax-count) - start;
    bcopy((char *) from, (char *) to, shiftCount * sizeof(TextChar));

  /* copy in new chars */
    from = chars.tc_ptr;
    to   = buffer + start;
    bcopy((char *) from, (char *) to, count * sizeof(TextChar));

  return count;
}




static
void deleteChars(FortEditor ed, int start, int count)
{
  TextChar * buffer = R(ed)->curText.tc_ptr;
  int buffCount = R(ed)->curText.num_tc;
  int last1,last2,last3;

  if( count == 0 ) return;

  if( curLineIsComment(ed) )
    { last1 = deletePortion(buffer,buffCount,0,INFINITY,start,count);
      R(ed)->curText.num_tc = last1;
    }

  else
   { if( start < CONTINUE_COL )
        last1 = deletePortion(buffer,buffCount,0,CONTINUE_COL,start,count);
      else
        last1 = -1;

      if( start == CONTINUE_COL )
        last2 = deletePortion(buffer,buffCount,CONTINUE_COL,CONTINUE_COL+1,start,count);
      else
        last2 = -1;

      last3 = deletePortion(buffer,buffCount,CONTINUE_COL+1,INFINITY,start,count);

      R(ed)->curText.num_tc = max(last1,max(last2,last3));
    }

  R(ed)->curLineChanged = true;
}




static
int deletePortion(TextChar *buffer, int buffCount, int fieldStart, int fieldMax, 
                  int start, int count)
{
  int fieldCount,deleted;
  Boolean fill;
  int thisStart,thisCount;

  fieldCount = min(fieldMax,buffCount) - fieldStart;
  thisStart  = max(fieldStart, min(fieldMax, start));
  thisCount  = max( 0, min(fieldMax,start+count)-thisStart );
  fill       = BOOL( buffCount >= fieldMax );

  if( count > 0 )
    deleted = deleteInField(buffer,
                            fieldStart,fieldCount,fieldMax,
                            thisStart,thisCount,fill);
  else
    deleted = 0;

  /* return the position beyond new end of field */
    return fieldStart + fieldCount - deleted;
}



static
int deleteInField(TextChar *buffer, int fieldStart, int fieldCount, int fieldMax, 
                  int start, int count, Boolean fill)
{
  int fieldLast = fieldStart + fieldCount;
  TextChar * from;
  TextChar * to;
  int shiftCount;

  /* don't delete more chars than there are */
    count = max(0, min(count, fieldLast-start));

  /* don't delete 0 characters */
    if( count == 0 )
      return 0;

  /* shift existing chars left to close gap */
    from = buffer + start + count;
    to   = buffer + start;
    shiftCount = fieldLast - (start+1);
    bcopy((char *) from, (char *) to, shiftCount * sizeof(TextChar));

    fieldLast -= count;

  /* add white space to the end of the field if necessary */
    if( fill )
      while( fieldLast < fieldMax )
        { buffer[fieldLast] = makeTextChar(' ',STYLE_NORMAL);
          fieldLast += 1;
        }

  /* return the number of actually deleted characters */
    return count;
}




static
void insertNewLine(FortEditor ed, int lineNum)
{
  FortTextTree ftt = R(ed)->ftt;
  int curLine  = R(ed)->curLine;
  int numLines = ed_NumLines(ed);


  beginEdit(ed);

  /* put away the old line */
    if( curLine >= numLines )
      ftt_InsertLine(ftt, curLine, R(ed)->curText);
    else if( R(ed)->curLineChanged )
      ftt_SetLine(ftt, curLine, R(ed)->curText);

  /* insert the new line */
    ftt_InsertLine(ftt, lineNum, emptyTextString);
    getCurLine(ed, lineNum);
    docChanged(ed, UNUSED, true);
    initialLineSelection(ed, lineNum);

  endEdit(ed);
}




static
void initialLineSelection(FortEditor ed, int lineNum)
{
  int indent;

  indent = ftt_GetLineIndent(R(ed)->ftt,lineNum);
  setSelection(ed,lineNum,indent,indent-1);
  R(ed)->changeInitialLineSel = true;
}




static
Boolean curLineIsComment(FortEditor ed)
{
  return BOOL( R(ed)->curText.num_tc > 0  &&
               ( R(ed)->curText.tc_ptr[0].ch == 'C'  ||
                 R(ed)->curText.tc_ptr[0].ch == 'c'  ||
                 R(ed)->curText.tc_ptr[0].ch == '*'
               )
             );
}




static
void beginEdit(FortEditor ed)
{
  R(ed)->freeze += 1;
  ftt_BeginEdit(R(ed)->ftt);
}




static
void endEdit(FortEditor ed)
{
  R(ed)->freeze -= 1;
  ftt_EndEdit(R(ed)->ftt);

  if( R(ed)->freeze == 0  &&  R(ed)->changeDeferred )
    { if( R(ed)->needCurLine  &&  ! R(ed)->curLineChanged )
        getCurLine(ed, R(ed)->curLine);
      docChanged(ed, R(ed)->changeLine, R(ed)->changeAutoscroll);
      if( R(ed)->changeInitialLineSel )
        initialLineSelection(ed, R(ed)->sel.line);
      else
        setSelection(ed, R(ed)->sel.line, R(ed)->sel.sel1, R(ed)->sel.sel2);
    }
}




static
Boolean shouldInsertAfter(FortEditor ed)
{
  Boolean blanks;
  int k;

  /* ASSERT: selection is an IP on 'curLine' */

  if( R(ed)->changeInitialLineSel )
    return false;
  else if( R(ed)->sel.sel1 == R(ed)->curText.num_tc )
    return true;
  else
    { blanks = true;
      for( k = 0;  k < R(ed)->sel.sel1;  k++ )
        blanks = BOOL( blanks  &&  R(ed)->curText.tc_ptr[k].ch == ' ' );

      return NOT( blanks );
    }
}




static
void other(FortEditor ed)	   	/* notify Expander of other edit */
{
  if( ex_Other(R(ed)->ex) )
    { removeUnderlining(ed);
      docChanged(ed,R(ed)->sel.line,false);
    }
}




static
void removeUnderlining(FortEditor ed)
{
  TextChar * buffer = R(ed)->curText.tc_ptr;
  int buffCount = R(ed)->curText.num_tc;
  int k;

  for( k = 0;  k < buffCount;  k++ )
    buffer[k].style &= ~ATTR_UNDERLINE;
}




static
void addNotifiee(FortEditor ed, Generic ob, NotifyMethod method)

{
  Flex * notifiees = R(ed)->notifiees;
  NotifyRequest req;

  req.ob = ob;
  req.method = method;
  flex_insert_one(notifiees, flex_length(notifiees), (char *) &req);
}




static
void notify(FortEditor ed, int kind, Boolean autoscroll, FortTreeNode node, 
            int first, int last, int delta)
{
  Flex * notifiees = R(ed)->notifiees;
  int oldLine1, oldLine2, newLine1, newLine2;
  int k, dummy;
  NotifyRequest req;

  if( kind == NOTIFY_SEL_CHANGED )
    { getSelection(R(ed)->oldSel, &oldLine1, &dummy, &oldLine2, &dummy);
      getSelection(R(ed)->sel,    &newLine1, &dummy, &newLine2, &dummy);

      node  = UNUSED;
      first = min(oldLine1,newLine1),
      last  = max(oldLine2,newLine2);
      delta = 0;
    }
  
  for( k = 0;  k < flex_length(notifiees);  k++ )
    { (void) flex_get_buffer(notifiees, k, 1, (char *) &req);
      (req.method) (req.ob, kind, autoscroll, node, first, last, delta);
    }
}




 
