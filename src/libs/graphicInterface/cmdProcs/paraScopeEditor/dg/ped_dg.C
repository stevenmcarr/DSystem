/* $Id: ped_dg.C,v 1.1 1997/06/25 14:40:06 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ped_cp/dg/ped_dg.c						*/
/*									*/
/*	Routines to handle PedInfo & Info Array				*/
/*									*/
/*									*/
/************************************************************************/
 

#include <stdio.h>
#include <sys/stat.h>

#include <libs/support/misc/general.h>
#include <libs/support/database/context.h>
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
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dt.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>

#ifdef CPROP_STUBBED_OFF
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/cprop.h>
#endif

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/main_dg.h>
#include <libs/ipAnalysis/interface/IPQuery.h>



static int     INFO_SIDE_ARRAY_INITIALS = {-1};

#define MAX_LINE_LENGTH	2047
#define MAX_NAME	10
#define	EDGE_NUM	2000

/*	This File defines the following functions:
 *
 * pedInitialize()
 * pedFinalize  ()
 * pedReinitialize()
 * get_info()
 * put_info()
 */



/*---------------------------------------------------------------------

	pedInitialize()		Initialize key data for PED,
			   (dependence graph, loop info, side array)
			   
	NOTE:	Modelled after dg_all() located in dep/dg/main_dg.c

*/
Generic
pedInitialize(Generic ed_handle, FortTextTree ftt, FortTree ft, Context mod_context, 
              Context mod_in_prog_context, Context prog_context, Boolean has_errors,
              Boolean InputDep)
{
    PedInfo		ped;
    AST_INDEX		root;
    char		suffix[MAX_NAME];
    char		*depGraph_path;
    FILE		*mptr, *gptr, *iptr, *rsdptr;
    GraphType		 fileType;

    Boolean		openedOK;
    Boolean		ip_analysis;
    Boolean		readError;
    Context   		annot_context;

    /* Determine context in which PED is being used	*/
    annot_context	= ((mod_in_prog_context != CONTEXT_NULL) ?
			   mod_in_prog_context : mod_context);
    
    /*--------------------------------------*/
    /* set up the key data struct for Ped	*/
    
    ped = (PedInfo) get_mem(sizeof(Ped), "Dependence structure");
    PED_INFO(ped)	= (SideInfo *) get_mem(sizeof(SideInfo), "SideInfo structure");
    
    PED_ED_HANDLE(ped)	= ed_handle;
    PED_SELECTED_LOOP(ped)	= AST_NIL;

    PED_STACK_DEPTH(ped)	= 0;
    PED_EL_STACK(ped)	= NULL;
    
    PED_EDIT_ENABLED(ped) = true;		/* edits allowed	*/
    PED_EDIT_PERFORMED(ped) = false;		/* no edits performed	*/
    PED_EDIT_SAVED(ped) = true;			/* vacuously true	*/

    PED_PGM_CONTEXT(ped) = prog_context;/* save program context if any  */

    if (prog_context != CONTEXT_NULL) {
      PED_PGM_CALLGRAPH(ped) = (Generic) IPQuery_Init(prog_context); 
      /* program callgraph if any*/
    } 
    else 
      PED_PGM_CALLGRAPH(ped) = NULL;

    PED_FTT(ped) = (Generic) ftt;	        /* save ftt of program	*/
    PED_FT(ped)  = (Generic) ft;	        /* save ft of program	*/
    root = ft_Root(ft);				/* get & save root of AST */
    PED_ROOT(ped) = root;
    
    PED_MH_CONFIG(ped) = NULL;			/* no memory config yet */
    
#ifdef CPROP_STUBBED_OFF    
    /* constant propagation structure not built yet */
    PED_CPROP(ped) = (Generic) NULL;
#endif

    PED_INFO(ped) = create_side_info(ft);

    /*-----------------------------------------------------------------------*/
    /* Create EL_Instance, DG_Instance and LI_Instance, persistent structures*/
    /*-----------------------------------------------------------------------*/

    PED_EL(ped) = el_create_instance(10 /* num_edges */ );
    PED_DG(ped) = dg_create_instance();
    PED_LI(ped) = li_create_instance();

    /* Create the control-flow graph information */
    if (PED_CFG(ped) = cfg_Open(ft))
      {
	cfgval_Open(PED_CFG(ped), false);
	ssa_Open(PED_CFG(ped),
		 /* ipInfo    */ PED_PGM_CALLGRAPH(ped),     
		 /* ipSmush   */ false,
		 /* doArrays  */ false, 
		 /* doDefKill */ false, /* this should be true for output deps */
		 /* doGated   */ false);
      }

    PED_DT_INFO(ped)	= dt_init(root, PED_INFO(ped), PED_CFG(ped));
    
    dg_create_edge_structure( PED_DG(ped), EDGE_NUM);

    dg_set_external_analysis(PED_DG(ped), graph_local);	
    /* default is local graph	*/

    dg_set_set_interchange(PED_DG(ped), false);		
    /* set for some transformations	*/

    dg_set_input_dependences(PED_DG(ped), InputDep);	
    /* only create input dep. on request */

    dg_set_dependence_header(PED_DG(ped), "dependence analysis: ");	
    /* default header*/

    dg_set_local_analysis(PED_DG(ped), false);		
    /* do not analyze locally	*/

    if (!has_errors)
    {
      /*------------------------------------------------------------------*/
      /* All symbolic & dependence test analysis always performed,	*/ 
      /* at least until full distance/direction vectors stored		*/
      /*------------------------------------------------------------------*/

#ifdef CPROP_STUBBED_OFF
      PED_CPROP(ped) = 
	cprop_build_structs(PED_CPROP(ped), PED_FT(ped),
			    /* make_changes */	0,
			    /* print */		0,
			    /* kill_graphs */	0,
			    /* doValueTable */	1,
			    /* doExecAlg */	1);
#endif

      /*------------------*/
      /* check file dates */
      /*------------------*/
      dg_set_local_analysis(PED_DG(ped), 
			    NOT(dg_check_file_date(mod_context,
						   mod_in_prog_context,
						   prog_context)) );
    
      /*------------------------------------------*/
      /* Check type of graph file available	*/
      /*------------------------------------------*/

      if (NOT(dg_get_local_analysis(PED_DG(ped))))
      {
	dg_graph_filename( mod_context, suffix );
#ifdef ELIM_LINK_HACK
	depGraph_path = annotPath(annot_context, suffix);
      
	fileType	= dg_get_graph_type( depGraph_path );

	sfree(depGraph_path);
#else
	fileType = graph_unknown;
#endif

	switch( fileType )	{
	case	graph_unknown:
	  dg_set_local_analysis(PED_DG(ped), true);	/* want local analysis */
	  break;
	case	graph_local:
	  show_message2("Reading ParaScope Dependence File");
	  break;
	case	graph_pfc:
	  show_message2("Reading PFC Dependence File");
	  break;
	case	graph_updated_pfc:
	  show_message2("Reading Updated PFC Dependence File");
	  break;
	}
      }
      
      /*------------------------------------------------------*/
      /* Attempt to read  dependence graph			*/
      /*------------------------------------------------------*/
      
      readError	= false;
      if (NOT(dg_get_local_analysis(PED_DG(ped))))
      {
	readError = dg_readgraph( &PED_DG(ped), &PED_LI(ped),
				 mod_context, mod_in_prog_context, prog_context, 
				 ftt, ft, PED_INFO(ped), PED_DT_INFO(ped), root,
				 PED_CFG(ped));
	if (readError)
	{
	  dg_set_local_analysis(PED_DG(ped), true);
	  
	  hide_message2(/* Reading Dependence File */);
	  show_message2("Mismatched graph and map files.\n Analyzing Program Locally");
	}
      }
      
      /*--------------------------------------*/
      /* Or, build DG from local analysis	*/
      /*--------------------------------------*/  
    
      if( dg_get_local_analysis(PED_DG(ped)) )
      {
	if( NOT(readError) )
	  show_message2("Analyzing Program Locally");
	dg_build( root, ft, PED_DG(ped), PED_INFO(ped), PED_DT_INFO(ped), 
		 PED_LI(ped), PED_PGM_CALLGRAPH(ped), PED_CFG(ped));
      }


      /*----------------------------------------------------------*/
      /* save subscript text in side array for quicker access	*/

      subs_attach( root, ftt, PED_INFO(ped));
    
      /*-----------*/
      /* all done! */
    
      hide_message2(/* Analyzing Program */);
    }
    else
    {
      /* use this to indicate that no dependence graph is available here,
	 since the program contained errors
       */
      PED_EDIT_PERFORMED(ped) = true;
    }
  return (Generic) ped;
} /* end_pedInitialize */



