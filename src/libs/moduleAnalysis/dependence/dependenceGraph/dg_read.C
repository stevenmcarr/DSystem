/* $Id: dg_read.C,v 1.2 1999/03/31 21:49:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	dep/dg/dg_read.c					        */
/*									*/
/*	dg_read.c -- reads graph and map file				*/
/*									*/
/*	readgraph()							*/
/*	getfield()							*/
/*	get_dtype()							*/
/*	readrsd()                					*/
/*									*/
/*									*/
/*	All strings stored into data structures by these routines	*/
/*	are first duplicated using dt_ssave().	mpal, 910622		*/
/*									*/
/************************************************************************/



/*--------------------------------------------------------------------------
  readgraph() - Given a mapping, reads the graph file and sets up the 
  dependence graph for parascope. 
  
  readrsd() - Given a mapping, reads a  rsd file and attaches rsd strings
  to dependences that are in the graph read in by readgraph().
  -------------------------------------------------------------------------- */

#include <string.h>
#include <ctype.h>
#include <libs/support/strings/rn_string.h>

#include <libs/moduleAnalysis/dependence/utilities/read_util.h>

#include <libs/frontEnd/ast/asttree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/fortTextTree/MapInfo_c.h>

#include <libs/moduleAnalysis/dependence/dependenceGraph/dep_dg.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#include <libs/moduleAnalysis/dependence/dependenceTest/dep_dt.h>

#include <libs/graphicInterface/oldMonitor/include/dialogs/message.h>


Boolean         RG_DEBUG = false;

/*
 * This should be unnecessary 
 */
#define MAXLINE    10000
#define EDGE_NUM    2000		/* # of initial DG edges	*/


