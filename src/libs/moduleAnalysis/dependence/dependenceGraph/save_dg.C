/* $Id: save_dg.C,v 1.1 1997/06/25 15:06:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*	********************************************************************/
/*									   */
/*	dep/dg/save_dg.c						   */
/*									   */
/*	save_dg.c	-- Responsible for saving the Dependence Graph	   */
/*			   Representation into a file.			   */
/*									   */
/*	Exports:							   */
/*	Boolean	dg_save_instance();					   */
/*	Boolean	dg_save_edge();						   */
/*									   */
/*	Locals:								   */
/*	static char		 dtype_to_char();			   */
/*	static AST_INDEX	 begin_ast_stmt();			   */
/*	static AST_INDEX	 find_enclosing_DO();			   */
/*	static void              Translate_AST_to_line( );		   */
/*	static int		 var_to_index_preaction();		   */
/*	static int		 var_to_index();			   */
/*									   */
/*									   */
/*									   */
/*									   */
/*									   */
/*									   */
/*									   */
/*									   */
/*	**************************************************************     */


/*    ********************************************************************/
/*	Include Files:							 */

#include <stdio.h>
#include <string.h>

#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/ast/groups.h>

#include <libs/frontEnd/include/walk.h>

#include <libs/moduleAnalysis/dependence/dependenceGraph/dep_dg.h>

#include <libs/moduleAnalysis/dependence/dependenceGraph/private_dg.h>

/*	********************************************************************/
/*	Exported Declarations:						   */


	Boolean dg_save_instance( );
/*
 *	Boolean	dg_save_instance( FortTextTree ftt, DG_Instance *dg, File *depFP )
 *		Directly access the DG edge array. Output each used edge, 
 *	using dg_save_edge(), to the file depFP.
 */

	Boolean	dg_save_edge(  );
/*
 *	Boolean	dg_save_edge( FortTextTree, DG_Edge *, FILE * );
 *            This function prints an ASCII representation of the 
 *	dependence edge at the file pointer.  This needs the FortTextTree
 *	to obtain the line number associated with source and sink nodes.
 */



/********************************************************************	*/
/*	Local Declarations:						*/


	STATIC(char,		dtype_to_char,(DepType dtype));
/*
 *	char		dtype_to_char(DepType );
 *		This function converts from the internal format
 *		used to indicate Dependence Type, to the character
 *		format used in files created by PFC.
 */

	STATIC(AST_INDEX,	find_enclosing_DO,(AST_INDEX node));
/*
 *	AST_INDEX	find_enclosing_DO( AST_INDEX node );
 *		This function returns the AST_INDEX of the parallel
 *		or sequential loop construct enclosing `node'
 */

	STATIC(void,            Translate_AST_to_line,(FortTextTree ftt, AST_INDEX
                 astIndex, int *line, int *index));
/*
 *      void            Translate_AST_to_line(FortTextTree, AST_INDEX, int, int );
 *              Finds the starting line for this construct,
 *	        and  which occurrence this is of var on line.
 *	ERROR  -- *index is set to 0 when variable is not found.
 */


/*	**************************************************************** */
/*	================================================================ */
/*	**************************************************************** */

/*	--------------------------------------------------------------------	*/
/*
 * 			      dg_save_instance.c
 * 									
 *   INVOCATION:
 *	retcode = dg_save_instance ( ftt, dg, depFP )
 *
 *   FUNCTION:
 *	print a <Dependence Graph> representation into a file.
 *	The representation used is compatible with that output
 *	from PFC.
 *	Directly accesses the DG edge array, and uses dg_save_edge() 
 *	to output each used edge to the file depFP.
 *
 *   RETURNS:
 *	If everything is successful, a  0 is returned.
 *	If there is a problem,       a -1 is returned.
 *
 *   HISTORY:
 *	This code was modeled on the ft2text routine, pretty printer.
 *	Developed February, 1991 by Mike Paleczny.
 *	TimeKey=910201
 *   
 */

