/* $Id: PEditorCP.C,v 1.43 1997/03/11 14:32:03 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ped_cp/PEditorCP/PEditorCP.c					*/
/*									*/
/*	PEditorCP -- Rn CP Definition for the Parallel Fortran Editor	*/
/*      Last edited: June 18, 1993 at 4:00 pm                           */
/*									*/
/************************************************************************/

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include <libs/support/file/File.h>

#include <libs/support/msgHandlers/ErrorMsgHandler.h>
#include <libs/fileAttrMgmt/attributedFile/AttributedFile.h>
#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/FortTextTreeModAttr.h>

#include <libs/graphicInterface/cmdProcs/newEditor/FortEditor.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/desk_sm.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pedCP/PedPrivate.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/frontEnd/ast/groups.h>
#include <libs/graphicInterface/oldMonitor/include/sms/list_sm.h>
#include <libs/support/strings/rn_string.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/symtab.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/perf.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_ipperf.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/mh_config.h>
#include <libs/support/file/UnixFile.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pedCP/PEditorCP_opt.i>

#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#include <libs/moduleAnalysis/dependence/loopInfo/li_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>

/*----------------------*/
/* forward declarations	*/

STATIC(void,     pedcp_RedrawSrcSink,(PEditorCP pedcp, AST_INDEX loop, AST_INDEX src,
                                      AST_INDEX sink, AST_INDEX prev_src,
                                      AST_INDEX prev_sink));
STATIC(FortEditor, pedcp_GetFortEditor,(PEditorCP pedcp));
STATIC(void,     pedcp_RedrawLoop,(PEditorCP pedcp, AST_INDEX loop));
STATIC(void,     depFortFilter,(Generic pedcp, Boolean countOnly, FortEditor editor,
                                int line, int *subline, TextString *text, FF_LineData
                                *data));
STATIC(void,     pedcp_ResetView,(PEditorCP pedcp));

STATIC(void,     fileCommand,(PEditorCP pedcp, int choice));
STATIC(void,     editCommand,(PEditorCP pedcp, int choice));
STATIC(void,     viewCommand,(PEditorCP pedcp, int choice));
STATIC(void,     searchCommand,(PEditorCP pedcp, int choice));
STATIC(void,     toolsCommand,(PEditorCP pedcp, int choice));
STATIC(void,     cursorCommand,(PEditorCP pedcp, KbChar kb));
STATIC(Boolean,  keyCommand,(PEditorCP pedcp, KbChar kb));

STATIC(void,     file_menu_run,(PEditorCP pedcp));
STATIC(void,     edit_menu_run,(PEditorCP pedcp));
STATIC(void,     view_menu_run,(PEditorCP pedcp));
STATIC(void,     search_menu_run,(PEditorCP pedcp));
STATIC(void,     tools_menu_run,(PEditorCP pedcp));
STATIC(void,     analyze_dialog_run,(PEditorCP pedcp));

STATIC(void, setViewFilter,(PEditorCP pedcp));
STATIC(void, save,(PEditorCP pedcp, Context context));
STATIC(Boolean, askAndSave,(PEditorCP pedcp, Boolean saveAs, Boolean saveCopy));

STATIC(void, annotGotoFunc,(Generic cp));

/*--------------------------*/
/* external declarations	*/

EXTERN(void,     sc_Init,());

extern int ped_link_dc;
extern int ped_link_perf;

/* 
 * Program context (after PED has checked it for existence, etc), Regrettably, this
 * seems to be the only way to get access to this information (i.e. a hack).
 */

extern Context checked_program_context;



/*------------------*/
/* local variables	*/

/*************************/
/*  Menus				 */
/*************************/


/* main menu buttons */

static
char *pedcp_button1[] =
{
  "file",
  "edit",
  "search",
  "analyze",
  "variables",
  "transform",
  "tools",
};

#define FILE_B		0
#define EDIT        1
#define SEARCH      2
#define ANALYZE 	3
#define VARS		4
#define TRANSFORM	5
#define TOOLS       6
#define NUM_BUTTON1	7

#define VIEW        3  /* not currently installed */

static
Point pedcp_numButton1 = {NUM_BUTTON1, 1};

/* dependence pane menu buttons */

static
char *pedcp_button2[] =
{
  "prev loop",
  "next loop",
  "prev dep",
  "next dep",
  "filter",
  "type",
  "delete",
};

#define PREV_LOOP	0
#define NEXT_LOOP	1
#define PREV_DEP	2
#define NEXT_DEP	3
#define FILTER		4
#define DEPTYPE		5
#define DELETE_DEP	6
#define NUM_BUTTON2	7

static
Point pedcp_numButton2 = {NUM_BUTTON2, 1};


/*************************/
/* "file" submenu		 */
/*************************/

#define FILE_SAVE		0
#define FILE_SAVEAS		1
#define FILE_SAVECOPY	2
/* #define FILE_EXPORT		3 */
#define FILE_NEW		3
#define FILE_EDIT		4

#define MAX_LINE_LENGTH		2047

static
anOptionDef fileOptions[] =
{
  { "save",        (char *) 0 },
  { "save as",     (char *) 0 },
  { "save a copy", (char *) 0 },
/*  { "export",      (char *) 0 },   NO MORE EXPORT, 9/19/91 HARV */
  { "new",         (char *) 0 },
  { "edit",        (char *) 0 },
};

static
aChoiceDef fileChoices[] =
{
  { FILE_SAVE,     toKbChar(0), 1, &fileOptions[FILE_SAVE]     },
  { FILE_SAVEAS,   toKbChar(0), 1, &fileOptions[FILE_SAVEAS]   },
  { FILE_SAVECOPY, toKbChar(0), 1, &fileOptions[FILE_SAVECOPY] },
/*  { FILE_EXPORT,   toKbChar(0), 1, &fileOptions[FILE_EXPORT]   }, */
/* 9/19/91 Harv */
  { FILE_NEW,      toKbChar(0), 1, &fileOptions[FILE_NEW]      },
  { FILE_EDIT,     toKbChar(0), 1, &fileOptions[FILE_EDIT]     },
};

static
aMenuDef pedcp_fileMenuDef =
{
  "file",
  { 1, sizeof(fileChoices) / sizeof(aChoiceDef) },
  UNUSED,
  fileChoices
};

static aMenu *pedcp_fileMenu;

/*************************/
/* "edit" submenu		 */
/*************************/

#define ED_UNDO			0
#define ED_COPY			1
#define ED_CUT			2
#define ED_PASTE		3
#define ED_CLEAR		4
#define ED_EXPAND		5
#define ED_SEL_ALL		6
#define ED_MORE			7
#define ED_ENABLE		8
#define ED_DISABLE		9
#define ED_CHECK_LINE		10
#define ED_CHECK_MODULE		11

/* kb-only commands */
#define ED_CLEAR_TO_EOL		12


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
  { "enable edits", (char *) 0 },
  { "disable edits", (char *) 0 },
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
  { ED_ENABLE,       toKbChar(0),       1, &editOptions[ED_ENABLE]       },
  { ED_DISABLE,      toKbChar(0),       1, &editOptions[ED_DISABLE]      },
  { ED_CHECK_LINE,   ED_CHECK_LINE_KEY, 1, &editOptions[ED_CHECK_LINE]   },
  { ED_CHECK_MODULE, toKbChar(0),       1, &editOptions[ED_CHECK_MODULE] },
};

static
aMenuDef pedcp_editMenuDef =
{
  "edit",
  { 1, sizeof(editChoices) / sizeof(aChoiceDef) },
  UNUSED,
  editChoices
};

static aMenu *pedcp_editMenu;

/*************************/
/* "view" submenu		 */
/*************************/

#define VIEW_DEFINE			0
#define VIEW_SPLIT			1
#define VIEW_SELECTION		2
#define VIEW_CONCEAL		3
#define VIEW_REVEAL			4
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
aMenuDef pedcp_viewMenuDef =
{
  "view",
  { 1, sizeof(viewChoices) / sizeof(aChoiceDef) },
  UNUSED,
  viewChoices
};

static aMenu *pedcp_viewMenu;


/*************************/
/* "search" submenu		 */
/*************************/

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
	{ SEARCH_FIND, toKbChar(0), 1,
		 &searchOptions[SEARCH_FIND]},

	{ SEARCH_FIND_NEXT, SEARCH_FIND_NEXT_KEY, 1, 
		&searchOptions[SEARCH_FIND_NEXT]},

	{ SEARCH_FIND_PREVIOUS, SEARCH_FIND_PREVIOUS_KEY, 1, 
		&searchOptions[SEARCH_FIND_PREVIOUS] },

	{ SEARCH_REPLACE, toKbChar(0), 1, 
		&searchOptions[SEARCH_REPLACE]},

	{ SEARCH_GENERAL_INFO, toKbChar(0), 1, 
		&searchOptions[SEARCH_GENERAL_INFO] },

	{ SEARCH_SELECTED_INFO, toKbChar(0), 1, 
		&searchOptions[SEARCH_SELECTED_INFO] },	/* Sorry, Scott. --DGB */
};

static
aMenuDef pedcp_searchMenuDef =
{
  "search",
  { 1, sizeof(searchChoices) / sizeof(aChoiceDef) },
  UNUSED,
  searchChoices
};

static aMenu *pedcp_searchMenu;


/*************************/
/* "tools" submenu		 */
/*************************/

#define TOOLS_FORTD_MIMD		0
#define TOOLS_MEMORY_OPT		1


static
anOptionDef toolsOptions[] =
{
  { "Fortran D MIMD Compiler",            (char *) 0 },
  { "Memory Optimizing Compiler",         (char *) 0 },
};

static
aChoiceDef toolsChoices[] =
{
	{ TOOLS_FORTD_MIMD, toKbChar(0), 1,
		 &toolsOptions[TOOLS_FORTD_MIMD]},

	{ TOOLS_MEMORY_OPT, toKbChar(0), 1, 
		&toolsOptions[TOOLS_MEMORY_OPT] },
};

static
aMenuDef pedcp_toolsMenuDef =
{
  "tools",
  { 1, sizeof(toolsChoices) / sizeof(aChoiceDef) },
  UNUSED,
  toolsChoices
};

static aMenu *pedcp_toolsMenu;


/*************************************************/
/* "performance estimation" submenu		 */
/*************************************************/

#define PERFEST_MENU_EXPRESS	0
#define PERFEST_MENU_SHRDMEM	1

static
anOptionDef perfestmenuOption[] =
{
  { "EXPRESS performance estimator",          (char *) 0 },
  { "Shared-memory performance estimator",    (char *) 0 },
};

