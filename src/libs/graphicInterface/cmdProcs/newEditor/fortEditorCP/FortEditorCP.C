/* $Id: FortEditorCP.C,v 1.3 2001/09/17 00:31:19 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditorCP/FortEditorCP.c				*/
/*									*/
/*	FortEditorCP -- Rn CP Definition for New Fortran Editor		*/
/*	Last edited: June 18, 1993 at 4:00 pm                           */
/*									*/
/************************************************************************/



#include <unistd.h>
#include <assert.h>
#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/scroll_sm.h>


#include <libs/graphicInterface/cmdProcs/newEditor/fortEditorCP/FortEditorCP.i>
#include <libs/graphicInterface/cmdProcs/newEditor/FortEditor.h>

#include <libs/support/msgHandlers/ErrorMsgHandler.h>
#include <libs/fileAttrMgmt/attributedFile/AttributedFile.h>
#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/FortTextTreeModAttr.h>

#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/FortVFilter.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditorCP/ViewDialog.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditorCP/SearchDialog.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditorCP/AnnotDialog.h>

#include <libs/graphicInterface/oldMonitor/include/sms/button_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>
#include <libs/graphicInterface/cmdProcs/help/help_cp.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/find.h>

#include <libs/support/file/UnixFile.h>
#include <libs/support/time/times.h>
#include <libs/support/database/context.h>

#include <libs/graphicInterface/oldMonitor/include/dialogs/filer.h>

#include <libs/graphicInterface/cmdProcs/newEditor/FortEditor.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditorCP/FortEditorCP_opt.i>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/desk_sm.h>

EXTERN(void, sc_Init,());
EXTERN(void, sc_Fini,());





/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/


#define  MAX_SRC_PANES      2




/* pane with scrollbars */

typedef struct
  {
    Generic                src;
    Generic                legend;
    ScrollBar              hscroll;
    ScrollBar              vscroll;
    Generic                wasted;

  }  ScrolledPane;



/* CP object */

typedef struct
  {
    /* creation parameters */
      Generic              cp_id;
      FortranModule       *context;   /* module  context must be specified */
      Composition         *pcontext;  /* program context if specified */
      FortTreeModAttr     *ftAttr;
      FortTextTreeModAttr *fttAttr;

    /* window parts */
      Point                size;
      float                splitFrac[MAX_SRC_PANES];

      Generic              window;
      Generic              buttonPane;
      ScrolledPane         pane[MAX_SRC_PANES];

    /* contents */
      FortEditor           editor;
      ViewDialog           viewer;
      SearchDialog         searcher;
      AnnotDialog          browser;
      Generic              annotator;

    /* status */
      FortVFilter          filter[MAX_SRC_PANES];
      Boolean              changed;
      int                  curPaneNum;
      Generic              curSrcPane;

  } edcp_Repr;


#define R(ob)		((edcp_Repr *) ob)


extern short ned_cp_index;


#define INFINITY	9999








#define LEGEND_WIDTH		150

static short edcp_legendFont;






/*************************/
/*  Menus		 */
/*************************/


/* main menu buttons */

static
char *edcp_buttons[] =
{
  "save!",
  "edit",
  "view",
  "search",
  "file"
};

#define SAVE_BUTTON		0
#define EDIT_BUTTON		1
#define VIEW_BUTTON		2
#define SEARCH_BUTTON		3
#define FILE_BUTTON		4

#define NUM_BUTTONS		5

static
Point edcp_numButtons = {NUM_BUTTONS, 1};




/* "edit" submenu */

#define ED_UNDO			0
#define ED_COPY			1
#define ED_CUT			2
#define ED_PASTE		3
#define ED_CLEAR		4
#define ED_EXPAND		5
#define ED_SEL_ALL		6
#define ED_MORE			7
#define ED_CHECK_LINE		8
#define ED_CHECK_MODULE		9

/* kb-only commands */
#define ED_CLEAR_TO_EOL		10


#define ED_UNDO_KEY		KB_top(1)
#define ED_COPY_KEY		KB_top(2)
#define ED_CUT_KEY		KB_top(3)
#define ED_PASTE_KEY		KB_top(4)
#define ED_EXPAND_KEY		KB_top(5)
#define ED_SEL_ALL_KEY		KB_top(6)
#define ED_MORE_KEY		KB_top(7)
#define ED_CHECK_LINE_KEY	toKbChar('!')


static
anOptionDef editOptions[] =
{
  { "undo",         (char *) 0 },
  { "copy",         (char *) 0 },
  { "cut",          (char *) 0 },
  { "paste",        (char *) 0 },
  { "clear",        (char *) 0 },
  { "expand...",    (char *) 0 },
  { "select all",   (char *) 0 },
  { "more",         (char *) 0 },
  { "check line",   (char *) 0 },
  { "check module", (char *) 0 },
};

static
aChoiceDef editChoices[] =
{
  { ED_UNDO,         ED_UNDO_KEY,       1, &editOptions[ED_UNDO]         },
  { ED_COPY,         ED_COPY_KEY,       1, &editOptions[ED_COPY]         },
  { ED_CUT,          ED_CUT_KEY,        1, &editOptions[ED_CUT]          },
  { ED_PASTE,        ED_PASTE_KEY,      1, &editOptions[ED_PASTE]        },
  { ED_CLEAR,        toKbChar(0),       1, &editOptions[ED_CLEAR]        },
  { ED_EXPAND,       ED_EXPAND_KEY,     1, &editOptions[ED_EXPAND]       },
  { ED_SEL_ALL,      ED_SEL_ALL_KEY,    1, &editOptions[ED_SEL_ALL]      },
  { ED_MORE,         ED_MORE_KEY,       1, &editOptions[ED_MORE]         },
  { ED_CHECK_LINE,   ED_CHECK_LINE_KEY, 1, &editOptions[ED_CHECK_LINE]   },
  { ED_CHECK_MODULE, toKbChar(0),       1, &editOptions[ED_CHECK_MODULE] },
};

static
aMenuDef edcp_editMenuDef =
{
  "edit",
  { 1, sizeof(editChoices) / sizeof(aChoiceDef) },
  UNUSED,
  editChoices
};

static aMenu *edcp_editMenu;




/* "view" submenu */

#define VIEW_DEFINE		0
#define VIEW_SPLIT		1
#define VIEW_SELECTION		2
#define VIEW_CONCEAL		3
#define VIEW_REVEAL		4
#define VIEW_REVEAL_ALL		5


#define VIEW_SELECTION_KEY	toKbChar('?' & 0x1f)	/* control-? */
#define VIEW_CONCEAL_KEY	KB_top(8)
#define VIEW_REVEAL_KEY		KB_top(9)


