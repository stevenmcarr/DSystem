/* $Id: renaming.C,v 1.1 1997/06/25 13:52:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/moduleAnalysis/dependence/utilities/strong.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pt/private_pt.h>


/*   e x p o r t s   */

Boolean ar_removable_edge(); /* checks whether dependence edge can be removed by array renaming */

void pt_rename();     /* performs renaming ( 2. P H A S E ) */

void re_build_stmt_nodes(PedInfo ped, Adj_List *adj_list, AST_INDEX node, int level, 
                         AST_INDEX do_node);
void re_add_stmt_edges(PedInfo ped, Adj_List *adj_list, AST_INDEX node, int carry_level,
                        int depth);
void re_add_stmt_edges2(PedInfo ped, Adj_List *adj_list, AST_INDEX node, 
                        int carry_level, int depth, AST_INDEX selection);

/*   l o c a l   r o u t i n e s   */

STATIC(Boolean, in_subscipt,(int node));
STATIC(char, *walk1_stmt_list,(PedInfo ped, AST_INDEX list, int level, 
                               AST_INDEX selection));         
STATIC(void, walk2_stmt_list,());         
STATIC(char, *walk1B_assignment,(AST_INDEX node, int level, AST_INDEX selection));
STATIC(void, walk2_assignment,());
STATIC(char, *walk_expr,(AST_INDEX expr_tree, int level, AST_INDEX selection));
STATIC(void, expand,());
STATIC(void, re_build_do_nodes,(PedInfo ped, Adj_List *adj_list,
                                AST_INDEX node, int level));
STATIC(void, re_build_do_nodes2,(PedInfo ped, Adj_List *adj_list,AST_INDEX node,
                                 int level, AST_INDEX do_node));
STATIC(void, re_build_stmt_edges,(PedInfo ped, Adj_List *adj_list, 
                                  AST_INDEX node, int level, int depth));

STATIC(int, re_handle_scexpnd_query1, (PedInfo ped, AST_INDEX node, int level, 
				       AST_INDEX selection));
STATIC(int, re_handle_scexpnd_query2, (PedInfo ped, AST_INDEX node, int level, 
				       AST_INDEX selection));
STATIC(void, re_build_stmt_edges2, (PedInfo ped, Adj_List *adj_list, AST_INDEX node, 
				    int level, int depth, AST_INDEX selection));
/*   l o c a l  o b j e c t s   */

static Boolean create_prolog_stmt; /* flag, whether a prolog statement */  
                                   /* has to be generated or not       */
static Boolean create_epilog_stmt; /* flag, whether a epilog statement */  
                                   /* has to be generated or not       */
 
/*
 *   c h e c k s   w h e t h e r  d e p e n d e n c e   c a n   b e   r e m o v e d   
 */

Boolean
ar_removable_edge(PedInfo ped, EDGE_INDEX edge, AST_INDEX selection)
{
  DG_Edge   *edgeptr;
  AST_INDEX  src;

  if  ( edge == END_OF_LIST )
    printf("invalid edge\n");
  else  {
    edgeptr = dg_get_edge_structure( PED_DG(ped));
    src = edgeptr[edge].src;
    if  ( selection != src )
      return( false );  /* instances are not equal */
    else  
      switch  ( edgeptr[edge].type )
      {
      case dg_anti:
	/*            if  ( is_upex_use( src ) )   */
	/* all loop independent edges can be removed and              */
	/* there cannot be any loop carried anti dependences;         */
	/* this fact is not detected by the dependence analysis since */
	/* KILL-analysis is not performed for anti-dependences        */
	return( true );		/* loop independent edge */
	/*
	  else  {
	  if  ( edgeptr[edge].level > 0 )  
	  return( true );
	  else
	  return( false ); 
	  }
	  */

      case dg_output:
	/* all loop carried dependences can be removed */
	if  ( edgeptr[edge].level > 0 )  
	  return( true );
	else
	  return( false ); 

      }				/*switch*/
  } /*else*/
	return( false );

}

/*
 *  A S T - n o d e   in   s u b s c i p t ?
 */
  
static
Boolean
in_subscript(AST_INDEX node)
{
   AST_INDEX father;
   father = tree_out(node);
   return BOOL(is_subscript(father) && ( gen_SUBSCRIPT_get_name(father) == node ));
}


