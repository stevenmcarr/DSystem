/* $Id: dp.h,v 1.23 1997/03/11 14:31:16 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*--------------------------------------------------------------------

	dp.h		Definitions for basic PED internals 

*/
#ifndef dp_h
#define dp_h

#ifndef	 general_h
#include <libs/support/misc/general.h>
#endif
#ifndef	 point_h
#include <libs/graphicInterface/support/graphics/point.h>
#endif
#ifndef	 xstack_h
#include <libs/support/stacks/xstack.h>
#endif
#ifndef	 ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

#ifndef	 context_h
#include <libs/support/database/context.h>
#endif

#ifndef	 newdatabase_h
#include <libs/support/database/newdatabase.h>
#endif

#ifndef	 side_info_h
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#endif
#ifndef	 dg_instance_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#endif
#ifndef	 dg_header_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#endif

#ifndef cfg_h
#include <libs/moduleAnalysis/cfg/cfg.h>
#endif


/*------------------------------------------------------*/
/* PedInfo stores all the info for each PED command  	*/
/* processor instance not in PedCP						*/

typedef FUNCTION_POINTER(Generic, ped_GetFortEditorFunc,
 (Generic pedcp));
typedef FUNCTION_POINTER(void, ped_TreeWillChangeFunc,
 (Generic pedcp,AST_INDEX node));
typedef FUNCTION_POINTER(void, ped_TreeChangedFunc,
 (Generic ped,AST_INDEX node));
typedef FUNCTION_POINTER(void, ped_UpdateNodeInfoFunc,
 (Generic pedcp,AST_INDEX node, int dtinfo));
typedef FUNCTION_POINTER(char *, ped_GetLineFunc,
 (Generic pedcp,AST_INDEX node));
typedef FUNCTION_POINTER(void, ped_SelectNodeFunc,
 (Generic pedcp,AST_INDEX node));
typedef FUNCTION_POINTER(void, ped_RedrawSrcSinkFunc,
 (Generic pedcp,AST_INDEX loop, AST_INDEX node1, AST_INDEX node2,
  AST_INDEX src, AST_INDEX sink));
typedef FUNCTION_POINTER(void, ped_RedrawLoopFunc,
 (Generic pedcp,AST_INDEX node));
    
typedef struct PedInfoStruct
{
    /* handle and callbacks for editor cp */
    Generic			ed_handle;
    ped_GetFortEditorFunc	GetFortEditor;
    ped_TreeWillChangeFunc	TreeWillChange;
    ped_TreeChangedFunc		TreeChanged;
    ped_UpdateNodeInfoFunc	UpdateNodeInfo;
    ped_GetLineFunc		GetLine;
    ped_SelectNodeFunc		SelectNode;
    ped_RedrawSrcSinkFunc	RedrawSrcSink;
    ped_RedrawLoopFunc		RedrawLoop;
    
    /* the program */
    Generic		ftt;
    Generic		ft;
    AST_INDEX		root;

    /* control flow graph */
    CfgInfo		cfg_module;
    
    /* the selected node */
    AST_INDEX       selection;
    AST_INDEX       selected_loop;
    AST_INDEX       prev_loop;
    
    /* panes and such */
    Generic         dep_pane;
    Generic         dep_header_pane;
    Generic         dep_title_pane;
    Point           dep_size;
    
    /* dependence list fields */
    Generic         current_dependence;
    AST_INDEX       prev_src;
    AST_INDEX       prev_sink;
    
    /* handle on dependence graph abstraction */
    DG_Instance     *DG;
    
    /* info array & side array  */
    SideInfo	    *info;			/* abstracting side array */

    /* handle on loop information abstraction  */
    struct Loops    *LI;
    
    /* handle on edgelist abstraction         */
    Stack           EL_stack;
    struct el_instance *EL;
    int 	    stack_depth;
    
    /* handle on reference information abstraction  */
    struct dt_info_struct *dt_info;

#ifdef CPROP_STUBBED_OFF
    /* handle on the cprop structure          */
    Generic         cprop_info;
#endif

    /* handle on the memory hierarchy config structure          */
    int             mh_config;
    
    /* flags for incremental analysis */
    
    Boolean	edit_enabled;		/* arbitrary edits allowed	*/
    Boolean	edit_performed;		/* edits (possibly) performed	*/
    Boolean	edit_saved;		/* edits have been saved	*/

    /* support for dependence analysis of a module in the context of a program*/
    Context     pgm_context;		/* program context id           */
    Generic     pgm_callgraph;		/* program callgraph            */

} Ped, *PedInfo;