static
anOptionDef viewOptions[] =
{
  { "define filter...", (char *) 0 },
  { "split window",     (char *) 0 },
  { "show selection",   (char *) 0 },
  { "conceal",          (char *) 0 },
  { "reveal",           (char *) 0 },
  { "reveal all",       (char *) 0 },
};

static
aChoiceDef viewChoices[] =
{
  { VIEW_DEFINE,     toKbChar(0),        1, &viewOptions[VIEW_DEFINE]     },
  { VIEW_SPLIT,      toKbChar(0),        1, &viewOptions[VIEW_SPLIT]      },
  { VIEW_SELECTION,  VIEW_SELECTION_KEY, 1, &viewOptions[VIEW_SELECTION]  },
  { VIEW_CONCEAL,    VIEW_CONCEAL_KEY,   1, &viewOptions[VIEW_CONCEAL]    },
  { VIEW_REVEAL,     VIEW_REVEAL_KEY,    1, &viewOptions[VIEW_REVEAL]     },
  { VIEW_REVEAL_ALL, toKbChar(0),        1, &viewOptions[VIEW_REVEAL_ALL] },
};

static
aMenuDef edcp_viewMenuDef =
{
  "view",
  { 1, sizeof(viewChoices) / sizeof(aChoiceDef) },
  UNUSED,
  viewChoices
};

static aMenu *edcp_viewMenu;




/* "search" submenu */

#define SEARCH_FIND			0
#define SEARCH_FIND_NEXT		1
#define SEARCH_FIND_PREVIOUS		2
#define SEARCH_REPLACE			3
#define SEARCH_GENERAL_INFO		4
#define SEARCH_SELECTED_INFO		5


#define SEARCH_FIND_NEXT_KEY		KB_right(6)
#define SEARCH_FIND_PREVIOUS_KEY	KB_right(4)


static
anOptionDef searchOptions[] =
{
  { "find...",              (char *) 0 },
  { "find next",            (char *) 0 },
  { "find previous",        (char *) 0 },
  { "replace...",           (char *) 0 },
  { "general info...",      (char *) 0 },
  { "selected info...",     (char *) 0 },
};

static
aChoiceDef searchChoices[] =
{
  { SEARCH_FIND,                toKbChar(0),              1, &searchOptions[SEARCH_FIND]                },
  { SEARCH_FIND_NEXT,           SEARCH_FIND_NEXT_KEY,     1, &searchOptions[SEARCH_FIND_NEXT]           },
  { SEARCH_FIND_PREVIOUS,       SEARCH_FIND_PREVIOUS_KEY, 1, &searchOptions[SEARCH_FIND_PREVIOUS]       },
  { SEARCH_REPLACE,             toKbChar(0),              1, &searchOptions[SEARCH_REPLACE]             },
  { SEARCH_GENERAL_INFO,        toKbChar(0),              1, &searchOptions[SEARCH_GENERAL_INFO]        },
  { SEARCH_SELECTED_INFO,       toKbChar(0),              1, &searchOptions[SEARCH_SELECTED_INFO]       },	/* Sorry, Scott. --DGB */
};

static
aMenuDef edcp_searchMenuDef =
{
  "search",
  { 1, sizeof(searchChoices) / sizeof(aChoiceDef) },
  UNUSED,
  searchChoices
};

static aMenu *edcp_searchMenu;




/* "file" submenu */

#define FILE_NEW		0
#define FILE_EDIT		1
#define FILE_SAVEAS		2
#define FILE_SAVECOPY		3


static
anOptionDef fileOptions[] =
{
  { "new",         (char *) 0 },
  { "edit",        (char *) 0 },
  { "save as",     (char *) 0 },
  { "save a copy", (char *) 0 },
};

static
aChoiceDef fileChoices[] =
{
  { FILE_NEW,      toKbChar(0), 1, &fileOptions[FILE_NEW]      },
  { FILE_EDIT,     toKbChar(0), 1, &fileOptions[FILE_EDIT]     },
  { FILE_SAVEAS,   toKbChar(0), 1, &fileOptions[FILE_SAVEAS]   },
  { FILE_SAVECOPY, toKbChar(0), 1, &fileOptions[FILE_SAVECOPY] },
};

static
aMenuDef edcp_fileMenuDef =
{
  "file",
  { 1, sizeof(fileChoices) / sizeof(aChoiceDef) },
  UNUSED,
  fileChoices
};

static aMenu *edcp_fileMenu;






/*************************/
/*  Graphic appearance	 */
/*************************/


short edcp_srcFont;		/* font to use for Fortran */
short edcp_btnFont;		/* font to use for buttons */

Point edcp_buttonSize;		/* size in pixels of CP button pane */
Point edcp_minCharSize;		/* size in chars of minimum-size source pane */
Point edcp_defCharSize;		/* size in chars of default-size source pane */

Point edcp_minSize;		/* size in pixels of minimum-size CP window */
Point edcp_defSize;		/* size in pixels of default-size CP window */





/*************************/
/*  Miscellaneous	 */
/*************************/




/* startup structure */

typedef struct startup
  {
    FortranModule          *mcontext;
    Composition            *pcontext;
  } Startup;

static Startup edcp_defaultStartup;



/* help file names */

static char * edcp_helpfile = "nedcp.H";


/* saving session in database */

static char * edcp_sessionAttribute = "NedSession";

int    edcp_thisVersion = NED_SAVE_CURRENT;
int    edcp_saveVersion;			/* version of doc being opened */


/* 
 * Program context (after NED has checked it for existence, etc), Regrettably, this
 * seems to be the only way to get access to this information (i.e. a hack).
 */

extern Composition *checked_program_context;


/* initialization information */

static int    edcp_InitCount = 0;




/*************************/
/*  Forward declarations */
/*************************/


EXTERN(Boolean,  edcp_Start, (Generic cpm));    /* ped2 needs to init ned */
EXTERN(void,  edcp_Finish, (void));    /* ped2 needs to finish ned */

STATIC(Generic,  edcp_CreateInstance,(Generic parent_id, Generic cp_id, 
                                      Startup *startup));
STATIC(Generic,  createInstance,(Generic parent_id, Generic cp_id, 
                                 Startup *startup));
STATIC(Boolean,  edcp_HandleInput,(FortEditorCP edcp, Generic generator, 
                                   Generic event_type, Point info, char *msg));
STATIC(void,     edcp_DestroyInstance,(FortEditorCP edcp, Boolean panicked));
/* static */ void     edcp_Finish();    /* ped2 needs to fini ned */
STATIC(Point,	 windowSize,(Point charSize));
STATIC(void,     tileWindow,(FortEditorCP edcp, Point size, 
                             float splitFrac[MAX_SRC_PANES]));
