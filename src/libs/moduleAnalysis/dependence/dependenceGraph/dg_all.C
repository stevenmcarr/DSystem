/* $Id: dg_all.C,v 1.1 1997/06/25 15:06:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/************************************************************************/
/*									*/
/*	dep/dg/dg_interface.c						*/
/*									*/
/*	Routines to provide an interface to the dependence graph	*/
/*									*/
/*									*/
/************************************************************************/


/************************************************************************/
/*			Include Files					*/
/************************************************************************/
#include <libs/moduleAnalysis/dependence/dependenceGraph/private_dg.h>

#include <sys/types.h>


#include <stdio.h>
#include <sys/stat.h>

#include <libs/support/misc/general.h>
#include <libs/support/database/newdatabase.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>

#include <libs/moduleAnalysis/cfg/cfg.h>
#include <libs/moduleAnalysis/cfgValNum/cfgval.h>
#include <libs/moduleAnalysis/ssa/ssa.h>

#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#include <libs/moduleAnalysis/dependence/loopInfo/li_instance.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_instance.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_header.h>
#include <libs/moduleAnalysis/dependence/dependenceTest/dep_dt.h>

#include <libs/ipAnalysis/interface/IPQuery.h>

#include <libs/support/strings/rn_string.h>

/************************************************************************/
/*	Declaration of Local Declarations, Functions and Variables	*/
/************************************************************************/
#define MAX_LINE_LENGTH	2047
#define MAX_NAME	10
#define	EDGE_NUM	2000

static int     INFO_SIDE_ARRAY_INITIALS = {-1};

STATIC(void,	dg_safe_free, (int * array) );


		/******************************/
		/* Dependence Graph Routines  */
		/******************************/

/*	**************************************************************	*/

/*
 *	dg_all -- constructs a DG_Instance, EL_Instance, and LI_Instance
 *	then sets the handles passed to the routine to point to these.
 *	
 *	The DG_Instance and LI_Instance will be read from files if these
 *	exist, otherwise they will be constructed based upon the ft and ftt
 *	arguments.  ft and ftt must exist and match the context information.
 *	
 *	This routine may also construct the program call graph.
 *	
 *	WARNING !	This routine has an expanded twin "pedInitialize"
 *			which will probably need to be updated when this is.
 */

void dg_all(Context module_context, Context mod_in_prog_context, Context prog_context, 
            FortTextTree ftt, FortTree ft, DG_Instance **DG, EL_Instance **EL, 
            LI_Instance **LI, SideInfo **SI,DT_info **DT,
	    CfgInfo *cfgModule, Boolean InputDep)
{
  Context    	 annot_context;
  AST_INDEX	 root;
  char		 suffix[MAX_NAME];
  char		*depGraph_path;
  FILE		*gptr, *iptr, *rsdptr;
  GraphType	 fileType;
  Boolean	 ip_analysis;
  Boolean	 readError;

  DG_Instance	*dg;
  EL_Instance	*el;
  LI_Instance	*li;
  SideInfo	*infoPtr;
  DT_info	*dt_info;
  C_CallGraph	 pgm_callgraph;	


  annot_context	= ((mod_in_prog_context != NULL) ? 
		   mod_in_prog_context : module_context);

  /*----------------------------------------------------------------*/
  /* Create the structures needed to read or build dependence Graph */
  /*----------------------------------------------------------------*/

  if (prog_context != CONTEXT_NULL) 
    pgm_callgraph = IPQuery_Init(prog_context); /* program callgraph if any*/
  else
    pgm_callgraph = NULL;

  root = ft_Root(ft);		/* get & save root of AST 	*/

  /* Create SideInfo, allows sharing side array access with PED */
  infoPtr = create_side_info(ft);

  /* Create EL_Instance, DG_Instance and LI_Instance, persistent structures */

  el = el_create_instance(10);  /* num_edges = 10 */
  dg = dg_create_instance();
  li = li_create_instance();

  /* Create the control-flow graph information */
  if (*cfgModule = cfg_Open(ft))
    {
      cfgval_Open(*cfgModule, false);
      ssa_Open(*cfgModule,
	       /* ipInfo    */ (Generic)pgm_callgraph,     
	       /* ipSmush   */ false,
	       /* doArrays  */ false, 
	       /* doDefKill */ false, /* this should be true for output deps */
	       /* doGated   */ false);
    }

  dt_info = dt_init(root, infoPtr, *cfgModule);

  dg_create_edge_structure(dg, EDGE_NUM);
  dg_set_external_analysis(dg, graph_local); /* default is local graph	*/
  dg_set_set_interchange(dg, false); /* set for some transformations	*/
  dg_set_input_dependences(dg, InputDep); /* only create input dep on request */
  dg_set_dependence_header(dg, (char *)ssave("dependence analysis: "));
  dg_set_local_analysis(dg, false); /* do not analyze locally	*/

  /*-----------------------------------------------------------*/
  /* All symbolic & dependence test analysis always performed, */ 
  /* at least until full distance/direction vectors stored     */
  /*-----------------------------------------------------------*/

#ifdef CPROP_STUBBED_OFF
  Generic	cprop_info;

  cprop_info	= 
    cprop_build_structs( cprop_info, ft,
			/* make_changes */	0,
			/* print */		0,
			/* kill_graphs */	0,
			/* doValueTable */	1,
			/* doExecAlg */	1);
#endif

  /*------------------*/
  /* check file dates */
  /*------------------*/
  dg_set_local_analysis
    (dg, NOT(dg_check_file_date(module_context,
				mod_in_prog_context,
				prog_context)) );
    
  /*------------------------------------*/
  /* Check type of graph file available	*/
  /*------------------------------------*/

  if (NOT(dg_get_local_analysis(dg)))
    {
#ifdef ELIM_LINK_HACK
      dg_graph_filename(module_context, suffix);

      depGraph_path = annotPath(annot_context, suffix);
	    
      fileType = dg_get_graph_type(depGraph_path);

      sfree(depGraph_path);
#else
      fileType = graph_unknown;
#endif

      if( fileType == graph_unknown )
	{
	  dg_set_local_analysis(dg, true); /* want local analysis */
	}
    }

  /*-----------------------------------*/
  /* Attempt to read  dependence graph */
  /*-----------------------------------*/

  readError = false;
  if (NOT(dg_get_local_analysis(dg)))
    {
      readError	= 
	dg_readgraph(&dg, &li,
		     module_context, mod_in_prog_context, prog_context, 
		     ftt, ft, infoPtr, dt_info, root, *cfgModule); 
      if(readError)
	{
	  dg_set_local_analysis(dg, true);
	}
    }

  /*----------------------------------*/
  /* Or, build DG from local analysis */
  /*----------------------------------*/  
  
  if(dg_get_local_analysis(dg))
    {
      dg_build(root, ft, dg, infoPtr, dt_info, li, (Generic)pgm_callgraph, *cfgModule);
    }

  *DG	= dg;
  *EL	= el;
  *LI	= li;
  *SI	= infoPtr;
  *DT   = dt_info;

}

