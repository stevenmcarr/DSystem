/* $Id: AnnotDialog.C,v 1.1 1997/06/25 13:47:08 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditorCP/AnnotDialog.c				*/
/*									*/
/*	AnnotDialog -- browse and navigate annotations			*/
/*	Last edited: June 18, 1993 at 4:00 pm                           */
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditorCP/FortEditorCP.i>

#include <libs/graphicInterface/cmdProcs/newEditor/fortEditorCP/AnnotDialog.h>

#include <libs/support/arrays/FlexibleArray.h>
#include <libs/graphicInterface/oldMonitor/include/mon/dialog.h>
#include <libs/graphicInterface/oldMonitor/include/items/button.h>
#include <libs/graphicInterface/oldMonitor/include/items/check_box.h>
#include <libs/graphicInterface/oldMonitor/include/items/title.h>
#include <libs/graphicInterface/oldMonitor/include/items/text.h>
#include <libs/graphicInterface/oldMonitor/include/items/item_list.h>
#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/scroll_sm.h>








/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* an 'AnnotDialog' object manages a collection of 'AnnotBrowser's */


typedef struct
  {
    /* creation parameters */
      FortEditorCP         edcp;
      FortEditor           editor;

    /* contents */
      Flex *               browsers;

  } adlg_Repr;

#define	R(ob)		((adlg_Repr *) ob)




/* an 'AnnotBrowser' is a modeless dialog */

#define MAX_LEVEL  3

typedef struct
  {
    /* creation parameters */
      AnnotDialog          adlg;

    /* contents */
      Generic              theAnnot;

      Dialog *             dialog;
      DiaDesc *            list[1+MAX_LEVEL];
      DiaDesc *            text;
      DiaDesc *            goButton;
      DiaDesc *            originButton;

    /* status */
      int                  level;
      Generic              annot[1+MAX_LEVEL];
      Generic              selection[1+MAX_LEVEL];
      Generic              textSelection;

  } AnnotBrowserRepr;


typedef AnnotBrowserRepr *  AnnotBrowser;





/************************/
/* Dialog item numbers	*/
/************************/




#define ITEM_LISTS		100	/* base of list-item numbers       */
#define NUM_LISTS	          3	/* == MAX_LEVEL                    */

#define ITEM_LIST1		101	/* 1st list of annot names         */
#define ITEM_LIST2		102	/* 2nd list of annot names         */
#define ITEM_LIST3		103	/* 3rd list of annot names         */

#define ITEM_TEXT		105	/* text with links (list of lines) */
#define ITEM_GO			106	/* go to selected line's link      */
#define ITEM_ORIGIN		107	/* go to origin of this browser    */






/************************/
/* Forward declarations	*/
/************************/




STATIC(AnnotBrowser,     browserCreate,(Context context, DB_FP *fp, AnnotDialog adlg,
                                        Generic annot));
STATIC(void,             browserDestroy,(AnnotBrowser br));
STATIC(void,             makeDialog,(AnnotBrowser br));
STATIC(Boolean,          dialogHandler,(Dialog *dialog, AnnotBrowser br, 
                                        Generic item_id));
STATIC(void,             enableButtons,(AnnotBrowser br));
STATIC(void,             noteSelection,(AnnotBrowser br, int level));
STATIC(void,             pushDisplay,(AnnotBrowser br, Generic annot));
STATIC(void,             popDisplay,(AnnotBrowser br));
STATIC(ListItemEntry,    annotListProc,(AnnotBrowser br, DiaDesc *dd, Boolean first,
                                        int prevLineNum));
STATIC(ListItemEntry,    textListProc,(AnnotBrowser br, DiaDesc *dd, Boolean first,
                                       int prevLineNum));
STATIC(void,		 navigate,(AnnotBrowser br, Boolean followLink));




/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Initialization	*/
/************************/




void adlg_Init()
{
  /* nothing */
}




void adlg_Fini()
{
  /* nothing */
}




AnnotDialog adlg_Open(Context context, DB_FP *fp, FortEditorCP edcp, FortEditor editor)
{
  AnnotDialog adlg;
  int numBrowsers, k;

  /* allocate a new instance */
    adlg = (AnnotDialog) get_mem(sizeof(adlg_Repr),"FortEditorCP:AnnotDialog");

  /* initialize the parts */
    /* set creation parameters */
      R(adlg)->edcp = edcp;
      R(adlg)->editor = editor;

    /* set contents */
      R(adlg)->browsers = flex_create(sizeof(AnnotBrowser));

  return adlg;
}




void adlg_Close(AnnotDialog adlg)
{
  Flex * browsers = R(adlg)->browsers;
  int k;
  AnnotBrowser br;

  /* close all browsers -- backwards since 'browserDestroy' */
  /*    deletes from the 'browsers' flexarray             */
    for( k = flex_length(browsers)-1;  k >= 0;  k-- )
      { (void) flex_get_buffer(browsers, k, 1, (char *) &br);
        browserDestroy(br);
      }

  flex_destroy(browsers);

  free_mem((void*) adlg);
}