STATIC(void, 	 titleWindow,(FortEditorCP edcp));
STATIC(void,	 help,(FortEditorCP edcp, Point pt));
STATIC(void,	 menuCommand,(FortEditorCP edcp, int button));
STATIC(void,	 editCommand,(FortEditorCP edcp, int choice));
STATIC(void,	 viewCommand,(FortEditorCP edcp, int choice));
STATIC(void,	 searchCommand,(FortEditorCP edcp, int choice));
STATIC(void,	 fileCommand,(FortEditorCP edcp, int choice));
STATIC(Boolean,	 keyCommand,(FortEditorCP edcp, KbChar kb));
STATIC(void,	 cursorCommand,(FortEditorCP edcp, KbChar kb));
STATIC(void,	 askAndSave,(FortEditorCP edcp, Boolean saveAs, 
                             Boolean saveCopy));
STATIC(void,	 save,(FortEditorCP edcp, Context context));
STATIC(void,	 setCurPane,(FortEditorCP edcp, int paneNum));
STATIC(void,	 setViewFilter,(FortEditorCP edcp, int paneNum));
STATIC(void,	 setLegend,(FortEditorCP edcp, int paneNum));
STATIC(void,	 markChanged,(FortEditorCP edcp));
STATIC(void,	 noteChange,(FortEditorCP edcp, int kind, Boolean autoScroll,
                             FortTreeNode node, int first, int last, int delta));
STATIC(void,	 annotGotoFunc,(FortEditorCP edcp));






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Command Processor	*/
/************************/


/* part of interface is exported as structures of "private" proc values */

aProcessor ned_Processor =
  {
    "new source editor",
    false,
    (Generic) &edcp_defaultStartup,
    (cp_root_starter_func) cp_standard_root_starter,
    (cp_start_cp_func) edcp_Start,
    (cp_create_instance_func) edcp_CreateInstance,
    (cp_handle_input_func) edcp_HandleInput,
    (cp_destroy_instance_func) edcp_DestroyInstance,
    (cp_finish_cp_func) edcp_Finish,
    CP_UNSTARTED
  };



int nedcp_Edit(int argc, char **argv)
{
  char *mod_name = ned_cp_module_name();
  char *pgm_name = ned_cp_program_name();

  Startup startup;

  Composition *comp = 0;
  if (pgm_name) {
    comp = new Composition;
    if (comp->Open(pgm_name) != 0 || 
	comp->IsCompleteAndConsistent() == false) {
      errorMsgHandler.HandleMsg
	("Not using program context %s, since it contains errors\n",
	 pgm_name);
      delete comp;
      comp = 0;
    } else {
      /* stash away into the global variable for use by ip browser */
      checked_program_context = comp;
    }
  }
  startup.pcontext = comp;
  
  if (comp) {
    OrderedSetOfStrings *names = comp->LookupModuleNameBySuffix(mod_name);
    int numNames = names->NumberOfEntries();
    switch(numNames) {
    case 0:
      errorMsgHandler.HandleMsg
	("Module %s not found in composition %s.\n", mod_name, pgm_name);
      delete names;
      return -1;
    case 1: 
      {
	char *filename = (*names)[0];
	if (!file_access(filename, R_OK)) {
	  errorMsgHandler.HandleMsg
	    ("Module %s is unreadable, or does not exist.\n", filename);
	  delete names;
	  return -1;
	}
	startup.mcontext = (FortranModule *) comp->GetModule(filename);
	delete names;
	assert(startup.mcontext != 0);
	break;
      }
    default: 
      {
	errorMsgHandler.HandleMsg
	  ("Module %s does not uniquely specify a member of composition %s.\n\
Members found: ", mod_name, pgm_name);
	for (; numNames--;) {
	  errorMsgHandler.HandleMsg("%s ", (*names)[numNames]);
	}
	delete names;
	return -1;
      }
    }
  } else {

    /* force the specified file to exist, if it doesn't already */

    if (!file_access(mod_name, R_OK)) { 
      // cannot read the file, can we create it?
	(void) file_touch(mod_name);
	if (!file_access(mod_name, R_OK)) {
	  errorMsgHandler.HandleMsg
	    ("Module %s is unreadable, or does not exist and cannot be created.\n", mod_name);
	    return -1;
	}
      }

    startup.mcontext = new FortranModule;
    if (startup.mcontext->Open(mod_name) != 0) {
      errorMsgHandler.HandleMsg
	("Errors encountered in creating context for %s.\n", mod_name);
      delete startup.mcontext; 
      return -1;
    }
  }

  if (cp_new((anInstance*)cp_root_cp_id(), ned_cp_index, (Generic) &startup) 
      == UNUSED)
  {
     errorMsgHandler.HandleMsg("Attempt to start nedcp on source failed.");  
    return -1;
  }
  else
    return 0;
}





/******************/
/*  View control  */
/******************/


void nedcp_EnsureSelVisible(FortEditorCP edcp)
{
  ed_ViewEnsureSelVisible(R(edcp)->editor, R(edcp)->curSrcPane);
}




