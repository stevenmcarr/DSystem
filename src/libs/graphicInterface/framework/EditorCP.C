/* $Id: EditorCP.C,v 1.8 1997/03/11 14:32:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/EditorCP.C						*/
/*									*/
/*	EditorCP -- Editor-like Command Processor			*/
/*	Last edited: October 13, 1993 at 5:56 pm			*/
/*									*/
/************************************************************************/


#include <assert.h>


#include <libs/graphicInterface/framework/EditorCP.h>

#include <libs/fileAttrMgmt/attributedFile/AttributedFile.h>
#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>

#include <libs/graphicInterface/framework/Editor.h>
#include <libs/support/database/context.h>
#include <libs/support/database/newdatabase.h>
#include <libs/support/time/times.h>
#include <libs/graphicInterface/framework/View.h>

#include <libs/support/file/UnixFile.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/filer.h>



/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* EditorCP object */

typedef struct EditorCP_Repr_struct
  {
    /* creation parameters */
      Context           mod_context;
      Context           pgm_context;
      Context           mod_in_pgm_context;

    /* contents */
      Editor *		editor;
      View *		view;

    /* session state */
      Point		minSize;
      Point		contentsSize;

    /* status */
      Boolean		dirty;

  } EditorCP_Repr;


#define R(ob)		(ob->EditorCP_repr)


#define INHERITED	CP






/*************************/
/*  Miscellaneous	 */
/*************************/

/* 
 * Program context (after PED has checked it for existence, etc),
 * Regrettably, this seems to be the only way to get access to this
 * information (i.e. a hack).
 */

extern Context checked_program_context;




/* startup data structure */

EditorCPStartupStruct EditorCP_startup;







/*************************/
/*  Forward declarations */
/*************************/




static void openSessionContext(EditorCP * edcp,
                               DB_FP * &session_fp);

static void getSessionInfo(EditorCP * edcp, DB_FP * fp);

static void putSessionInfo(EditorCP * edcp, DB_FP * fp);

static void save(EditorCP * edcp, Context context);

static void askAndSave(EditorCP * edcp, Boolean saveAs, Boolean saveCopy);







/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void EditorCP::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(CP);

  /* initialize default startup descriptor */
    EditorCP_startup.mod_context        = CONTEXT_NULL;
    EditorCP_startup.mod_in_pgm_context = CONTEXT_NULL;
    EditorCP_startup.pgm_context        = CONTEXT_NULL;
}




void EditorCP::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(EditorCP)




EditorCP::EditorCP(Generic parent_id, Generic cp_id, void * startup):
               CP(parent_id, cp_id)
{
  EditorCPStartupStruct * ess = (EditorCPStartupStruct *) startup;
  Context mod_in_pgm_context;
  Context pgm_context;

  /* allocate instance's private data */
    this->EditorCP_repr = (EditorCP_Repr *) get_mem(sizeof(EditorCP_Repr),
                                                    "EditorCP instance");

  /* save creation arguments */
    R(this)->mod_context = ess->mod_context;

  /* if a program context is specified, see if the composition is okay? */
    pgm_context = ess->pgm_context;

  /* stash away the program context for use by the IP annotation browser -- hack */
    checked_program_context = pgm_context;

    R(this)->pgm_context = pgm_context;

    if (pgm_context == CONTEXT_NULL) mod_in_pgm_context = CONTEXT_NULL;
    else {
      mod_in_pgm_context = new FortranModule;
      int code = 
	mod_in_pgm_context->Open(R(this)->mod_context->ReferenceFilePathName(),
				 pgm_context);
      assert(code == 0);
    }

   R(this)->mod_in_pgm_context = mod_in_pgm_context;

  /* initialize so init code can tell if the editor has been made yet */
    R(this)->editor = nil;

  /* initialize status */
    R(this)->dirty = false;
}




EditorCP::~EditorCP(void)
{
  /* destroy the contents */

    delete R(this)->view;
    delete R(this)->editor;

    // delete R(this)-> mod_context;
    delete R(this)-> pgm_context;

  free_mem((void*) this->EditorCP_repr);
}