void adlg_Save(AnnotDialog adlg, Context context, DB_FP *fp)
{
  /* nothing */
}










/************************/
/*  User interaction	*/
/************************/




void adlg_Dialog(AnnotDialog adlg, Generic annot)
{
  AnnotBrowser br;

  br = browserCreate(CONTEXT_NULL, DB_NULLFP, adlg, annot);
}




/************************/
/*  Notification	*/
/************************/




/*ARGSUSED*/

void adlg_NoteChange(AnnotDialog adlg, int kind, Boolean autoScroll, FortTreeNode node, 
                     int first, int last, int delta)
{
  /* ... */
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/************************/
/*  AnnotBrowser	*/
/************************/




static
AnnotBrowser browserCreate(Context context, DB_FP *fp, AnnotDialog adlg, Generic annot)
{
  AnnotBrowser br;
  int k;

  /* allocate a new instance */
    br = (AnnotBrowser) get_mem(sizeof(AnnotBrowserRepr),"FortEditorCP:AnnotBrowser");

  /* initialize the parts */
    /* set creation parameters */
      br->adlg = adlg;

    /* get our persistent information */
      if( fp != DB_NULLFP )
        { /* TEMPORARY */
            br->theAnnot = nil;
            br->level = 0;
        }
      else
        { br->theAnnot = annot;
          br->level = 0;
          pushDisplay(br, br->theAnnot);
        }

    /* create the subparts */
      makeDialog(br);		/* needs the preceding initializations */

  /* add it to owner's list */
    flex_insert_one(R(adlg)->browsers, flex_length(R(adlg)->browsers),
                    (char *) &br);

  dialog_modeless_show(br->dialog);
  /* the following seems to compensate for a dialog bug */
  /*    s.t. list items in modeless dialogs do not show */
  /*    their initial contents.                         */
    for( k = 1;  k <= MAX_LEVEL;  k++ )
      dialog_item_modified(br->dialog, ITEM_LISTS+k);
    dialog_item_modified(br->dialog, ITEM_TEXT);
  return br;
}




static
void browserDestroy(AnnotBrowser br)
{
  Flex * browsers;
  int k, index;
  AnnotBrowser br_k;

  /* remove the browser from owner's list */
    browsers = R(br->adlg)->browsers;
    for( k = 0;  k < flex_length(browsers);  k++ )
      { flex_get_buffer(browsers, k, 1, (char *) &br_k);
        if( (Generic) br_k == (Generic) br )  index = k;
      }
    flex_delete(browsers, index, 1);

  /* destroy the browser */
    dialog_destroy(br->dialog);
    fan_Destroy(br->theAnnot);
    free_mem((void*) br);
}






/************************/
/*  Dialogs		*/
/************************/




static
void makeDialog(AnnotBrowser br)
{
  AnnotDialog adlg = br->adlg;
  char * title = "Source Annotations";
  DiaDesc *list_dd, *text_dd, *whole_dd;
  Point listSize, textSize;
  int k;

  listSize = makePoint(20, 8);
  textSize = makePoint(67,10);

  list_dd =
      dialog_desc_group(
          DIALOG_HORIZ_CENTER,
          3,
          item_list(ITEM_LISTS+1, 
                    (char*)0, 
                    (Generic)br, 
                    (item_list_elem_proc)annotListProc,
		    &br->selection[1], 
                    (Generic)UNUSED, 
                    false, 
                    DEF_FONT_ID,
                    listSize),
          item_list(ITEM_LISTS+2, 
                    (char*)0, 
                    (Generic)br, 
                    (item_list_elem_proc)annotListProc,
		    &br->selection[2], 
                    (Generic)UNUSED, 
                    false, 
                    DEF_FONT_ID,
                    listSize),
          item_list(ITEM_LISTS+3, 
                    (char*)0, 
                    (Generic)br, 
                    (item_list_elem_proc)annotListProc,
		    &br->selection[3], 
                    (Generic)UNUSED, 
                    false, 
                    DEF_FONT_ID,
                    listSize)
          );

  text_dd =
      dialog_desc_group(
          DIALOG_VERT_CENTER,
          2,
          item_list(ITEM_TEXT, 
                    (char*)0, 
                    (Generic)br, 
                    (item_list_elem_proc)textListProc,
		    &br->textSelection, 
                    (Generic)UNUSED, 
                    false, 
                    DEF_FONT_ID,
                    textSize),
          dialog_desc_group(
              DIALOG_HORIZ_CENTER,
              1 /*** 2 ***/,
              item_button(ITEM_GO, "go to link", DEF_FONT_ID, false)
          /******
            , item_button(ITEM_ORIGIN, "go to origin", DEF_FONT_ID, false)
          ******/
              )
          );

  whole_dd =
      dialog_desc_group(
          DIALOG_VERT_CENTER,
          2,
          list_dd,
          text_dd
          );

  br->dialog = dialog_create(title, 
                             (dialog_handler_callback)dialogHandler, 
                             (dialog_helper_callback)0, 
                             (Generic)br,
                             whole_dd);

  for( k = 1;  k <= MAX_LEVEL; k++ )
    br->list[k] = dialog_desc_lookup(br->dialog, ITEM_LISTS+k);

  br->text         = dialog_desc_lookup(br->dialog, ITEM_TEXT);
  br->goButton     = dialog_desc_lookup(br->dialog, ITEM_GO);
/******
  br->originButton = dialog_desc_lookup(br->dialog, ITEM_ORIGIN);
******/

  enableButtons(br);
}




/*ARGSUSED*/

static
Boolean dialogHandler(Dialog *dialog, AnnotBrowser br, Generic item_id)
{
  switch( item_id )
    {
      case ITEM_LIST1:
      case ITEM_LIST2:
      case ITEM_LIST3:
        noteSelection(br, item_id - ITEM_LISTS);
        enableButtons(br);
        break;

      case ITEM_GO:
        navigate(br, true);
        enableButtons(br);
        break;

      case ITEM_ORIGIN:
        navigate(br, false);
        enableButtons(br);
        break;

      case ITEM_TEXT:
        enableButtons(br);	
	break;

      case DIALOG_CANCEL_ID:
        browserDestroy(br);
        break;
    }

  return DIALOG_NOMINAL;
}




static
void enableButtons(AnnotBrowser br)
{
  Dialog * dialog = br->dialog;
  Generic annot = br->annot[br->level];

# define ABLE(i,p)  dialog_item_ability(dialog,i,(p ? DIALOG_ENABLE : DIALOG_DISABLE))

  ABLE( ITEM_GO,
          ! fan_IsCompound(annot)  &&
            br->textSelection != UNUSED  &&
              fan_HasLink(annot, br->textSelection) );

/******
  ABLE( ITEM_ORIGIN,
          true );
******/
}




static
void noteSelection(AnnotBrowser br, int level)
{
  int oldLevel = br->level;
  Generic annot;
  int leftmost, rightmost, k;

  while( level < br->level )  popDisplay(br);

  annot = fan_GetElement(br->annot[level], br->selection[level]);
  pushDisplay(br, annot);

  dialog_item_modified(br->dialog, ITEM_TEXT);
  leftmost  = level + 1;
  rightmost = max(oldLevel, br->level);
  for( k = rightmost;  k >= leftmost;  k-- )
    dialog_item_modified(br->dialog, ITEM_LISTS+k);
  enableButtons(br);
}




static
void pushDisplay(AnnotBrowser br, Generic annot)
{
  br->level += 1;
  br->annot[br->level] = annot;

  while( fan_IsCompound(annot) && fan_NumElements(annot) == 1 )
    { br->selection[br->level] = 1;
      annot = fan_GetElement(annot, 1);
      br->level += 1;
      br->annot[br->level] = annot;
    }
  br->selection[br->level] = UNUSED;

  if( ! fan_IsCompound(annot) && fan_NumTextLines(annot) == 1 )
    br->textSelection = 1;
  else
    br->textSelection = UNUSED;
}




static
void popDisplay(AnnotBrowser br)
{
  int oldLevel = br->level;

  br->level -= 1;
}




/*ARGSUSED*/

static
ListItemEntry annotListProc(AnnotBrowser br, DiaDesc *dd, Boolean first, int prevLineNum)
{
  int lineNum = (first ? 1 : prevLineNum + 1);
  int level,k;
  Generic annot;
  char * name;
  ListItemEntry line;

  /* determine the level whose line is wanted */
    level = UNUSED;    /* trick for calls before 'br' is initialized */
    for( k = 1;  k <= MAX_LEVEL; k++ )
      if( dd == br->list[k] )  level = k;

  /* see whether the line exists and get its text if so */
    if( level != UNUSED  &&
          level <= br->level  &&
            fan_IsCompound(br->annot[level])  &&
              lineNum <= fan_NumElements(br->annot[level])
      )
      { annot = fan_GetElement(br->annot[level], lineNum);
        fan_GetName(annot,&name);
        line = item_list_create_entry(dd, (Generic) lineNum, name, true);
      }
    else
      line = NULL_LIST_ITEM_ENTRY;

  return line;
}




/*ARGSUSED*/

static
ListItemEntry textListProc(AnnotBrowser br, DiaDesc *dd, Boolean first, int prevLineNum)
{
  int lineNum = (first ? 1 : prevLineNum + 1);
  Generic annot = br->annot[br->level];
  char * lineString;
  ListItemEntry line;

  if( ! fan_IsCompound(annot)  &&
        lineNum <= fan_NumTextLines(annot) )
    { fan_GetTextLine(annot, lineNum, &lineString);
      line = item_list_create_entry(dd, (Generic) lineNum, lineString, true);
    }
  else
    line = NULL_LIST_ITEM_ENTRY;

  return line;
}






/****************/
/*  Navigation  */
/****************/




static
void navigate(AnnotBrowser br, Boolean followLink)
{
  if( followLink )
    fan_GotoLink(br->annot[br->level], br->textSelection);
  else
    /* ... go to origin */ ;
}