static
aChoiceDef perfestmenuChoices[] =
{
	{ PERFEST_MENU_EXPRESS, toKbChar(0), 1,
		 &perfestmenuOption[PERFEST_MENU_EXPRESS]},

	{ PERFEST_MENU_SHRDMEM, toKbChar(0), 1,
		 &perfestmenuOption[PERFEST_MENU_SHRDMEM]},
};

static
aMenuDef pedcp_perfestMenuDef =
{
  "performance estimation",
  { 1, sizeof(perfestmenuChoices) / sizeof(aChoiceDef) },
  UNUSED,
  perfestmenuChoices
};

static aMenu *pedcp_perfestMenu;

/****************************************************************************
  Dependence View menu - 
	for selecting the part of the dependence graph for viewing 
 ****************************************************************************/

/*************************/
/*  Graphic appearance	 */
/*************************/

static short pedcp_srcFont;		/* font to use for Fortran */
static short pedcp_btnFont;		/* font to use for buttons */

static Point pedcp_button1Size;	/* size in pixels of CP button pane */
static Point pedcp_button2Size;	/* size in pixels of CP button pane */
static Point pedcp_minCharSize;	/* size in chars of minimum-size source pane */
static Point pedcp_defCharSize;	/* size in chars of default-size source pane */

static Point pedcp_depTitleSize;
static Point pedcp_depHeaderSize;

static Point pedcp_minSize;		/* size in pixels of minimum-size CP window */
static Point pedcp_defSize;		/* size in pixels of default-size CP window */

/*************************/
/*  Miscellaneous	 */
/*************************/

/* startup structure */

struct startup
  {
    FortranModule *mod_context;
    Composition   *pgm_context;
  };

static Startup pedcp_defaultStartup;
    
/* help file names */

static char * pedcp_helpfile = "pedcp.H";

/* saving session in database */

static char * pedcp_sessionAttribute = "NedSession";

/* initialization information */

static int    pedcp_InitCount = 0;

/*------------------*/
/* global variable	*/

/* part of interface is exported as structures of "private" proc values */

aProcessor      ped_Processor =
{
	"new source editor",
	false,
	(Generic) & pedcp_defaultStartup,
	(cp_root_starter_func)cp_standard_root_starter,
	pedcp_Start,
	(cp_create_instance_func)pedcp_CreateInstance,
	(cp_handle_input_func)pedcp_HandleInputDispatch,
	pedcp_DestroyInstance,
	pedcp_Finish,
	CP_UNSTARTED
};