void nedcp_WindowToTop(FortEditorCP edcp)
{
  sm_desk_win_top((Window *) R(edcp)->window);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/*ARGSUSED*/

/* static */                          /* ped2 needs to init ned */
Boolean edcp_Start(Generic cpm)
{
  if( edcp_InitCount++ == 0 )
    { /* initialize all submodules */
        ut_Init();
        sc_Init();
        ed_Init();

      /* initialize annotation sources */
        FortAnnotMgr_Init();
        ContentsSrc_Init();
        DeclSrc_Init();
        LoopsSrc_Init();
        InterProcSrc_Init();

        vdlg_Init();
        sdlg_Init();
        adlg_Init();

      /* initialize CP window appearance constants */
        edcp_srcFont = DEF_FONT_ID;
        edcp_btnFont = DEF_FONT_ID;
        edcp_legendFont = fontOpen("screen.7.rnf");

        edcp_buttonSize = sm_button_pane_size(edcp_numButtons, edcp_buttons,
                                          edcp_srcFont);
        edcp_minCharSize = makePoint(20, 4);
        edcp_defCharSize = makePoint(72, 20);

        edcp_minSize = windowSize(edcp_minCharSize);
        edcp_defSize = windowSize(edcp_defCharSize);

      /* create the menus */
        edcp_editMenu   = create_menu(&edcp_editMenuDef);
        edcp_viewMenu   = create_menu(&edcp_viewMenuDef);
        edcp_searchMenu = create_menu(&edcp_searchMenuDef);
        edcp_fileMenu   = create_menu(&edcp_fileMenuDef);
    }

  return true;
}


static
Generic edcp_CreateInstance(Generic parent_id, Generic cp_id, Startup *startup)
{
  return createInstance(parent_id, cp_id, startup);
}





/*ARGSUSED*/

static
Generic createInstance(Generic parent_id, Generic cp_id, Startup *startup)
{
  FortEditorCP edcp;
  FortranModule *context, *openingContext;
  Generic entity, type;
  DB_FP *fp;
  time_t sourceTime, sessionTime;
  Boolean gotSavedInfo;
  Point winSize;
  float splitFrac[MAX_SRC_PANES];
  Point scrollPos[MAX_SRC_PANES];
  char * filterName[MAX_SRC_PANES];
  int p;
  FortVFilter filter;
  
  /* allocate a new instance */
  edcp = (FortEditorCP) get_mem(sizeof(edcp_Repr),"FortEditorCP");
  if( (Generic) edcp == 0 ) return UNUSED;
  
  /* get our persistent information */
  gotSavedInfo = false;
  
  context = startup->mcontext;
  openingContext = startup->mcontext;

  fp = context->GetExternalAttributeFile(edcp_sessionAttribute, 1);

  /* if up-to-date editor session information exists */
  if( fp != DB_NULLFP )
    { (void) db_buffered_read(fp, (char *) &edcp_saveVersion, sizeof(int));
      if( edcp_saveVersion >= NED_SAVE_OLDEST &&
	  edcp_saveVersion <= NED_SAVE_CURRENT )
	{ gotSavedInfo = true;
	  (void) db_buffered_read(fp, (char *) &winSize,   sizeof(winSize));
	  (void) db_buffered_read(fp, (char *) splitFrac, sizeof(splitFrac));
	  (void) db_buffered_read(fp, (char *) scrollPos, sizeof(scrollPos));
	  for( p = 0;  p < MAX_SRC_PANES;  p++ )
	    filterName[p] = db_buffered_read_name(fp, "FortEditorCP: filter name");
	} else { /* we do not know how to read this version */
	  context->CloseExternalAttributeFile(fp);
	  fp = DB_NULLFP;
	}
    }

    if( ! gotSavedInfo )
      { /* use default values */
          winSize = edcp_defSize;
          scrollPos[0]  = Origin;
          scrollPos[1]  = Origin;
          filterName[0] = ssave("normal");
          filterName[1] = ssave("normal");
          splitFrac[0]  = 1.0;
          splitFrac[1]  = 0.0;
      }

  /* initialize the parts */
    /* set creation parameters */
      R(edcp)->cp_id        = cp_id;
      R(edcp)->context      = context;
      R(edcp)->pcontext     = startup->pcontext;
  
      R(edcp)->ftAttr = (FortTreeModAttr *) 
	context->AttachAttribute(CLASS_NAME(FortTreeModAttr));
      R(edcp)->fttAttr =  (FortTextTreeModAttr *)
	context->AttachAttribute(CLASS_NAME(FortTextTreeModAttr));


    /* create the subparts */
      R(edcp)->editor = ed_Open(openingContext, fp, R(edcp)->fttAttr->ftt, 
				R(edcp)->ftAttr->ft);
      ed_Notify(R(edcp)->editor, edcp, noteChange);

      R(edcp)->viewer    = vdlg_Open(openingContext, fp, edcp, R(edcp)->editor);
      R(edcp)->searcher  = sdlg_Open(openingContext, fp, edcp, R(edcp)->editor);
      R(edcp)->browser   = adlg_Open(openingContext, fp, edcp, R(edcp)->editor);
      R(edcp)->annotator = fam_Open (openingContext, fp, edcp, annotGotoFunc, R(edcp)->editor);

    /* initialize status */
      for( p = 0;  p < MAX_SRC_PANES;  p++ )
        { vdlg_GetFilterByName(R(edcp)->viewer,filterName[p],&filter);
          R(edcp)->filter[p]     = filter;
        }
      R(edcp)->changed = false;
      R(edcp)->curPaneNum = UNUSED;  /* forces initial inversion later */

    /* tile the window */
      R(edcp)->window = CP_WIN_RESIZABLE;
      R(edcp)->buttonPane  = sm_button_get_index();
      for( p = 0;  p < MAX_SRC_PANES;  p++ )
        { R(edcp)->pane[p].src     = ed_ViewScreenModuleIndex();
          R(edcp)->pane[p].legend  = sm_vanilla_get_index();
          R(edcp)->pane[p].hscroll = (ScrollBar) sm_scroll_get_index();
          R(edcp)->pane[p].vscroll = (ScrollBar) sm_scroll_get_index();
          R(edcp)->pane[p].wasted  = sm_vanilla_get_index();
        }
      tileWindow(edcp, winSize, splitFrac);

    /* initialize panes of the tiled window */
      titleWindow(edcp);
      sm_button_create_btns((Pane*)R(edcp)->buttonPane,
                          edcp_numButtons, edcp_buttons, edcp_btnFont, false);
      setCurPane(edcp,0);
      for( p = 0;  p < MAX_SRC_PANES;  p++ )
        { ed_ViewInit(R(edcp)->editor, R(edcp)->pane[p].src,
                      R(edcp)->filter[p], scrollPos[p], edcp_srcFont);
          setLegend(edcp,p);
          ed_ViewScrollBars(R(edcp)->editor, R(edcp)->pane[p].src,
                            R(edcp)->pane[p].hscroll, R(edcp)->pane[p].vscroll);
          ed_ViewSetSelectionBehavior(R(edcp)->editor, R(edcp)->pane[p].src,
                            ed_SB_NORMAL);
        }

  /* close the session attribute file descriptor if necessary */
  if( fp != DB_NULLFP ) context->CloseExternalAttributeFile(fp);
  
  for( p = 0;  p < MAX_SRC_PANES;  p++ ) 
    if (filterName[p]) sfree(filterName[p]);
	  
  return (Generic) edcp;
}




/*ARGSUSED*/

static
Boolean edcp_HandleInput(FortEditorCP edcp, Generic generator, Generic event_type, 
                         Point info, char *msg)
{
  Boolean kill = false;
  Boolean wantSave;
  Point oldSize;
  KbChar kb;
  Boolean handled;
  int p;

  /* moncontrol(1); doesn't work with sun 4.0.1 */

  switch( event_type )
    {
      case EVENT_MESSAGE:
        break;

      case EVENT_KILL:
        if( R(edcp)->changed )
         { if( yes_no("Save before quitting?", &wantSave, true) )
             { if( wantSave )
                 askAndSave(edcp,false,false);
               kill = true;
             }
          }
        else
          kill = true;
        break;

      case EVENT_RESIZE:
        oldSize = R(edcp)->size;
        tileWindow(edcp, info, R(edcp)->splitFrac);
        if( NOT(equalPoint(oldSize, R(edcp)->size)) )
        break;

      case EVENT_HELP:
        help(edcp,info);
        break;

      case EVENT_SELECT:
        if( generator == R(edcp)->buttonPane )
          menuCommand(edcp,info.x);
        else  /* a source pane */
          for( p = 0;  p < MAX_SRC_PANES;  p++ )
            if( generator == R(edcp)->pane[p].src )
              setCurPane(edcp,p);
            else if( generator == R(edcp)->pane[p].legend )
              setViewFilter(edcp,p);
        break;

      case EVENT_KEYBOARD:
        kb = toKbChar(info.x);
        handled = keyCommand(edcp,kb);
        if( ! handled )
          ed_Key(R(edcp)->editor, kb);
        break;

    }

  /* moncontrol(0); doesn't work with sun 4.0.1 */

  return kill;
}




/* ARGSUSED */

static
void edcp_DestroyInstance(FortEditorCP edcp, Boolean panicked)
{
  int p;

  /* close any windows associated with this one */
  help_cp_terminate_help(R(edcp)->cp_id);
  
  /* destroy the subparts */
  for( p = 0;  p < MAX_SRC_PANES;  p++ )
    ff_Close(R(edcp)->filter[p]);
  
  fam_Close(R(edcp)->annotator);
  adlg_Close(R(edcp)->browser);
  sdlg_Close(R(edcp)->searcher);
  vdlg_Close(R(edcp)->viewer);
  ed_Close(R(edcp)->editor);
  
  cp_window_destroy((Window*)R(edcp)->window, (anInstance*)R(edcp)->cp_id);

  R(edcp)->context->DetachAttribute(R(edcp)->ftAttr);
  R(edcp)->context->DetachAttribute(R(edcp)->fttAttr);

  if (R(edcp)->pcontext) delete R(edcp)->pcontext;
  else delete R(edcp)->context;

  free_mem((void*) edcp);
}



void edcp_Finish()
{
  if( --edcp_InitCount == 0 )
    { fontClose(edcp_legendFont);

      /* finalize all submodules */
        adlg_Fini();
        sdlg_Fini();
        vdlg_Fini();

      /* finalize annotation sources */
        InterProcSrc_Fini();
        LoopsSrc_Fini();
        DeclSrc_Fini();
        ContentsSrc_Fini();
        FortAnnotMgr_Fini();

        ed_Fini();

        sc_Fini();
        ut_Fini();

      /* destroy all global menus */
        destroy_menu(edcp_editMenu);
        destroy_menu(edcp_viewMenu);
        destroy_menu(edcp_searchMenu);
        destroy_menu(edcp_fileMenu);

      /* free our global storage */
    }
}




static
Point windowSize(Point charSize)
{
  int BORDERS = 1 + 1;
  Point size;

  size   = ed_ViewSize(charSize, edcp_srcFont);
  size.x = LEGEND_WIDTH + max(SB_MIN_LENGTH, size.x - LEGEND_WIDTH) +
               SB_WIDTH + BORDERS;
  size.y = max(SB_MIN_LENGTH,size.y) + edcp_buttonSize.y + SB_WIDTH + BORDERS;

  return size;
}




static
void tileWindow(FortEditorCP edcp, Point size, float splitFrac[MAX_SRC_PANES])
{
  Point buttonSize;
  Point srcSize[MAX_SRC_PANES];
  Point legendSize[MAX_SRC_PANES];
  Point hscrollSize[MAX_SRC_PANES];
  Point vscrollSize[MAX_SRC_PANES];
  Point wastedSize[MAX_SRC_PANES];
  Generic all_src,one_src;
  int totalHeight,p;

  /* enforce the minimum size */
    R(edcp)->size.x = max(size.x,edcp_minSize.x);
    R(edcp)->size.y = max(size.y,edcp_minSize.y);

    for( p = 0;  p < MAX_SRC_PANES;  p++ )
      R(edcp)->splitFrac[p] = splitFrac[p];

  /* calculate sizes of all the panes */
    /* the button pane */
      buttonSize.x = R(edcp)->size.x;
      buttonSize.y = edcp_buttonSize.y;

    totalHeight = R(edcp)->size.y - buttonSize.y;
    for( p = 0;  p < MAX_SRC_PANES;  p++ )
      if( splitFrac[p] < 0.1 )
        { srcSize[p]     = Origin;
          hscrollSize[p] = Origin;
          legendSize[p]  = Origin;
          vscrollSize[p] = Origin;
          wastedSize[p]  = Origin;
        }
      else
        { /* the source pane */
            srcSize[p].x = R(edcp)->size.x - SB_WIDTH;
            srcSize[p].y = (int) ((splitFrac[p] * totalHeight) - SB_WIDTH);

          /* the view legend */
            legendSize[p].x = LEGEND_WIDTH;
            legendSize[p].y = SB_WIDTH;

          /* the horizontal scrollbar */
            hscrollSize[p].x = srcSize[p].x - legendSize[p].x;
            hscrollSize[p].y = SB_WIDTH;

          /* the vertical scrollbar */
            vscrollSize[p].x = SB_WIDTH;
            vscrollSize[p].y = srcSize[p].y;

          /* the wasted bottom right corner */
            wastedSize[p].x = SB_WIDTH;
            wastedSize[p].y = SB_WIDTH;
        }

  /* tile the window */
    for( p = 0;  p < MAX_SRC_PANES;  p++ )
      { one_src =
          cp_td_join(
            TILE_UP,
            (aTilingDesc*)cp_td_join(
              TILE_LEFT,
              (aTilingDesc*)cp_td_pane((Pane**)&R(edcp)->pane[p].src,     srcSize[p]),
              (aTilingDesc*)cp_td_pane((Pane**)&R(edcp)->pane[p].vscroll, vscrollSize[p])
              ),
            (aTilingDesc*)cp_td_join(
              TILE_LEFT,
              (aTilingDesc*)cp_td_pane((Pane**)&R(edcp)->pane[p].legend, legendSize[p]),
              (aTilingDesc*)cp_td_join(
                TILE_LEFT,
                (aTilingDesc*)cp_td_pane((Pane**)&R(edcp)->pane[p].hscroll, hscrollSize[p]),
                (aTilingDesc*)cp_td_pane((Pane**)&R(edcp)->pane[p].wasted,  wastedSize[p])
                )
              )
            );
        if( p == 0 )
          all_src = one_src;
        else
          all_src = cp_td_join(TILE_UP,(aTilingDesc*)all_src,(aTilingDesc*)one_src);
      }

    cp_window_tile((Window**)&R(edcp)->window, (anInstance*)R(edcp)->cp_id,
                   (aTilingDesc*)cp_td_join(TILE_UP,
                              (aTilingDesc*)cp_td_pane((Pane**)&R(edcp)->buttonPane, buttonSize),
                              (aTilingDesc*)all_src
                             )
                  );
}




static
void titleWindow(FortEditorCP edcp)
{
  char *verb, *dirty;


  /* set the title  */
    verb  = "Editing";
    dirty = (R(edcp)->changed   ?  (char *)"* "  : (char *) "  ");
  
    cp_window_set_title((Window*)R(edcp)->window, "%s%s %s",
			dirty,verb,(R(edcp)->context)->ReferenceFilePathName());

}




/*ARGSUSED*/

static
void help(FortEditorCP edcp, Point pt)
{
  char * helpfile = edcp_helpfile;
  help_cp_give_help(R(edcp)->cp_id, helpfile, 0, (short *) 0, 0);
}




static
void menuCommand(FortEditorCP edcp, int button)
{
  int choice;
  Boolean isSplit;

  switch( button )
    {
      case EDIT_BUTTON:
	default_menu_choices(edcp_editMenu);
/************************* TEMPORARY FOR TESTING
	modify_menu_choice(edcp_editMenu, ED_UNDO, false, false);
*************************************************/ 
        choice = select_from_menu(edcp_editMenu, false);
        if( choice != UNUSED )  editCommand(edcp, choice);
        break;

      case SAVE_BUTTON:
        askAndSave(edcp,false,false);
        break;

      case VIEW_BUTTON:
	isSplit = NOT( R(edcp)->splitFrac[0] == 1.0  ||
                       R(edcp)->splitFrac[1] == 1.0 );
	default_menu_choices(edcp_viewMenu);
	modify_menu_choice(edcp_viewMenu, VIEW_SPLIT, isSplit, true);
        choice = select_from_menu(edcp_viewMenu, false);
        if( choice != UNUSED )  viewCommand(edcp, choice);
        break;

      case SEARCH_BUTTON:
	default_menu_choices(edcp_searchMenu);
        choice = select_from_menu(edcp_searchMenu, false);
        if( choice != UNUSED )  searchCommand(edcp, choice);
        break;
 
      case FILE_BUTTON:
	default_menu_choices(edcp_fileMenu);
	modify_menu_choice(edcp_fileMenu, FILE_NEW,    false, false);
	modify_menu_choice(edcp_fileMenu, FILE_EDIT,   false, false);
        choice = select_from_menu(edcp_fileMenu, false);
        if( choice != UNUSED )  fileCommand(edcp, choice);
        break;
    }
}




static
void editCommand(FortEditorCP edcp, int choice)
{
  FortEditor editor;
  FortTextTree ftt;	/* TEMPORARY FOR TESTING */
  FortTreeNode node;	/* TEMPORARY FOR TESTING */
  int id;		/* TEMPORARY FOR TESTING */

  editor = R(edcp)->editor;

  switch( choice )
    {
      case ED_UNDO:
        ed_GetTextTree(editor, &ftt);
        ed_GetSelectedNode(editor, &node);
        ftt_NodeToID(ftt, node, &id);
        ftt_IDToNode(ftt, id, &node);
/***************
        notImplemented("edit/undo");
****************/
        break;

      case ED_COPY:
        ed_Copy(editor, (FortScrap *) nil);
        break;

      case ED_CUT:
        ed_Cut(editor, (FortScrap *) nil);
        break;

      case ED_PASTE:
        ed_Paste(editor, nil);
        break;

      case ED_CLEAR:
        ed_Clear(editor);
        break;

      case ED_EXPAND:
        ed_Expand(editor, UNUSED);
        break;

      case ED_SEL_ALL:
        ed_SetSelection(editor,UNUSED,0,INFINITY);
        break;

      case ED_MORE:
        ed_MoreSelection(editor);
        break;

      case ED_CHECK_LINE:
        ed_CheckLine(R(edcp)->editor);
        break;

      case ED_CHECK_MODULE:
        if( ed_CheckModule(R(edcp)->editor) )
          (void) message("The module is correct.");
        else
          { (void) message("The module contains errors.");
            ff_SetShowErrors(R(edcp)->filter[R(edcp)->curPaneNum],true);
            setLegend(edcp,R(edcp)->curPaneNum);
          }
        break;

      case ED_CLEAR_TO_EOL:
        ed_ClearToEndOfLine(R(edcp)->editor);
        break;

    }
}




static
void viewCommand(FortEditorCP edcp, int choice)
{
  FortEditor editor;
  FortEditorView srcPane;
  int paneNum,p;
  Boolean changed;
  int line,sel1,sel2;
  Point ensurePt;

  editor = R(edcp)->editor;
  srcPane = R(edcp)->curSrcPane;
  paneNum = R(edcp)->curPaneNum;

  switch( choice )
    {
      case VIEW_DEFINE:
        changed = vdlg_Dialog(R(edcp)->viewer);
        if( changed )  markChanged(edcp);
        break;

      case VIEW_SPLIT:
        if( R(edcp)->splitFrac[0] == 1.0  ||  R(edcp)->splitFrac[1] == 1.0 )
          { R(edcp)->splitFrac[0] = 0.5;
            R(edcp)->splitFrac[1] = 0.5;
          }
        else
          { R(edcp)->splitFrac[paneNum    ] = 1.0;
            R(edcp)->splitFrac[1 - paneNum] = 0.0;
          }
        tileWindow(edcp,R(edcp)->size,R(edcp)->splitFrac);
        break;

      case VIEW_SELECTION:
        ed_GetSelection(editor, &line, &sel1, &sel2);
        if( line != UNUSED )
          ensurePt = makePoint(sel1-1,line);
        else
          ensurePt = makePoint(0,sel1);
        for( p = 0;  p < MAX_SRC_PANES;  p++ )
          ed_ViewEnsureVisible(editor,R(edcp)->pane[p].src,ensurePt);
        break;

      case VIEW_CONCEAL:
      case VIEW_REVEAL:
        ed_GetSelection(editor,&line,&sel1,&sel2);
        if( line != UNUSED )  sel1 = sel2 = line;
        ed_ViewSetConceal(editor,srcPane,sel1,sel2,BOOL(choice == VIEW_CONCEAL));
	markChanged(edcp);
        break;

      case VIEW_REVEAL_ALL:
        ed_GetSelection(editor,&line,&sel1,&sel2);
        if( line != UNUSED )  sel1 = sel2 = line;
        ed_ViewSetConcealNone(editor,srcPane,sel1,sel2);
        markChanged(edcp);
        break;
    }

  setLegend(edcp,paneNum);
}




static
void searchCommand(FortEditorCP edcp, int choice)
{
  int line, sel1, sel2, l1, c1, l2, c2;

  switch( choice )
    {
      case SEARCH_FIND:
        sdlg_Find(R(edcp)->searcher);
        break;

      case SEARCH_FIND_NEXT:
        sdlg_FindNext(R(edcp)->searcher);
        break;

      case SEARCH_FIND_PREVIOUS:
        sdlg_FindPrevious(R(edcp)->searcher);
        break;

      case SEARCH_REPLACE:
        sdlg_Replace(R(edcp)->searcher);
        break;

      case SEARCH_GENERAL_INFO:
        adlg_Dialog(R(edcp)->browser, fam_GetGlobal(R(edcp)->annotator));
        break;

      case SEARCH_SELECTED_INFO:
        ed_GetSelection(R(edcp)->editor, &line, &sel1, &sel2);
        if( line == UNUSED )
          { l1 = sel1;  c1 = UNUSED;
            l2 = sel2;  c2 = UNUSED;
          }
        else
          { l1 = line;  c1 = sel1;
            l2 = line;  c2 = sel2;
          }
        adlg_Dialog(R(edcp)->browser,
                    fam_GetSelection(R(edcp)->annotator, l1, c1, l2, c2));
        break;
    }
}




static
void fileCommand(FortEditorCP edcp, int choice)
{
  switch( choice )
    {
      case FILE_NEW:
        notImplemented("file/new");
        break;

      case FILE_EDIT:
        notImplemented("file/edit");
        break;

      case FILE_SAVEAS:
        askAndSave(edcp,true,false);
        break;

      case FILE_SAVECOPY:
        askAndSave(edcp,true,true);
        break;
    }
}




static
Boolean keyCommand(FortEditorCP edcp, KbChar kb)
{
  Boolean handled = true;    /* unless changed below */

  switch( kb )
    {
      case ED_UNDO_KEY:
        editCommand(edcp, ED_UNDO);
        break;

      case ED_COPY_KEY:
        editCommand(edcp, ED_COPY);
        break;

      case ED_CUT_KEY:
        editCommand(edcp, ED_CUT);
        break;

      case ED_PASTE_KEY:
        editCommand(edcp, ED_PASTE);
        break;

      case ED_EXPAND_KEY:
        editCommand(edcp, ED_EXPAND);
        break;

      case ED_SEL_ALL_KEY:
        editCommand(edcp, ED_SEL_ALL);
        break;

      case ED_MORE_KEY:
        editCommand(edcp, ED_MORE);
        break;

      case ED_CHECK_LINE_KEY:
        editCommand(edcp, ED_CHECK_LINE);
        break;

      case toKbChar('\013'):	/* ^K */
        /* erase to end of line */
        editCommand(edcp, ED_CLEAR_TO_EOL);
        break;

      case VIEW_CONCEAL_KEY:
        viewCommand(edcp, VIEW_CONCEAL);
        break;

      case VIEW_REVEAL_KEY:
        viewCommand(edcp, VIEW_REVEAL);
        break;

      case VIEW_SELECTION_KEY:
        viewCommand(edcp, VIEW_SELECTION);
        break;

      case toKbChar('\023'):	/* ^S */
      case SEARCH_FIND_NEXT_KEY:
        searchCommand(edcp,SEARCH_FIND_NEXT);
        break;

      case toKbChar('\022'):	/* ^R */
      case SEARCH_FIND_PREVIOUS_KEY:
        searchCommand(edcp,SEARCH_FIND_PREVIOUS);
        break;

      case KB_Tab:
        /*** ed_ToggleTypingField(R(edcp)->editor); ***/
        ed_FindPlaceholder(R(edcp)->editor, FRD_FORWARD);
        break;

      case KB_Linefeed:
        ed_FindPlaceholder(R(edcp)->editor, FRD_BACKWARD);
        break;

      case KB_ArrowU:
      case KB_ArrowD:
      case KB_ArrowL:
      case KB_ArrowR:
      case toKbChar('\001'):	/* ^A */
      case KB_right(1):
      case toKbChar('\005'):	/* ^E */
      case KB_right(3):
      case toKbChar('\026'):	/* ^V */
      case KB_right(15):
      case KB_right(13):
        cursorCommand(edcp, kb);
        break;

      default:
        handled = false;
        break;
    }

  return handled;
}




static
void cursorCommand(FortEditorCP edcp, KbChar kb)
{
  Point size;

  FortEditor ed = R(edcp)->editor;
  int selLine,sel1,sel2;

  ed_GetSelection(ed,&selLine,&sel1,&sel2);

  switch( kb )
    {
      case KB_ArrowU:
        if( selLine == UNUSED )
          { selLine = max(0,sel1 - 1);
            sel1 = ed_GetLineIndent(ed,selLine);
            sel2 = sel1 - 1;
          }
        else
          { selLine = max(0,selLine - 1);
            sel2 = sel1 - 1;
          }
        break;

      case KB_ArrowD:
        if( selLine == UNUSED )
          { selLine = sel2 + 1;
            sel1 = ed_GetLineIndent(ed,selLine);
            sel2 = sel1 - 1;
          } else { selLine += 1;
            sel1 = sel2 + 1;
          }
        break;

      case KB_ArrowL:
        if( selLine == UNUSED )
          { selLine = max(0,sel1 - 1);
            sel1 = ed_GetLineIndent(ed,selLine);
            sel2 = sel1 - 1;
          }
	else if( sel2  ==  sel1 - 1 )
	  { sel1 -= 1;
            sel2 -= 1;
          }
        else
          sel2 = sel1 - 1;
        break;

      case KB_ArrowR:
        if( selLine == UNUSED )
          { selLine = sel2 + 1;
            sel1 = ed_GetLineIndent(ed,selLine);
            sel2 = sel1 - 1;
          }
	else if( sel2  ==  sel1 - 1 )
	  { sel1 += 1;
            sel2 += 1;
          }
        else
          sel1 = sel2 + 1;
        break;

      case toKbChar('\001'):	/* ^A */
      case KB_right(1):
        /* cursor to beginning of line */
        if( selLine == UNUSED)  selLine = sel1;
        sel1 = ed_GetLineIndent(ed,selLine);
        sel2 = sel1 - 1;
        break;

      case toKbChar('\005'):	/* ^E */
      case KB_right(3):
        /* cursor to end of line */
        if( selLine == UNUSED )  selLine = sel2;
        sel1 = ed_GetLineLength(ed,selLine);
        sel2 = sel1 - 1;
        break;

      case toKbChar('\026'):	/* ^V */
      case KB_right(15):
        /* page forward */
        size = ed_ViewGetSize(ed,R(edcp)->curSrcPane);
        if( selLine == UNUSED )
          { selLine = sel2;
            sel1 = ed_GetLineIndent(ed,selLine);
            sel2 = sel1 - 1;
          }
        selLine += size.y;
        break;

      case KB_right(13):
        /* page backward */
        size = ed_ViewGetSize(ed,R(edcp)->curSrcPane);
        if( selLine == UNUSED )
          { selLine = sel1;
            sel1 = ed_GetLineIndent(ed,selLine);
            sel2 = sel1 - 1;
          }
        selLine -= size.y;
        break;

    }

  ed_SetSelection(ed,selLine,sel1,sel2);
  ed_ViewEnsureVisible(ed,R(edcp)->curSrcPane,
                       makePoint(sel2,selLine));
}




static
void askAndSave(FortEditorCP edcp, Boolean saveAs, Boolean saveCopy)
{
  Context context = R(edcp)->context;
  Boolean wantSave = true;   /* unless changed below */

  /* grab the name of the reference source file */
  char *oldPathName = ssave(context->ReferenceFilePathName());
  char *newPathName = ssave(context->ReferenceFilePathName());

  /* determine where to save */
  if( saveAs )
    wantSave = BOOL(file_select_dialog_run("Save Source", "save", &newPathName,
					   file_select_ok_to_write, edcp));

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
      context->Rename(newPathName);
    }
    
    save(edcp,context);
    
    if( !saveCopy ) R(edcp)->changed = false;
    else if ( saveAs  && pathsDiffer ) { 
      context->Rename(oldPathName);
    }
    
    titleWindow(edcp);
  }

  /* free local storage */
  sfree(oldPathName);
  sfree(newPathName);
}