void EditorCP::Init(void)
{
  DB_FP * session_fp;

  openSessionContext(this, session_fp);

  this->Open(R(this)->mod_context,	
	     R(this)->mod_in_pgm_context,
	     R(this)->pgm_context, session_fp);

  if( session_fp != DB_NULLFP ) 
    R(this)->mod_context->CloseExternalAttributeFile(session_fp);

  /* sigh! superclass wants to send self subclass-messages */
    this->INHERITED::Init();
}




void EditorCP::Fini(void)
{
  /* close editor */
    this->Close();
}




Editor * EditorCP::openEditor(Context mod_context,
			      Context mod_in_pgm_context,
			      Context pgm_context,
			      DB_FP * session_fp)
{
  SUBCLASS_RESPONSIBILITY("EditorCP::openEditor");
  return nil;
}




void EditorCP::closeEditor(void)
{
  SUBCLASS_RESPONSIBILITY("EditorCP::closeEditor");
}




View * EditorCP::getView(void)
{
  return R(this)->view;
}






/*******************/
/*  Window layout  */
/*******************/




char * EditorCP::GetWindowTitle(void)
{
  Editor * ed = R(this)->editor;
  char   * dirty;
  const char   * context;
  char   * format;
  static char title[200];	/* static so can return a pointer to it */

  /* calculate the contents-is-dirty string */
    dirty = (R(this)->dirty  ?  "*"  : " ");
    context = R(this)->mod_context->ReferenceFilePathName();

  /* allow for subclass's preference  */
    format = this->GetWindowTitleFormat();
    (void) sprintf(title, format, dirty, context);

  return title;    /* this works because 'title' is 'static' */
}




char * EditorCP::GetWindowTitleFormat(void)
{
  return "%s %s";
}





void EditorCP::GetContentsSizePrefs(Point &minSize, Point &defSize)
{
  minSize = R(this)->minSize;
  defSize = R(this)->contentsSize;
}




Generic EditorCP::GetContentsTiling(Boolean init, Point contentsSize)
{
  R(this)->contentsSize = contentsSize;
  return R(this)->view->GetTiling(init, contentsSize);
}




void EditorCP::InitContentsPanes(void)
{
  R(this)->view->InitPanes();
}







/***********/
/*  Menus  */
/***********/




void EditorCP::InitMenuBar(MenuBar * mb)
{
  this->INHERITED::InitMenuBar(mb);

  mb->AddMenu("file",   UNUSED, "");
  mb->AddMenu("edit",   UNUSED, "");
  mb->AddMenu("view",   UNUSED, "");
  mb->AddMenu("search", UNUSED, "");
}