int pedcp_Edit(int argc, char **argv)
{
  char *mod_name = ped_cp_module_name();
  char *pgm_name = ped_cp_program_name();
  
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
  startup.pgm_context = comp;
  
  if (comp) {
    OrderedSetOfStrings *names = comp->LookupModuleNameBySuffix(mod_name);
    int numNames = names->NumberOfEntries();
    switch(numNames) {
    case 0:
      errorMsgHandler.HandleMsg
	("Module %s not found in composition %s.\n", mod_name, pgm_name);
      delete names;
    case 1: 
      {
	char *filename = (*names)[0];
	if (!file_access(filename, R_OK)) {
	  errorMsgHandler.HandleMsg
	    ("Module %s is unreadable, or does not exist.\n", filename);
	  delete names;
	  return -1;
	}
	startup.mod_context = (FortranModule *) comp->GetModule(filename);
	delete names;
	assert(startup.mod_context != 0);
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
    
    /* force the specified file to exist, if it does not already */
    
    if (!file_access(mod_name, R_OK)) { 
      // cannot read the file, can we create it?
      (void) file_touch(mod_name);
      if (!file_access(mod_name, R_OK)) {
	errorMsgHandler.HandleMsg
	  ("Module %s is unreadable, or does not exist and cannot be created.\n", mod_name);
	return -1;
      }
    }
    
    startup.mod_context = new FortranModule;
    if (startup.mod_context->Open(mod_name) != 0) {
      errorMsgHandler.HandleMsg
	("Errors encountered in creating context for %s.\n", mod_name);
      delete startup.mod_context; 
      return -1;
    }
  }
  
  //----------------------------------------------------------------
  // while editting, we want explicit control over saves so that
  // values are not saved except at the request of the user
  //----------------------------------------------------------------
  startup.mod_context->DisableAttributeCaching();
  
  if (cp_new((anInstance*)cp_root_cp_id(), ped_cp_index, (Generic) &startup) 
      == UNUSED)
    {
      errorMsgHandler.HandleMsg("Attempt to start nedcp on source failed.");  
      return -1;
    }
  else
    return 0;
}


#if 0
int pedcp_Edit(int argc, char **argv)
{
  Startup startup;
  char *mod_name;
  char *pgm_name;

  mod_name = ped_cp_module_name();

  if (!file_access(mod_name, R_OK))
  {/* cannot read the file, can we create it? */
    (void) file_touch(mod_name);

    if (!file_access(mod_name, R_OK))
    {
      fprintf(stderr, "Module %s is unreadable, or does not exist and cannot be created.\n", mod_name);
      return -1;
    }
  }

  startup.mod_context = ctxAlloc(ObjectFortSrc, mod_name);
  if (startup.mod_context == CONTEXT_NULL)
  {
    fprintf(stderr, "Errors encountered in creating context for %s.\n", mod_name);
    return -1;
  }

  if (pgm_name = ped_cp_program_name())
    startup.pgm_context = ctxAlloc(ObjectFortComp, pgm_name);
  else
    startup.pgm_context = CONTEXT_NULL;

  if( cp_new((anInstance*)cp_root_cp_id(), ped_cp_index, (Generic) &startup) == UNUSED )
  {
    message("Attempt to start pedcp on source failed.");  
    return -1;
  }
  else
    return 0;
}
#endif

/*------------------*/
/* global functions */

/*---------------------------------------------------------------------

*/

/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/

/*======================================================================*/
/*	Allow application using PED to find out which Editor is used	*/
/*----------------------------------------------------------------------*/
FortEditor	ped_cp_get_FortEditor(PedInfo ped)
{
  return ped->GetFortEditor(PED_ED_HANDLE(ped));
}

/*------------------*/
/* local functions	*/

/*---------------------------------------------------------------------

*/

/*ARGSUSED*/
/*static*/
Boolean 
pedcp_Start(Generic cpm)
{
    if (!pedcp_InitCount++)
    {
	/* initialize all submodules */
	
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
	ff_Init();
	
	/* initialize CP window appearance constants */
	
	pedcp_srcFont = DEF_FONT_ID;
	pedcp_btnFont = DEF_FONT_ID;
	
	pedcp_button1Size = sm_button_pane_size(pedcp_numButton1,
						pedcp_button1,
						pedcp_srcFont);
	
	pedcp_button2Size = sm_button_pane_size(pedcp_numButton2,
						pedcp_button2,
						pedcp_srcFont);
	
	pedcp_depTitleSize  = sm_vanilla_pane_size("XXX", DEF_FONT_ID);
	pedcp_depHeaderSize = sm_vanilla_pane_size("XXX", DEF_FONT_ID);
	
	/* set startup & mininum Ped window size */
	
	pedcp_minCharSize = makePoint(20, 14);
	pedcp_defCharSize = makePoint(84, 30);
	
	pedcp_minSize = windowSize(pedcp_minCharSize);
	pedcp_defSize = windowSize(pedcp_defCharSize);
	
	/* create the menus */
	
	pedcp_editMenu   = create_menu(&pedcp_editMenuDef);
	/* pedcp_viewMenu   = create_menu(&pedcp_viewMenuDef); */
	pedcp_searchMenu = create_menu(&pedcp_searchMenuDef);
	pedcp_fileMenu   = create_menu(&pedcp_fileMenuDef);
	pedcp_toolsMenu   = create_menu(&pedcp_toolsMenuDef);
	pedcp_perfestMenu   = create_menu(&pedcp_perfestMenuDef);
    }
    
    return true;
}


/*---------------------------------------------------------------------

*/

/*static*/
Generic 
pedcp_CreateInstance(Generic parent_id, Generic cp_id, Startup *startup)
{
    return createInstance(parent_id, cp_id, startup);
}

/*---------------------------------------------------------------------

*/

/*ARGSUSED*/
/*static*/
Generic 
createInstance(Generic parent_id, Generic cp_id, Startup *startup)
{
    PEditorCP       pedcp;
    FortranModule *         context            = CONTEXT_NULL;
    Composition *         pgm_context        = CONTEXT_NULL;
    FortranModule *        mod_in_pgm_context = CONTEXT_NULL;

    Generic         entity, type;
    DB_FP          *fp;
    Point           winSize, scrollPos;
    FortTreeNode    node;
    FortVFilterSpec filter_temp;
    FortTextTree    ftt;
    FortTree        ft;
    Boolean         has_errors;

    /* allocate a new instance */
    
    pedcp = (PEditorCP) get_mem(sizeof(pedcp_Repr), "PEditorCP");
    if (!pedcp)
	return UNUSED;
    
    /* default values */
    
    winSize = pedcp_defSize;
    scrollPos = Origin;

    context = startup->mod_context;
    pgm_context = startup->pgm_context;
    
    /* stash away into the global variable for use by ip browser */
    checked_program_context = pgm_context;

    /* set creation parameters */
    
    R(pedcp)->cp_id = cp_id;
    R(pedcp)->mod_context =  context;
    R(pedcp)->pgm_context =  pgm_context;

    if (pgm_context == CONTEXT_NULL)
      mod_in_pgm_context = CONTEXT_NULL;
    else{
      mod_in_pgm_context = new FortranModule;
      int code = 
	mod_in_pgm_context->Open(R(pedcp)->mod_context->ReferenceFilePathName(),
				 pgm_context);
      assert(code == 0);
    }

    R(pedcp)->mod_in_pgm_context = (FortranModule *) mod_in_pgm_context;

    /* apparantly always NULL -- why isn't session info used? */
    fp = DB_NULLFP;

    /* create the subparts */
  
    R(pedcp)->ftAttr = (FortTreeModAttr *) 
      context->AttachAttribute(CLASS_NAME(FortTreeModAttr));
    R(pedcp)->fttAttr =  (FortTextTreeModAttr *)
      context->AttachAttribute(CLASS_NAME(FortTextTreeModAttr));

    
    R(pedcp)->editor = ed_Open(context, fp, R(pedcp)->fttAttr->ftt, 
				R(pedcp)->ftAttr->ft);
    ed_Notify(R(pedcp)->editor, pedcp, pedcp_NoteChanges);
    
    R(pedcp)->viewer    = vdlg_Open(context, fp, pedcp, R(pedcp)->editor);
    R(pedcp)->searcher  = sdlg_Open(context, fp, pedcp, R(pedcp)->editor);
    R(pedcp)->annotator = fam_Open (context, fp, pedcp, annotGotoFunc, R(pedcp)->editor);
    R(pedcp)->browser   = adlg_Open(context, fp, pedcp, R(pedcp)->editor);
    
    /* initialize status */
    
    R(pedcp)->changed = false;

    /* get the ftt, ft, and check for errors */

    ed_GetTextTree(R(pedcp)->editor, &ftt);	/* get handles to the tree */
    ed_GetTree(R(pedcp)->editor, &ft);
    ft_AstSelect(ft);

    /* check the module to see if it contains errors */
    has_errors = BOOL(R(pedcp)->ftAttr->GetCheckedState() != ft_CORRECT);

    /* setup view filtering */
    filter_temp = ffs_Open(CONTEXT_NULL, DB_NULLFP, R(pedcp)->editor);
    ffs_Customize(filter_temp, pedcp, depFortFilter);
    R(pedcp)->filter = ff_Open(CONTEXT_NULL, DB_NULLFP,
			       R(pedcp)->editor, filter_temp);
    /*
      vdlg_GetFilterByName(R(pedcp)->viewer, 
      ssave("Ped filter"), &R(pedcp)->filter);
      */
    
    /*-----------------------------*/
    /* get/create dependence graph */
    
    /* for now, pass NULL program context and call graph in interactive sessions.
     * eventually we should be able to invoke ped from the composition editor with
     * a program context and callgraph -- JMC 29 April 1992
     */
    R(pedcp)->ped = pedInitialize(pedcp, ftt, ft, 
				  R(pedcp)->mod_context,       /* for info specific to just
								 the module */
				  R(pedcp)->mod_in_pgm_context,/* for info specific to the module
								 and the program */
				  R(pedcp)->pgm_context,      /* for info specific to the program
								 and the program */
				  has_errors,
				  ped_cp_input_flag()       /* get input flag for memory analysis */);
    
    /* tile the window */
    
    R(pedcp)->window = CP_WIN_RESIZABLE;
    R(pedcp)->button1Pane = sm_button_get_index();
    R(pedcp)->button2Pane = sm_button_get_index();
    R(pedcp)->srcPane = ed_ViewScreenModuleIndex();
    R(pedcp)->hscrollPane = (ScrollBar) ((int) sm_scroll_get_index());
    R(pedcp)->vscrollPane = (ScrollBar) ((int) sm_scroll_get_index());
    R(pedcp)->wastedPane = sm_vanilla_get_index();
    
    R(pedcp)->depTitlePane = sm_vanilla_get_index();
    R(pedcp)->depPane = sm_list_get_index();
    R(pedcp)->depHeaderPane = sm_vanilla_get_index();
    
    tileWindow(pedcp, winSize);
    
    /* initialize panes of the tiled window */
    
    titleWindow(pedcp);
    
    sm_button_create_btns((Pane*)R(pedcp)->button1Pane,
			  pedcp_numButton1, pedcp_button1, pedcp_btnFont, false);
    
    sm_button_create_btns((Pane*)R(pedcp)->button2Pane,
			  pedcp_numButton2, pedcp_button2, pedcp_btnFont, false);
    
    ed_ViewInit(R(pedcp)->editor, R(pedcp)->srcPane,
		R(pedcp)->filter, scrollPos, pedcp_srcFont);
    
    ed_ViewScrollBars(R(pedcp)->editor, R(pedcp)->srcPane,
		      R(pedcp)->hscrollPane, R(pedcp)->vscrollPane);
    
    ed_GetSelectedNode(R(pedcp)->editor, &node);
    
    pedRegister(
		(PedInfo) R(pedcp)->ped,
		R(pedcp)->depTitlePane,
		R(pedcp)->depHeaderPane,
		R(pedcp)->depPane,
		R(pedcp)->depSize,
		pedcp_GetFortEditor,
		pedcp_TreeChanged,
		pedcp_TreeWillChange,
		pedcp_UpdateNodeInfo,
		pedcp_GetLine,
		pedcp_SelectNode,
		pedcp_RedrawSrcSink,
		pedcp_RedrawLoop);
    
    /* close the attribute file descriptor */
    
    if (fp != DB_NULLFP)
    {
	(void) db_buffered_close(fp);
	fp = DB_NULLFP;
    }
    
    pedUpdate((PedInfo) R(pedcp)->ped, node);
    
    /* set global dependence handle for performance estimator	*/
    /* Vas of course ... who else! */
    
    Global_Dep_Ptr = (PedInfo) R(pedcp)->ped;
    
    R(pedcp)->code_type = IBM;
    
    /* create the modeless dialogs */
    
    pedcp_create_dialogs(pedcp);

    return (Generic) pedcp;
}


/*---------------------------------------------------------------------

*/

/* ARGSUSED */
/*static*/ void
pedcp_DestroyInstance(PEditorCP pedcp, Boolean panicked)
{
    FortTree        ft;
    PedInfo			ped = (PedInfo) R(pedcp)->ped;
    
    /* close any windows associated with this one */
    
    help_cp_terminate_help(R(pedcp)->cp_id);
    
    /* free the side arrays use the default table */
    
    ed_GetTree(R(pedcp)->editor, &ft);
    ft_AstSelect(ft);
    pedFinalize(ped);
    
    /* destroy all dialogs associated with this window */
    
    pedcp_destroy_dialogs(pedcp);
    
    /* destroy the subparts */
    
    fam_Close(R(pedcp)->annotator);
    adlg_Close(R(pedcp)->browser);
    sdlg_Close(R(pedcp)->searcher);
    vdlg_Close(R(pedcp)->viewer);
    ed_Close(R(pedcp)->editor);
    ff_Close(R(pedcp)->filter);
    
    /* scrollbars and src view are destroyed implicitly */
    
    cp_window_destroy((Window*)R(pedcp)->window, (anInstance*)R(pedcp)->cp_id);
    free_mem((void*) pedcp);
}

/*---------------------------------------------------------------------

*/

/*static*/ void
pedcp_Finish()
{
    if (!--pedcp_InitCount)
    {			/* finalize all submodules */
	ff_Fini();

        /* finalize annotation sources */
        InterProcSrc_Fini();
        LoopsSrc_Fini();
        DeclSrc_Fini();
        ContentsSrc_Fini();
        FortAnnotMgr_Fini();

        adlg_Fini();
        sdlg_Fini();
        vdlg_Fini();
	ed_Fini();
	sc_Fini();
	ut_Fini();
	
	/* destroy all global menus */
	destroy_menu(pedcp_editMenu);
	/* destroy_menu(pedcp_viewMenu); */
	destroy_menu(pedcp_searchMenu);
	destroy_menu(pedcp_fileMenu);
	destroy_menu(pedcp_toolsMenu);
    }
}

/*---------------------------------------------------------------------

*/

Generic
pedRegister(PedInfo ped,
	    Generic dep_title_pane,
	    Generic dep_header_pane,
	    Generic dep_pane,
	    Point dep_size,
	    ped_GetFortEditorFunc GetFortEditor,
	    ped_TreeChangedFunc TreeChanged,
	    ped_TreeWillChangeFunc TreeWillChange,
	    ped_UpdateNodeInfoFunc UpdateNodeInfo,
	    ped_GetLineFunc GetLine,
	    ped_SelectNodeFunc SelectNode,
	    ped_RedrawSrcSinkFunc redrawSrcSink,
	    ped_RedrawLoopFunc redrawLoop)
    
{
    PED_DEP_PANE(ped) = dep_pane;
    PED_DEP_TITLE_PANE(ped) = dep_title_pane;
    PED_DEP_HEADER_PANE(ped) = dep_header_pane;
    PED_DEP_SIZE(ped) = dep_size;
    ped->GetFortEditor = GetFortEditor;
    ped->TreeChanged = TreeChanged;
    ped->TreeWillChange = TreeWillChange;
    ped->UpdateNodeInfo = UpdateNodeInfo;
    ped->GetLine = GetLine;
    ped->SelectNode = SelectNode;
    ped->RedrawSrcSink = redrawSrcSink;
    ped->RedrawLoop = redrawLoop;
    
    PED_CURRENT_DEPENDENCE(ped) = -1;
    PED_STACK_DEPTH(ped) = 0;
    
    PED_PREV_SRC(ped) = AST_NIL;
    PED_PREV_SINK(ped) = AST_NIL;
    
    PED_SELECTION(ped) = AST_NIL;
    PED_SELECTED_LOOP(ped) = AST_NIL;
    
    PED_EL_STACK(ped) = stack_create(sizeof(Generic));
    stack_push(PED_EL_STACK(ped), (Generic *) & PED_EL(ped));
    
	depDispInit(ped);

    return (Generic) ped;
}

/*---------------------------------------------------------------------

	pedRoot()		Reset ped Root & Ftt fields

*/

void
pedRoot(PedInfo ped)
{
    FortEditor      editor = ped->GetFortEditor(PED_ED_HANDLE(ped));
    FortTextTree    ftt;
    FortTree        ft;
    
    ed_GetTextTree(editor, &ftt);
    ed_GetTree(editor, &ft);
    ft_AstSelect(ft);
    
    PED_FTT(ped)  = (Generic) ftt;
    PED_FT(ped)   = (Generic) ft;
    PED_ROOT(ped) = ft_Root(ft);
}

/*---------------------------------------------------------------------

	pedLine()		Return current line number selected

*/

int
pedLine(PedInfo ped)
{
    int line, sel1, sel2;
    
    ed_GetSelection(R(PED_ED_HANDLE(ped))->editor, 
		    &line, &sel1, &sel2);
    
    return line == UNUSED ? sel1 : line;
}


/******************************/
/*  Top level input handlers  */
/******************************/

/*---------------------------------------------------------------------

*/

/*ARGSUSED*/
/*static*/
Boolean 
pedcp_HandleInputDispatch(PEditorCP pedcp, Generic generator, 
                          Generic event_type, Point info, char *msg)
{
    if (generator == R(pedcp)->window ||
	generator == R(pedcp)->button1Pane ||
	generator == R(pedcp)->button2Pane ||
	generator == R(pedcp)->srcPane)
	return pedcp_HandleInput(pedcp, generator, event_type, info, msg);
    
    else if (generator == R(pedcp)->depTitlePane ||
	     generator == R(pedcp)->depPane ||
	     generator == R(pedcp)->depHeaderPane)
	return pedcp_DepHandleInput(pedcp, generator, event_type, info, msg);
    
    else
	return true;	/* I'm confused.  So kill me. */
}

/*---------------------------------------------------------------------

*/

/*ARGSUSED*/
/*static*/
Boolean 
pedcp_HandleInput(PEditorCP pedcp, Generic generator, 
                  Generic event_type, Point info, char *msg)
{
    Point           oldSize;
    AST_INDEX       choice;
    PedInfo         ped;
    Boolean			ok;
    KbChar          kb;

    ped = (PedInfo) R(pedcp)->ped;
    
    switch (event_type)
    {
    case EVENT_MESSAGE:
	break;
	
    case EVENT_KILL:
	if (NOT(PED_EDIT_SAVED(ped)) || R(pedcp)->changed)
	{
	    (void) yes_no("Save edits before quitting?", &ok, true);
	    if (ok && NOT(askAndSave(pedcp,false,false)))
	    {
		(void) yes_no("Program not saved\nQuit anyway?", &ok, false);
		if (NOT(ok))
		    return false;		/* don't kill Ped	*/
	    }
	}

	return true; 	/* kill Ped	 */
	
    case EVENT_RESIZE:
	oldSize = R(pedcp)->size;
	tileWindow(pedcp, info);
	
	/* Don't consider resize as change for Ped	*/
	/*
	  if (NOT(equalPoint(oldSize, R(pedcp)->size)))
	  R(pedcp)->changed = true;
	  */
	
	break;
	
    case EVENT_HELP:
	help(pedcp, generator, info);
	break;
	
    case EVENT_SELECT:
	
	if (generator == R(pedcp)->button1Pane)
	    button1Command(pedcp, info.x);
	
	else if (generator == R(pedcp)->button2Pane)
	    button2Command(pedcp, info.x);
	
	else		/* srcPane */
	{
	    choice = R(pedcp)->th.selection;
	    ed_ViewMouse(R(pedcp)->editor, R(pedcp)->srcPane, info);
	    
	    if (PED_SELECTED_LOOP(ped) != choice)
		dialogUpdate(pedcp);
	}
	break;
	
    case EVENT_KEYBOARD:
	
	if ((generator == R(pedcp)->button1Pane) ||
	    (generator == R(pedcp)->button2Pane))
	    break;
	
	/*----------------------------------------------*/
	/* check whether arbitrary edits are enabled	 */
	
	if (NOT(PED_EDIT_ENABLED(ped)))
	{
	    message("Edits not enabled");
	}
	else
	{
	    if (!R(pedcp)->changed)
	      pedcp_markChanged(pedcp);		/* mark edit		 */

	    if (NOT(PED_EDIT_PERFORMED(ped)))	/* first edit		 */
	    {
		pedcp_ResetView(pedcp);		/* turn off displays	 */
		PED_EDIT_PERFORMED(ped) = true;	/* remember edit	 */
		PED_EDIT_SAVED(ped) = false;	/* edits not saved yet	 */
	    }
	    
	    kb = toKbChar(info.x);		/* handle input		 */
	    
	    if (NOT(keyCommand(pedcp, kb)))
		ed_Key(R(pedcp)->editor, kb);
	}
	
	break;
    }
    
    return false;		/* don't kill Ped	 */
}

/*---------------------------------------------------------------------

*/

/*ARGSUSED*/
/*static*/
Boolean 
pedcp_DepHandleInput(PEditorCP pedcp, Generic generator, 
                     Generic event_type, Point info, char *msg)
{
    PedInfo     ped;
    
    switch (event_type)
    {
    case EVENT_MOVE:
	break;
	
    case EVENT_SELECT:
	if (generator == R(pedcp)->depPane)
	{
	    ped = (PedInfo) R(pedcp)->ped;
	    
	    /* don't look at dependences if editing program	 */
	    
	    if (PED_EDIT_PERFORMED(ped))
		message("Reanalyze program\nDependences out of date");
	    else
		pedDepSelect(ped, info.y);
	    
	    break;
	}
	
    case EVENT_KEYBOARD:
	break;
    }
    
    return false;
}


/*********************/
/*  Button handlers  */
/*********************/

/*---------------------------------------------------------------------

*/

/*static*/ void
button1Command(PEditorCP pedcp, int button)
{
    SharDia        *sh = &(R(pedcp)->sh);
    PedInfo         ped;
    
    ped = (PedInfo) R(pedcp)->ped;
    
    switch (button)
    {
    case FILE_B:
	file_menu_run(pedcp);
	break;
	
    case VARS:
	if (NOT(PED_EDIT_PERFORMED(ped)))
	    shared_dialog_run(sh);
	else
	    message("No variable info\nwhile editing");
	break;
	
    case EDIT:
	edit_menu_run(pedcp);
	break;
	
    case SEARCH:
	search_menu_run(pedcp);
	break;
	
    case TRANSFORM:
	if (NOT(PED_EDIT_PERFORMED(ped)))
	    transform_menu(&(R(pedcp)->th));
	else
	    message("No transformations\nwhile editing");
	break;
	
    case ANALYZE:
	analyze_dialog_run(pedcp);
	dialogUpdate(pedcp);
	break;
	
    case TOOLS:
	tools_menu_run(pedcp);
	dialogUpdate(pedcp);
	break;
    }
}


/*---------------------------------------------------------------------

*/

/*static*/ void
button2Command(PEditorCP pedcp, int button)
{
    AST_INDEX       choice;
    PedInfo         ped;
    Boolean         ok;
    
    ped = (PedInfo) (R(pedcp)->ped);
    
    /* don't look at dependences if editing program	 */
    
    if (PED_EDIT_PERFORMED(ped))
    {
	message("Reanalyze program\nDependences out of date");
	return;
    }
    
    choice = R(pedcp)->th.selection;
    
    switch (button)
    {
    case PREV_LOOP:
	pedPrevLoop(ped);
	if (PED_SELECTED_LOOP(ped) != choice)
	    dialogUpdate(pedcp);
	break;
	
    case NEXT_LOOP:
	pedNextLoop(ped);
	if (PED_SELECTED_LOOP(ped) != choice)
	    dialogUpdate(pedcp);
	break;
	
    case PREV_DEP:
	pedPrevDep(ped);
	break;
	
    case NEXT_DEP:
	pedNextDep(ped);
	break;
	
    case FILTER:
	if (PED_SELECTED_LOOP(ped) != AST_NIL)
	{
	    R(pedcp)->eh.EL = PED_EL(ped);
	    edge_dialog_run(&(R(pedcp)->eh));
	    
	}
	break;
	
    case DEPTYPE:
	dep_type_dialog_run (ped);
	forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped));
	break;
	
    case DELETE_DEP:
	yes_no("Confirm deletion\nof dependence:", &ok, false);
	if (ok)
	    pedDelete(pedcp, ped);
	break;
    }
}