/*
 *   w a l k   a s s i g n m e n t   s t a t e m e n t   ( 1. v e r s i o n )
 */

/* ---------------------------------------------------------------------------------------
  
    The following routines 

	     (1) walk1A_assignment
	     (2) check_backward_true_dep

    can be used if KILL informations are available. This would
    allow to detect upwards exposed uses of a variable "var" by looking at the true
    dependences of "var" inside the loop. 
                                                Uli, June 88 
   --------------------------------------------------------------------------------------- */

/*
 *      w a l k   e x p r e s s i o n  -  t r e e 
 */

/*
 *  this routine walks an arithmetic expression; 
 *  if a functioncall is encountered an error message is generated;
 *  the routine searchs for uses of "var" in the expression tree that
 *  are upwards exposed; "var" must not occur as part of an index expression
 *  of an array reference.
 */

static char
*walk_expr(AST_INDEX expr_tree, int level, AST_INDEX selection)
{

  char *temp;

    temp = (char *) 0;
    switch (gen_get_node_type(expr_tree))
    {
      case GEN_INVOCATION:
              return ssave("Sorry, cannot handle function calls inside the loop");
      case GEN_IDENTIFIER:
/*	      if  ( !stop_upex_search )  
                if  ( strcmp( gen_get_text( selection ), gen_get_text( expr_tree ) ) == 0 )  
		  insert_upex_use( expr_tree );
*/
	      break;
      case GEN_SUBSCRIPT:
	      /* no check whether "var" occurs in that subscipt */
	      break;
      case GEN_CONSTANT:
	      break;
      case GEN_UNARY_MINUS:
              temp = walk_expr( gen_UNARY_MINUS_get_rvalue( expr_tree ), level, selection );	
              break;
      case GEN_BINARY_EXPONENT:
              temp = walk_expr( gen_BINARY_EXPONENT_get_rvalue1( expr_tree), level, selection );	
              if  ( temp != (char *) 0 ) 
                return ( temp );
              temp = walk_expr( gen_BINARY_EXPONENT_get_rvalue2( expr_tree), level, selection );	
              break;
      case GEN_BINARY_TIMES:
              temp = walk_expr( gen_BINARY_TIMES_get_rvalue1( expr_tree), level, selection );	
              if  ( temp != (char *) 0 ) 
                return ( temp );
              temp = walk_expr( gen_BINARY_TIMES_get_rvalue2( expr_tree), level, selection );	
	      break;
      case GEN_BINARY_DIVIDE:
              temp = walk_expr( gen_BINARY_DIVIDE_get_rvalue1( expr_tree), level, selection );	
              if  ( temp != (char *) 0 ) 
                return ( temp );
              temp = walk_expr( gen_BINARY_DIVIDE_get_rvalue2( expr_tree), level, selection );	
	      break;
      case GEN_BINARY_PLUS:
              temp = walk_expr( gen_BINARY_PLUS_get_rvalue1( expr_tree), level, selection );	
              if  ( temp != (char *) 0 ) 
                return ( temp );
              temp = walk_expr( gen_BINARY_PLUS_get_rvalue2( expr_tree), level, selection );	
	      break;
      case GEN_BINARY_MINUS:
              temp = walk_expr( gen_BINARY_MINUS_get_rvalue1( expr_tree), level, selection );	
              if  ( temp != (char *) 0 ) 
                return ( temp );
              temp = walk_expr( gen_BINARY_MINUS_get_rvalue2( expr_tree), level, selection );	
	      break;
      default:
              printf("Confused in walk_expr: found node %s\n",
	              gen_node_type_get_text(gen_get_node_type(expr_tree)));
              break;
    }
    return ( temp );
}



/*
 *   w a l k   a s s i g n m e n t   s t a t e m e n t   ( 2. v e r s i o n )
 */

/*----------------------------------------------------------------------
   walk1B_assignment - walk the definitions of "var"          
 ----------------------------------------------------------------------*/