void EditorCP::InitMenu(char * name, CMenu * m)
{
# define CTRL_QU	toKbChar('?' & 0x1f)	/* control-'?' */

  this->INHERITED::InitMenu(name, m);

  if( strcmp("file", name) == 0 )
    {
      m->AddItem(CMD_NEW,           KB_Nul,      "new",              "", UNUSED, 0);
      m->AddItem(CMD_EDIT,          KB_Nul,      "edit",             "", UNUSED, 0);
      m->AddItem(CMD_BROWSE,        KB_Nul,      "browse",           "", UNUSED, 0);
      m->AddItem(CMD_SAVE_AS,       KB_Nul,      "save as",          "", UNUSED, 0);
      m->AddItem(CMD_SAVE_COPY,     KB_Nul,      "save a copy",      "", UNUSED, 0);
      m->AddItem(CMD_SAVE,          KB_Nul,      "save",             "", UNUSED, 0);
    }

  if( strcmp("edit", name) == 0 )
    {
      m->AddItem(CMD_UNDO,          KB_top(1),   "undo",             "", UNUSED, 0);
      m->AddItem(CMD_COPY,          KB_top(2),   "copy",             "", UNUSED, 0);
      m->AddItem(CMD_CUT,           KB_top(3),   "cut",              "", UNUSED, 0);
      m->AddItem(CMD_PASTE,         KB_top(4),   "paste",            "", UNUSED, 0);
      m->AddItem(CMD_CLEAR,         KB_Nul,      "clear",            "", UNUSED, 0);
      m->AddItem(CMD_SELECT_ALL,    KB_Nul,      "select all",       "", UNUSED, 0);
    }

  if( strcmp("view", name) == 0 )
    {
      m->AddItem(CMD_DEF_FILTER,    KB_Nul,      "define filter...", "", UNUSED, 0);
      m->AddItem(CMD_SET_FILTER,    KB_Nul,      "set filter",       "", UNUSED, 0);
      m->AddItem(CMD_SPLIT_VIEW,    KB_Nul,      "split window",     "", UNUSED, 0);
      m->AddItem(CMD_SHOW_SEL,      CTRL_QU,     "show selection",   "", UNUSED, 0);
    }

  if( strcmp("search", name) == 0 )
    {
      m->AddItem(CMD_FIND,          KB_Nul,      "find...",          "", UNUSED, 0);
      m->AddItem(CMD_FIND_NEXT,     KB_right(6), "find next",        "", UNUSED, 0);
      m->AddItem(CMD_FIND_PREV,     KB_right(4), "find previous",    "", UNUSED, 0);
      m->AddItem(CMD_REPLACE,       KB_Nul,      "replace...",       "", UNUSED, 0);
      m->AddItem(CMD_GENERAL_INFO,  KB_Nul,      "general info...",  "", UNUSED, 0);
      m->AddItem(CMD_SELECTED_INFO, KB_Nul,      "selected info...", "", UNUSED, 0);
    }
}






/********************/
/*  Input handling  */
/********************/




Boolean EditorCP::QueryKill(void)
{
  Boolean kill, wantSave;

  if( R(this)->dirty )
    { kill = yes_no("Save before quitting?", &wantSave, true);
      if( kill && wantSave )
        askAndSave(this, false, false);
    }
  else
    kill = true;

  return kill;
}




void EditorCP::SelectionEvent(Generic generator, Point info)
{
  (void) R(this)->view->SelectionEvent(generator, info);
}





Boolean EditorCP::MenuChoice(Generic cmd)
{
  Boolean handled = true;

  switch( cmd )
    {
      /* "file" menu commands */

      case CMD_NEW:
        notImplemented("file/new");
        break;

      case CMD_EDIT:
        notImplemented("file/edit");
        break;

      case CMD_BROWSE:
        notImplemented("file/browse");
        break;

      case CMD_SAVE:
        askAndSave(this, false, false);
        break;

      case CMD_SAVE_AS:
        askAndSave(this, true, false);
        break;

      case CMD_SAVE_COPY:
        askAndSave(this, true, true);
        break;


      /* "edit" menu commands */

      case CMD_UNDO:
      case CMD_COPY:
      case CMD_CUT:
      case CMD_PASTE:
      case CMD_CLEAR:
      case CMD_SELECT_ALL:
        handled = R(this)->editor->MenuChoice(cmd);
        break;


      /* "view" menu commands */

      case CMD_SPLIT_VIEW:
        notImplemented("view/split window");
        break;

      case CMD_SHOW_SEL:
        notImplemented("view/show selection");
        break;

      case CMD_DEF_FILTER:
      case CMD_SET_FILTER:
        handled = R(this)->editor->MenuChoice(cmd);
        break;

      /* "search" menu commands */

      case CMD_FIND:
        notImplemented("search/find");
        break;

      case CMD_FIND_NEXT:
        notImplemented("search/find next");
        break;

      case CMD_FIND_PREV:
        notImplemented("search/find previous");
        break;

      case CMD_REPLACE:
        notImplemented("search/replace");
        break;

      case CMD_GENERAL_INFO:
        notImplemented("search/general info");
        break;

      case CMD_SELECTED_INFO:
        notImplemented("search/selected info");
        break;


      /* other menu commands */

      default:
        handled = this->INHERITED::MenuChoice(cmd);
        break;
    }

  return handled;
}