/* --------------------------------------------------------------------
 * -------------------------------------------------------------------- */
Boolean dg_save_instance(FortTextTree ftt, DG_Instance *dg_instance, FILE *depFP)
{
    /*	access to instance of the dependence graph	*/

    DG_Edge		*edgeArray;
    DG_Edge		*edge;
    int			 eindex;

    char		*graphType;

    /*	- - - - - - - START of CODE - - - - - - - - - - 	*/

    switch( dg_get_external_analysis(dg_instance) )
      {
      case graph_local:
	graphType	= "local";
	break;
      case graph_pfc:
	graphType	= "pfc";
	break;
      case graph_updated_pfc:
	graphType	= "updated pfc";
	break;
      default:
	graphType	= "unknown";
	printf( "\nCAUTION: dg_save_instance()  dependence graph has unspecified source.\n" );
	break;
      }

    fprintf( depFP,"%s%s\n", dg_get_dependence_header(dg_instance), graphType );

    edgeArray   = dg_get_edge_structure( dg_instance );

    /*      Low-level method to access data in edgeArray	*/
    /*      Save each used edge.				*/
    for( eindex = dg_instance->num_edges - 1; eindex >=0; eindex-- )
      {
	edge	= &(edgeArray[eindex]);
	if( edge->used == true )  dg_save_edge( ftt, edge, depFP );
      }

    return true;
} /* end_dg_save_instance */



/*	**************************************************************** */
/*	================================================================ */
/*	**************************************************************** */



/*	--------------------------------------------------------------------	*/
/*
 * 			      dg_save_edge.c
 * 									
 *   INVOCATION:
 *	retcode = dg_save_edge( ftt, edge, depFP )
 *
 *   FUNCTION:
 *	Print the contents of a <Dependence Edge> into the file 
 *	pointed to by depFP.
 *	The representation used is compatible with that output
 *	from PFC, and read by ped_cp/el/readgraph.c
 *
 *   RETURNS:
 *	If everything is successful, a  'true' is returned.
 *	If there is a problem,       a  'false' is returned.
 *
 *   HISTORY:
 *	This code was modeled as an inverse of readgraph.c
 *	Developed April, 1991 by Mike Paleczny.
 *	TimeKey=910402
 *   
 */