static char 
*walk1B_assignment(AST_INDEX node, int level, AST_INDEX selection)
{
  char           *temp;
  AST_INDEX     lvalue;
  AST_INDEX     rvalue;

  rvalue = gen_ASSIGNMENT_get_rvalue(node);
  lvalue = gen_ASSIGNMENT_get_lvalue(node);

  temp = walk_expr( rvalue, level, selection );
  if  ( temp != (char *) 0 )
    return ( temp );

  /* we are looking for identifiers with subscripts. rich */

  if ( ( ! is_identifier(lvalue) ) 
      && (in_subscript(lvalue) ) )
/*
    if  ( strcmp( gen_get_text( selection ), gen_get_text( lvalue ) ) == 0 )  {
      stop_upex_search = true; 
    }
*/
  return (char *) 0;
}



/*
 *   w a l k   s t a t e m e n t   l i s t
 */

/*----------------------------------------------------------------------
   walk1_stmt_list() - walk the list of statements in original loop
 ----------------------------------------------------------------------*/

static char
*walk1_stmt_list(PedInfo ped, AST_INDEX list, int level, AST_INDEX selection)
{
  char     *temp;
  AST_INDEX curr;

  curr = list_first( list );
  while  ( curr != AST_NIL)  {
    switch (gen_get_node_type(curr))
    {
    	case GEN_ASSIGNMENT:
		temp = walk1B_assignment(curr,level,selection);
		if  ( temp != (char *) 0 )
		  return ( temp );
		break;
    	case GEN_DO:
		return ssave("Sorry, cannot handle nested loops");
        case GEN_ARITHMETIC_IF:
        case GEN_IF:
        case GEN_LOGICAL_IF:
        case GEN_GOTO:
        case GEN_ASSIGNED_GOTO:
        case GEN_COMPUTED_GOTO:
		return ssave("Sorry, cannot handle control flow branches inside the loop");
        case GEN_CALL:
		return ssave("Sorry, cannot handle subroutine calls inside the loop");
        case GEN_PRINT:
        case GEN_WRITE:
        case GEN_READ_SHORT:
        case GEN_READ_LONG:
		return ssave("Sorry, cannot handle I/O inside the loop");
    	case GEN_COMMENT:
		break;
    	case GEN_CONTINUE:
		break;
    	default:
		return ssave("Sorry, cannot handle this loop");
    }

    curr = list_next( curr );
  }
  return (char *) 0;
}



/*
 *   s c a l a r   e x p a n s i o n    p o s s i b l e  ?
 */

char * ar_can_expand(PedInfo ped, AST_INDEX loop, AST_INDEX selection)
/*AST_INDEX   loop;  outermost DO-statement */
{
  int   level;			/* nesting level of DO-loop */
  AST_INDEX   outof_selection;

  /* 
    make sure that the selection is an identifier and that it is
    a scalar - for now this means that the node is not contained
    in a subscript node
    */

  /* we are looking for a subscripted expresssion. rich */

  if (!is_identifier(selection)) 
    return ssave("Only array variables can be expanded.");

  if (!in_subscript(selection))
    return ssave("This does not look like a subscripted variable.\nPlease select an array variable to rename.");

  outof_selection = out(selection);
  while ( (outof_selection != AST_NIL)
	 && (outof_selection != loop) )
    {
      outof_selection = out(outof_selection);
    }
  if (outof_selection != loop)
    return ssave("Selection is not in selected loop.");

  level = 1;			/* for now */

  return ( (char *) walk1_stmt_list( ped, gen_DO_get_stmt_LIST(loop), level, selection ) );

  /*
   *   walk the tree to determine if it contains any nodes that
   *   we do not currently handle 
   *   -- 
   *   also make sure that the node is a normal do loop with
   *   a loop induction variable and that the loop is 
   *   normalized 
   *   --
   *   build the list of upwards exposed uses of "var" 
   */
    
}



/*
 *   r e n a m i n g    p o s s i b l e   a n d   p r o f i t a b l e  ?
 */