void EditorCP::Keystroke(KbChar kb)
{
  R(this)->editor->Keystroke(kb);
}







/************************/
/*  Change notification */
/************************/




void EditorCP::NoteChange(Object * ob, int kind, void * change)
{
  NoteChangeTrace trace(this, ob, kind, change);

  if( kind == CHANGE_DOCUMENT )
    { R(this)->dirty = true;
      this->SetWindowTitle(this->GetWindowTitle());
    }
}








/*************/
/*  Database */
/*************/





void EditorCP::GetSessionAttribute(char * &session_attr,
                                   int &oldest,
                                   int &newest)
{
  session_attr = "EditorSession";
  oldest       = 1;
  newest       = 1;
}




void EditorCP::GetContentsAttribute(char * &contents_attr)
{
  contents_attr = "source";    /* TEMPORARY */
}




void EditorCP::Open(Context mod_context,
		    Context mod_in_pgm_context,
		    Context pgm_context,
		    DB_FP * fp)
{
  /* session info */
    getSessionInfo(this, fp);

  /* editor */
    R(this)->editor = this->openEditor(mod_context,
				       mod_in_pgm_context,
				       pgm_context,
				       fp);
    R(this)->editor->Notify(this, true);

  /* view */
    R(this)->view = R(this)->editor->OpenView(mod_context, fp);

  /* get size prefs if not stored previously */
    if( R(this)->contentsSize.x == 0 )
      R(this)->view->GetSizePrefs(R(this)->minSize,
                                    R(this)->contentsSize);
}




void EditorCP::Close(void)
{
  /* close editor */
    this->closeEditor();
}




void EditorCP::Save(Context mod_context, DB_FP * fp)
{
  putSessionInfo(this, fp);
  R(this)->editor->Save(mod_context, fp);
  R(this)->view->Save(mod_context, fp);
}








/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void openSessionContext(EditorCP * edcp,
                        DB_FP * &session_fp)
{
  Context context;
  DB_FP *fp;
#if 0
  time_t sessionTime, contentsTime;
  Boolean existing, timeOK;
#endif
  Boolean formatOK;
  char * session_attr;
#if 0
  char * contents_attr;
#endif
  int oldest, newest, format;

  context = R(edcp)->mod_context;

  edcp->GetSessionAttribute(session_attr, oldest, newest);
#if 0
  edcp->GetContentsAttribute(contents_attr);
#endif

#if 0
  existing = true;

  /* see if the session info is out of date */
    if( existing )
      { 
	sessionTime  = annotModTimeStamp(context, session_attr);
	contentsTime = annotModTimeStamp(context, contents_attr);
        if (time_compare(contentsTime, sessionTime) > 0)
          timeOK = true;
        else
          timeOK = false;
      }

  /* see if the session info is in obsolete format */
    if( existing && timeOK )
      { 
#endif
	fp = context->GetExternalAttributeFile(session_attr, 1);
        if( fp != DB_NULLFP )
          { (void) db_buffered_read(fp, (char *) &format, sizeof(int));
            formatOK = BOOL(format >= oldest && format <= newest);
            if( ! formatOK ) context->CloseExternalAttributeFile(fp);
          }
        else
          formatOK = false;
#if 0
      }
#endif

  /* compute desired result */
#if 0
    session_fp       = (existing && timeOK && formatOK ? fp : DB_NULLFP);
#endif

    session_fp       = (formatOK ? fp : DB_NULLFP);
}




static
void getSessionInfo(EditorCP * edcp, DB_FP * fp)
{

  /* ASSERT: save-format int has already been read */

  if( fp == DB_NULLFP )
    { /* note that we must ask the contents when it's available */
      R(edcp)->contentsSize = Origin;
    }
  else
    { /* read info from database */
      (void) db_buffered_read(fp, (char *) &R(edcp)->minSize,      sizeof(Point));
      (void) db_buffered_read(fp, (char *) &R(edcp)->contentsSize, sizeof(Point));
    }
}