Boolean
readgraph (FILE *gfile_ptr, MapInfoOpaque map, DG_Instance *DG, 
           DT_info *dt, SideInfo *infoPtr, FortTextTree ftt)
{
    /* Declarations for control of routine	*/
    DG_Edge	*Edge_array;
    int		 eindex;
    char	*fields;
    Boolean	 removeEdges;
    int		 i;
    			/* Declarations for temporary storage of input	*/
    AST_INDEX   sink_stmt_index, 
		src_stmt_index;
    int         src_indx, 
    		sink_indx,
                level, 
    		src_line_num,
    		sink_line_num;
    char        dtype, 
    		*src_name, 
    		*src_type, 
    		*sink_name,  
    		*sink_type;
    char        *src_str,
		*sink_str,
		*missing_entries,
		*junk;
    char        gline[MAXLINE],
    *gbuf;
    int         src_lvector = -1,
    		sink_lvector = -1,
    		src_ref = -1,
    		sink_ref = -1;
    Boolean     statement_only,
    		ic_sensitive,
    		ic_preventing;
    ConsistentType	consistent;
    DepType	type;

    /*	Create DG_Instance and read in data from file to build graph	*/

    Edge_array = dg_get_edge_structure( DG );
    
    /* read in first (bogus) graph file line, and ignore it */
    (void) fgets(gline, MAXLINE, gfile_ptr);
    
    /* check for "local", "pfc", or "updated pfc" analysis */
    if( (i=find(gline,"local")) != -1 )
    {
      dg_set_external_analysis( DG, graph_local); /* reading local file*/
    }
    else if( (i=find(gline,"updated")) != -1 )
      {
	dg_set_external_analysis( DG, graph_updated_pfc); /* reading updated pfc file */
      }
    else if( (i=find(gline,"pfc")) != -1 )
      {
	dg_set_external_analysis( DG, graph_pfc); /* reading pfc file	*/
      }
    else
      {
	dg_set_external_analysis( DG, graph_pfc); /* reading pfc file	*/
	i	= strlen(gline) - 1;
      }
    gline[i]	= '\0';

    dg_set_dependence_header( DG, dt_ssave(gline, dt) );
    
    removeEdges	= false;			/* check for mismatched graph and map file	*/
    
    while (fgets(gline, MAXLINE, gfile_ptr) != NULL)
    {
	/* convert into lower case for compatibility with NED */
	for(i=0; i<strlen(gline); i++) 
	{
	    if(isalpha(gline[i]))
		if(isupper(gline[i]))
		    gline[i] = tolower(gline[i]);
	}
	fields = gline;
	/* scan gline and get the fields */
	fields = get_field_d(fields, &src_line_num );	/* position 1	*/
	fields = get_field_d(fields, &level );	/* position 2	*/
	
	/* fields = get_field_s(fields, &gbuf );*/	/* position 3	*/
	dtype = *fields;
	fields = fields+2;
	
	fields = get_field_s(fields, &src_name );	/* position 4	*/
	fields = get_field_d(fields, &src_indx );	/* position 5	*/
	fields = get_field_s(fields, &src_type );	/* position 6	*/
	fields = get_field_d(fields, &sink_line_num );	/* position 7	*/
	fields = get_field_s(fields, &sink_name );	/* position 8	*/
	fields = get_field_d(fields, &sink_indx );	/* position 9	*/
	fields = get_field_s(fields, &sink_type );	/* position 10	*/
	
	fields = get_field_s(fields, &src_str );	/* position 11	*/
	fields = get_field_s(fields, &sink_str );	/* position 12	*/
	
	fields = skip_field(fields, &missing_entries );	/* position 13	*/
	fields = skip_field(fields, &junk );	/* position 14	*/
	fields = skip_field(fields, &junk );	/* position 15	*/
	
	/* expect either a 1 or nothing in fields 16 and 17 */
	fields = get_field_b(fields, &ic_sensitive );	/* position 16	*/
	fields = get_field_b(fields, &ic_preventing );	/* position 17	*/
	
	fields = skip_field(fields, &junk );	/* position 18	*/
	
	fields = get_field_c(fields, &consistent );	/* position 19	*/
	
	
	type = get_dtype(dtype);

	if( type == dg_input )
	  {
	    dg_set_input_dependences( DG, true);
	  }

	if (type == dg_call || type == dg_exit ||
	    type == dg_control || type == dg_io)
	{
	    if ((src_line_num <= 0) || (sink_line_num <=0))
	    {
		printf("Negative dependence line number(s) %d %d", 
		       src_line_num, sink_line_num);
		continue;
	    }
	    statement_only = true;
	}
	else 
	{
	    statement_only = false;
	    if (pfc_bogus(src_name) || pfc_bogus(sink_name) || 
		(src_line_num <= 0) || (sink_line_num <=0))
	    {
		if ((src_line_num <= 0) || (sink_line_num <=0))
		    printf("Negative dependence line number(s) %d %d", 
			   src_line_num, sink_line_num);
		continue;
	    }
	}
	eindex = dg_alloc_edge( DG, &Edge_array);
	
	Edge_array[eindex].type = type; 

	/* Modified 8/91(kats) to change to new control dependence format */
	/* Control dependence format
	 * fields 1 and 7 are source and sink, as usual
	 * field 5  is the branch label (eg true,false, unconditional)
	 * field 9  is the type of source statement
	 * field 13 is the label of the sink statement (ignored for now)
	 */
	if (Edge_array[eindex].type == dg_control)
	{
	    Edge_array[eindex].cdlabel = src_indx;
	    Edge_array[eindex].cdtype  = (CdBranchType) sink_indx;
	    ic_sensitive  = false;
	    ic_preventing = false;
	}
	
	/* following accomodates loop independent edges, LOOP_INDEPENDENT = -1 */
	if (level == 21)
	    level = LOOP_INDEPENDENT;		       
	
	Edge_array[eindex].level = level;
	
	/* ----------- sink_stmt_index needed to check module root edges ----- */
	sink_stmt_index	= MapLineToIndex(map,sink_line_num);
	
	/* ...................... for SOURCE stmt ............................. */
	
	if(!statement_only || (Edge_array[eindex].type == dg_control))
	{
	    src_stmt_index = MapLineToIndex(map,src_line_num);

	    /* ------- check for control edges from module root ---------- */

	    if( (src_line_num==1) 
	       && (is_function(sink_stmt_index) || is_program(sink_stmt_index) 
		   || is_subroutine(sink_stmt_index)) )
	      {		/* This is an edge from module root */
		src_stmt_index	= ftt_Root(ftt);
	      }

	    /* ------ process source info ------------- */

	    if (Edge_array[eindex].type == dg_control)
	    {
		Edge_array[eindex].src = src_stmt_index;
	    }
	    else
	    {
		if( (Edge_array[eindex].src = 
		     MapVarToIndex(map,src_line_num,src_name,src_indx)) == -1)
		{
		    printf("Mismatched .graph and .map files\n");
		    removeEdges = true;
		    break;
		}
	    }
	    /* does this stmt have a level vector? if not allocate one */
	    if ((src_lvector = dg_get_info( infoPtr, src_stmt_index, type_levelv)) == -1)
	    {
		src_lvector = dg_alloc_level_vector( DG, MAXLOOP);
		dg_put_info( infoPtr, src_stmt_index, type_levelv, src_lvector);
	    }
	    Edge_array[eindex].src_vec = src_lvector;
	    
	    if (Edge_array[eindex].type != dg_control)
	    {
		/* does this variable have a reference list? 
		 * if not allocate one 
		 */
		if ((src_ref = dg_get_info( infoPtr, Edge_array[eindex].src, type_levelv)) == -1)
		{
		    src_ref = dg_alloc_ref_list( DG );
		    dg_put_info( infoPtr, Edge_array[eindex].src, type_levelv, src_ref);
		}
		Edge_array[eindex].src_ref = src_ref;
	    }
	}
	
	/* ...................... for SINK stmt ............................. */

	if (statement_only)
	{
	    Edge_array[eindex].sink = sink_stmt_index;
	}
	else
	{
	    if( (Edge_array[eindex].sink =
		 MapVarToIndex(map,sink_line_num,sink_name,sink_indx)) == -1)
	    {
		printf("Mismatched .graph and .map files\n");
		removeEdges = true;
		break;
	    }
	}
	
	/* does this stmt have a level vector? if not allocate one */
	if ((sink_lvector = dg_get_info( infoPtr, sink_stmt_index, type_levelv)) == -1)
	{
	    sink_lvector = dg_alloc_level_vector( DG, MAXLOOP);
	    dg_put_info( infoPtr, sink_stmt_index, type_levelv, sink_lvector);
	}
	Edge_array[eindex].sink_vec = sink_lvector;
	
	if(statement_only && (Edge_array[eindex].type != dg_control))
	{
	    Edge_array[eindex].src = Edge_array[eindex].sink;
	    Edge_array[eindex].src_vec = Edge_array[eindex].sink_vec;
	}
	if (statement_only)
	{
	    Edge_array[eindex].src_ref = -1;
	    Edge_array[eindex].sink_ref = -1;
	}
	else 
	{
	    /* does this variable have a reference list? if not allocate one */
	    if ((sink_ref = dg_get_info( infoPtr, Edge_array[eindex].sink, type_levelv)) == -1)
	    {
		sink_ref = dg_alloc_ref_list( DG );
		dg_put_info( infoPtr, Edge_array[eindex].sink, type_levelv, sink_ref);
	    }
	    Edge_array[eindex].sink_ref = sink_ref;
	}
	/* ........... now add this to the dependence graph .......... */
	
	Edge_array[eindex].ic_sensitive = ic_sensitive;
	Edge_array[eindex].ic_preventing = ic_preventing;
	Edge_array[eindex].consistent = consistent;
	Edge_array[eindex].src_str  = (char *)( *src_str == '\0' ? NULL : dt_ssave( src_str, dt) );
	Edge_array[eindex].sink_str = (char *)( *sink_str == '\0' ? NULL : dt_ssave( sink_str, dt) );
	
        Edge_array[eindex].dt_type = DT_UNKNOWN;    /* cwt Mar 90 */
	
	if (RG_DEBUG)
	{
	    printf("edge_index = %d\n", (int) eindex);
	    if(! statement_only)
		printf("from %s (stmt %d) to %s (stmt %d)\n",
		       src_name, src_stmt_index, sink_name, sink_stmt_index);
	    
	    printf(".src       = %d\n", (int) Edge_array[eindex].src);
	    if (Edge_array[eindex].src != AST_NIL)
		tree_print(Edge_array[eindex].src);
	    
	    printf(".sink      = %d\n", (int) Edge_array[eindex].sink);
	    if (Edge_array[eindex].sink != AST_NIL)
		tree_print(Edge_array[eindex].sink);
	    
	    printf(".type      = %d\n", (int) Edge_array[eindex].type);
	    printf(".level     = %d\n", (int) Edge_array[eindex].level);
	    printf(".src_ref   = %d\n", (int) Edge_array[eindex].src_ref);
	    printf(".sink_ref  = %d\n", (int) Edge_array[eindex].sink_ref);
	    printf(".src_vec   = %d\n", (int) Edge_array[eindex].src_vec);
	    printf(".sink_vec  = %d\n", (int) Edge_array[eindex].sink_vec);
	    printf(".ic_sensitive  = %d\n", (int) Edge_array[eindex].ic_sensitive);
	    printf(".ic_preventing  = %d\n", (int) Edge_array[eindex].ic_preventing);
	    printf(".consistent  = %d\n", (int) Edge_array[eindex].consistent);
	    printf("\n");
	}
        if (Edge_array[eindex].level > MAXLOOP)
	{
	    message("Unless lvector realloc fixed... we are doomed.");
	}
	
	dg_add_edge( DG, eindex);
	
	/* free of all unique names kats 9/88 */
	/* Storage only allocated using dt_ssave() for edges */
	/* read_free (src_name, src_type, sink_name, sink_type); */
    }

    return	removeEdges;
    
} /* end_readgraph */



