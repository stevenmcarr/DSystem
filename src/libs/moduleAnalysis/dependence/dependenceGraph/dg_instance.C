/* $Id: dg_instance.C,v 1.1 1997/06/25 15:06:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/************************************************************************/
/*									*/
/*	dep/dg/dg_instance.c						*/
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
#include <string.h>
#include <ctype.h>

#include <libs/support/misc/general.h>
#include <libs/support/database/newdatabase.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/fortTextTree/MapInfo_c.h>

#include <libs/moduleAnalysis/cfg/cfg.h>
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>

#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dep_dg.h>
#include <libs/moduleAnalysis/dependence/loopInfo/li_instance.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_instance.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_header.h>
#include <libs/moduleAnalysis/dependence/dependenceTest/dep_dt.h>

#include <libs/support/strings/rn_string.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/support/arrays/ExtensibleArray.h>


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

/************************************************/
/* Externally Available Functions		*/
/************************************************/

/*---------------------------------------------------------------------*/
/* (DG_Instance *) dg_create_instance()
 *
 *	This creates a pointer to an instance structure for a single
 * 	dependence graph.
 */

DG_Instance *
dg_create_instance(void)
{
	return (DG_Instance *) get_mem(sizeof(DG_Instance), "DG_Instance");
}

/*----------------------------------------------------------------------*/
/*     dg_destroy()    Destroy the DG stuff				*/

void
dg_destroy(DG_Instance *dg)
{
	/* get rid of the DG stuff */

	dg_safe_free((int *)dg->edgeptr);
	dg_safe_free((int *)dg->ref_list);
	dg_safe_free((int *)dg->vec_list);
	dg_safe_free((int *)dg->vmd);
	dg_safe_free((int *)dg->free_vmd);
	dg_safe_free((int *)dg->ref);
	stack_destroy(dg->free_vmd_stack);
	stack_destroy(dg->ref_fstack);
	stack_destroy(dg->fstack);
	free_mem((void*)dg);
}



/************************************************/
/* Externally Available Field Access Functions	*/
/************************************************/

/*----------------------------------------- external_analysis	*/

void	dg_set_external_analysis(DG_Instance *dg, GraphType gtype)
{
	dg->external_analysis	= gtype;
}

GraphType	dg_get_external_analysis(DG_Instance *dg)
{
	return	(dg->external_analysis);
}


/*----------------------------------------- dependence_header	*/

void	dg_set_dependence_header(DG_Instance *dg, char *str)
{
	dg->dependence_header	= str;
}

char *	dg_get_dependence_header(DG_Instance *dg)
{
	return	(dg->dependence_header);
}


/*----------------------------------------- local_analysis	*/

void	dg_set_local_analysis(DG_Instance *dg, Boolean flag)
{
	dg->local_analysis	= flag;
}

Boolean	dg_get_local_analysis(DG_Instance *dg)
{
	return	(dg->local_analysis);
}

/*----------------------------------------- set_interchange	*/

void	dg_set_set_interchange(DG_Instance *dg, Boolean flag)
{
	dg->set_interchange	= flag;
}

Boolean	dg_get_set_interchange(DG_Instance* dg)
{
	return	(dg->set_interchange);
}

/*----------------------------------------- input_dependences	*/

void	dg_set_input_dependences(DG_Instance *dg, Boolean flag)
{
	dg->input_dependences	= flag;
}

Boolean	dg_get_input_dependences(DG_Instance *dg)
{
	return	(dg->input_dependences);
}


/************************************************/
/* Externally Available Functions		*/
/************************************************/

void
dg_graph_filename(Context module_context, char *filename)
{
	sprintf( filename, "graph");
}


void
dg_index_filename(Context module_context, char *filename)
{
	sprintf( filename, "index");
}

void
dg_rsd_filename(Context module_context, char *filename)
{
	sprintf( filename, "ip");
}

    /*------------------*/
    /* check file dates */
    /*------------------*/