int pt_test_renaming(PedInfo ped, AST_INDEX loop, AST_INDEX selection, char **msg)
{
  int          query1,
	       query2;
  int          level;   /* nesting level of DO-statement */
  int         has_anti; /* selection is the source of an anti dependance */
  char        *var;   /* text-string of the scalar variable */
  char	       ar_test_response[256];
  DG_Edge	 *Elist;
  int         reference_list, edge_index;

  *msg = ar_can_expand(ped, loop, selection);
  if ( *msg != (char *) 0)  {  
    return ( CANNOT_CHANGE );
  }

  /* print_upex_use();*/   /* TEST */

  level = 1;			/*  for now */
  var = gen_get_text(selection);
  has_anti = 0;
  Elist = dg_get_edge_structure( PED_DG(ped));
  reference_list = get_info(ped, selection, type_levelv);
  if(reference_list > 0)
    {
      for(edge_index=dg_first_src_ref( PED_DG(ped), reference_list); edge_index >= 0;
	  edge_index=dg_next_src_ref( PED_DG(ped), edge_index)) 

/* changed from: edge_index=dg_next_src_ref( PED_DG(ped), reference_list)) cwt - Apr 90 */

	{
	  if ( Elist[edge_index].type == dg_anti )
	      has_anti = 1;
	}
    }

  /*
   *  Test whether scalar expansion is profitable or not
   */
  query1 = re_handle_scexpnd_query1( ped, loop, level, selection ); 
  query2 = re_handle_scexpnd_query2( ped, loop, level, selection );

  if  ( has_anti == 0 )
    {
      sprintf(ar_test_response, "%s is not the source for an antidependence.\n\
This version of array renaming is profitable\n\
only for breaking antidependences. \n", var);
      *msg = ssave (ar_test_response);
    }
  else
    {
      sprintf(ar_test_response, "%s is the source for an antidependence.\n\
Array renaming is profitable for\n\
breaking this antidependence. \n\
        %d more SCRs\n\
        %d more parallel statements", var, query1, query2 );
      *msg = ssave (ar_test_response);
    }
  return ( CAN_CHANGE );
}



/*
 *    p e r f o r m   r e n a m i n g
 */
