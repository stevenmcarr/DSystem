/* $Id: FilterDefSet.C,v 1.9 1997/06/25 14:43:41 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/FilterDefSet.C					*/
/*									*/
/*	FilterDefSet -- Editable set of UserFilterDefs			*/
/*	Last edited: October 13, 1992 at 6:00 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/FilterDefSet.h>

#include <libs/graphicInterface/framework/Editor.h>
#include <libs/graphicInterface/framework/UserFilterDef.h>
#include <libs/graphicInterface/framework/UserFilter.h>

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




/* FilterDefSet object */


#define MAXDEFS     32


typedef struct FilterDefSet_Repr_struct
  {
    /* creation parameters */
      Editor *		editor;

    /* contents */
      int		numDefs;
      UserFilterDef *	def[1+MAXDEFS];
      int		numStdDefs;

      Dialog *		dialog;
      DiaDesc *		nameItem;
      DiaDesc *		textItem;

    /* status */
      Generic		selectedNum;
      int		curNum;
      char *		curName;
      char *		curText;
      Boolean		curConcealed;
      Boolean		curErrors;

  } FilterDefSet_Repr;


#define R(ob)		(ob->FilterDefSet_repr)

#define INHERITED	DBObject






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






/*************************/
/*  Miscellaneous	 */
/*************************/




/* global set by 'dialogHandler' when user does editing  */

static Boolean FilterDefSet_changedAnything;







/*************************/
/*  Forward declarations */
/*************************/




static void addStandardDefs(FilterDefSet * fs);

static void makeDialog(FilterDefSet * fs);

static Boolean dialogHandler(Dialog * dialog,
                             FilterDefSet * fs,
                             Generic item_id);

static void enableButtons(FilterDefSet * fs);

static void addDef(FilterDefSet * fs, int &num);

static void deleteDef(FilterDefSet * fs, int num);

static void getCurDef(FilterDefSet * fs, int num);

static Boolean putCurDef(FilterDefSet * fs);

static void ensureCurDef(FilterDefSet * fs, int num);

static ListItemEntry listProc(FilterDefSet * fs,
                              DiaDesc * dd,
                              Boolean first,
                              int prevLineNum);







/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void FilterDefSet::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(Editor);
    REQUIRE_INIT(UserFilterDef);
}




void FilterDefSet::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(FilterDefSet)




FilterDefSet::FilterDefSet(Context context, DB_FP * session_fp,
                           Editor * editor)
   : DBObject (context, session_fp)
{
  /* allocate instance's private data */
    this->FilterDefSet_repr = (FilterDefSet_Repr *)
                              get_mem(sizeof(FilterDefSet_Repr),
                                      "FilterDefSet instance");

  /* set creation parameters */
    R(this)->editor = editor;

  /* set status */
    R(this)->selectedNum  = UNUSED;
    R(this)->curNum       = UNUSED;
    R(this)->curName      = ssave("");
    R(this)->curText       = ssave("");
    R(this)->curConcealed = false;
    R(this)->curErrors    = false;
}




FilterDefSet::~FilterDefSet()
{
  int k;

  sfree(R(this)->curName);
  sfree(R(this)->curText);

  for( k = 1;  k <= R(this)->numDefs;  k++ )
    delete R(this)->def[k];

  dialog_destroy(R(this)->dialog);

  free_mem((void*) this->FilterDefSet_repr);
}




Editor * FilterDefSet::GetEditor(void)
{
  return R(this)->editor;
}






/*************/
/*  Database */
/*************/




void FilterDefSet::isnew(Context context)
{
  addStandardDefs(this);
  makeDialog(this);
}




void FilterDefSet::read(DB_FP * fp, DB_FP * session_fp)
{
  int k;

  (void) db_buffered_read(session_fp, (char *) &R(this)->numDefs,    sizeof(int));
  (void) db_buffered_read(session_fp, (char *) &R(this)->numStdDefs, sizeof(int));

  for( k = 1;  k <= R(this)->numDefs;  k++ )
    R(this)->def[k] = R(this)->editor->OpenFilterDef(CONTEXT_NULL,
						     CONTEXT_NULL, 
						     CONTEXT_NULL, 
						     session_fp);

  makeDialog(this);
}