static
void putSessionInfo(EditorCP * edcp, DB_FP * fp)
{
  char * dummy_attr;
  int version, dummy_int;

  /* version int */
    edcp->GetSessionAttribute(dummy_attr, dummy_int, version);
    (void) db_buffered_write(fp, (char *) &version, sizeof(int));

  /* size preferences */
    (void) db_buffered_write(fp, (char *) &R(edcp)->minSize,      sizeof(Point));
    (void) db_buffered_write(fp, (char *) &R(edcp)->contentsSize, sizeof(Point));
}




static
void save(EditorCP * edcp, Context mod_context)
{
  char * session_attr;
  int dummy;
  DB_FP * fp;

  edcp->GetSessionAttribute(session_attr, dummy, dummy);
  fp =  mod_context->CreateExternalAttributeFile(session_attr);

  edcp->Save(mod_context, fp);

  if ( fp != DB_NULLFP ) mod_context->CloseExternalAttributeFile(fp);
}



static
void askAndSave(EditorCP *edcp, Boolean saveAs, Boolean saveCopy)
{
  Context mod_context = R(edcp)->mod_context;
  Boolean wantSave = true;   /* unless changed below */

  /* grab the name of the reference source file */
  char *oldPathName = ssave(mod_context->ReferenceFilePathName());
  char *newPathName = ssave(mod_context->ReferenceFilePathName());

  /* determine where to save */
  if( saveAs )
    wantSave = BOOL(file_select_dialog_run("Save Source", "save", &newPathName,
					   file_select_ok_to_write, 
					   (Generic) edcp));

  if( wantSave ) { /* ensure the specified object exists and can be modified */
    if (!file_ok_to_write(newPathName)) {
	message("Selected file is not writable.\n");
	wantSave = false;
      }
    }

  /* save if still wanted */
  if( wantSave ) {
    int pathsDiffer = strcmp(newPathName, oldPathName);
    if ( saveAs  && pathsDiffer ) {
      file_touch(newPathName); // file must exist for rename to succeed
      mod_context->Rename(newPathName);
    }
    
    save(edcp, mod_context);
    
    if( !saveCopy ) R(edcp)->dirty = false;
    else if ( saveAs  && pathsDiffer ) { 
      mod_context->Rename(oldPathName);
    }
    
    edcp->SetWindowTitle(edcp->GetWindowTitle());
  }

  /* free local storage */
  sfree(oldPathName);
  sfree(newPathName);
}



#if 0
static
void askAndSave(EditorCP * edcp, Boolean saveAs, Boolean saveCopy)
{
  Boolean     wantSave;
  Context     mod_context;
  char       *toptr;

  mod_context = R(edcp)->mod_context;

  wantSave = true;   /* unless changed below */

  /* grab the name of the reference source file */
  toptr = ssave(ctxAbsLocation(mod_context));

  /* determine where to save */
    if( saveAs )
	wantSave = BOOL(file_select_dialog_run("Save Source", "save", &toptr, 
					       file_select_ok_to_write, (Generic) edcp));

  if( wantSave )
    { /* ensure the specified object exists and can be modified */
      if (!file_ok_to_write(toptr))
      {
	message("Selected file is not writable.\n");
	wantSave = false;
      }
    }

  /* save if still wanted */
    if( wantSave )
      {
	/* create the unix file if it doesn't yet exist */
	file_touch(toptr);

	if ( !saveCopy )
	  ctxRename(mod_context, toptr);
	else
	  mod_context = ctxAlloc(ObjectFortSrc, toptr);

        save(edcp, mod_context);
        if( ! saveCopy )  
	{
	  R(edcp)->dirty = false;
	}
	else
	  ctxDealloc(mod_context);

	edcp->SetWindowTitle(edcp->GetWindowTitle());
      }

  /* free local storage */
     sfree(toptr);
}
#endif