void pt_rename(PedInfo ped, AST_INDEX loop, AST_INDEX selection)
{
  char        *var;		/* text-string of the scalar variable */
  AST_INDEX  inductive;
  AST_INDEX   zero,dim, decl_len, lb_expr, ub_expr, ub_expr_list,
  prolog_stmt, epilog_stmt, n, s, unit, newdecl, decllist, declpos;
  char        *newvar;
  int	      i;
  DG_Instance	*dg	= PED_DG(ped);
  LI_Instance	*li	= PED_LI(ped);

  var = gen_get_text(selection);
  newvar = (char *) get_mem (sizeof (char *) * (strlen (var) + 1), "pt_rename");
  for (i = 0; i < (int)strlen(var); i++)
    newvar[i] = var[i];
  newvar[strlen(var)] = '$';
  newvar[strlen(var) + 1] = '\0';
  inductive = gen_DO_get_control(loop);

  create_prolog_stmt = true;
  create_epilog_stmt = false;

  zero = gen_CONSTANT();
  gen_put_text(zero, "0", STR_CONSTANT_INTEGER);
  n = gen_IDENTIFIER();
  gen_put_text(n, newvar, STR_IDENTIFIER);
  s = gen_IDENTIFIER();
  gen_put_text(s, var, STR_IDENTIFIER);
  lb_expr = tree_copy( gen_INDUCTIVE_get_rvalue1(inductive) );
  ub_expr = tree_copy( gen_INDUCTIVE_get_rvalue2(inductive) );
  ub_expr_list = list_create( ub_expr );

  /* n = new identifier. s = old identifier. */
   
  if  ( create_prolog_stmt )  
    {
      /* generate the new assignment statement */
      AST_INDEX   new_subscript, new_identifier, new_assignment;
      AST_INDEX   new2_subscript, new2_identifier;
      AST_INDEX   old_subscript, old_identifier;
      AST_INDEX   old2_subscript, old2_identifier;
      EDGE_INDEX  old_edge, new_edge;
      DG_Edge	 *Elist, *old_dg_edge, *new_dg_edge;
      int	new_lvector;
      int	new_ref;
      int	new2_ref;
      int	old2_ref;
      int	old_ident_ref;

      /* Action of array renaming.
       * Replace
       * 	... use of old( subscript ) ...
       * with
       *	new( i ) = old2( subscript )
       *	... use of new2( i ) ...
       */
      {
	AST_INDEX	list;
	
	old_identifier = selection;
	old_subscript = tree_out (selection);
	old2_subscript = tree_copy (old_subscript);
	old2_identifier = gen_SUBSCRIPT_get_name(old2_subscript);
	new_identifier = tree_copy( n );
	list	= gen_LIST_OF_NODES();
	list	= list_insert_first( list, tree_copy(gen_INDUCTIVE_get_name(inductive)) );
	new_subscript = gen_SUBSCRIPT( new_identifier, list );
	new_assignment = gen_ASSIGNMENT( AST_NIL, new_subscript, old2_subscript);
	(void) list_insert_before( out (selection), new_assignment );
      }
      
      /* Move edges from use of old( )  to use of old2( )	*/
      
      new_lvector	= dg_alloc_level_vector( dg, MAXLOOP );
      new_ref		= dg_alloc_ref_list( dg);
      old2_ref	= dg_alloc_ref_list( dg);
      
      /* This statement must occur before any modifications using 
       * put_info( ped, old2_identifier, ...)
       * since tree_copy did a copy-by-reference of the side-array-info.
       */
      old_ident_ref = get_info(ped, old_identifier, type_levelv);
      
      put_info (ped, new_assignment, type_levelv, new_lvector );
      /* Weird, we put ref information into the type_levelv field for variables */
      put_info (ped, new_identifier, type_levelv, new_ref );
      put_info (ped, old2_identifier, type_levelv, old2_ref );
      
      if(old_ident_ref > 0)
	{
	  for(old_edge=dg_first_sink_ref( dg, old_ident_ref); old_edge >= 0;
	      old_edge=dg_first_sink_ref( dg, old_ident_ref))
	    {
	      Elist = dg_get_edge_structure( dg);
	      old_dg_edge = &Elist[old_edge];
	      new_edge = dg_alloc_edge( dg, &Elist);
	      new_dg_edge = &Elist[new_edge];
	      new_dg_edge->src = old_dg_edge->src;
	      new_dg_edge->sink = old2_identifier;
	      new_dg_edge->src_str = NULL;
	      new_dg_edge->sink_str = NULL;
	      new_dg_edge->type = old_dg_edge->type;
	      new_dg_edge->level = old_dg_edge->level;
	      new_dg_edge->src_vec = old_dg_edge->src_vec;
	      new_dg_edge->sink_vec = new_lvector;
	      new_dg_edge->src_ref = old_dg_edge->src_ref;
	      new_dg_edge->sink_ref = old2_ref;
	      new_dg_edge->ic_preventing = old_dg_edge->ic_preventing;
	      new_dg_edge->ic_sensitive = old_dg_edge->ic_sensitive;
	      new_dg_edge->used = old_dg_edge->used;
	      
	      dg_add_edge( dg, new_edge);
	      dg_delete_free_edge( dg, old_edge);	/* 910604, mpal */
	      /*	     dg_free_edge( dg, Elist, old_edge); */
	    }
	  
	  for(old_edge=dg_first_src_ref( dg, old_ident_ref); old_edge >= 0;
	      old_edge=dg_first_src_ref( dg, old_ident_ref))
	    {
	      Elist = dg_get_edge_structure( dg);
	      old_dg_edge = &Elist[old_edge];
	      new_edge = dg_alloc_edge( dg, &Elist);
	      new_dg_edge = &Elist[new_edge];
	      new_dg_edge->src = old2_identifier;
	      new_dg_edge->sink = old_dg_edge->sink;
	      new_dg_edge->src_str = NULL;
	      new_dg_edge->sink_str = NULL;
	      new_dg_edge->type = old_dg_edge->type;
	      new_dg_edge->level = old_dg_edge->level;
	      new_dg_edge->src_vec = new_lvector;
	      new_dg_edge->sink_vec = old_dg_edge->sink_vec;
	      new_dg_edge->src_ref = old2_ref;
	      new_dg_edge->sink_ref = old_dg_edge->sink_ref;
	      new_dg_edge->ic_preventing = old_dg_edge->ic_preventing;
	      new_dg_edge->ic_sensitive = old_dg_edge->ic_sensitive;
	      new_dg_edge->used = old_dg_edge->used;
	      
	      dg_add_edge( dg, new_edge);
	      dg_delete_free_edge( dg, old_edge);	/* 910604, mpal */
	      /*	     dg_free_edge( dg, Elist, old_edge); */
	    }
	  
	  /* Replace the selected variable with the new name */
	  
	  new2_identifier	= tree_copy( n );
	  {
	    AST_INDEX	list;

	    list	= gen_LIST_OF_NODES();
	    list	= list_insert_first( list, tree_copy(gen_INDUCTIVE_get_name(inductive)) );
	    new2_subscript = gen_SUBSCRIPT( new2_identifier, list );
	  }
	  new2_ref	= dg_alloc_ref_list( dg);
	  put_info (ped, new2_identifier, type_levelv, new2_ref );
	  
	  (void) tree_replace (old_subscript, new2_subscript);
	  /*      (void) tree_free (old_subscript); */
	  
	  Elist	= dg_get_edge_structure( dg);
	  new_edge = dg_alloc_edge( dg, &Elist);
	  new_dg_edge = &Elist[new_edge];
	  new_dg_edge->src = new_identifier;
	  new_dg_edge->sink = new2_identifier;
	  new_dg_edge->src_str = NULL;
	  new_dg_edge->sink_str = NULL;
	  new_dg_edge->type = dg_true;
	  new_dg_edge->level = LOOP_INDEPENDENT;	/* loop independant edge */
	  
	  new_dg_edge->src_vec = new_lvector;
	  new_dg_edge->sink_vec= get_info (ped, out (new2_identifier), type_levelv);
	  new_dg_edge->src_ref = new_ref;
	  new_dg_edge->sink_ref= new2_ref;
	  
	  new_dg_edge->ic_preventing = false;
	  new_dg_edge->ic_sensitive = false;
	  new_dg_edge->used = true;
	  
	  dg_add_edge( dg, new_edge);
	  
	}
    }
  
  if  ( create_epilog_stmt )
    {
      epilog_stmt = gen_ASSIGNMENT( AST_NIL, tree_copy( s ),
				   gen_SUBSCRIPT( tree_copy( n ), ub_expr_list ) );
      (void) list_insert_after( loop, epilog_stmt );
    }
  
  /* update the shared variable list */
  if  ( create_epilog_stmt && create_prolog_stmt )
    el_change_shared_var_name( li, loop, var, newvar, prolog_stmt, epilog_stmt, 1);
  else  
    {
      if  ( create_prolog_stmt )
	el_add_shared_var( li, loop, el_create_new_node(newvar, AST_NIL, AST_NIL, var_shared, "", 1));
      if  ( create_epilog_stmt )
	el_change_shared_var_name( li, loop, var, newvar, AST_NIL, epilog_stmt, 1);
    }
  
  /* generate declaration statement */
  
  {
    AST_INDEX	list, dim, decl_len, type;
    /* dim = gen_DIM(tree_copy(zero), tree_copy(ub_expr) ); */
    /* Should also check to see if loop is reversed, and correctly create dim(s)	*/
    
    dim	= gen_DIM(tree_copy(lb_expr), tree_copy(ub_expr) );			/* mpal, 910627	*/
    dim	= list_create( dim );
    decl_len	= gen_ARRAY_DECL_LEN( tree_copy(n), AST_NIL, dim, AST_NIL );
    decl_len	= list_create( decl_len );
    newdecl	= gen_DIMENSION( AST_NIL, decl_len );
    ft_SetComma( newdecl, false );		/* remove optional comma mpal:910712	*/
  }
  
  unit = find_scope( loop );
  switch  ( gen_get_node_type( unit ) )
    {
    case  GEN_PROGRAM:
      decllist = gen_PROGRAM_get_stmt_LIST( unit );            
      declpos = list_first( decllist );
      list_insert_before(declpos, tree_copy( newdecl ) );
      break;
    case  GEN_SUBROUTINE:
      decllist = gen_SUBROUTINE_get_stmt_LIST( unit );            
      declpos = list_first( decllist );
      list_insert_before(declpos, tree_copy( newdecl ) );
      break;
    case  GEN_FUNCTION:
      decllist = gen_FUNCTION_get_stmt_LIST( unit );            
      declpos = list_first( decllist );
      list_insert_before(declpos, tree_copy( newdecl ) );
      break;
    default:
      printf("cannot handle program unit\n");
      break;
    }

/*  sfree (newvar); */
} /* end_pt_rename */