void
readrsd(FILE *rsdptr, MapInfoOpaque map, DG_Instance *dg, DT_info *dt, 
        SideInfo *infoPtr, FortTextTree ftt)
{
    DG_Edge        *eArray;
    EDGE_INDEX      edge;
    AST_INDEX       reference;
    int             ref_list;
    char     	foo[MAXLINE], *buf;
    int             i, pos;
    char            *fields;
    Boolean         b;
    int             line;                         /* field 1 */
    char            *var;                         /* field 2 */
    int             occurrence;                   /* field 3 */
    char            *use, *mod;                   /* field 12 & 13 */
    
    eArray = dg_get_edge_structure( dg);
    
    while(fgets(foo, MAXLINE, rsdptr) != NULL) 
    {
	/* convert into lower case for compatibility with NED */
	for(i=0; i<strlen(foo); i++) 
	{
	    if(isalpha(foo[i]))
		if(isupper(foo[i]))
		    foo[i] = tolower(foo[i]);
	}
	/* scan and fill in the fields */
	
	fields = foo;
	fields = get_field_d(fields, &line );		/* field 1 */
	fields = get_field_s(fields, &var );		/* field 2 */
	fields = get_field_d(fields, &occurrence );	/* field 3 */
	/* ignore the subroutine name */
	fields = skip_field(fields, &buf );		/* field 4 */
	
	/* fields 5-8 must have the values (false, true, false, true)
	   respectively, or the record is to be ignored. 
	   */
	fields = get_field_b(fields, &b );		/* field 5 */
	if( b )
	{
	    continue;	/* if not empty, ignore this record */
	}
	fields = get_field_b(fields, &b );		/* field 6 */
	if( ! b )
	{
	    continue;	/* if empty, ignore this record */
	}
	fields = get_field_b(fields, &b );		/* field 7 */
	if( b )
	{
	    continue;	/* if not empty, ignore this record */
	}
	fields = get_field_b(fields, &b );		/* field 8 */
	if( ! b )
	{
	    continue;	/* if empty, ignore this record */
	}
	
	/* ignore 9 - 11 */
	fields = skip_field(fields, &buf );		/* field 9 */
	fields = skip_field(fields, &buf );		/* field 10 */
	/* This field is not actually used (a discrepency in the docs and 
	 * what pfc actually generates).
	 * fields = skip_field(fields, &buf );
	 */
	
	/* the rsd strings/descriptors */
	fields = get_field_s(fields, &use );		/* field 12 */
	fields = get_field_s(fields, &mod );		/* field 13 */
	
	
	if ((reference = MapVarToIndex(map,line,var,occurrence) ) == -1)
	    printf("Malformed ip file\n");
	
	ref_list = dg_get_info( infoPtr, reference, type_levelv);
	
	for (edge = dg_first_src_ref( dg, ref_list); 
	     edge >= 0;
	     edge = dg_next_src_ref( dg, edge))
	{
	    switch (eArray[edge].type)
	    {
	    case dg_true:
	    case dg_output:
		eArray[edge].src_str = (char *)dt_ssave(mod, dt );
		break;
		
	    case dg_anti:
	    case dg_input:
		eArray[edge].src_str = (char *)dt_ssave(use, dt );
		break;
		
	    default:
		break;
	    }
	}
	for (edge = dg_first_sink_ref( dg, ref_list); 
	     edge >= 0;
	     edge = dg_next_sink_ref( dg, edge))
	{
	    switch (eArray[edge].type)
	    {
	    case dg_anti:
	    case dg_output:
		eArray[edge].sink_str = (char *)dt_ssave(mod, dt );
		break;
		
	    case dg_true:
	    case dg_input:
		eArray[edge].sink_str = (char *)dt_ssave(use, dt );
		break;
		
	    default:
		break;
	    }			
	}
    }
    
    return;
}