/*---------------------------------------------------------------------

	Caller:	often called by pedcp_DestroyInstance()
*/
void
pedFinalize(PedInfo ped)
{ 
    /* destroy cfg information */
    if (PED_CFG(ped))
      {
	cfgval_Close(PED_CFG(ped));
	ssa_Close(PED_CFG(ped));
	cfg_Close(PED_CFG(ped));
      }

    dg_destroy( PED_DG(ped));		/* get rid of the DG stuff */
    ped_li_free(ped);			/* free the LI stuff	   */
    dt_free(PED_DT_INFO(ped));		/* free the dt stuff	   */
    subs_free(PED_INFO(ped));		/* free the subscript info */
    destroy_side_info( (FortTree)PED_FT(ped), PED_INFO(ped) );
  
    /* free the callgraph structures */
    if (PED_PGM_CALLGRAPH(ped))
    {
	IPQuery_Fini((C_CallGraph) PED_PGM_CALLGRAPH(ped));
	PED_PGM_CALLGRAPH(ped) = (Generic) NULL;
    }
  
#ifdef CPROP_STUBBED_OFF
    /* free the cprop structures */
    if (PED_CPROP(ped))
    {
	cprop_free_globals(PED_CPROP(ped));
	PED_CPROP(ped) = (Generic) NULL; 
    }
#endif

    /* free the stack of EL structures */
    
    while (PED_STACK_DEPTH(ped) > 0)
    {
	/* free the current EL */
	el_destroy_instance(PED_EL(ped));
	PED_STACK_DEPTH(ped)--;
	stack_pop(PED_EL_STACK(ped), (Generic *)&PED_EL(ped));
    }
    
    el_destroy_instance(PED_EL(ped));		/* free the stack bottom */
    if (PED_EL_STACK(ped)) stack_destroy(PED_EL_STACK(ped));	/* free the stack		*/
    free_mem((void*) ped);
} /* end_pedFinalize */