/***********************/
/*  Keyboard handlers  */
/***********************/

/*---------------------------------------------------------------------

*/

static void
cursorCommand(PEditorCP pedcp, KbChar kb)
{
    Point           size;
    
    FortEditor      ed = R(pedcp)->editor;
    int             selLine, sel1, sel2;
    
    ed_GetSelection(ed, &selLine, &sel1, &sel2);
    
    switch (kb)
    {
    case KB_ArrowU:
	if (selLine == UNUSED)
	{
	    selLine = max(0, sel1 - 1);
	    sel1 = ed_GetLineIndent(ed, selLine);
	    sel2 = sel1 - 1;
	}
	else
	{
	    selLine = max(0, selLine - 1);
	    sel2 = sel1 - 1;
	}
	break;
	
    case KB_ArrowD:
	if (selLine == UNUSED)
	{
	    selLine = sel2 + 1;
	    sel1 = ed_GetLineIndent(ed, selLine);
	    sel2 = sel1 - 1;
	}
	else
	{
	    selLine += 1;
	    sel1 = sel2 + 1;
	}
	break;
	
    case KB_ArrowL:
	if (selLine == UNUSED)
	{
	    selLine = max(0, sel1 - 1);
	    sel1 = ed_GetLineIndent(ed, selLine);
	    sel2 = sel1 - 1;
	}
	else if (sel2 == sel1 - 1)
	{
	    sel1 -= 1;
	    sel2 -= 1;
	}
	else
	    sel2 = sel1 - 1;
	break;
	
    case KB_ArrowR:
	if (selLine == UNUSED)
	{
	    selLine = sel2 + 1;
	    sel1 = ed_GetLineIndent(ed, selLine);
	    sel2 = sel1 - 1;
	}
	else if (sel2 == sel1 - 1)
	{
	    sel1 += 1;
	    sel2 += 1;
	}
	else
	    sel1 = sel2 + 1;
	break;
	
    case toKbChar('\001'):	/* ^A */
    case KB_right(1):
	/* cursor to beginning of line */
	if (selLine == UNUSED)
	    selLine = sel1;
	sel1 = ed_GetLineIndent(ed, selLine);
	sel2 = sel1 - 1;
	break;
	
    case toKbChar('\005'):	/* ^E */
    case KB_right(3):
	/* cursor to end of line */
	if (selLine == UNUSED)
	    selLine = sel2;
	sel1 = ed_GetLineLength(ed, selLine);
	sel2 = sel1 - 1;
	break;
	
    case toKbChar('\026'):	/* ^V */
    case KB_right(15):
	/* page forward */
	size = ed_ViewGetSize(ed, R(pedcp)->srcPane);
	if (selLine == UNUSED)
	{
	    selLine = sel2;
	    sel1 = ed_GetLineIndent(ed, selLine);
	    sel2 = sel1 - 1;
	}
	selLine += size.y;
	break;
	
    case KB_right(13):
	/* page backward */
	size = ed_ViewGetSize(ed, R(pedcp)->srcPane);
	if (selLine == UNUSED)
	{
	    selLine = sel1;
	    sel1 = ed_GetLineIndent(ed, selLine);
	    sel2 = sel1 - 1;
	}
	selLine -= size.y;
	break;
	
    }
    ed_SetSelection(ed, selLine, sel1, sel2);
    ed_ViewEnsureVisible(ed, R(pedcp)->srcPane, makePoint(sel2, selLine));
}

/*---------------------------------------------------------------------

*/

static
Boolean 
keyCommand(PEditorCP pedcp, KbChar kb)
{
    switch (kb)
    {
    case ED_UNDO_KEY:
	editCommand(pedcp, ED_UNDO);
	break;
	
    case ED_COPY_KEY:
	editCommand(pedcp, ED_COPY);
	break;
	
    case ED_CUT_KEY:
	editCommand(pedcp, ED_CUT);
	break;
	
    case ED_PASTE_KEY:
	editCommand(pedcp, ED_PASTE);
	break;
	
    case ED_EXPAND_KEY:
	editCommand(pedcp, ED_EXPAND);
	break;
	
    case ED_SEL_ALL_KEY:
	editCommand(pedcp, ED_SEL_ALL);
	break;
	
    case ED_MORE_KEY:
	editCommand(pedcp, ED_MORE);
	break;
	
    case ED_CHECK_LINE_KEY:
	editCommand(pedcp, ED_CHECK_LINE);
	break;
	
    case toKbChar('\013'):	/* ^K */
	/* erase to end of line */
	editCommand(pedcp, ED_CLEAR_TO_EOL);
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
	cursorCommand(pedcp, kb);
	break;
	
    default:
	return false;	/* not handled here	 */
    }
    
    return true;		/* handled here	 */
}


