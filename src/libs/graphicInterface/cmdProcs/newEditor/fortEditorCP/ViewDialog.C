/* $Id: ViewDialog.C,v 1.1 1997/06/25 13:47:08 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditorCP/ViewDialog.c				*/
/*									*/
/*	ViewDialog -- see and change view filters			*/
/*	Last edited: May 5, 1989 at 12:10 pm				*/
/*									*/
/************************************************************************/


#include <libs/support/misc/general.h>


#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditorCP/FortEditorCP.i>

#include <libs/graphicInterface/cmdProcs/newEditor/fortEditorCP/ViewDialog.h>

#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/FortVFilter.h>

#include <libs/graphicInterface/oldMonitor/include/mon/dialog.h>
#include <libs/graphicInterface/oldMonitor/include/items/button.h>
#include <libs/graphicInterface/oldMonitor/include/items/check_box.h>
#include <libs/graphicInterface/oldMonitor/include/items/title.h>
#include <libs/graphicInterface/oldMonitor/include/items/text.h>
#include <libs/graphicInterface/oldMonitor/include/items/item_list.h>








/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




#define MAXSPECS     32


typedef struct
  {
    /* creation parameters */
      FortEditorCP         edcp;
      FortEditor           editor;

    /* contents */
      int                  numSpecs;
      FortVFilterSpec      spec[1+MAXSPECS];
      int                  numStdSpecs;

      Dialog *             dialog;
      DiaDesc *            nameItem;
      DiaDesc *            defItem;

    /* status */
      Generic              selectedNum;
      int                  curNum;
      char *               curName;
      char *               curDef;
      Boolean              curConcealed;
      Boolean              curErrors;

  } vdlg_Repr;

#define	R(ob)		((vdlg_Repr *) ob)






/************************/
/* Dialog items etc	*/
/************************/




#define ITEM_FILTERS		101	/* list of filter names               */
#define ITEM_ADD		102	/* add a filter name                  */
#define ITEM_DEL		103	/* remove a filter name               */
#define ITEM_CURNAME		104	/* text of current filter name        */
#define ITEM_CURDEF		105	/* text of current filter definition  */
#define ITEM_CURCONCEALED	106	/* 'concealed' of current filter def  */
#define ITEM_CURERRORS		107	/* 'errors' of current filter def     */
#define ITEM_OK			108	/* accept changes to current filter   */
#define ITEM_CANCEL		109	/* undo changes to current filter     */






/************************/
/* Miscellaneous	*/
/************************/




/* glsobal set by 'dialogHandler' when user does editing  */

static Boolean vdlg_changedAnything;






/************************/
/* Forward declarations	*/
/************************/




STATIC(void,             addStandardSpecs,(ViewDialog vdlg));
STATIC(FortVFilter,      makeFilter,(ViewDialog vdlg, int num));
STATIC(void,             makeDialog,(ViewDialog vdlg));
STATIC(Boolean,          dialogHandler,(Dialog *dialog, ViewDialog vdlg, 
                                        Generic item_id));
STATIC(void,             enableButtons,(ViewDialog vdlg));
STATIC(void,             addSpec,(ViewDialog vdlg, int *num));
STATIC(void,             deleteSpec,(ViewDialog vdlg, int num));
STATIC(void,             getCurSpec,(ViewDialog vdlg, int num));
STATIC(Boolean,          putCurSpec,(ViewDialog vdlg));
STATIC(void,             ensureCurSpec,(ViewDialog vdlg, int num));
STATIC(ListItemEntry,    listProc,(ViewDialog vdlg, DiaDesc *dd, Boolean first,
                                   Generic prevLineNum));



typedef FUNCTION_POINTER(void,NullDialogFunc,(Dialog*,Generic,Generic));


/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Initialization	*/
/************************/




void vdlg_Init()
{
  /* nothing */
}




void vdlg_Fini()
{
  /* nothing */
}