/******************************************************************************/
/* all that is changed here from distrib.c is that we pass
selection rather than var, so that we can determine which
instance of the array was selected for renaming. rich */

static void re_build_do_nodes(PedInfo ped, Adj_List *adj_list, AST_INDEX node,
                              int level)
{
    AST_INDEX 	do_node;       
	   
    if (!is_do(node)) 
       return;
    do_node = AST_NIL;
    node = list_first(gen_DO_get_stmt_LIST(node));
    while(node != AST_NIL)
    {
       do_node = AST_NIL;
       if (is_do(node)) do_node = node;
       re_build_stmt_nodes(ped,adj_list,node,level,do_node);
       node = list_next(node);
    }
}

static void re_build_do_nodes2(PedInfo ped, Adj_List *adj_list, AST_INDEX node,
                               int level, AST_INDEX do_node)
{
    if (!is_do(node))  return;


    node = list_first(gen_DO_get_stmt_LIST(node));
    while(node != AST_NIL)
    {
       re_build_stmt_nodes(ped,adj_list,node,level,do_node);
       node = list_next(node);
    }
}

void re_build_stmt_nodes(PedInfo ped, Adj_List *adj_list, AST_INDEX node,
                         int level, AST_INDEX do_node)
{
    int         type;
   
   type = gen_get_node_type(node);
   switch(type)
   {
      case  GEN_COMMENT     : return;
      case  GEN_CONTINUE    :  return;
      case  GEN_STOP        :  return;
      default               :  break;
   }

    
    if (is_do(node))
       re_build_do_nodes2(ped,adj_list,node,level,do_node);
    else
       (void) pt_allocnode(adj_list,node,do_node);
}