Boolean
dg_check_file_date(Context module_context, Context mod_in_prog_context, 
                   Context prog_context)
{    
#ifdef ELIM_LINK_HACK
	Boolean		 graphFileCurrent;
	Context    	 annot_context;
	struct stat	 statbuf;            /* buffer for stat() call */
	time_t		 callGraph_time, depGraph_time, ast_time;
	char		 *callGraph_path;
	char		 *depGraph_path;
	char		 *ast_path;

	char		 suffix[MAX_NAME];
	FILE		 *gptr, *iptr, *rsdptr;

	graphFileCurrent	= true;
	annot_context	= ((mod_in_prog_context != NULL) ? mod_in_prog_context : module_context);

	/*	!! dg_get_db_path( annot_context )	*/

	callGraph_time = depGraph_time = ast_time = 0;
	dg_graph_filename( module_context, suffix );
	depGraph_path = annotPath(annot_context, suffix);
    
	/* Get graph file time	*/

	if (!stat(depGraph_path,&statbuf))
	  depGraph_time = statbuf.st_mtime;

	sfree(depGraph_path);

	/* Compare to ast	*/

	ast_path = annotPath(module_context, "source");

	if (!stat(ast_path,&statbuf))
	  ast_time = statbuf.st_mtime;
	if (ast_time > depGraph_time)
	  graphFileCurrent	= false;

	sfree(ast_path);

	/* and program call graph.	*/

	if (prog_context) {	
	  callGraph_path = annotPath(prog_context, "callGraph");

	  if (!stat(callGraph_path,&statbuf))
	    callGraph_time = statbuf.st_mtime;
	  if (callGraph_time > depGraph_time)
	    graphFileCurrent	= false;

	  sfree(callGraph_path);
	}
	return(graphFileCurrent);
#else
	return false;
#endif
} /* end_dg_check_file_date */


GraphType
dg_get_graph_type(char depFile_path[DB_PATH_LENGTH])
{
	GraphType	 fileType;
	char	 	 firstLine[ MAX_LINE_LENGTH ];
	FILE		*gptr;
	
	if ( gptr = fopen(depFile_path, "r") )		/* Graph File Exists	*/
	{
	    (void) fgets(firstLine, MAX_LINE_LENGTH, gptr);
	    fclose( gptr );
	    
	    if( find(firstLine,"local") == -1 )	/* file contains external analysis	*/
	    {
		if( find(firstLine,"updated") == -1 )
		{
		    fileType	= graph_pfc;
		}
		else
		{
		    fileType	= graph_updated_pfc;
		}
	    }
	    else
	    {
		fileType	= graph_local;
	    }
	}
	else
	{
	    fileType	= graph_unknown;
	}

	return	fileType;
}


Boolean
dg_open_files(Context module_context, Context mod_in_prog_context, 
              Context prog_context, char *access, FILE **gptr, FILE **iptr, 
              FILE **rsdptr)
{
#ifdef ELIM_LINK_HACK
	Boolean		 openedOK;		/* return flag	*/

	Context    	 annot_context;

	char		 suffix[MAX_NAME];
	char		 *depGraph_path;
	char		 *index_path;
	char		 *rsd_path;


	openedOK	= true;
	annot_context	= ((mod_in_prog_context != NULL) ? mod_in_prog_context : module_context);

	/* open graph file	*/
    
	if( openedOK )
	  {
	    dg_graph_filename( module_context, suffix );
	    depGraph_path = annotPath(annot_context, suffix);
	    
	    if (!(*gptr = fopen(depGraph_path, access)))
	      {
		openedOK	= false;
	      }

	    sfree(depGraph_path);
	  }
    
	/* open index file	*/
	
	if( openedOK )
	  {
	    dg_index_filename( module_context, suffix );
	    index_path = annotPath(annot_context, suffix);
	    
	    if (!(*iptr = fopen(index_path, access)))
	      {
		fclose(*gptr);
		openedOK	= false;
	      }

	    sfree(index_path);
	  }
	
	/* open rsd file	*/
	
	if( openedOK )
	  {
	    dg_rsd_filename( module_context, suffix );
	    rsd_path = annotPath(annot_context, suffix);
	    *rsdptr	= fopen(rsd_path, access);
	    sfree(rsd_path);
	  }

	return	openedOK;
#else
	return false;
#endif
}