ViewDialog vdlg_Open(Context context, DB_FP *fp, FortEditorCP edcp, FortEditor editor)
{
  ViewDialog vdlg;
  int k;

  /* allocate a new instance */
    vdlg = (ViewDialog) get_mem(sizeof(vdlg_Repr),"FortEditorCP:ViewDialog");

  /* initialize the parts */
    /* set creation parameters */
      R(vdlg)->edcp = edcp;
      R(vdlg)->editor = editor;

    /* get our persistent information */
      if( fp != DB_NULLFP )
        { (void) db_buffered_read(fp, (char *) &R(vdlg)->numSpecs, sizeof(int));
          (void) db_buffered_read(fp, (char *) &R(vdlg)->numStdSpecs, sizeof(int));
          for( k = 1;  k <= R(vdlg)->numSpecs;  k++ )
            R(vdlg)->spec[k] = ffs_Open(context,fp,editor);
        }
      else
        addStandardSpecs(vdlg);

    /* set status */
      R(vdlg)->selectedNum  = UNUSED;
      R(vdlg)->curNum       = UNUSED;
      R(vdlg)->curName      = ssave("");
      R(vdlg)->curDef       = ssave("");
      R(vdlg)->curConcealed = false;
      R(vdlg)->curErrors    = false;

      makeDialog(vdlg);    /* needs preceding initializations */

  return vdlg;
}




void vdlg_Close(ViewDialog vdlg)
{
  int k;

  sfree(R(vdlg)->curName);
  sfree(R(vdlg)->curDef);

  for( k = 1;  k <= R(vdlg)->numSpecs;  k++ )
    ffs_Close(R(vdlg)->spec[k]);

  dialog_destroy(R(vdlg)->dialog);

  free_mem((void*) vdlg);
}




void vdlg_Save(ViewDialog vdlg, Context context, DB_FP *fp)
{
  int k;

  if( fp != DB_NULLFP )
    { (void) db_buffered_write(fp, (char *) &R(vdlg)->numSpecs, sizeof(int));
      (void) db_buffered_write(fp, (char *) &R(vdlg)->numStdSpecs, sizeof(int));
      for( k = 1;  k <= R(vdlg)->numSpecs;  k++ )
        ffs_Save(R(vdlg)->spec[k],context,fp);
    }
}




/************************/
/*  User interaction	*/
/************************/




Boolean vdlg_Dialog(ViewDialog vdlg)
{
  vdlg_changedAnything = false;

  enableButtons(vdlg);
  dialog_modal_run(R(vdlg)->dialog);

  return vdlg_changedAnything;
}




Boolean vdlg_Menu(ViewDialog vdlg, FortVFilter *filter)
{
  char * names[MAXSPECS];
  int k,choice;
  Boolean show;

  /* make a 0-based array of spec names w/special 1st choice */
    ff_GetShowErrors(*filter,&show);
    if( show )
      names[0] = "(hide error messages)";
    else
      names[0] = "(show error messages)";

    for( k = 1;  k <= R(vdlg)->numSpecs;  k++ )
      ffs_GetName(R(vdlg)->spec[k],&names[k]);

  choice = menu_select("Set filter:",R(vdlg)->numSpecs+1,names);

  if( choice != UNUSED )
    if( choice == 0 )
      ff_SetShowErrors(*filter,NOT(show));
    else
      *filter = makeFilter(vdlg,choice);

  return BOOL( choice != UNUSED );
}






/************************/
/*  Access to filters	*/
/************************/




int vdlg_NumFilterSpecs(ViewDialog vdlg)
{
  return R(vdlg)->numSpecs;
}




void vdlg_GetFilterSpec(ViewDialog vdlg, int num, char **name, char **definition,
                        Boolean *concealed, Boolean *errors)
{
  FortVFilterSpec spec = R(vdlg)->spec[num];

  ffs_GetName(spec,name);
  ffs_GetDefinition(spec,definition,concealed,errors);
}