/*---------------------------------------------------------------------

	pedReinitialize()	Renitialize key data for PED,
			    (dependence graph, loop info, side array)

*/
void
pedReinitialize(PedInfo ped)
{
    AST_INDEX root;
    int       count;
    int       info_elem_quantity;
    
    /* store old type filter values 
     */
    Boolean lc 	    = el_get_view_lc(PED_EL(ped));
    Boolean control = el_get_view_control(PED_EL(ped));
    Boolean li 	    = el_get_view_li(PED_EL(ped));
    Boolean Private = el_get_view_private(PED_EL(ped));

    /* save these values for new instances */
    Boolean input_flag	= dg_get_input_dependences(PED_DG(ped)); 

    /*----------------------------------*/
    /* first free most of PED internals	*/

    /* destroy cfg information */
    if (PED_CFG(ped))
      {
	cfgval_Close(PED_CFG(ped));
	ssa_Close(PED_CFG(ped));
	cfg_Close(PED_CFG(ped));
      }
    
    dg_destroy( PED_DG(ped));		/* get rid of the DG stuff */
    ped_li_free(ped);			/* free the LI stuff		*/
    dt_free(PED_DT_INFO(ped));		/* free the dt stuff		*/
    subs_free(PED_INFO(ped));		/* free the subscript info */
    
    /* reinitialize interprocedural information -- JMC 2/93 */
    if (PED_PGM_CONTEXT(ped) != CONTEXT_NULL) {
      if (PED_PGM_CALLGRAPH(ped) != NULL) 
	IPQuery_Fini((C_CallGraph) PED_PGM_CALLGRAPH(ped));
      PED_PGM_CALLGRAPH(ped) = (Generic) IPQuery_Init(PED_PGM_CONTEXT(ped)); 
    } 
    else 
      PED_PGM_CALLGRAPH(ped) = NULL;


#ifdef CPROP_STUBBED_OFF
    /* free the cprop structures */
    if (PED_CPROP(ped))
    {
	cprop_free_globals(PED_CPROP(ped));
	PED_CPROP(ped) = (Generic) NULL;
    }
#endif

    /* free the stack of EL structures */
    
    while (PED_STACK_DEPTH(ped) > 0)
    {
	/* free the current EL */
	el_destroy_instance(PED_EL(ped));
	PED_STACK_DEPTH(ped)--;
	stack_pop(PED_EL_STACK(ped), (Generic *)&PED_EL(ped));
    }
    
    el_destroy_instance(PED_EL(ped));	/* free the stack bottom */
    
    /*------------------*/
    /* clear info array	*/
    clear_info_array( PED_INFO(ped) );
    
    /*------------------------------*/
    /* now rebuild PED internals	*/
    
    /* Create EL_Instance, DG_Instance and LI_Instance, persistent structures	*/
    PED_EL(ped) = el_create_instance(10 /* num_edges */ );
    PED_DG(ped) = dg_create_instance();
    dg_create_edge_structure( PED_DG(ped), EDGE_NUM);
    dg_set_external_analysis(PED_DG(ped), graph_local);	
    /* default is local graph	*/

    dg_set_set_interchange(PED_DG(ped), false);		
    /* set for some transformations	*/

    dg_set_input_dependences(PED_DG(ped), input_flag);	
    /* only create input dep. on request */

    dg_set_dependence_header(PED_DG(ped), "dependence analysis: ");	
    /* default header*/

    dg_set_local_analysis(PED_DG(ped), false);		
    /* do not analyze locally	*/

    PED_LI(ped) = li_create_instance();

    /* restore old type filter values */
    el_set_view_lc(PED_EL(ped), lc);
    el_set_view_control( PED_EL(ped), control);
    el_set_view_li( PED_EL(ped), li);
    el_set_view_private( PED_EL(ped), Private);
    
    /*----------------------------------------------*/
    /* now rebuild the dependence graph	*/
    
    pedRoot(ped);		/* root may have changed!	*/
    
    root = PED_ROOT(ped);

#ifdef CPROP_STUBBED_OFF 
    PED_CPROP(ped) = 
	cprop_build_structs(PED_CPROP(ped), PED_FT(ped),
			    /* make_changes */	0,
			    /* print */		0,
			    /* kill_graphs */	0,
			    /* doValueTable */	1,
			    /* doExecAlg */	1);
#endif

  /* Create the control-flow graph information */
  if (PED_CFG(ped) = cfg_Open((FortTree)PED_FT(ped)))
    {
      cfgval_Open(PED_CFG(ped), false);
      ssa_Open(PED_CFG(ped),
	       /* ipInfo    */ PED_PGM_CALLGRAPH(ped),
	       /* ipSmush   */ false,
	       /* doArrays  */ false, 
	       /* doDefKill */ false, /* this should be true for output deps */
	       /* doGated   */ false);
    }

    PED_DT_INFO(ped)	= dt_init(root, PED_INFO(ped), PED_CFG(ped));
    dg_build( root, (FortTree)PED_FT(ped), PED_DG(ped), PED_INFO(ped), 
	PED_DT_INFO(ped), PED_LI(ped), PED_PGM_CALLGRAPH(ped), PED_CFG(ped));
 
    /*----------------------------------------------------------*/
    /* save subscript text in side array for quicker access		*/

    subs_attach( root, PED_FTT(ped), PED_INFO(ped));
} /* end_pedReinitialize */



/*	******** ACCESS TO OLD  SIDE_ARRAY  IMPLEMENTATION ********	*/



/*  ALTERNATE VERSION OF dg_put_info WHICH ALLOWS OLD CALL FORMAT	*/
void
put_info(PedInfo ped, AST_INDEX astindex, Info_type infotype, Generic value)
{
  dg_put_info( PED_INFO(ped), astindex, infotype, value );
}


/*  ALTERNATE VERSION OF dg_get_info WHICH ALLOWS OLD CALL FORMAT	*/
Generic
get_info(PedInfo ped, AST_INDEX astindex, Info_type infotype)
{
  return	dg_get_info( PED_INFO(ped), astindex, infotype );
}




/*---------------------------------------------------------------------

  This is just to get the cdg stuff linked in

*/

static void
cdg_dummy()
{
/*  (void) ped_dg_build_cdg();  */
}