Boolean
dg_readgraph(DG_Instance  **DG, LI_Instance **LI, Context module_context, 
             Context mod_in_prog_context, Context prog_context, 
             FortTextTree ftt, FortTree ft, SideInfo *infoPtr, DT_info *dt_info, 
             AST_INDEX root, CfgInfo cfgModule)
{
	DG_Instance	*dg;
	LI_Instance	*li;
	FILE		*gptr, *iptr, *rsdptr;
	Boolean		 ip_analysis;
	Boolean		 readError;
	MapInfoOpaque	map;

	dg	= *DG;
	li	= *LI;

	/*------------------------------------------------------*/
	/* Open files necessary to read dependence graph	*/
	/*------------------------------------------------------*/
	
	if( dg_open_files(module_context,mod_in_prog_context,prog_context,"r",
			   &gptr, &iptr, &rsdptr) )
	  {		/* Check for presence of Regular Section Descriptors	*/
	    if( rsdptr )
	      ip_analysis = true;
	    else
	      ip_analysis = false;
	  }
	else	/* Open Failed	*/
	  {
	    dg_set_local_analysis(dg, true);	/* want local analysis */
	  }
	
	/*------------------------------------------------------*/
	/* If still possible, attempt to read  dependence graph */
	/*------------------------------------------------------*/

	if( NOT(dg_get_local_analysis(dg)) )
	  {
	    map	= CreateMapInfo(ftt);
	
	    readError	= readgraph(gptr, &map, dg, dt_info, infoPtr, ftt);
	
	    if( !readError )
	      {
		/* build LI from index file 	*/
		li	= *LI	= readindex(iptr, map);
		
		/* attach any rsd information */
		if (ip_analysis)
		  {
		    readrsd(rsdptr, map, dg, dt_info, infoPtr, ftt);
		  }
		
		/* filter dependence graph from file with local analysis */
		dt_update_tests( dg, dt_info, infoPtr, root, cfgModule);
		
		/* if this is a pfc file it does not have control dependences
		 * build them locally.
		 */
		if( dg_get_external_analysis(dg) == graph_pfc )
		  {
		    if (cfgModule != NULL) 
		      {
			dg_build_module_cds(dg, infoPtr, cfgModule);
			dg_set_external_analysis(dg, graph_updated_pfc);
		      }
		  } 
	      }		/*--------------------------------------*/
	    else	/* readgraph failed, do local analysis	*/
	      {		/*--------------------------------------*/
		/* Remove invalid structures				*/
		dg_destroy(dg);		/* get rid of the DG stuff */
		li_free(li);		/* free the LI stuff		*/
		/* eventually will want to clean-up the DT_info structure also*/
		/* dt_free(dt_info);	free the dt stuff		*/
		
		/* Create DG_Instance and LI_Instance, persistent structures */
		dg	= *DG	= dg_create_instance();
		dg_create_edge_structure( *DG, EDGE_NUM);
		dg_set_external_analysis(dg, graph_local);	
		/* default is local graph	*/

		dg_set_set_interchange(dg, false);		
		/* set for some transformations	*/
	
		dg_set_input_dependences(dg, false);		
		/* only create input dep. on request */

		dg_set_dependence_header(dg, "dependence analysis: ");	
		/* default header*/

		dg_set_local_analysis(dg, false);		
		/* default analyze locally	*/

		li = *LI = li_create_instance();
		
		dg_set_local_analysis(dg, true);
	      }

	    DestroyMapInfo(map);
	    /* Close files used for dependence graph	*/
	    dg_close_files( gptr, iptr, rsdptr );
	  }

	return	readError;
} /* end_dg_readgraph */


void
dg_close_files(FILE *gptr, FILE *iptr, FILE *rsdptr)
{
	fclose(gptr);
	fclose(iptr);
	if( rsdptr )
	    fclose(rsdptr);
}


/*---------------------------------------------------------------------*/
/* dg_safe_free checks to see if the block size is good before freeing */

static void
dg_safe_free(int *array)
{
	if (array[-1] > 0 && array[-2] > 0)
		xfree(array);
	else
		fprintf(stderr, "dg_safe_free(): corrupt xarray\n");
}



/*---------------------------------------------------------------------
 
  dg_var_name()   Get name of variable for dependence

*/

char *
dg_var_name(DG_Edge *Edge)
{
  int i;
  char name[512];

  if ((Edge->type != dg_true) &&  /* no var with edge */
      (Edge->type != dg_anti) && 
      (Edge->type != dg_input) &&
      (Edge->type != dg_output)) 
    return NULL;

  else if (is_identifier(Edge->src))	/* local AST */
    return ssave(gen_get_text(Edge->src)); 

  else if (Edge->src_str) /* interprocedural edge */
  {
    strcpy(name,Edge->src_str);
    for (i = 0; isalnum(name[i]); i++)
      ;
    name[i] = '\0';
    return ssave(name);
  }

  return NULL;   /* can't find name of var */
}