Boolean 
dg_save_edge(FortTextTree ftt, DG_Edge *edge, FILE *depFP)
{
    int		 src_indx	= -1,
    		 sink_indx	= -1;
    int		 level	= -1;
    int          src_line_num	= -1;
    int		 src_line_pos;
    char	*src_line_text;
    int          sink_line_num	= -1;
    int		 sink_line_pos;
    char	*sink_line_text;
    char	 dtype	= '?';
    char	 src_type[2];		/* not used */
    char	 sink_type[2];		/* not used */
    char	*src_name	= "src",
        	*sink_name	= "sink";
    char	*src_str	= "src_str",
        	*sink_str	= "sink_str",
        	*missing_entries	= "\0",	/* not used */
        	*junk	= "junk";
    char	 gline[256],
        	*gbuf;
    char	*ic_sensitive	= "-1!";
    char	*ic_preventing	= "-1!";
    char	*consistent	= "-1!";
    int		 junk1, junk2;
    int		 pos, i;
    DepType 	 type = edge->type;

    /* Initialization of some defaults                          */
    src_type[0]   = '\000';
    sink_type[0]  = '\000';

    /*	- - - - - - - START of CODE - - - - - - - - - - 	*/

    switch( type )	/* different output formats	*/
      {
      case (dg_true):
      case (dg_anti):
      case (dg_output):
      case (dg_input):
      case (dg_inductive):

	/*	get info associated with source node in AST		*/
	/* position 1	--- Source variable record number		*/
	/* position 4	--- Source Variable Name			*/
	/* position 5	--- Occurrence on line				*/
	if( edge->src != AST_NIL )
	  {
	    src_name	= gen_get_text(edge->src);
	    Translate_AST_to_line( ftt, edge->src, &src_line_num, &src_indx );
	  }

	/*	get info associated with sink node in AST		*/
	/* position 7	Sink Line Number				*/
	/* position 8	Sink Variable Name				*/
	/* position 9	Sink Variable Index --- occurrence on line	*/
	if( edge->sink != AST_NIL )
	  {
	    sink_name	= gen_get_text(edge->sink);
	    Translate_AST_to_line( ftt, edge->sink, &sink_line_num, &sink_indx ); 
	  }
	break;

      case  dg_control :
	/* Updated 8/91(kats) to support new control dependence structure */
	
	/* position 1	--- Source variable record number		*/
	if( edge->src != AST_NIL )
	{
	    Translate_AST_to_line( ftt, edge->src, &src_line_num, &src_indx );
	}
	/* position 3	dtype	= 'C';  normal definition. 			*/
	/* position 5	control branch label				*/
	src_indx = edge->cdlabel;
	
	/* position 7	sink_line_num	-- sink variable record number		*/
	if( edge->sink != AST_NIL )
	{
	    Translate_AST_to_line( ftt, edge->sink, &sink_line_num, &sink_indx );
	}
	/* position 9	type of control dependence		*/
	sink_indx = (int) edge->cdtype;
	break;
	
      case  dg_call:
      case  dg_exit:
      case  dg_io:		/* !!?? work on this		*/

	/* position 1	--- Loop Header line number		*/
	/* not used in readgraph for statement_only dependence	*/
	{
	  AST_INDEX	doIndex;
	  doIndex	= find_enclosing_DO( edge->src );

	  if( doIndex != AST_NIL )
	    {
	      Translate_AST_to_line( ftt, doIndex, &src_line_num, &src_indx );
	    }
	}
	/* position 2	--- level, normal definition		*/
	/* position 3	--- dtype = 'P|X|R';  normal definition. 	*/
					
	/* position 7	--- sink_line_num = Statement line number	*/
	if( edge->sink != AST_NIL )
	  {
	    Translate_AST_to_line( ftt, edge->sink, &sink_line_num, &sink_indx );
	  }

	/* position 13	--- missing_entry point name.		*/
	/* 		not used in readgraph			*/
	/* if( edge->type == dg_call )
	 *  {
	 *    missing_entries	= "missing entry point name";
	 *  }
	 */
	break;

      case (dg_unknown):	
	
	break;
	
      } /* end switch( type ) */

    if( edge->level == LOOP_INDEPENDENT )		/* position 2	*/
      level	= 21;
    else
      level	= edge->level;
    dtype	= (char) dtype_to_char(type);		/* position 3	*/
    if (type == dg_call || type == dg_exit || 
	type == dg_control || type == dg_io)
    { /* a dependence on a statement, no variables */
	src_name = "\0";
	sink_name = "\0";
    }
    src_str  = (char *) edge->src_str;			/* position 11	*/
    sink_str = (char *) edge->sink_str;			/* position 12	*/
    if( edge->ic_sensitive == true )			/* position 16	*/
      ic_sensitive	= "1!";
    else
      ic_sensitive	= "!";
    if( edge->ic_preventing == true )			/* position 17	*/
      ic_preventing	= "1!";
    else
      ic_preventing	= "!";
    switch( edge->consistent )				/* position 19	*/
      {
      case	inconsistent:
	consistent	= "0!";
	break;
      case	consistent_SIV:
	consistent	= "1!";
	break;
      case	consistent_MIV:
	consistent	= "2!";
	break;
      default:
	break;
      }

    /*	---------------------------------------- Print data to file	*/

    fprintf(depFP,"%d!", src_line_num  );	/* position 1	*/
    fprintf(depFP,"%d!", level  );		/* position 2	*/
    fprintf(depFP,"%c!", dtype  );		/* position 3	*/
    fprintf(depFP,"%s!", src_name  );		/* position 4	*/
    if( type == dg_control || src_indx != 0 )
	fprintf(depFP,"%d!", src_indx  );	/* position 5	*/
    else
	fprintf(depFP,"!" );
    fprintf(depFP,"%.1s!", src_type  );		/* position 6	not used */
    fprintf(depFP,"%d!", sink_line_num  );	/* position 7	*/
    fprintf(depFP,"%s!", sink_name  );		/* position 8	*/
    if( type == dg_control || sink_indx != 0 )
	fprintf(depFP,"%d!", sink_indx  );	/* position 9	*/
    else
	fprintf(depFP,"!" );
    fprintf(depFP,"%.1s!", sink_type  );	/* position 10	not used */
    fprintf(depFP,"%s!", src_str ? src_str : "");	/* position 11	*/
    fprintf(depFP,"%s!", sink_str ? sink_str : "");	/* position 12	*/
    fprintf(depFP,"!" );			/* position 13	not used */
    fprintf(depFP,"!" );        	        /* position 14	not used */
    fprintf(depFP,"!" );	                /* position 15	not used */
    fprintf(depFP,"%s", ic_sensitive  );	/* position 16	*/
    fprintf(depFP,"%s", ic_preventing  );	/* position 17	*/
    
    /* reserved (?)			*/
    fprintf(depFP,"!"  );			/* position 18	not used */
    
    /* consistent threshold		*/
    fprintf(depFP,"%s", consistent  );		/* position 19  */
    
    /* parallel loop preventing flags	*/
    fprintf(depFP,"!"  );			/* position 20	not used */
    /* reserved (fusion preventing flag)	*/
    fprintf(depFP,"!"  );			/* position 21	not used */
    /* reserved (fusion level)		*/
    fprintf(depFP,"!"  );			/* position 22	not used */
    /* reserved (greatest common nest level)*/
    fprintf(depFP,"!"  );			/* position 23	not used */
    
    fprintf(depFP,"\n");
    

    return( true );
} /* end_dg_save_edge */


