/* $Id: PedPrivate.h,v 1.13 1997/03/11 14:32:04 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ped_cp/PEditorCP/PedPrivate.h					*/
/*									*/
/*	PedPrivate.h --  Rn CP include file for the 			*/
/*			Parallel Fortran Editor				*/
/*      Last edited: June 18, 1993 at 4:00 pm                           */
/*									*/
/************************************************************************/

#ifndef PedPrivate_h
#define PedPrivate_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/FortTextTreeModAttr.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped.h>
#include <libs/support/database/context.h>
#include <libs/support/database/newdatabase.h>

#include <libs/graphicInterface/oldMonitor/include/mon/cp_def.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/textTree/TextTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/FortVFilter.h>
#include <libs/graphicInterface/cmdProcs/newEditor/FortEditor.h>

#include <libs/graphicInterface/oldMonitor/include/mon/dialog.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/filer.h>

/* #include <dialogs/code_type.h> */

#include <libs/graphicInterface/include/FortEditorCP.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditorCP/ViewDialog.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditorCP/SearchDialog.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditorCP/AnnotDialog.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotPI.h>

#include <libs/graphicInterface/cmdProcs/help/help_cp.h>
#include <libs/graphicInterface/support/graphics/rect.h>
#include <libs/graphicInterface/support/graphics/rect_list.h>
#include <libs/graphicInterface/oldMonitor/include/mon/gfx.h>
#include <libs/graphicInterface/oldMonitor/include/mon/event.h>
#include <libs/graphicInterface/oldMonitor/include/mon/event_codes.h>
#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/scroll_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/button_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/ted_sm.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_structs.h>

#include <libs/frontEnd/prettyPrinter/ft2text.h>

typedef Generic PEditorCP;




/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/

/************************/
/*  Representation	*/
/************************/


typedef struct
{
    /* creation parameters */
      Generic              cp_id;
      FortranModule       *mod_context;   /* module  context must be specified */
      Composition         *pgm_context;  /* program context if specified */
      FortranModule       *mod_in_pgm_context;

      FortTreeModAttr      *ftAttr;
      FortTextTreeModAttr  *fttAttr;

      Generic              libraryList;
      Boolean              browsing;

    /* window parts */
      Point                size;

      Generic              window;
      Generic              button1Pane;
      Generic              button2Pane;
      Generic              srcPane;
      ScrollBar            hscrollPane;
      ScrollBar            vscrollPane;
      Generic              wastedPane;

    /* contents */
      FortEditor           editor;
      FortVFilter          filter;
      ViewDialog           viewer;
      SearchDialog         searcher;
      AnnotDialog          browser;
      Generic              annotator;

    /* status */
      Boolean              changed;

    /* dependence window */
      Point                depSize;
      Generic              depPane;
      Generic              depHeaderPane;
      Generic              depTitlePane;

      Generic              ped;		/* dependence handle */
      
    /* Modeless dialog handles */
      Dialog		  *fusionDi;

    /* Modeless dialog structures */
      SharDia	sh;
      InterDia	ih;
      DistrDia	dh;
      SinterDia	sih;
      EdgeDia   eh;
      PerfDia   pm;  /* For Performance Estimation */

    /* Modal dialog structures */
      StripDia  smih;
      AllDia    allh;

    /* menu for dialogs */
      TransMenu th;

      MemoryMenu mm;

      /* source submission */
      Generic              subdia;
	
      /* saving destination */
      char 	*filename;

      /* logging constructs */
      Boolean    added_logging;
      char      *version_name;
      
      /* ptree code type */
      Company       code_type;
} pedcp_Repr;


#define R(ob)		((pedcp_Repr *) ob)

extern short ped_cp_index;

EXTERN (void, sc_Fini, ());
EXTERN (void, ut_Init, ());
EXTERN (void, ut_Fini, ());

typedef struct startup Startup;

/*****************/
/*  declarations */
/*****************/

EXTERN(void, pedcp_markChanged, (PEditorCP pedcp));
EXTERN(void, 	 pedcp_NoteChanges, (PEditorCP pedcp, int kind, Boolean 
                                    autoscroll, FortTreeNode xnode, int first,
                                    int last, int delta));
EXTERN(void,     pedcp_TreeWillChange, (PEditorCP pedcp, FortTreeNode node));
EXTERN(void,     pedcp_TreeChanged, (PEditorCP pedcp, FortTreeNode node));
EXTERN(void,     pedcp_SelectNode, (PEditorCP pedcp, FortTreeNode node));
EXTERN(char*,    pedcp_GetLine, (PEditorCP pedcp, FortTreeNode node));
EXTERN(void,     pedcp_UpdateNodeInfo, (PEditorCP pedcp, AST_INDEX node, int opts));

EXTERN(Boolean,  pedcp_Start, (Generic cpm));
EXTERN(Generic,  pedcp_CreateInstance, (Generic parent_id, Generic cp_id,
                                        Startup *startup));