/*******************************/
/*  Menu displaying functions  */
/*******************************/

/*---------------------------------------------------------------------

*/

static void
edit_menu_run(PEditorCP pedcp)
{
    PedInfo         ped;
    int             choice;
    
    ped = (PedInfo) (R(pedcp)->ped);
    default_menu_choices(pedcp_editMenu);
    
    if (PED_EDIT_ENABLED(ped))
    {
	modify_menu_choice(pedcp_editMenu, ED_UNDO,         false, false);
	modify_menu_choice(pedcp_editMenu, ED_COPY,         false, true);
	modify_menu_choice(pedcp_editMenu, ED_CUT,          false, true);
	modify_menu_choice(pedcp_editMenu, ED_PASTE,        false, true);
	modify_menu_choice(pedcp_editMenu, ED_CLEAR,        false, true);
	modify_menu_choice(pedcp_editMenu, ED_EXPAND,       false, true);
	modify_menu_choice(pedcp_editMenu, ED_SEL_ALL,      false, true);
	modify_menu_choice(pedcp_editMenu, ED_MORE,         false, true);
	modify_menu_choice(pedcp_editMenu, ED_CHECK_LINE,   false, true);
	modify_menu_choice(pedcp_editMenu, ED_CHECK_MODULE, false, true);
	modify_menu_choice(pedcp_editMenu, ED_ENABLE,	    false, false);
	modify_menu_choice(pedcp_editMenu, ED_DISABLE,	    false, true);
    }
    else
    {
	modify_menu_choice(pedcp_editMenu, ED_UNDO,         false, false);
	modify_menu_choice(pedcp_editMenu, ED_COPY,         false, false);
	modify_menu_choice(pedcp_editMenu, ED_CUT,          false, false);
	modify_menu_choice(pedcp_editMenu, ED_PASTE,        false, false);
	modify_menu_choice(pedcp_editMenu, ED_CLEAR,        false, false);
	modify_menu_choice(pedcp_editMenu, ED_EXPAND,       false, false);
	modify_menu_choice(pedcp_editMenu, ED_SEL_ALL,      false, true);
	modify_menu_choice(pedcp_editMenu, ED_MORE,         false, true);
	modify_menu_choice(pedcp_editMenu, ED_CHECK_LINE,   false, true);
	modify_menu_choice(pedcp_editMenu, ED_CHECK_MODULE, false, true);
	modify_menu_choice(pedcp_editMenu, ED_ENABLE,	    false, true);
	modify_menu_choice(pedcp_editMenu, ED_DISABLE,	    false, false);
    }
    
    choice = select_from_menu(pedcp_editMenu, false);
    if (choice != UNUSED)
	editCommand(pedcp, choice);
}

/*---------------------------------------------------------------------

*/

static void
file_menu_run(PEditorCP pedcp)
{
    int             choice;
    
    default_menu_choices(pedcp_fileMenu);
    
    modify_menu_choice(pedcp_fileMenu, FILE_NEW,     false, false);
    modify_menu_choice(pedcp_fileMenu, FILE_EDIT,    false, false);
    
    choice = select_from_menu(pedcp_fileMenu, false);
    if (choice != UNUSED)
	fileCommand(pedcp, choice);
}

/*---------------------------------------------------------------------

*/

static void
search_menu_run(PEditorCP pedcp)
{
    int             choice;
    
    default_menu_choices(pedcp_searchMenu);
    choice = select_from_menu(pedcp_searchMenu, false);
    if (choice != UNUSED)
	searchCommand(pedcp, choice);
}


/*---------------------------------------------------------------------

*/

static void
analyze_dialog_run(PEditorCP pedcp)
{
    PedInfo         ped;
    Boolean         ok;
    AST_INDEX       node;
    FortTree	    ft;
    
    ped = (PedInfo) (R(pedcp)->ped);
    show_message2("Analyzing program");
    
    /*----------------------------------------------------------*/
    /* first make sure that program is syntactically correct	*/
    
    if (!ed_CheckModule(R(pedcp)->editor))
    {
	hide_message2();
	
	message("The module contains errors\nAnalysis not possible");
	ff_SetShowErrors(R(pedcp)->filter, true);
	return;
    }
    
    /*----------------------------------*/
    /* if correct, then analyze program	 */
    
    PED_EDIT_PERFORMED(ped) = false;	/* reset flag			*/

    ed_GetTree(R(pedcp)->editor, &ft);
    ft_AstSelect(ft);    
    pedReinitialize(ped); /*, ft);*/		/* rebuild internals	*/
       /*pedReinitialize takes 1 arg only*/
    

    hide_message2();
    
    /*--------------------------------------*/
    /* now find & store new selection point	*/
    
    ed_GetSelectedNode(R(pedcp)->editor, &node);
    PED_SELECTION(ped) = node;
    forcePedUpdate(ped, is_loop(node) ? node : AST_NIL, node);
}


/*---------------------------------------------------------------------

*/

static void
tools_menu_run(PEditorCP pedcp)
{
    int             choice;
    
	default_menu_choices(pedcp_toolsMenu); 
	choice = select_from_menu(pedcp_toolsMenu, false);
	if (choice != UNUSED)
		toolsCommand(pedcp, choice);
}



/**********************************/
/*  Command handlers for menus    */
/**********************************/

/*---------------------------------------------------------------------

*/

static void 
fileCommand(PEditorCP pedcp, int choice)
{
    switch (choice)
    {
    case FILE_SAVE:
	(void) askAndSave(pedcp, false, false);
	break;
	
    case FILE_SAVEAS:
	(void) askAndSave(pedcp, true, false);
	break;
	
    case FILE_SAVECOPY:
	(void) askAndSave(pedcp, true, true);
	break;
	
    case FILE_NEW:
	break;
	
    case FILE_EDIT:
	break;
	
    }
}

/*---------------------------------------------------------------------

*/

static void 
editCommand(PEditorCP pedcp, int choice)
{
    Boolean         edited;
    FortEditor      editor = R(pedcp)->editor;
    PedInfo         ped = (PedInfo) (R(pedcp)->ped);
    
    edited = true;
    switch (choice)
    {
    case ED_COPY:
	ed_Copy(editor, (FortScrap *) 0);
	break;
	
    case ED_CUT:
	ed_Cut(editor, (FortScrap *) 0);
	break;
	
    case ED_PASTE:
	ed_Paste(editor, 0);
	break;
	
    case ED_CLEAR:
	ed_Clear(editor);
	break;
	
    case ED_EXPAND:
	ed_Expand(editor, UNUSED);
	break;
	
    case ED_SEL_ALL:
	edited = false;
	ed_SetSelection(editor, UNUSED, 0, 9999);
	break;
	
    case ED_MORE:
	edited = false;
	ed_MoreSelection(editor);
	break;
	
    case ED_CHECK_LINE:
	edited = false;
	ed_CheckLine(R(pedcp)->editor);
	break;
	
    case ED_CHECK_MODULE:
	edited = false;
	if (ed_CheckModule(R(pedcp)->editor))
	    message("The module is correct.");
	else
	{
	    message("The module contains errors.");
	    ff_SetShowErrors(R(pedcp)->filter, true);
	}
	break;
	
    case ED_CLEAR_TO_EOL:
	ed_ClearToEndOfLine(R(pedcp)->editor);
	break;
	
    case ED_ENABLE:
	edited = false;
	PED_EDIT_ENABLED(ped) = true;			/* enable edits		 */
	break;
	
    case ED_DISABLE:
	edited = false;
	PED_EDIT_ENABLED(ped) = false;			/* disable edits	 */
	break;
    }
    
    if (edited)
    {
	if (NOT(PED_EDIT_PERFORMED(ped)))		/* first edit			 */
	{
	    pedcp_ResetView(pedcp);			/* turn off displays	 */
	    PED_EDIT_PERFORMED(ped) = true;		/* remember edit		 */
	}
    }

}

/*---------------------------------------------------------------------

*/

static void 
viewCommand(PEditorCP pedcp, int choice)
{
    FortEditor      editor;
    FortEditorView  srcPane;
    Boolean         changed;
    int             line, sel1, sel2;
    Point           ensurePt;
    
    editor = R(pedcp)->editor;
    srcPane = R(pedcp)->srcPane;
    
    switch (choice)
    {
    case VIEW_DEFINE:
	changed = vdlg_Dialog(R(pedcp)->viewer);
	if (changed)
	    pedcp_markChanged(pedcp);
	break;
	
    case VIEW_SPLIT:
	notImplemented("view/split");
	break;
	
    case VIEW_SELECTION:
	ed_GetSelection(editor, &line, &sel1, &sel2);
	if (line != UNUSED)
	    ensurePt = makePoint(sel1 - 1, line);
	else
	    ensurePt = makePoint(0, sel1);
	ed_ViewEnsureVisible(editor, srcPane, ensurePt);
	break;
	
    case VIEW_CONCEAL:
    case VIEW_REVEAL:
	ed_GetSelection(editor, &line, &sel1, &sel2);
	if (line != UNUSED)
	    sel1 = sel2 = line;
	ed_ViewSetConceal(editor, srcPane, sel1, sel2, 
			  BOOL(choice == VIEW_CONCEAL));
	pedcp_markChanged(pedcp);
	break;
	
    case VIEW_REVEAL_ALL:
	ed_GetSelection(editor, &line, &sel1, &sel2);
	if (line != UNUSED)
	    sel1 = sel2 = line;
	ed_ViewSetConcealNone(editor, srcPane, sel1, sel2);
	pedcp_markChanged(pedcp);
	break;
    }
}

/*---------------------------------------------------------------------

*/

static void 
searchCommand(PEditorCP pedcp, int choice)
{
  int line, sel1, sel2;
  int l1, c1, l2, c2;

    switch (choice)
    {
    case SEARCH_FIND:
	sdlg_Find(R(pedcp)->searcher);
	break;
	
    case SEARCH_FIND_NEXT:
	sdlg_FindNext(R(pedcp)->searcher);
	break;
	
    case SEARCH_FIND_PREVIOUS:
	sdlg_FindPrevious(R(pedcp)->searcher);
	break;
	
    case SEARCH_REPLACE:
	sdlg_Replace(R(pedcp)->searcher);
	break;
	
    case SEARCH_GENERAL_INFO:
        adlg_Dialog(R(pedcp)->browser, fam_GetGlobal(R(pedcp)->annotator));
	break;
	
    case SEARCH_SELECTED_INFO:
        ed_GetSelection(R(pedcp)->editor, &line, &sel1, &sel2);
        if( line == UNUSED )
          { l1 = sel1;  c1 = UNUSED;
            l2 = sel2;  c2 = UNUSED;
          }
        else
          { l1 = line;  c1 = sel1;
            l2 = line;  c2 = sel2;
          }
        adlg_Dialog(R(pedcp)->browser,
                    fam_GetSelection(R(pedcp)->annotator, l1, c1, l2, c2));
	break;
    }
}