/*ARGSUSED*/


static
void save(FortEditorCP edcp, Context context)
{
  DB_FP *fp;
  int p;
  float splitFrac[MAX_SRC_PANES];
  Point scrollPos[MAX_SRC_PANES];
  char * name;

  /* prepare the output bytestream */
  fp =  context->CreateExternalAttributeFile(edcp_sessionAttribute);

  /* save our persistent information */
    if( fp != DB_NULLFP )
      { /* identifying number for this save format */
          (void) db_buffered_write(fp, (char *) &edcp_thisVersion, sizeof(int));

        /* window size */
          (void) db_buffered_write(fp, (char *) &R(edcp)->size, sizeof(Point));

        /* split fractions */
          for( p = 0;  p < MAX_SRC_PANES;  p++ )
            splitFrac[p] = R(edcp)->splitFrac[p];
          (void) db_buffered_write(fp, (char *) splitFrac, sizeof(splitFrac));

        /* scroll positions */
          for( p = 0;  p < MAX_SRC_PANES;  p++ )
            scrollPos[p] = ed_ViewGetScroll(R(edcp)->editor,R(edcp)->pane[p].src);
          (void) db_buffered_write(fp, (char *) scrollPos, sizeof(scrollPos));

        /* view filters */
          for( p = 0;  p < MAX_SRC_PANES;  p++ )
            { name = ff_GetName(R(edcp)->filter[p],false);
              (void) db_buffered_write_name(fp, name);
            }
      }

  /* save the subparts */
    ed_Save(R(edcp)->editor,context,fp,true);
    vdlg_Save(R(edcp)->viewer,context,fp);
    sdlg_Save(R(edcp)->searcher,context,fp);
    fam_Save(R(edcp)->annotator,context,fp);
    adlg_Save(R(edcp)->browser,context,fp);

  /* close the output bytestream */
    if ( fp != DB_NULLFP )
      context->CloseExternalAttributeFile(fp);
}