void vdlg_GetFilterByName(ViewDialog vdlg, char *name, FortVFilter *filter)
{
  Boolean found;
  int k;
  char * thisName;

  found = false;
  k = 1;
  while( ! found  &&  k <= R(vdlg)->numSpecs )
    { ffs_GetName(R(vdlg)->spec[k],&thisName);
      if( strcmp(name,thisName) == 0 )
        found = true;
      else
        k += 1;
    }

  if( found )
    *filter = makeFilter(vdlg,k);
  else
    *filter = nil;
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void addStandardSpecs(ViewDialog vdlg)
{
  FortVFilterSpec spec;
  char * dummy;

  /* TEMPORARY */

  spec = ffs_Open(CONTEXT_NULL,DB_NULLFP,R(vdlg)->editor);
  ffs_SetName(spec,"normal");
  (void) ffs_SetDefinition(spec,"bold if erroneous",true,false,&dummy);
  R(vdlg)->spec[1] = spec;

  spec = ffs_Open(CONTEXT_NULL,DB_NULLFP,R(vdlg)->editor);
  ffs_SetName(spec,"all");
  (void) ffs_SetDefinition(spec,"dim if concealed",false,true,&dummy);
  R(vdlg)->spec[2] = spec;

  spec = ffs_Open(CONTEXT_NULL,DB_NULLFP,R(vdlg)->editor);
  ffs_SetName(spec,"errors only");
  (void) ffs_SetDefinition(spec,"hide if not erroneous",true,true,&dummy);
  R(vdlg)->spec[3] = spec;

  spec = ffs_Open(CONTEXT_NULL,DB_NULLFP,R(vdlg)->editor);
  ffs_SetName(spec,"headings only");
  (void) ffs_SetDefinition(spec,"hide if not heading",true,false,&dummy);
  R(vdlg)->spec[4] = spec;

  spec = ffs_Open(CONTEXT_NULL,DB_NULLFP,R(vdlg)->editor);
  ffs_SetName(spec,"no comments");
  (void) ffs_SetDefinition(spec,"hide if comment",true,false,&dummy);
  R(vdlg)->spec[5] = spec;

  R(vdlg)->numStdSpecs = 5;
  R(vdlg)->numSpecs = 5;
}




static
FortVFilter makeFilter(ViewDialog vdlg, int num)
{
  return ff_Open(CONTEXT_NULL,DB_NULLFP,R(vdlg)->editor,R(vdlg)->spec[num]);
}




static
void makeDialog(ViewDialog vdlg)
{
  char * title = "Define View Filters";
  DiaDesc *list_dd,*cur_dd,*whole_dd;
  Point listSize,defSize;

  listSize = makePoint(20,10);
  defSize  = makePoint(40,10);

  list_dd =
      dialog_desc_group(
          DIALOG_VERT_CENTER,
          3,
          item_title(DIALOG_UNUSED_ID,"Filters",DEF_FONT_ID),
          item_list(ITEM_FILTERS,(char *)0,vdlg,listProc,
		    &R(vdlg)->selectedNum,(Generic)UNUSED,false,DEF_FONT_ID,
                    listSize),
          dialog_desc_expand(
              dialog_desc_group(
                  DIALOG_HORIZ_CENTER,
                  2,
                  item_button(ITEM_ADD,"add",DEF_FONT_ID,false),
                  item_button(ITEM_DEL,"remove",DEF_FONT_ID,false)
                  )
              )
          );

  cur_dd =
      dialog_desc_group(
          DIALOG_VERT_CENTER,
          4,
          item_text(ITEM_CURNAME,(char*)0,DEF_FONT_ID,&R(vdlg)->curName,defSize.x),
          item_text2(ITEM_CURDEF,(char*)0,DEF_FONT_ID,&R(vdlg)->curDef,defSize),
          dialog_desc_group(
              DIALOG_HORIZ_CENTER,
              2,
              item_check_box(ITEM_CURCONCEALED,"auto conceal",DEF_FONT_ID,&R(vdlg)->curConcealed),
              item_check_box(ITEM_CURERRORS,"show error messages",DEF_FONT_ID,&R(vdlg)->curErrors)
              ),
          dialog_desc_expand(
              dialog_desc_group(
                  DIALOG_HORIZ_CENTER,
                  2,
                  item_button(ITEM_OK,"ok",DEF_FONT_ID,false),
                  item_button(ITEM_CANCEL,"cancel",DEF_FONT_ID,false)
                  )
              )
          );

  whole_dd =
      dialog_desc_group(
          DIALOG_HORIZ_TOP,
          2,
          list_dd,
          cur_dd
          );

  R(vdlg)->dialog   = dialog_create(title,dialogHandler,(NullDialogFunc)0,vdlg,whole_dd);
  R(vdlg)->nameItem = dialog_desc_lookup(R(vdlg)->dialog,ITEM_CURNAME);
  R(vdlg)->defItem  = dialog_desc_lookup(R(vdlg)->dialog,ITEM_CURDEF);
}




/*ARGSUSED*/

static
Boolean dialogHandler(Dialog *dialog, ViewDialog vdlg, Generic item_id)
{
  Boolean result = DIALOG_NOMINAL;    /* unless changed below */
  int num;

  switch( item_id )
    {
      case ITEM_FILTERS:
        ensureCurSpec(vdlg,R(vdlg)->selectedNum);
        break;

      case ITEM_ADD:
        if( putCurSpec(vdlg) )
          { addSpec(vdlg,&num);
            getCurSpec(vdlg,num);
          }
        break;

      case ITEM_DEL:
        num = R(vdlg)->curNum;
        getCurSpec(vdlg,UNUSED);
        deleteSpec(vdlg,num);
        break;

      case ITEM_OK:
        (void) putCurSpec(vdlg);
        break;

      case ITEM_CANCEL:
        getCurSpec(vdlg,R(vdlg)->curNum);
        break;

      case DIALOG_DEFAULT_ID:
        item_text_handle_keyboard(R(vdlg)->defItem,KB_Enter,false);
        break;

      case DIALOG_CANCEL_ID:
        (void) putCurSpec(vdlg);
        result = DIALOG_QUIT;
        break;
    }

  enableButtons(vdlg);

  return result;
}




static
void enableButtons(ViewDialog vdlg)
{
  Dialog * dialog = R(vdlg)->dialog;
  int numSpecs    = R(vdlg)->numSpecs;
  int numStdSpecs = R(vdlg)->numStdSpecs;
  int curNum      = R(vdlg)->curNum;
  Boolean nonStd  = BOOL(curNum != UNUSED && curNum > numStdSpecs);

# define ABLE(i,p)  dialog_item_ability(dialog,i,(p ? DIALOG_ENABLE : DIALOG_DISABLE))

  ABLE(ITEM_ADD,     numSpecs < MAXSPECS);
  ABLE(ITEM_DEL,     nonStd && ! ffs_InUse(R(vdlg)->spec[curNum]));

  ABLE(ITEM_OK,      nonStd  &&  strlen(R(vdlg)->curName) > (size_t)0);
  ABLE(ITEM_CANCEL,  nonStd);
}




static
void addSpec(ViewDialog vdlg, int *num)
{
  int nextSpec = 1 + R(vdlg)->numSpecs;

  R(vdlg)->spec[nextSpec] = ffs_Open(CONTEXT_NULL,DB_NULLFP,R(vdlg)->editor);
  R(vdlg)->numSpecs = nextSpec;
  *num = nextSpec;

  dialog_item_modified(R(vdlg)->dialog,ITEM_FILTERS);
  vdlg_changedAnything = true;
}




static
void deleteSpec(ViewDialog vdlg, int num)
{
  int k;

  ffs_Close(R(vdlg)->spec[num]);

  for( k = num;  k < R(vdlg)->numSpecs;  k++ )
    R(vdlg)->spec[k] = R(vdlg)->spec[k+1];

  R(vdlg)->numSpecs -= 1;

  dialog_item_modified(R(vdlg)->dialog,ITEM_FILTERS);
  vdlg_changedAnything = true;
}




static
void getCurSpec(ViewDialog vdlg, int num)
{
  FortVFilterSpec spec = (num == UNUSED  ?  nil  :  R(vdlg)->spec[num]);
  char * name;
  char * def;
  Boolean concealed,errors;

  /* adjust the current spec number */
    R(vdlg)->curNum = num;
    R(vdlg)->selectedNum = num;
    dialog_item_modified(R(vdlg)->dialog,ITEM_FILTERS);

  /* adjust the current spec name */
    sfree(R(vdlg)->curName);
    if( num == UNUSED )  name = "";
    else ffs_GetName(spec,&name);
    R(vdlg)->curName = ssave(name);
    dialog_item_modified(R(vdlg)->dialog,ITEM_CURNAME);
    item_text_set_xy(R(vdlg)->nameItem,makePoint(0,9999));

  /* adjust the current spec definition */
    sfree(R(vdlg)->curDef);
    if( num == UNUSED )
      { def = "";
        concealed = true;
        errors = false;
      }
    else
      ffs_GetDefinition(spec,&def,&concealed,&errors);

    R(vdlg)->curDef = ssave(def);
    dialog_item_modified(R(vdlg)->dialog,ITEM_CURDEF);
    item_text_set_xy(R(vdlg)->defItem,makePoint(9999,9999));

    R(vdlg)->curConcealed = concealed;
    dialog_item_modified(R(vdlg)->dialog,ITEM_CURCONCEALED);

    R(vdlg)->curErrors = errors;
    dialog_item_modified(R(vdlg)->dialog,ITEM_CURERRORS);

  /* set the keyboard focus */
    if( strlen(R(vdlg)->curName) == 0 )
      (void) dialog_item_set_focus(R(vdlg)->dialog,ITEM_CURNAME);
    else
      (void) dialog_item_set_focus(R(vdlg)->dialog,ITEM_CURDEF);
}




static
Boolean putCurSpec(ViewDialog vdlg)
{
  FortVFilterSpec spec;
  Boolean ok;
  char * errorMsg;

  if( R(vdlg)->curNum != UNUSED )
    { spec = R(vdlg)->spec[R(vdlg)->curNum];

      /* store the current spec definition first, in case it's wrong */
        ok = ffs_SetDefinition(spec,
                               R(vdlg)->curDef, R(vdlg)->curConcealed, R(vdlg)->curErrors, 
                               &errorMsg);

      if( ok )
        { /* store the current spec name */
            ffs_SetName(spec,R(vdlg)->curName);
            dialog_item_modified(R(vdlg)->dialog,ITEM_FILTERS);

          vdlg_changedAnything = true;
        }
      else
        dialog_error(R(vdlg)->dialog,errorMsg,1,ITEM_CURDEF);
    }
  else
    ok = true;

  return ok;
}




static
void ensureCurSpec(ViewDialog vdlg, int num)
{
  int curNum = R(vdlg)->curNum;

  if( num != curNum )
    if( putCurSpec(vdlg) )
      getCurSpec(vdlg,num);
    else
      dialog_item_modified(R(vdlg)->dialog,ITEM_FILTERS);
}




/*ARGSUSED*/

static
ListItemEntry listProc(ViewDialog vdlg, DiaDesc *dd, Boolean first, Generic prevLineNum)
{
  int lineNum = (first)  ?  1  :  prevLineNum + 1;
  ListItemEntry line;
  char * name;

  if( lineNum <= R(vdlg)->numSpecs )
    { ffs_GetName(R(vdlg)->spec[lineNum],&name);
      if( strlen(name) == 0 )  name = "(new filter)";

      line = item_list_create_entry(dd, (Generic) lineNum, name, true);
    }
  else
    line = NULL_LIST_ITEM_ENTRY;

  return line;
}