/*---------------------------------------------------------------------

*/

static void 
toolsCommand(PEditorCP pedcp, int choice)
{
#if 0
	PedInfo ped = (PedInfo) R(pedcp)->ped;

	switch (choice)
	{
	case TOOLS_FORTD_MIMD:
		if (!ped_link_dc)
			message("Fortran D compiler stubbed off");
		else if (!ed_CheckModule(R(pedcp)->editor))
		{
			message("The module contains errors\nCompilation not possible");	
			ff_SetShowErrors(R(pedcp)->filter, true);
		}
		else
		{
			show_message2("The Fortran D compiler for MIMD distributed\nmemory machines is in progress");
			dist_compiler(ped);
			hide_message2();
		}
		break;

	case TOOLS_MEMORY_OPT:
		/* ensure input_dependences are included */
		if (NOT(dg_get_input_dependences(PED_DG(ped))))
		  {
		   dg_set_input_dependences(PED_DG(ped),true);
		   analyze_dialog_run(pedcp);
		   dialogUpdate(pedcp);
		  }

		/* no config file read */
		if (!PED_MH_CONFIG(ped))
		  {
		   PED_MH_CONFIG(ped) = (int) get_mem(sizeof(config_type), 
                                                      "toolsCommand");
		   mh_get_config((config_type *)PED_MH_CONFIG(ped),NULL);
		  }
		ed_GetTree(R(pedcp)->editor,(FortTree*)&(R(pedcp)->mm.ft));
		ft_AstSelect((FortTree)R(pedcp)->mm.ft);
		memory_menu(&(R(pedcp)->mm));
		break;

	}
#endif
}

/*********************************/
/*  Screen appearance functions  */
/*********************************/


/*---------------------------------------------------------------------

*/

static
Boolean lineCheck (int val, int lower, int upper)
{
    if (upper == -1)
	return false;
    else if (val == lower)
	return true;
    else
	return false;
}

/*---------------------------------------------------------------------

	depFortFilter()	- redraws lines in the text display

	If a loop is selected and "line" is the loop header, the filter
	emphasizes the loop. If the "line" is the previous loop header, the
	filter deemphasizes the header.  If the line contains the src or sink
	of a selected dependence, the filter emphasizes those.  Otherwise the
	filter leaves the line as is.

*/

static void 
depFortFilter(Generic pedcp, Boolean countOnly, FortEditor editor, 
              int line, int *subline, TextString *text, FF_LineData *data)
{
    PedInfo         ped;
    int             edge;
    DG_Edge        *ea;
    int             bracket;
    FortTreeNode    node;
    FortTextTree    ftt;
    AST_INDEX       src;
    AST_INDEX       prev_src;
    AST_INDEX       src_subscript;
    AST_INDEX       sink;
    AST_INDEX       prev_sink;
    AST_INDEX       sink_subscript;
    int             line1;
    int             char1;
    int             line2;
    int             char2;
    
    int             i;	/* for loop index */
    
    if (countOnly)
    {
	*subline = 1;
	return;
    }
    
    ped = (PedInfo) R(pedcp)->ped;
    
    if (PED_EDIT_PERFORMED(ped))	/* no filtering	 */
	return;					/* if editing	 */
    
    if (PED_SELECTED_LOOP(ped) <= AST_NIL)	/* if a loop is not selected */
	return;
    
    ed_GetTextTree(editor, &ftt);
    ftt_GetLineInfo(ftt, line, &node, &bracket);
    
    /* emphasize the loop header */
    
    if (PED_SELECTED_LOOP(ped) == (AST_INDEX) node)
    {
	for (i = 0; (i < text->num_tc); i++)
	    text->tc_ptr[i].style = STYLE_ITALIC;
    }
    
    /* clear the old loop header */
    
    else if (PED_PREV_LOOP(ped) == (AST_INDEX) node)
    {
	for (i = 0; (i < text->num_tc); i++)
	    text->tc_ptr[i].style = STYLE_NORMAL;
    }
    
    /* The following is an else if assuming that the src
     * and sink of a dependence are never carried by a
     * loop header - note this might need to be made into
     * an if instead of an else if when region selection
     * is possible because a line to redraw might contain
     * a loop header and a variable which is the source
     * or sink of a dependence.  */
    
    else if ((PED_SELECTED_LOOP(ped) > AST_NIL) && (PED_CURRENT_DEPENDENCE(ped) >= 0))
    {
	ea = dg_get_edge_structure( PED_DG(ped));
	edge = get_dependence( PED_EL(ped), PED_CURRENT_DEPENDENCE(ped));
	
	if (edge < 0)	/* not a valid edge */
	    return;
	
	src = ea[edge].src;
	sink = ea[edge].sink;
	
	/*----------------------------------------------*/
	/* calculate src & sink subscripts on the fly	 */
	
	src_subscript = ((src > AST_NIL) &&
			 ((src_subscript = tree_out(src)) > AST_NIL) &&
			 (is_subscript(src_subscript))) ?
			     gen_SUBSCRIPT_get_rvalue_LIST(src_subscript) : AST_NIL;
	
	sink_subscript = ((sink > AST_NIL) &&
			  ((sink_subscript = tree_out(sink)) > AST_NIL) &&
			  (is_subscript(sink_subscript))) ?
			      gen_SUBSCRIPT_get_rvalue_LIST(sink_subscript) : AST_NIL;
	
	/* deemphasize the previous src and sink */
	
	prev_src = PED_PREV_SRC(ped);
	prev_sink = PED_PREV_SINK(ped);
	
	if ((prev_src == prev_sink) && (prev_src != AST_NIL))
	{
	    ftt_NodeToText(ftt, prev_src, &line1, 
			   &char1, &line2, &char2);
	    
	    if (lineCheck(line, line1, line2)) 
	    {
		/* found the line of the source */
		
		if (line1 != line2)
		{
		    char1 = ftt_GetLineIndent (ftt, line);
		    char2 = ftt_GetLineLength (ftt, line);
		}
		for (i = char1; ((i <= char2) && (i >= 0)); i++)
		{
		    text->tc_ptr[i].style = STYLE_NORMAL;
		}
	    }
	}
	
	if (prev_src != prev_sink)
	{
	    if (prev_src != AST_NIL)
	    {
		ftt_NodeToText(ftt, prev_src, &line1, 
			       &char1, &line2, &char2);
		
		if (lineCheck(line, line1, line2)) 
		{
		    /* found the line of the source */
		    
		    if (line1 != line2)
		    {
			char1 = ftt_GetLineIndent (ftt, line);
			char2 = ftt_GetLineLength (ftt, line);
		    }
		    for (i = char1; ((i <= char2) && (i >= 0)); i++)
		    {
			text->tc_ptr[i].style = STYLE_NORMAL;
		    }
		}
	    }
	    else	/* prev_src line not found */
	    {
		if (prev_sink != AST_NIL)
		{
		    ftt_NodeToText(ftt, prev_sink, &line1, 
				   &char1, &line2, &char2);
		    
		    if (lineCheck(line, line1, line2)) 
		    {
			/* found the line of the source */
			
			if (line1 != line2)
			{
			    char1 = ftt_GetLineIndent (ftt, line);
			    char2 = ftt_GetLineLength (ftt, line);
			}
			for (i = char1; ((i <= char2) && (i >= 0)); i++)
			{
			    text->tc_ptr[i].style = STYLE_NORMAL;
			}
		    }
		}
	    }
	}	/* end if (prev_src != ... */
	
	/* if source and sink the same underline and bold */
	
	if ((src == sink) && (src != AST_NIL))
	{
	    /* emphasize the src/sink */
	    
	    ftt_NodeToText(ftt, src, &line1, &char1, &line2, &char2);
	    
	    if (lineCheck(line, line1, line2)) 
	    {
		/* found the line of the source */
		
		if ((line1 != line2) || (ea[edge].type == dg_control) || 
		    (ea[edge].type == dg_io) || (ea[edge].type == dg_call) || 
		    (ea[edge].type == dg_exit))
		{
		    char1 = ftt_GetLineIndent (ftt, line);
		    char2 = ftt_GetLineLength (ftt, line);
		}
		for (i = char1; ((i <= char2) && (i >= 0)); i++)
		{
		    text->tc_ptr[i].style = STYLE_BOLD;
		    text->tc_ptr[i].style |= ATTR_UNDERLINE;
		}
	    }
	    
	    /*----------------------------------------------*/
	    
	    if (src_subscript != AST_NIL)
	    {
		/* emphasize the src/sink subscript */
		
		ftt_NodeToText(ftt, src_subscript, &line1, 
			       &char1, &line2, &char2);
		
		if (lineCheck(line, line1, line2)) 
		{
		    /* found the line of the source */
		    
		    if (line1 != line2)
		    {
			char1 = ftt_GetLineIndent (ftt, line);
			char2 = ftt_GetLineLength (ftt, line);
		    }
		    
		    for (i = char1 - 1; ((i <= (char2 + 1)) && (i >= 0)); i++)
		    {
			/* need from char1-1 to char2+1 */
			/* to get the parens underlined */
			
			text->tc_ptr[i].style = STYLE_BOLD;
			text->tc_ptr[i].style |= ATTR_UNDERLINE;
		    }
		}
		
	    }	/* end if (src_subscript ...) */
	    
	}	/* end if (src == sink ... */
	
	else	/* src != sink */
	{
	    /* emphasize the src */
	    if (src != AST_NIL)
	    {
		ftt_NodeToText(ftt, src, &line1, 
			       &char1, &line2, &char2);
		
		if (lineCheck(line, line1, line2)) 
		{
		    /* found the line of the source */
		    
		    if (line1 != line2)
		    {
			char1 = ftt_GetLineIndent (ftt, line);
			char2 = ftt_GetLineLength (ftt, line);
		    }
		    for (i = char1; ((i <text->num_tc) && (i <= char2) && (i >= 0)); i++)
		    {
			text->tc_ptr[i].style = STYLE_NORMAL;
			text->tc_ptr[i].style |= ATTR_UNDERLINE;
		    }
		}
	    }
	    if (src_subscript != AST_NIL)
	    {
		/* emphasize the src subscript */
		
		ftt_NodeToText(ftt, src_subscript, &line1, 
			       &char1, &line2, &char2);
		
		if (lineCheck(line, line1, line2)) 
		{
		    /* found the line of the source */
		    
		    if (line1 != line2)
		    {
			char1 = ftt_GetLineIndent (ftt, line);
			char2 = ftt_GetLineLength (ftt, line);
		    }
		    
		    for (i = char1 - 1; ((i <= (char2 + 1)) && (i >= 0)); i++)
		    {
			/* need from char1-1 to char2+1 */
			/* to get the parens underlined */
			
			text->tc_ptr[i].style = STYLE_NORMAL;
			text->tc_ptr[i].style |= ATTR_UNDERLINE;
		    }
		}
		
	    }	/* end if (src_subscript ... */
	    
	    /*-----------------------------------------------------*/
	    /* emphasize the sink */
	    
	    if (sink != AST_NIL)
	    {
		ftt_NodeToText(ftt, sink, &line1, 
			       &char1, &line2, &char2);
		
		if (lineCheck(line, line1, line2)) 
		{
		    /* found the line of the source */
		    
		    if (line1 != line2)
		    {
			char1 = ftt_GetLineIndent (ftt, line);
			char2 = ftt_GetLineLength (ftt, line);
		    }
		    for (i = char1; ((i <= char2) && (i >= 0)); i++)
		    {
			text->tc_ptr[i].style = STYLE_BOLD;
		    }
		}
	    }
	    
	    if (sink_subscript != AST_NIL)
	    {
		/* emphasize the sink subscript */
		
		ftt_NodeToText(ftt, sink_subscript, 
			       &line1, &char1, &line2, &char2);
		
		if (lineCheck(line, line1, line2)) 
		{
		    /* found the line of the source */
		    
		    if (line1 != line2)
		    {
			char1 = ftt_GetLineIndent (ftt, line);
			char2 = ftt_GetLineLength (ftt, line);
		    }
		    for (i = char1; ((i <= char2) && (i >= 0)); i++)
		    {
			text->tc_ptr[i].style = STYLE_BOLD;
		    }
		}
	    }
	}
    }
    
    text->ephemeral = false;
}