static
void setCurPane(FortEditorCP edcp, int paneNum)
{
  int old = R(edcp)->curPaneNum;
  int newPane = paneNum;

  /* ASSERT:  'new != UNUSED' */

  if( old != UNUSED )  sm_vanilla_invert((Pane*)R(edcp)->pane[old].legend);

  R(edcp)->curPaneNum = newPane;
  R(edcp)->curSrcPane = R(edcp)->pane[newPane].src;

  sm_vanilla_invert((Pane*)R(edcp)->pane[newPane].legend);
}




static
void setViewFilter(FortEditorCP edcp, int paneNum)
{
  FortEditor editor = R(edcp)->editor;
  Generic pane = R(edcp)->pane[paneNum].src;
  FortVFilter old,newVfilter;
  Boolean changed;

  ed_ViewGetFilter(editor,pane,&old);
  newVfilter = old;
  changed = vdlg_Menu(R(edcp)->viewer,&newVfilter);

  if( changed )
    { if( newVfilter != old )
        { ed_ViewSetFilter(editor,pane,newVfilter);
          ff_Close(old);

          R(edcp)->filter[paneNum] = newVfilter;
        }
      setLegend(edcp,paneNum);
    }
}




static
void setLegend(FortEditorCP edcp, int paneNum)
{
  Generic legend = R(edcp)->pane[paneNum].legend;
  char * name = ff_GetName(R(edcp)->filter[paneNum],true);

  sm_vanilla_set_text((Pane*)legend,name,edcp_legendFont,STYLE_NORMAL,VSM_JUSTIFY_CENTER);
}




static
void markChanged(FortEditorCP edcp)
{
  if( ! R(edcp)->changed )
    { R(edcp)->changed = true;
      titleWindow(edcp);
    }
}




static
void noteChange(FortEditorCP edcp, int kind, Boolean autoScroll, 
                FortTreeNode node, int first, int last, int delta)
{
  switch( kind )
    {
      case NOTIFY_DOC_WILL_CHANGE:
        break;

      case NOTIFY_DOC_CHANGED:
      case NOTIFY_SEL_CHANGED:
        if( autoScroll )
          ed_ViewEnsureSelVisible(R(edcp)->editor, R(edcp)->curSrcPane);

        if( kind == NOTIFY_DOC_CHANGED )  markChanged(edcp);

        sdlg_NoteChange(R(edcp)->searcher,
                        kind, autoScroll, node, first, last, delta);

        adlg_NoteChange(R(edcp)->browser,
                        kind, autoScroll, node, first, last, delta);
    }
}




static
void annotGotoFunc(FortEditorCP edcp)
{
  nedcp_EnsureSelVisible(edcp);
  nedcp_WindowToTop(edcp);
}