/* static void re_build_stmt_edges(); */

void re_build_do_edges(PedInfo ped, Adj_List *adj_list, AST_INDEX node, int level, 
                       int depth)
{
    if (!is_do(node))  return;

    node = list_first(gen_DO_get_stmt_LIST(node));
    while(node != AST_NIL)
    {
       re_build_stmt_edges(ped,adj_list,node,level,depth);
       node = list_next(node);
    }
}

static void re_build_stmt_edges(PedInfo ped, Adj_List *adj_list, AST_INDEX node, 
                                int level, int depth)
{
    if (is_do(node))
       re_build_do_edges(ped,adj_list,node,level,++depth);
    else
       re_add_stmt_edges(ped,adj_list,node,level,depth);
}

void re_add_stmt_edges(PedInfo ped, Adj_List *adj_list, AST_INDEX node, 
                       int carry_level, int depth)
{
   int vector;
   int edge;
   DG_Edge *edgeptr;
   AST_INDEX sink_node;
   int level;
   Boolean carried;
   int type;
   
   type = gen_get_node_type(node);
   switch(type)
   {
      case  GEN_COMMENT     : return;
      case  GEN_CONTINUE    :  return;
      case  GEN_STOP        :  return;
      default               :  break;
   }
   
   vector  = get_info(ped, node, type_levelv);
   edgeptr = dg_get_edge_structure( PED_DG(ped));

   /*
    *  Get edgeptr                        
    */
   
   level = LOOP_INDEPENDENT;
   while (level <= depth)
   {
       if ((level > 0 ) && (level < carry_level)) 
       {
	  level++;
	  continue;
       }

       edge = dg_first_src_stmt( PED_DG(ped),vector,level);
       while(edge != NIL)
       {
          sink_node = edgeptr[edge].sink;
	  if (!is_statement(sink_node))
	       sink_node = out(sink_node);

	  if (level == carry_level || level == 0)	/* SCALAR */
	     carried = true;
	  else 
	     carried = false;
	  pt_add_edge(adj_list, node , sink_node , carried);
          edge = dg_next_src_stmt( PED_DG(ped),edge);
       }
       level++;
    }
}

   


static void re_build_scexpnd_edges (PedInfo ped, Adj_List *adj_list, AST_INDEX node,
				    int level, int depth, AST_INDEX selection)
{
    if (!is_do(node))  return;

    node = list_first(gen_DO_get_stmt_LIST(node));
    while(node != AST_NIL)
    {
       re_build_stmt_edges2(ped,adj_list,node,level,depth,selection);
       node = list_next(node);
    }
}

void re_build_stmt_edges2(PedInfo ped, Adj_List *adj_list, AST_INDEX node, 
			  int level, int depth, AST_INDEX selection)
{
    if (is_do(node))
       re_build_scexpnd_edges(ped,adj_list,node,level,++depth, selection);
    else
       re_add_stmt_edges2(ped,adj_list,node,level,depth, selection);
}