EXTERN(Generic,  pedcp_Browser_CreateInstance, ());
EXTERN(Generic,  createInstance, (Generic parent_id, Generic cp_id,
                                  Startup *startup));

EXTERN(Boolean,	 pedcp_HandleInputDispatch, (PEditorCP pedcp, Generic generator,
                                             Generic event_type, Point info,
                                             char *msg));
EXTERN(Boolean,  pedcp_HandleInput, (PEditorCP pedcp, Generic generator, Generic
                                    event_type, Point info, char *msg));
EXTERN(Boolean,  pedcp_DepHandleInput, (PEditorCP pedcp, Generic generator, 
                                        Generic event_type, Point info, 
                                        char *msg));
EXTERN(Boolean,  pedcp_HandleIo, ());

EXTERN(void,	 pedcp_DestroyInstance, (PEditorCP pedcp, Boolean panicked));
EXTERN(void, pedcp_Finish, (void));
EXTERN(Point,    windowSize, (Point charSize));
EXTERN(void,     tileWindow, (PEditorCP pedcp, Point size));
EXTERN(void,	 titleWindow, (PEditorCP pedcp));
EXTERN(void,	 help, (PEditorCP pedcp, Generic generator, Point pt));
EXTERN(void,	 button1Command, (PEditorCP pedcp, int button));
EXTERN(void,     button2Command, (PEditorCP pedcp, int button));

EXTERN(void,	 pedcp_create_dialogs, (PEditorCP pedcp));
EXTERN(void,	 pedcp_destroy_dialogs, (PEditorCP pedcp));

extern aProcessor     ped_Processor;
EXTERN(void,     pedcp_NewCP,(/* parent, context, avail */));

/*--------------------------*/
/* external function decls	*/

EXTERN(Generic,	pedInitialize, (Generic ed_handle, FortTextTree ftt,
                                FortTree ft, Context mod_context,
                                Context mod_in_prog_context, Context prog_context,
                                Boolean has_errors, Boolean InputDep));
EXTERN(void,	pedFinalize,   (PedInfo ped));
EXTERN(void, pedUpdate, (PedInfo ped, AST_INDEX node));
EXTERN(void, forcePedUpdate, (PedInfo ped, AST_INDEX loop, AST_INDEX node));

EXTERN(Generic, pedRegister, (PedInfo ped,
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
			      ped_RedrawLoopFunc redrawLoop));

EXTERN(Boolean, pedDepSelect, (PedInfo ped, int newsel));
EXTERN(void, 	pedSelect, ());
EXTERN(void, 	pedUnselect, ());

EXTERN(void, pedPrevDep, (PedInfo ped));
EXTERN(void, pedNextDep, (PedInfo ped));
EXTERN(void, pedNextStep, (PedInfo ped));
EXTERN(void, pedPrevLoop, (PedInfo ped));
EXTERN(void, pedNextLoop, (PedInfo ped));
EXTERN(void,	makeParallel, (PedInfo ped));
EXTERN(void, 	makeSequential, (PedInfo ped));

EXTERN(void,	pedDelete,     (Generic PD, PedInfo ped));

EXTERN(void,	doInterchange,  (InterDia *ih, Generic handle));
EXTERN(void,	doScalarExpansion,  (PedInfo ped));
EXTERN(void,	doRenamingExpansion,  (PedInfo ped));
EXTERN(void,	doDistribution,  (DistrDia *dh, Generic handle, int type));
EXTERN(void,    doAddStmt, (PedInfo ped, char *arg));
EXTERN(void,	doDeleteStmt,  (PedInfo ped));
EXTERN(void,	doStmtInter,  (PedInfo ped));
EXTERN(void,	doPeelIterations,  (PedInfo ped, Boolean iteration, char *iter));
EXTERN(void,    doSplit, (PedInfo ped, char *step));
EXTERN(void,	doSkew,  (PedInfo ped, char *skew_degree));
EXTERN(void,	doStripMine,  (PedInfo ped, char *step));
EXTERN(void,    doCProp, (PedInfo ped));
EXTERN(void,    doReverse, (PedInfo ped));
EXTERN(void,    doAdjust, (PedInfo ped, char *adjust));
EXTERN(void,    doAlign, (PedInfo ped, char *arg));
EXTERN(void,    doReplaceS, (PedInfo ped));
EXTERN(void,    doUnroll, (PedInfo ped, char *arg));
EXTERN(void,    doUnrollJam, (PedInfo ped, char *arg));
EXTERN(void,    doFusion, (PedInfo ped));
EXTERN(void,    doUnswitch, (PedInfo ped));
EXTERN(void, edgeUpdate, (PedInfo ped));
EXTERN(EL_Instance*, edgePop, (PedInfo ped));
EXTERN(EL_Instance*, edgePush, (PedInfo ped));

#endif