/* ============================================== LOCAL FUNCTIONS ====== */


/*------------------------------------------------------------------------
  begin_ast_stmt()	-- finds which occurrence this is of var on line.
			-- Identical to ped_cp/dt/build_dg/dg_ast_stmt ?!
 ------------------------------------------------------------------------*/
static	AST_INDEX
begin_ast_stmt(AST_INDEX node)
{
	while( (node != AST_NIL) && (! is_statement(node)) )
		node	= tree_out( node );

	return node;

} /* end_begin_ast_stmt */


/*------------------------------------------------------------------------
  find_enclosing_DO()	-- returns the AST_INDEX of the enclosing loop
			   construct in the AST.
 ------------------------------------------------------------------------*/
static	AST_INDEX
find_enclosing_DO(AST_INDEX node)
{
	begin_ast_stmt( node );

	while((node != AST_NIL) && !is_loop(node)) 
	  {
	    node	= tree_out( node );
	    begin_ast_stmt( node );
	  }

	return( node );

} /* end_find_enclosing_DO */


/*------------------------------------------------------------------------
  V2I_Params	-- parameter structure used during walk of expressions,
 ------------------------------------------------------------------------*/
typedef	struct V2I_Params_struct
	{
	AST_INDEX	varNode;	/* node we are looking for	*/
	int		varCount;	/* count of times var has been seen */
	char		*varLexeme;	/* variable name		*/
	int		done;
	} 
	V2I_Params;


/*------------------------------------------------------------------------
  var_to_index_preaction()	-- Counts the number of occurrences through
				   r->varNode
 ------------------------------------------------------------------------*/