/************************************************************************/
/*	P F C.    T O    P A R A S C O P E    T R A N S L A T I O N	*/
/************************************************************************/

/*------------------------------------------------------------------------
  get_dtype() - return a number that represents the dependence type.
  ------------------------------------------------------------------------*/
DepType 
get_dtype(char dchar)
{
    switch (dchar)
    {
    case 't':
	return (dg_true);
    case 'a':
	return (dg_anti);
    case 'o':
	return (dg_output);
    case 'n':
	return (dg_input);
    case 'i':
	return (dg_inductive);
    case 'x':
	return (dg_exit);
    case 'r':
	return (dg_io);
    case 'p':
	return (dg_call);
    case 'c':
	return (dg_control);
    default:
	return (dg_unknown);
    }
}


/*-------------------------------------------------------------------------
  pfc_bogus ():  determines if a variable is a "created" variable due to 
  pfc's loop normalization.  If so it ruturns true, otherwise
  false.
  
  Any variables of the following form are considered "pfc bogus".
  (X below is the cross product)
  
  i               var
  j       X       stp     X       <positive integers>
  k               lwb
  l               upb
  
  and
  
  &
  &S              X               <positive integers>
  &L
  &R
  e.g. ivar1, lstp20, &5 
  -------------------------------------------------------------------------*/

Boolean
    pfc_bogus (char *name)
{
    char tmp[4];
    
    if (name[0] == 'i'  || name[0] == 'j' || name[0] == 'k' || name[0] == 'l')
    {
	strncpy (tmp, &name[1], 3);
	tmp[3] = '\0';
	if ((strcmp("var", tmp) == 0) || (strcmp("stp", tmp) == 0) || 
	    (strcmp("lwb", tmp) == 0) || (strcmp("upb", tmp) == 0))
	    
	{
	    if (isdigit(name[4]))
		return true;
	}
    }
    else if (name[0] == '&')
    {
	if (isdigit(name[1]))
	    return true;
	else if (name[1] == 'S' || name[1] == 'L' || name[1] == 'R')
	    if (isdigit(name[2]))
		return true;
    }
    return false;
}

/* frees four strings */
void
    read_free(char *str1, char *str2, char *str3, char *str4)
{
    sfree (str1);
    sfree (str2);
    sfree (str3);
    sfree (str4);
}