/* fixed */
void re_add_stmt_edges2(PedInfo ped, Adj_List *adj_list, AST_INDEX node, 
                        int carry_level, int depth, AST_INDEX selection)
{
   int vector;
   int edge;
   DG_Edge *edgeptr;
   AST_INDEX sink_node;
   int level;
   Boolean carried;
   int type;
   
   type = gen_get_node_type(node);
   switch(type)
   {
      case  GEN_COMMENT     : return;
      case  GEN_CONTINUE    :  return;
      case  GEN_STOP        :  return;
      default               :  break;
   }
   
   vector  = get_info(ped, node, type_levelv);
   edgeptr = dg_get_edge_structure( PED_DG(ped));

   /*
    *  Get edgeptr                        
    */
   
   level = LOOP_INDEPENDENT;
   while (level <= depth)
   {
       if ((level > 0 ) && (level < carry_level)) 
       {
	  level++;
	  continue;
       }

       edge = dg_first_src_stmt( PED_DG(ped),vector,level);
       while(edge != NIL)
       {
          if  (! ar_removable_edge( ped, edge, selection ))  {
            sink_node = edgeptr[edge].sink;
	    if (!is_statement(sink_node))
	         sink_node = out(sink_node);

	    if (level == carry_level || level == 0)	/* SCALAR */
	       carried = true;
	    else 
	     carried = false;
	    pt_add_edge(adj_list, node , sink_node , carried);
	   }
            edge = dg_next_src_stmt( PED_DG(ped),edge);
       }
       level++;
    }
}

int  re_count_par_stmts(Adj_List *adj_list)
{
   int i, count , stmt;

   count = 0;
   for(i=0;i<=adj_list->max_region;i++)
   {
      if (adj_list ->region_array[i].parallel == true)
      {
          for( stmt = adj_list -> region_array[i].first_stmt;stmt != NIL;
	       stmt = adj_list-> node_array[stmt].rlink)
	     count++;
      }
   }
   return (count);
}
	 

Adj_List *re_make_adj_list(PedInfo ped, AST_INDEX node, int level)
{
    Adj_List *adj_list;
    adj_list = pt_create_adj_list(); 
    re_build_do_nodes(ped,adj_list,node,level);
    re_build_do_edges(ped,adj_list,node,level,level);
    pt_strong_regions(adj_list);
    return(adj_list);
}

static int re_handle_scexpnd_query1 (PedInfo ped, AST_INDEX node, int level, 
				     AST_INDEX selection)
{
   Adj_List     *adj_list;
   int		new_no_of_regions;
   int		old_no_of_regions;

   
   adj_list = re_make_adj_list(ped,node,level);   
   old_no_of_regions = (adj_list -> max_region) + 1;   
   pt_destroy_adj_list(adj_list);  
   adj_list = pt_create_adj_list();
   re_build_do_nodes(ped,adj_list,node,level);       
   re_build_scexpnd_edges(ped,adj_list,node,level,level,selection);
   pt_strong_regions(adj_list);
   new_no_of_regions = (adj_list -> max_region) + 1;
   pt_destroy_adj_list(adj_list);
   return (new_no_of_regions - old_no_of_regions);
}


/* Routine to be calleld by scalar expansion routines
 * to check if scalar expansion helps in reducing the
 * no of strongly connected region
 */
static int re_handle_scexpnd_query2 (PedInfo ped, AST_INDEX node, int level, 
				     AST_INDEX selection)
{
   Adj_List     *adj_list;
   int          old_no_of_par_stmts;
   int   	new_no_of_par_stmts;
   
   adj_list = re_make_adj_list(ped,node,level);   
   old_no_of_par_stmts = re_count_par_stmts(adj_list);
   pt_destroy_adj_list(adj_list);  
   adj_list = pt_create_adj_list();
   re_build_do_nodes(ped,adj_list,node,level);       
   re_build_scexpnd_edges(ped,adj_list,node,level,level,selection);
   pt_strong_regions(adj_list);
   new_no_of_par_stmts = re_count_par_stmts(adj_list);
   pt_destroy_adj_list(adj_list);
   return (new_no_of_par_stmts - old_no_of_par_stmts);
}