static	int
var_to_index_preaction(AST_INDEX node, V2I_Params *r)
    /* node: node being examined		*/
    /* r: structure used for params during walk */
{

int	line_num,line_pos,junk1,junk2;

    char		*lexeme;

    if( r->done == 0 )
    {
	if( is_identifier(node) )
		{
		lexeme	= gen_get_text( node );
		if( strcmp(lexeme,r->varLexeme) == 0 )	
			{
			(r->varCount)++;
			if( node == r->varNode )
				{
				  r->done	= 1;
				  return( WALK_ABORT );
				}
			}
		}
    } /* ! r->done */

	return( WALK_CONTINUE );

} /* end_var_to_index_preaction */


/*------------------------------------------------------------------------
  var_to_index()	-- finds which occurrence this is of var on line.
		ERROR	-- returns 0 when variable is not found.
 ------------------------------------------------------------------------*/
static	int
var_to_index(AST_INDEX varNode)
{
    V2I_Params		params;
    AST_INDEX		stmt;
    AST_INDEX		expr1,
			expr2;
    int			rc;

	/* start at beginning of statement containing variable ref	*/
	stmt	= begin_ast_stmt( varNode );

	/* walk stmt, and count occurrences before varNode		*/

	/* initialize parameters for walk.				*/
	params.varNode	= varNode;
	params.varCount	= 0;
	params.varLexeme	= gen_get_text( varNode );
	params.done	= 0;

	/* walk graph...						*/
/*
 *	if( is_assignment(stmt) )
 *		{
 *		expr1	= gen_ASSIGNMENT_get_lvalue(stmt);
 *		expr2	= gen_ASSIGNMENT_get_rvalue(stmt);
 *		rc	= walk_expression( expr1, 
 *			var_to_index_preaction, NULL, (Generic)&params );
 *		if( rc == WALK_CONTINUE )
 *			{
 *			rc	= walk_expression( expr2, 
 *				var_to_index_preaction,NULL, (Generic)&params );
 *			if( rc == WALK_CONTINUE )
 *				printf("ERROR: var_to_index didn't find var\n");
 *			}
 *		return( params.varCount );
 *		}
 */
	rc	= walk_expression( stmt, 
		(WK_EXPR_CLBACK)var_to_index_preaction, NULL, (Generic)&params );
	if( params.done != 0 )
		return( params.varCount );
	else
		return( 0 );


} /* end_var_to_index */




/*------------------------------------------------------------------------
  Translate_AST_to_line() -- finds the starting line for this construct,
                             and  which occurrence this is of var on line.
		   ERROR  -- *index is set to 0 when variable is not found.
 ------------------------------------------------------------------------*/
static	void Translate_AST_to_line(FortTextTree ftt, AST_INDEX astIndex, 
                                   int *line, int *index )
{
  int        position;
  int        endingLine;
  int        endingPos;

  /* Return the starting line number of astIndex        */
  /* using current screen format.               	*/
  ftt_NodeToText( ftt, astIndex, 
		 line, &position,
		 &endingLine, &endingPos );

  /* Correct for starting index at zero.                 */
  (*line)++;

  /* Return which occurrence of the variable we have    */
  *index	= var_to_index( astIndex );

  /*    Associated info which may be usefull, someday.
   *{
   *    src_line_text	= ftt_GetTextLine(ftt,src_line_num);
   *}
   */

} /* end_Translate_AST_to_line */


/*------------------------------------------------------------------------
  dtype_to_char() - return a character that represents the dependence type.
 ------------------------------------------------------------------------*/
static	char          
dtype_to_char(DepType dtype)
{
    switch (dtype)
    {
    case (dg_true):	
	return ('t');
    case (dg_anti):	
	return ('a');
    case (dg_output):		
	return ('o');
    case (dg_input):	
	return ('n');
    case (dg_inductive):	
	return ('i');
    case (dg_exit):		
	return ('x');
    case (dg_io):		
	return ('r');
    case (dg_call):		
	return ('p');
    case (dg_control):
	return ('c');
    case (dg_unknown):		
	return ('u');
    }

} /* end_dtype_to_char */