/*---------------------------------------------------------------------

	pedcp_RedrawLoop() -	Activate the dependence filter 
							on the loop indicated
*/

static void 
pedcp_RedrawLoop(PEditorCP pedcp, AST_INDEX loop)
{
    FortTextTree    ftt;
    int             line1;
    int             line2;
    int             char1;
    int             char2;
    Rectangle       redraw_area;
    
    ed_GetTextTree(R(pedcp)->editor, &ftt);
    
    /* redraw the loop header */
    
    ftt_NodeToText(ftt, loop, &line1, &char1, &line2, &char2);
    redraw_area = makeRect(makePoint(0, line1), makePoint(600, line2));
    
    ed_ViewSetFilterFast(R(pedcp)->editor, 
			 (FortEditorView) R(pedcp)->srcPane,
			 R(pedcp)->filter, redraw_area);
}

/*---------------------------------------------------------------------

	pedcp_RedrawSrcSink()	-	Emphasize the source and sink 
								of a dependence in the text display 

*/

static void 
pedcp_RedrawSrcSink(PEditorCP pedcp, AST_INDEX loop, AST_INDEX src, 
                    AST_INDEX sink, AST_INDEX prev_src, AST_INDEX prev_sink)
{
    FortTextTree    ftt;
    int             line1;
    int             line2;
    int             char1;
    int             char2;
    Rectangle       redraw_area;
    
    ed_GetTextTree(R(pedcp)->editor, &ftt);
    
    /* redraw the prev_src line */
    if (prev_src != AST_NIL)
    {
	ftt_NodeToText(ftt, prev_src, &line1, &char1, &line2, &char2);
	redraw_area = makeRect(makePoint(0, line1), makePoint(600, line1));
	ed_ViewSetFilterFast(R(pedcp)->editor, 
			     (FortEditorView) R(pedcp)->srcPane,
			     R(pedcp)->filter, redraw_area);
    }
    
    /* redraw the prev_sink line */
    if (prev_sink != AST_NIL)
    {
	ftt_NodeToText(ftt, prev_sink, &line1, &char1, &line2, &char2);
	redraw_area = makeRect(makePoint(0, line1), makePoint(600, line1));
	ed_ViewSetFilterFast(R(pedcp)->editor, 
			     (FortEditorView) R(pedcp)->srcPane,
			     R(pedcp)->filter, redraw_area);
    }
    
    
    /* redraw the src line */
    if (src != AST_NIL)
    {
	ftt_NodeToText(ftt, src, &line1, &char1, &line2, &char2);
	redraw_area = makeRect(makePoint(0, line1), makePoint(600, line1));
	ed_ViewSetFilterFast(R(pedcp)->editor, 
			     (FortEditorView) R(pedcp)->srcPane,
			     R(pedcp)->filter, redraw_area);
    }
    
    /* redraw the sink line */
    if (sink != AST_NIL)
    {
	ftt_NodeToText(ftt, sink, &line1, &char1, &line2, &char2);
	redraw_area = makeRect(makePoint(0, line1), makePoint(600, line1));
	ed_ViewSetFilterFast(R(pedcp)->editor, 
			     (FortEditorView) R(pedcp)->srcPane,
			     R(pedcp)->filter, redraw_area);
    }
    
}

/*-----------------------------------------------------------------------

	pedcp_ResetView()	clears all emphasis from Ped

*/

static void
pedcp_ResetView(PEditorCP pedcp)
{
    PedInfo ped;
    AST_INDEX node;
    
    ped = (PedInfo) R(pedcp)->ped;
    
    /* clear edge stack	 */
    
    while (PED_STACK_DEPTH(ped) > 0)
    {
	el_destroy_instance(PED_EL(ped));	/* free the current EL */
	PED_STACK_DEPTH(ped)--;
	stack_pop(PED_EL_STACK(ped), (Generic*)&(PED_EL(ped)));
    }
    
    /* clear emphasis on current loop	 */
    
    loopUpdate(ped, AST_NIL);
    
    /* clear emphasis on src/sink of current dependence	 */
    pedcp_RedrawSrcSink(pedcp, AST_NIL, AST_NIL, AST_NIL,
			PED_PREV_SRC(ped), PED_PREV_SINK(ped));

    /* make sure we don't try to redraw these again later... */
    PED_PREV_SRC(ped) = AST_NIL;
    PED_PREV_SINK(ped) = AST_NIL;
    
    /* clear the edge display */
    edgeClear(ped);

    /* find & store new selection point	*/
    
    ed_GetSelectedNode(R(pedcp)->editor, &node);
    PED_SELECTION(ped) = node;
    
    /* update dialogs	 */
    
    dialogUpdate(pedcp);
    pedcp_markChanged(pedcp);

}

/*******************/
/*  Misc utilities */
/*******************/

/*---------------------------------------------------------------------

*/

/*static*/
Point 
windowSize(Point charSize)
{
    int             BORDERS = 1 + 1;
    Point           size;
    
    size = ed_ViewSize(charSize, pedcp_srcFont);
    size.x = max(SB_MIN_LENGTH, size.x) + SB_WIDTH + BORDERS;
    size.y = max(SB_MIN_LENGTH, size.y) +
	pedcp_button1Size.y + pedcp_depHeaderSize.y +
	    pedcp_depTitleSize.y + SB_WIDTH + BORDERS;
    
    return size;
}

/*---------------------------------------------------------------------

*/

/*static*/ void
tileWindow(PEditorCP pedcp, Point size)
{
    Point           button1Size, button2Size, depTitleSize, 
    srcSize, depSize, depHeaderSize, hscrollSize, 
    vscrollSize, wastedSize;
    
    Generic         src, ped;
    
    /* enforce the minimum size */
    R(pedcp)->size.x = max(size.x, pedcp_minSize.x);
    R(pedcp)->size.y = max(size.y, pedcp_minSize.y);
    
    /* calculate sizes of all the panes */
    /* the button1 pane */
    button1Size.x = R(pedcp)->size.x;
    button1Size.y = pedcp_button1Size.y;
    
    /* the button2 pane */
    button2Size.x = R(pedcp)->size.x;
    button2Size.y = pedcp_button2Size.y;
    
    /* the dependence title pane */
    depTitleSize.x = R(pedcp)->size.x;
    depTitleSize.y = pedcp_depTitleSize.y;
    
    /* the dependence title pane */
    depHeaderSize.x = R(pedcp)->size.x;
    depHeaderSize.y = pedcp_depHeaderSize.y;
    
    /* the source pane */
    srcSize.x = R(pedcp)->size.x - SB_WIDTH;
    srcSize.y = (2 * (R(pedcp)->size.y - SB_WIDTH - button1Size.y -
		      depTitleSize.y)) / 3;
    
    /* the dependence pane */
    depSize.x = R(pedcp)->size.x;
    depSize.y = R(pedcp)->size.y - SB_WIDTH -
	button1Size.y - depTitleSize.y - depHeaderSize.y - srcSize.y;
    
    /* the horizontal scrollbar */
    hscrollSize.x = srcSize.x;
    hscrollSize.y = SB_WIDTH;
    
    /* the vertical scrollbar */
    vscrollSize.x = SB_WIDTH;
    vscrollSize.y = srcSize.y;
    
    /* the wasted bottom right corner */
    wastedSize.x = SB_WIDTH;
    wastedSize.y = SB_WIDTH;
    
    /* tile the window */
    src = cp_td_join(TILE_UP,
		     (aTilingDesc*)cp_td_pane((Pane**)&R(pedcp)->button1Pane, button1Size),
		     (aTilingDesc*)cp_td_join(TILE_UP,
				(aTilingDesc*)cp_td_join(TILE_LEFT,
					   (aTilingDesc*)cp_td_pane((Pane**)&R(pedcp)->srcPane, srcSize),
					   (aTilingDesc*)cp_td_pane((Pane**) & R(pedcp)->vscrollPane, vscrollSize)),
				(aTilingDesc*)cp_td_join(TILE_LEFT,
					   (aTilingDesc*)cp_td_pane((Pane**) & R(pedcp)->hscrollPane, hscrollSize),
					   (aTilingDesc*)cp_td_pane((Pane**)&R(pedcp)->wastedPane, wastedSize)))
		     );
    
    ped = cp_td_join(TILE_UP,
		     (aTilingDesc*)cp_td_join(TILE_UP,
				(aTilingDesc*)cp_td_pane((Pane**)&R(pedcp)->depTitlePane, depTitleSize),
				(aTilingDesc*)cp_td_pane((Pane**)&R(pedcp)->button2Pane, button2Size)),
		     (aTilingDesc*)cp_td_join(TILE_UP,
				(aTilingDesc*)cp_td_pane((Pane**)&R(pedcp)->depHeaderPane, depHeaderSize),
				(aTilingDesc*)cp_td_pane((Pane**)&R(pedcp)->depPane, depSize))
		     );
    
    cp_window_tile((Window**)&R(pedcp)->window, (anInstance*)R(pedcp)->cp_id,
		   (aTilingDesc*)cp_td_join(TILE_UP,(aTilingDesc*)src,(aTilingDesc*)ped));
}