void FilterDefSet::write(DB_FP * fp, DB_FP * session_fp)
{
  int k;

  (void) db_buffered_write(session_fp, (char *) &R(this)->numDefs,    sizeof(int));
  (void) db_buffered_write(session_fp, (char *) &R(this)->numStdDefs, sizeof(int));

  for( k = 1;  k <= R(this)->numDefs;  k++ )
    R(this)->def[k]->Save(CONTEXT_NULL, session_fp);
}






/************************/
/*  Access to filters	*/
/************************/




int FilterDefSet::NumFilterDefs(void)
{
  return R(this)->numDefs;
}




void FilterDefSet::AddFilterDef(UserFilterDef * def)
{
  int nextDef = 1 + R(this)->numDefs;

  R(this)->def[nextDef] = def;
  R(this)->numDefs = nextDef;

  dialog_item_modified(R(this)->dialog, ITEM_FILTERS);
}




void FilterDefSet::GetFilterDef(int k, UserFilterDef * &def)
{
  def = R(this)->def[k];
}




void FilterDefSet::GetFilterDefByName(char * name, UserFilterDef * &def)
{
  Boolean found;
  int k;
  char * thisName;

  found = false;
  k = 1;
  while( ! found  &&  k <= R(this)->numDefs )
    { R(this)->def[k]->GetName(thisName);
      if( strcmp(name, thisName) == 0 )
        found = true;
      else
        k += 1;
    }

  if( found )
    def = R(this)->def[k];
  else
    def = nil;
}






/************************/
/*  User interaction	*/
/************************/




Boolean FilterDefSet::DoDialog(void)
{
  FilterDefSet_changedAnything = false;

  enableButtons(this);
  dialog_modal_run(R(this)->dialog);

  return FilterDefSet_changedAnything;
}




Boolean FilterDefSet::DoMenu(UserFilter * &filter, void * environment)
{
  char * names[MAXDEFS];
  int k, choice;
  Boolean show;

  /* make a 0-based array of def names w/special 1st choice */
    filter->GetShowErrors(show);
    if( show )
      names[0] = "(hide error messages)";
    else
      names[0] = "(show error messages)";

    for( k = 1;  k <= R(this)->numDefs;  k++ )
      R(this)->def[k]->GetName(names[k]);

  choice = menu_select("Set filter:", R(this)->numDefs+1, names);

  if( choice != UNUSED )
    if( choice == 0 )
      filter->SetShowErrors(NOT(show));
    else
      filter = R(this)->def[choice]->MakeFilter(environment);

  return BOOL( choice != UNUSED );
}







/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void addStandardDefs(FilterDefSet * fs)
{
  UserFilterDef * def;
  char * dummy;

  /* TEMPORARY */

#ifdef NOTDEF

  def = R(fs)->editor->OpenFilterDef(CONTEXT_NULL,
				     CONTEXT_NULL, CONTEXT_NULL, DB_NULLFP);
  def->SetName("normal");
  (void) def->SetDefinition("bold if erroneous", true, false, dummy);
  R(fs)->def[1] = def;

  def = R(fs)->editor->OpenFilterDef(CONTEXT_NULL,
				     CONTEXT_NULL, CONTEXT_NULL, DB_NULLFP);
  def->SetName("all");
  (void) def->SetDefinition("dim if concealed", false, true, dummy);
  R(fs)->def[2] = def;

  def = R(fs)->editor->OpenFilterDef(CONTEXT_NULL,
				     CONTEXT_NULL, CONTEXT_NULL, DB_NULLFP);
  def->SetName("errors only");
  (void) def->SetDefinition("hide if not erroneous", true, true, dummy);
  R(fs)->def[3] = def;

  def = R(fs)->editor->OpenFilterDef(CONTEXT_NULL,
				     CONTEXT_NULL, CONTEXT_NULL, DB_NULLFP);
  def->SetName("headings only");
  (void) def->SetDefinition("hide if not heading", true, false, dummy);
  R(fs)->def[4] = def;

  def = R(fs)->editor->OpenFilterDef(CONTEXT_NULL,
				     CONTEXT_NULL, CONTEXT_NULL, DB_NULLFP);
  def->SetName("no comments");
  (void) def->SetDefinition("hide if comment", true, false, dummy);
  R(fs)->def[5] = def;

  R(fs)->numStdDefs = 5;
  R(fs)->numDefs = 5;

#else

  R(fs)->numStdDefs = 0;
  R(fs)->numDefs = 0;

#endif
}