/* constants for UpdateNodeInfo */

#define UPDATE_SUBS            0x001
#define UPDATE_DTINFO          0x002
#define UPDATE_DG              0x004

/*--------------------------------------*/
/* macros to access elements of PedInfo	*/

#define	PED_DG(ped) 			(ped->DG)
#define	PED_LI(ped) 			(ped->LI)
#define	PED_EL(ped)			(ped->EL)
#define	PED_SELECTED_LOOP(ped) 		(ped->selected_loop)
#define	PED_PREV_LOOP(ped)		(ped->prev_loop)
#define	PED_CURRENT_DEPENDENCE(ped)	(ped->current_dependence)
#define	PED_SELECTION(ped)		(ped->selection)
#define	PED_INFO(ped)			(ped->info)
#define	PED_INFO_SIDE_ARRAY(ped)	(ped->info->info_side_array)
#define	PED_INFO_ARRAY(ped)		(ped->info->info_array)
#define	PED_INFO_ARRAY_FIRST_FREE(ped)	(ped->info->info_array_first_free)
#define	PED_ED_HANDLE(ped)		(ped->ed_handle)
#define	PED_EXTERNAL_ANALYSIS(ped)	(ped->error)
#define	PED_DEPENDENCE_HEADER(ped)	(ped->error)
#define	PED_LOCAL_ANALYSIS(ped)		(ped->error)
#define	PED_INPUT_DEPENDENCES(ped)	(ped->error)
#define	PED_SET_INTERCHANGE(ped)	(ped->error)
#define	PED_EDIT_ENABLED(ped)		(ped->edit_enabled)
#define	PED_EDIT_PERFORMED(ped)		(ped->edit_performed)
#define	PED_EDIT_SAVED(ped)		(ped->edit_saved)
#define	PED_DEP_PANE(ped)		(ped->dep_pane)
#define	PED_DEP_TITLE_PANE(ped)		(ped->dep_title_pane)
#define	PED_DEP_HEADER_PANE(ped)	(ped->dep_header_pane)
#define	PED_DEP_SIZE(ped)		(ped->dep_size)
#define	PED_STACK_DEPTH(ped)		(ped->stack_depth)
#define	PED_PREV_SRC(ped)		(ped->prev_src)
#define	PED_PREV_SINK(ped)		(ped->prev_sink)
#define	PED_EL_STACK(ped)		(ped->EL_stack)
#define PED_FTT(ped)			(ped->ftt)
#define PED_FT(ped)			(ped->ft)
#define PED_ROOT(ped)			(ped->root)
#define PED_CFG(ped)			(ped->cfg_module)

#ifdef CPROP_STUBBED_OFF
#define PED_CPROP(ped)			(ped->cprop_info)
#endif

#define	PED_DT_INFO(ped)		(ped->dt_info)
#define	PED_MH_CONFIG(ped)		(ped->mh_config)
#define	PED_PGM_CONTEXT(ped)		(ped->pgm_context)
#define	PED_PGM_CALLGRAPH(ped)		(ped->pgm_callgraph)

/*--------------------------*/
/* external function decls	*/

EXTERN(Generic, get_info,(PedInfo ped,AST_INDEX astindex,
				  Info_type infotype));
EXTERN(void, create_info,(SideInfo * infoPtr,AST_INDEX astindex));
EXTERN(void, put_info,(PedInfo ped, AST_INDEX astindex,
			       Info_type infotype,Generic value));


/*-------------------------------------- /dt/dt_info.c ---------*/
/* initialize the fields at "infoPtr" based on values in ped	*/

EXTERN( void, ped_init_infoPtr,
		(PedInfo ped, SideInfo *infoPtr) );

/* initialize the fields in ped based on values at "infoPtr"	*/
EXTERN( void, ped_gets_infoPtr,
		( PedInfo ped, SideInfo *infoPtr) );


#endif