/*---------------------------------------------------------------------

*/

/*static*/ void
titleWindow(PEditorCP pedcp)
{
  char *verb, *dirty;

  /* set the title  */
    verb  = "Editing";
    dirty = (R(pedcp)->changed   ?  "* "  :  "  ");
  
    cp_window_set_title((Window*)R(pedcp)->window, "%s%s %s",
			dirty,verb,
			R(pedcp)->mod_context->ReferenceFilePathName());
}

/*---------------------------------------------------------------------

*/

/*static*/ void
help(PEditorCP pedcp, Generic generator, Point pt)
    /* generator: to decide which display      */
    /* pt: x,y position within display  */
{
    int     position_number;        /* starting position within help        */
    
    /*      find out info associated with "pt", the position on the screen,
     *      and pass it along to the help utility.
     */
    position_number = pt.x;
    
    if (generator == R(pedcp)->button1Pane)         position_number += 10;
    else if (generator == R(pedcp)->button2Pane)    position_number += 20;
    else                                            position_number =   0;
    
    help_cp_give_help(R(pedcp)->cp_id, "ped.H", 0, (short *) 0, 
		      position_number );
}


/*---------------------------------------------------------------------

	save_as_pfc()	-	takes care of saving source, graph and index
	                files to the database

	Returns 	false		if save failed
			true		if everything saved
*/

Boolean  save_as_pfc(PEditorCP pedcp, Context module_context, 
                     Context mod_in_prog_context, Context prog_context)
{
    FortTree 	    ft;
    FortTextTree    ftt;
    Company	    ctype;
    Boolean         savePossible  = true;
    DG_Instance	*dg_instance;
    LI_Instance *li_instance;

    FILE            *srcFP;
    FILE	    *depFP;
    FILE	    *indxFP;
    FILE	    *rsdFP;

    /* get data structures needed by pretty printer */

    ed_GetTree (R(pedcp)->editor, &ft);
    ed_GetTextTree (R(pedcp)->editor, &ftt);
    dg_instance	= PED_DG( ((PedInfo)(R(pedcp)->ped)) );
    li_instance	= PED_LI( ((PedInfo)(R(pedcp)->ped)) );

    if( ! dg_open_files( module_context, mod_in_prog_context, prog_context, "w",
			&depFP, &indxFP, &rsdFP ) )
    {
      savePossible = false;
    }

    if( savePossible )
      {
      /* source file was previously prettyprinted here, should no longer
         be necessary */

      /*
       *	write out the dependence graph in a format compatible with
       *	the routines for importing data from PFC
       */

	(void) dg_save_instance (ftt, dg_instance, depFP );

      /*
       *      write out the index file in a format compatible with the routines
       *      for importing the *.index file generated by PFC.
       */

	(void) li_save_index( ftt, li_instance, indxFP );

      /*
       *      output regular section descriptors
       */

      /* close utility file  */

	dg_close_files( depFP, indxFP, rsdFP );

    } 

    return( savePossible );
     
} /* end_save_as_pfc */




/*---------------------------------------------------------------------

	askAndSave()	-	takes care of saving AST to database

	Returns 	false		if saved failed
			true		if AST saved
*/

static Boolean
askAndSave(PEditorCP pedcp, Boolean saveAs, Boolean saveCopy)
    /* saveAs: false if save in place	*/
    /* saveCopy: false if saveAs = false */
{
  Context mod_context = R(pedcp)->mod_context;
  Context         prog_context;
  Context         mod_in_prog_context;

  Boolean wantSave = true;   /* unless changed below */
  PedInfo         ped = (PedInfo) (R(pedcp)->ped);

  /* grab the name of the reference source file */
  char *oldPathName = ssave(mod_context->ReferenceFilePathName());
  char *newPathName = ssave(mod_context->ReferenceFilePathName());

  Boolean	    ok;
  
    
  /*----------------------------------------------------------*/
  /* first make sure that program is syntactically correct	*/
  
  if (PED_EDIT_PERFORMED(ped) && !ed_CheckModule(R(pedcp)->editor)) {
    ff_SetShowErrors(R(pedcp)->filter, true);
    yes_no("Program contains errors\nSave anyway?",
	   &ok, false);
    
    if (NOT(ok)) wantSave = false;
  }
    
  /*--------------------------------------------*/
  /* if save elsewhere, determine where to save */

  if( wantSave && saveAs )
    wantSave = BOOL(file_select_dialog_run("Save Source", "save", &newPathName,
					   file_select_ok_to_write, pedcp));


  if( wantSave ) { /* ensure the specified object exists and can be modified */
    if (!file_ok_to_write(newPathName)) {
	message("Selected file is not writable.\n");
	wantSave = false;
      }
    }


  if ( wantSave ) {
    /* if context selected is the same, then convert to save	*/
    int pathsDiffer = strcmp(newPathName, oldPathName);
    if ( saveAs  && pathsDiffer ) {
      saveAs = false;
      saveCopy = false;
    }
  }

  /* if save in place, check for graph/index files	*/
    
  if (wantSave && NOT(saveAs)) {
    /* Check for presence of graph file	*/
    File *depFP = mod_context->GetExternalAttributeFile("graph", 1);
    
    char       firstLine[ MAX_LINE_LENGTH ];
    
    if ( depFP )	{ /* graph file present	*/
      /*	Get first line from dependence file	*/
      char *nullIfError = depFP->Gets(firstLine, MAX_LINE_LENGTH);
      mod_context->CloseExternalAttributeFile(depFP);
      
      if( nullIfError && find(firstLine,"local") == -1 ) {
	/* file contains external analysis	*/
	if( ! dg_get_external_analysis(PED_DG(ped)) )
	  {/* But we have local analysis	*/
	    yes_no("Existing dependence file will\nbe replaced with local analysis.\n     Save anyway?", 
		   &ok, false);
	    if ( !ok) {/* cancel save over existing info */
	      wantSave = false; 		 	
	    }
	  }
      }
      else  
	{   /* file = local analysis	*/
	  /* can destroy former local analysis without worry. */
	}
    }
    else 
      {   /* NO graph file present	*/
	/* nothing to be destroyed or replaced, just save.	*/  
      }
    
  }

   
  /* save if still wanted */
  if( wantSave ) {
    int pathsDiffer = strcmp(newPathName, oldPathName);
    if ( saveAs  && pathsDiffer ) {
      file_touch(newPathName); // file must exist for rename to succeed
      mod_context->Rename(newPathName);
    }
    
    save(pedcp, mod_context);
    
    if( !PED_EDIT_PERFORMED(ped) ) {	/* dep analysis available */
      /* save graph, index files. */
      mod_in_prog_context = R(pedcp)->mod_in_pgm_context;
      prog_context = R(pedcp)->pgm_context;
      save_as_pfc( pedcp, mod_context, mod_in_prog_context, prog_context );
    }

    if( !saveCopy ) {
      PED_EDIT_SAVED(ped) = true;
      R(pedcp)->changed = false;
    }
    else if ( saveAs  && pathsDiffer ) { 
      mod_context->Rename(oldPathName);
    }
    
    titleWindow(pedcp);
  }
 

  /* free local storage */
  sfree(oldPathName);
  sfree(newPathName);

  return wantSave;		/* return whether save actually performed */
}

/*---------------------------------------------------------------------

*/

static void
save(PEditorCP pedcp, Context context)
{
    DB_FP         *fp;
    
    /* prepare the output bytestream */
    
    fp =  context->CreateExternalAttributeFile(pedcp_sessionAttribute);

    /* save our persistent information? */
    
    /* save the subparts */
    
    ed_Save(R(pedcp)->editor, context, fp, true);
    vdlg_Save(R(pedcp)->viewer, context, fp);
    sdlg_Save(R(pedcp)->searcher, context, fp);
    fam_Save(R(pedcp)->annotator,context,fp);
    adlg_Save(R(pedcp)->browser,context,fp);
    
    /* close the output bytestream */
    
    if (fp != DB_NULLFP)
      context->CloseExternalAttributeFile(fp);
}

/*---------------------------------------------------------------------

*/

void
pedcp_markChanged(PEditorCP pedcp)
{
  if (!R(pedcp)->changed)
  {
    R(pedcp)->changed = true;
    titleWindow(pedcp);
  }
}


/***********************************/
/* following code currently unused */
/***********************************/

#ifdef UNUSED_CODE

/*---------------------------------------------------------------------
  
 */

static void
setViewFilter(PEditorCP pedcp)
{
    FortEditor      editor = R(pedcp)->editor;
    Generic         pane = R(pedcp)->srcPane;
    FortVFilter     old, new;
    Boolean         changed;
    
    ed_ViewGetFilter(editor, pane, &old);
    new = old;
    changed = vdlg_Menu(R(pedcp)->viewer, &new);
    
    if (changed)
    {
	if (new != old)
	{
	    ed_ViewSetFilter(editor, pane, new);
	    ff_Close(old);
	    
	    R(pedcp)->filter = new;
	}
    }
}


/*---------------------------------------------------------------------
  
 */

static void
    view_menu_run(PEditorCP pedcp)
{
    int             choice;
    
    default_menu_choices(pedcp_viewMenu);
    
    modify_menu_choice(pedcp_viewMenu, VIEW_DEFINE,     false, false);
    modify_menu_choice(pedcp_viewMenu, VIEW_SPLIT,      false, false);
    modify_menu_choice(pedcp_viewMenu, VIEW_SELECTION,	false, false);
    modify_menu_choice(pedcp_viewMenu, VIEW_CONCEAL,    false, false);
    modify_menu_choice(pedcp_viewMenu, VIEW_REVEAL,     false, false);
    modify_menu_choice(pedcp_viewMenu, VIEW_REVEAL_ALL, false, false);
    
    choice = select_from_menu(pedcp_viewMenu, false);
    if (choice != UNUSED)
	viewCommand(pedcp, choice);
}


static
void setLegend(PEditorCP pedcp)
{
    sm_vanilla_set_text(legend,name,edcp_legendFont,
			STYLE_NORMAL,VSM_JUSTIFY_CENTER);
}

#endif

static
void annotGotoFunc(Generic cp)
{
  PEditorCP pedcp = (PEditorCP) cp;

  ed_ViewEnsureSelVisible(R(pedcp)->editor, R(pedcp)->srcPane);
  sm_desk_win_top((Window *) R(pedcp)->window);
}




/* SKW added this for the benefit of the Ped2 user interface */

static
FortEditor pedcp_GetFortEditor(PEditorCP pedcp)
{
  return R(pedcp)->editor;
}