static
void makeDialog(FilterDefSet * fs)
{
  char * title = "Define View Filters";
  DiaDesc * list_dd, * cur_dd, * whole_dd;
  Point listSize, defSize;

  listSize = makePoint(20, 10);
  defSize  = makePoint(40, 10);

  list_dd =
      dialog_desc_group(
          DIALOG_VERT_CENTER,
          3,
          item_title(DIALOG_UNUSED_ID, "Filters", DEF_FONT_ID),
          item_list(ITEM_FILTERS, (char *) nil, (Generic) fs,
                    (item_list_elem_proc) listProc,
		    &R(fs)->selectedNum, (Generic) UNUSED, false, DEF_FONT_ID,
                    listSize),
          dialog_desc_expand(
              dialog_desc_group(
                  DIALOG_HORIZ_CENTER,
                  2,
                  item_button(ITEM_ADD, "add", DEF_FONT_ID, false),
                  item_button(ITEM_DEL, "remove", DEF_FONT_ID, false)
                  )
              )
          );

  cur_dd =
      dialog_desc_group(
          DIALOG_VERT_CENTER,
          4,
          item_text(ITEM_CURNAME, (char *) nil, DEF_FONT_ID, &R(fs)->curName, defSize.x),
          item_text2(ITEM_CURDEF, (char *) nil, DEF_FONT_ID, &R(fs)->curText, defSize),
          dialog_desc_group(
              DIALOG_HORIZ_CENTER,
              2,
              item_check_box(ITEM_CURCONCEALED, "auto conceal", DEF_FONT_ID, &R(fs)->curConcealed),
              item_check_box(ITEM_CURERRORS, "show error messages", DEF_FONT_ID, &R(fs)->curErrors)
              ),
          dialog_desc_expand(
              dialog_desc_group(
                  DIALOG_HORIZ_CENTER,
                  2,
                  item_button(ITEM_OK, "ok", DEF_FONT_ID, false),
                  item_button(ITEM_CANCEL, "cancel", DEF_FONT_ID, false)
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

  R(fs)->dialog   = dialog_create(title, (dialog_handler_callback) dialogHandler, (dialog_helper_callback) 0, (Generic) fs, whole_dd);
  R(fs)->nameItem = dialog_desc_lookup(R(fs)->dialog, ITEM_CURNAME);
  R(fs)->textItem  = dialog_desc_lookup(R(fs)->dialog, ITEM_CURDEF);
}




/*ARGSUSED*/

static
Boolean dialogHandler(Dialog * dialog, FilterDefSet * fs, Generic item_id)
{
  Boolean result = DIALOG_NOMINAL;    /* unless changed below */
  int num;

  switch( item_id )
    {
      case ITEM_FILTERS:
        ensureCurDef(fs, R(fs)->selectedNum);
        break;

      case ITEM_ADD:
        if( putCurDef(fs) )
          { addDef(fs, num);
            getCurDef(fs, num);
          }
        break;

      case ITEM_DEL:
        num = R(fs)->curNum;
        getCurDef(fs, UNUSED);
        deleteDef(fs, num);
        break;

      case ITEM_OK:
        (void) putCurDef(fs);
        break;

      case ITEM_CANCEL:
        getCurDef(fs, R(fs)->curNum);
        break;

      case DIALOG_DEFAULT_ID:
        item_text_handle_keyboard(R(fs)->textItem, KB_Enter, false);
        break;

      case DIALOG_CANCEL_ID:
        (void) putCurDef(fs);
        result = DIALOG_QUIT;
        break;
    }

  enableButtons(fs);

  return result;
}




static
void enableButtons(FilterDefSet * fs)
{
  Dialog * dialog = R(fs)->dialog;
  int numDefs     = R(fs)->numDefs;
  int numStdDefs  = R(fs)->numStdDefs;
  int curNum      = R(fs)->curNum;
  Boolean nonStd  = BOOL(curNum != UNUSED && curNum > numStdDefs);

# define ABLE(i, p)  dialog_item_ability(dialog, i, (p ? DIALOG_ENABLE : DIALOG_DISABLE))

  ABLE(ITEM_ADD,     numDefs < MAXDEFS);
  ABLE(ITEM_DEL,     nonStd && ! R(fs)->def[curNum]->InUse());

  ABLE(ITEM_OK,      nonStd  &&  strlen(R(fs)->curName) > 0);
  ABLE(ITEM_CANCEL,  nonStd);
}




static
void addDef(FilterDefSet * fs, int &num)
{
  int nextDef = 1 + R(fs)->numDefs;

  R(fs)->def[nextDef] = R(fs)->editor->OpenFilterDef(CONTEXT_NULL,
						     CONTEXT_NULL,
						     CONTEXT_NULL,
						     DB_NULLFP);
  R(fs)->numDefs = nextDef;
  num = nextDef;

  dialog_item_modified(R(fs)->dialog, ITEM_FILTERS);
  FilterDefSet_changedAnything = true;
}




static
void deleteDef(FilterDefSet * fs, int num)
{
  int k;

  delete R(fs)->def[num];

  for( k = num;  k < R(fs)->numDefs;  k++ )
    R(fs)->def[k] = R(fs)->def[k+1];

  R(fs)->numDefs -= 1;

  dialog_item_modified(R(fs)->dialog, ITEM_FILTERS);
  FilterDefSet_changedAnything = true;
}




static
void getCurDef(FilterDefSet * fs, int num)
{
  UserFilterDef * def = (num == UNUSED  ?  nil  :  R(fs)->def[num]);
  char * name;
  char * text;
  Boolean concealed, errors;

  /* adjust the current def number */
    R(fs)->curNum = num;
    R(fs)->selectedNum = num;
    dialog_item_modified(R(fs)->dialog, ITEM_FILTERS);

  /* adjust the current def name */
    sfree(R(fs)->curName);
    if( num == UNUSED )  name = "";
    else def->GetName(name);
    R(fs)->curName = ssave(name);
    dialog_item_modified(R(fs)->dialog, ITEM_CURNAME);
    item_text_set_xy(R(fs)->nameItem, makePoint(0, 9999));

  /* adjust the current def's text */
    sfree(R(fs)->curText);
    if( num == UNUSED )
      { text = "";
        concealed = true;
        errors = false;
      }
    else
      def->GetDefinition(text, concealed, errors);

    R(fs)->curText = ssave(text);
    dialog_item_modified(R(fs)->dialog, ITEM_CURDEF);
    item_text_set_xy(R(fs)->textItem, makePoint(9999, 9999));

    R(fs)->curConcealed = concealed;
    dialog_item_modified(R(fs)->dialog, ITEM_CURCONCEALED);

    R(fs)->curErrors = errors;
    dialog_item_modified(R(fs)->dialog, ITEM_CURERRORS);

  /* set the keyboard focus */
    if( strlen(R(fs)->curName) == 0 )
      (void) dialog_item_set_focus(R(fs)->dialog, ITEM_CURNAME);
    else
      (void) dialog_item_set_focus(R(fs)->dialog, ITEM_CURDEF);
}




static
Boolean putCurDef(FilterDefSet * fs)
{
  UserFilterDef * def;
  Boolean ok;
  char * errorMsg;

  if( R(fs)->curNum != UNUSED )
    { def = R(fs)->def[R(fs)->curNum];

      /* store the current definition first, in case it's wrong */
        ok = def->SetDefinition(R(fs)->curText,
                                R(fs)->curConcealed, R(fs)->curErrors, 
                                errorMsg);

      if( ok )
        { /* store the current def name */
            def->SetName(R(fs)->curName);
            dialog_item_modified(R(fs)->dialog, ITEM_FILTERS);

          FilterDefSet_changedAnything = true;
        }
      else
        dialog_error(R(fs)->dialog, errorMsg, 1, ITEM_CURDEF);
    }
  else
    ok = true;

  return ok;
}




static
void ensureCurDef(FilterDefSet * fs, int num)
{
  int curNum = R(fs)->curNum;

  if( num != curNum )
    if( putCurDef(fs) )
      getCurDef(fs, num);
    else
      dialog_item_modified(R(fs)->dialog, ITEM_FILTERS);
}




/*ARGSUSED*/

static
ListItemEntry listProc(FilterDefSet * fs,
                       DiaDesc * dd,
                       Boolean first,
                       int prevLineNum)
{
  int lineNum = (first ? 1 : prevLineNum + 1);
  ListItemEntry line;
  char * name;

  if( lineNum <= R(fs)->numDefs )
    { R(fs)->def[lineNum]->GetName(name);
      if( strlen(name) == 0 )
        name = "(new filter)";

      line = item_list_create_entry(dd, (Generic) lineNum, name, true);
    }
  else
    line = NULL_LIST_ITEM_ENTRY;

  return line;
}
