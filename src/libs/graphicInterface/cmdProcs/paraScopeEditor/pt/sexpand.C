/* $Id: sexpand.C,v 1.1 1997/06/25 13:52:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*

        s c a l a r   e x p a n s i o n   version 17.06.88


        restrictions:

        (1)   only one single DO-loops
        (2)   no IF, BLOCK-IF, GOTO statements, no
              function or procedure calls in the bodies
              of the loops;
	      no check if function call occurs
	      inside an index expression of a subscipted variable
        (3)   the upper bounds have constant values greater than
              0 ( loops are normalized )
        (4)   the scalar variable A which is the candidate for expansion
              must not occur in a subscript of an array variable; this
	      condition is not checked; 
	      this condition might be to strong, e.g.:
		   
		     A = 
		     C(i) = C(i-1) + D(A)
		       = A 

        (5)   there is no KILL-information available for true-, output-,
	      or anti-dependences



        notation:

         L:|   DO I=1, <expr>
           |   
           |     ...
           |                    
           |   ENDDO


        Let A denote the scalar variable as the candidate for 
        expansion.

        DEF(A) :  statement in L where A is defined
        USE(A) :  statement in L where A is used


        note: (2) ==>  (i)  each DEF(A) is a MUSTDEF
                       (ii) each DEF(A) is an assignment statement

                                                Uli, June 88 

*/

#include <assert.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pt/private_pt.h>

#ifdef PARSE
  char str[60];
  FILE *parse;
#endif
 
#define PRINTERRORS(the_error)  fprintf( parse, "%s\n", the_error ); fflush( parse );

/*
 * MAX_UPEX_USE  is the maximal number of detectable upwards exposed
 * USEs of the variable "var"
 */
#define MAX_UPEX_USE   20
#define LOOP_CARRIED    1
#define LOOP_INDEP      2


/*   e x p o r t s   */

int se_test_scalar_expand(); /* determines profitability ( 1. P H A S E ) */
Boolean se_removable_edge(); /* checks whether dependence edge can be removed by scalar expansion */

void se_scalar_expand();     /* performs scalar expansion ( 2. P H A S E ) */


/*   l o c a l   r o u t i n e s   */

STATIC(void, init_upex_use,(void));
STATIC(void, insert_upex_use,(AST_INDEX use));
STATIC(Boolean, is_upex_use,(AST_INDEX use));
STATIC(void, print_upex_use,(void));

STATIC(Boolean, in_subscipt,(AST_INDEX node));
STATIC(void, check_backward_true_dep,(PedInfo ped));
STATIC(char, *walk1_stmt_list,(AST_INDEX list, int level, char *var));         
STATIC(void, walk2_stmt_list,(PedInfo ped, AST_INDEX list, int level, char *var,
                              char *newvar, char *indvar));         
STATIC(void, walk1A_assignment,(PedInfo ped, AST_INDEX list, int level, char *var));
STATIC(char, *walk1B_assignment,(AST_INDEX list, int level, char *var));
STATIC(void, walk2_assignment,(PedInfo ped, AST_INDEX node, char *var, char *newvar,
                               char *indvar));
STATIC(char, *walk_expr,(AST_INDEX expr_tree, int level, char *var));
STATIC(void, expand,(PedInfo ped, AST_INDEX def, char *newvar, char *indvar));
STATIC(void, remove_lc_edge,(PedInfo ped, EDGE_INDEX edge));
STATIC(void, remove_anti,(PedInfo ped, AST_INDEX use, short carried_indep));


/*   l o c a l  o b j e c t s   */

static Boolean create_prolog_stmt; /* flag, whether a prolog statement */  
                                   /* has to be generated or not       */
static Boolean create_epilog_stmt; /* flag, whether a epilog statement */  
                                   /* has to be generated or not       */
 
/* ---------------------------------------------------------------------
 * upex_USE implements a list of AST_INDEXs of the upwards exposed
 * USEs of the variable "var" in the DO-loop "loop".
 * --------------------------------------------------------------------*/
static AST_INDEX  upex_USE[MAX_UPEX_USE]; /* array of AST_INDEXs of upward */
                                          /* exposed USE of "var"          */
static int  upex_head; /* pointer to the next free entry in upex_USE */
static Boolean stop_upex_search; /* stops the search for upwards exposed   */
				 /* uses of a variable, if a definition of */
				 /* the variable is encountered            */



/*
 *   c h e c k s   w h e t h e r  d e p e n d e n c e   c a n   b e   r e m o v e d   
 */

Boolean
se_removable_edge(PedInfo ped, EDGE_INDEX edge, char *var)
{
  DG_Edge   *edgeptr;
  AST_INDEX  src;

  if  ( edge == END_OF_LIST )
    printf("invalid edge\n");
  else  {
    edgeptr = dg_get_edge_structure( PED_DG(ped));
    src = edgeptr[edge].src;
    if  ( strcmp( var, gen_get_text( src ) ) != 0 )
      return( false );  /* strings are not equal */
    else  
      switch  ( edgeptr[edge].type )
      {
        case dg_anti:
            if  ( is_upex_use( src ) )  
              /* all loop independent edges can be removed and              */
              /* there cannot be any loop carried anti dependences;         */
              /* this fact is not detected by the dependence analysis since */
              /* KILL-analysis is not performed for anti-dependences        */
              return( true );  /* loop independent edge */
            else  {  /* no upwards exposed use */
              /* all loop carried dependences can be removed */
              if  ( edgeptr[edge].level > 0 )  
                return( true );
              else
                return( false ); 
            }

        case dg_output:
            /* all loop carried dependences can be removed */
            if  ( edgeptr[edge].level > 0 )  
              return( true );
            else
              return( false ); 
      } /*switch*/
  } /*else*/

return( false );

}



/*
 *   l i s t  o f  u p w a r d s   e x p o s e d   u s e s
 */

static void
init_upex_use()
{
  int i;

  for  ( i = 0; i < MAX_UPEX_USE ; i++ )  
    upex_USE[i] = -1;
  upex_head = 0;
}


static void 
insert_upex_use(AST_INDEX use)
{
  if  ( gen_get_node_type( use ) != GEN_IDENTIFIER )
    printf(" confused in insert_upex_use; cannot insert %s\n", gen_get_text( use  ) );
  else  
    if  ( upex_head >= MAX_UPEX_USE )
      printf(" too many upwards exposed uses\n");
    else  {
      upex_USE[upex_head] = use;
      upex_head++;
    }
}


static Boolean
is_upex_use(AST_INDEX use)
{
  int i;

  for  ( i = 0; ((i < MAX_UPEX_USE) && (upex_USE[i] != -1)) ; i++ )  
    if  ( upex_USE[i] == use )
      return( true );
  return( false );
}

 
static void 
print_upex_use()
{
  int i;

  for  ( i = 0; ((i < MAX_UPEX_USE) && (upex_USE[i] != -1)) ; i++ ) { 
    printf("upex No %d: %s ", i, 
           gen_node_type_get_text( gen_get_node_type( upex_USE[i] ) ) );
    if  ( gen_get_node_type( upex_USE[i] ) == GEN_IDENTIFIER )
      printf("  %s\n", gen_get_text( upex_USE[i] ) );
  }
  printf("\n--------------------------------------\n");
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


#ifdef UNUSED_CODE

/*
 *   c o m p u t a t i o n   o f   U P E X - U S E S   u s i n g   t h e   c h a i n s
 */

/* This is only called from walk1_assignment, which is
   not called by anyone, currently. If this becomes
   a called routine, el_get_ref_ptr requires a pointer
   to ped->levelv_side_array.

   el_get_ref_ptr is no longer used.  levelv_side_array no longer exists
		- Tim Mullin  June, 1990
*/
static void
check_backward_true_dep(PedInfo ped)
{

  EDGE_INDEX  edge;
  DG_Edge     *edgeptr;
  int         src_ref; 
  
  edgeptr = dg_get_edge_structure( PED_DG(ped) );

/* ADD back */
/* src_ref = get_info( ped, def, type_levelv ); */

  edge = dg_first_src_ref( PED_DG(ped), src_ref );

  while (edge != END_OF_LIST)
  {
    if ( (edgeptr[edge].level > 0) && (edgeptr[edge].type == dg_true) )  
      /* loop carried backward true dependence */
      insert_upex_use( edgeptr[edge].sink );
    edge = dg_next_src_ref( PED_DG(ped),edge);
  }
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


/*----------------------------------------------------------------------
   walk1A_assignment - walk the definitions of "var"          
 ----------------------------------------------------------------------*/

static void
walk1A_assignment(PedInfo ped, AST_INDEX node, int level, char *var)
{
  AST_INDEX     lvalue;

/*
ifdef PARSE
  sprintf(str, "%s", "procedure \"walk1A_assignment\""); 
  PRINTERRORS(str);
endif
*/
  lvalue = gen_ASSIGNMENT_get_lvalue(node);

  if (!is_identifier(lvalue)) {
      return;
  }
  
  if  ( strcmp( var, gen_get_text( lvalue ) ) == 0 )  {
      /* strings are equal */
      check_backward_true_dep( ped);
  }
}

#endif


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
*walk_expr(AST_INDEX expr_tree, int level, char *var)
{

  char *temp;

#ifdef PARSE
  sprintf(str, "%s", "procedure \"walk_expr\""); 
  PRINTERRORS(str);
#endif

    temp = (char *) 0;
    switch (gen_get_node_type(expr_tree))
    {
      case GEN_INVOCATION:
              return ssave("Sorry, cannot handle function calls inside the loop");
      case GEN_IDENTIFIER:
	      if  ( !stop_upex_search )  
                if  ( strcmp( var, gen_get_text( expr_tree ) ) == 0 )  
		  /* strings are equal */
		  insert_upex_use( expr_tree );
	      break;
      case GEN_SUBSCRIPT:
	      /* no check whether "var" occurs in that subscipt */
	      break;
      case GEN_CONSTANT:
	      break;
      case GEN_UNARY_MINUS:
              temp = walk_expr( gen_UNARY_MINUS_get_rvalue( expr_tree ), level, var );	
              break;
      case GEN_BINARY_EXPONENT:
              temp = walk_expr( gen_BINARY_EXPONENT_get_rvalue1( expr_tree), level, var );	
              if  ( temp != (char *) 0 ) 
                return ( temp );
              temp = walk_expr( gen_BINARY_EXPONENT_get_rvalue2( expr_tree), level, var );	
              break;
      case GEN_BINARY_TIMES:
              temp = walk_expr( gen_BINARY_TIMES_get_rvalue1( expr_tree), level, var );	
              if  ( temp != (char *) 0 ) 
                return ( temp );
              temp = walk_expr( gen_BINARY_TIMES_get_rvalue2( expr_tree), level, var );	
	      break;
      case GEN_BINARY_DIVIDE:
              temp = walk_expr( gen_BINARY_DIVIDE_get_rvalue1( expr_tree), level, var );	
              if  ( temp != (char *) 0 ) 
                return ( temp );
              temp = walk_expr( gen_BINARY_DIVIDE_get_rvalue2( expr_tree), level, var );	
	      break;
      case GEN_BINARY_PLUS:
              temp = walk_expr( gen_BINARY_PLUS_get_rvalue1( expr_tree), level, var );	
              if  ( temp != (char *) 0 ) 
                return ( temp );
              temp = walk_expr( gen_BINARY_PLUS_get_rvalue2( expr_tree), level, var );	
	      break;
      case GEN_BINARY_MINUS:
              temp = walk_expr( gen_BINARY_MINUS_get_rvalue1( expr_tree), level, var );	
              if  ( temp != (char *) 0 ) 
                return ( temp );
              temp = walk_expr( gen_BINARY_MINUS_get_rvalue2( expr_tree), level, var );	
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
*walk1B_assignment(AST_INDEX node, int level, char *var)
{
  char           *temp;
  AST_INDEX     lvalue;
  AST_INDEX     rvalue;

#ifdef PARSE
  sprintf(str, "%s", "procedure \"walk1B_assignment\""); 
  PRINTERRORS(str);
#endif

  rvalue = gen_ASSIGNMENT_get_rvalue(node);
  lvalue = gen_ASSIGNMENT_get_lvalue(node);

  temp = walk_expr( rvalue, level, var );
  if  ( temp != (char *) 0 )
    return ( temp );

  if ( is_identifier(lvalue) ) 
    if  ( strcmp( var, gen_get_text( lvalue ) ) == 0 )  {
        /* strings are equal (MUSTKILL) */
        stop_upex_search = true; 
    }
  return (char *) 0;
}



/*
 *   w a l k   s t a t e m e n t   l i s t
 */

/*----------------------------------------------------------------------
   walk1_stmt_list() - walk the list of statements in original loop
 ----------------------------------------------------------------------*/

static char
*walk1_stmt_list(AST_INDEX list, int level, char *var)
{
  char     *temp;
  AST_INDEX curr;

#ifdef PARSE
  sprintf(str, "%s", "procedure \"walk1_stmt_list\""); 
  PRINTERRORS(str);
#endif

  curr = list_first( list );
  while  ( curr != AST_NIL)  {
    switch (gen_get_node_type(curr))
    {
    	case GEN_ASSIGNMENT:
             /*	walk1A_assignment(ped,curr,level,var); */
		temp = walk1B_assignment(curr,level,var);
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

char * se_can_expand(PedInfo ped, AST_INDEX loop, AST_INDEX selection)
/* loop: outermost DO-statement */
{
  char *indvar; /* induction variable */
  char *var;
  int   level; /* nesting level of DO-loop */
  AST_INDEX   outof_selection;

  /*   Make sure that the loop is in normal form.
   */
   if (!pt_loop_is_normal(loop))
     return ssave("Sorry, loop has to be in normal form.");

  /* 
   make sure that the selection is an identifier and that it is
   a scalar - for now this means that the node is not contained
   in a subscript node
   */

   if (!is_identifier(selection)) 
        return ssave("Only scalar variables can be expanded.");

   if (in_subscript(selection))
        return ssave("You cannot expand a subscripted variable.\nOnly scalar variables can be expanded.");

  outof_selection = out(selection);
  while ( (outof_selection != AST_NIL)
	 && (outof_selection != loop) )
    {
      outof_selection = out(outof_selection);
    }
  if (outof_selection != loop)
    return ssave("Selection is not in selected loop.");

   indvar = gen_get_text( gen_INDUCTIVE_get_name( gen_DO_get_control( loop ) ) );
   var = gen_get_text( selection );
   if  ( strcmp(var, indvar) == 0 )  
       /* strings are equal */
        return ssave("You cannot expand an induction variable.\nOnly scalar variables can be expanded.");
  
   level = 1;   /* for now */

   init_upex_use();
   stop_upex_search = false;
  /*
   *   walk the tree to determine if it contains any nodes that
   *   we do not currently handle 
   */
   return ( (char *) walk1_stmt_list( gen_DO_get_stmt_LIST(loop), level, var ) );

  /*   build the list of upwards exposed uses of "var" 
   */
    
}



/*
 *   s c a l a r   e x p a n s i o n    p o s s i b l e   a n d   p r o f i t a b l e  ?
 */

int se_test_scalar_expand(PedInfo ped, AST_INDEX loop, AST_INDEX selection, 
                          char **msg)
/* loop: outermost DO-statement */
{
  int          level;   /* nesting level of DO-statement */
  int          query1,
	       query2;
  char        *var;   /* text-string of the scalar variable */
  char	       se_test_response[100];
  
  *msg = se_can_expand(ped, loop, selection);
  if ( *msg != (char *) 0)  {  
    return ( CANNOT_CHANGE );
  }

  /* print_upex_use();*/   /* TEST */

  level = 1;			/*  for now */
  var = gen_get_text(selection);

  /*
   *  Test whether scalar expansion is profitable or not
   */
  query1 = pt_handle_scexpnd_query1( ped, loop, level, var ); 
  query2 = pt_handle_scexpnd_query2( ped, loop, level, var );

  if  ( query1 == 0  &&  query2 == 0 )
    *msg = ssave ("Scalar expansion is not profitable. \n");
  else
  {
  	sprintf(se_test_response, "Scalar expansion is profitable : \n        %d more SCRs\n        %d more parallel statements",
          query1, query2 ); 
  	*msg = ssave (se_test_response);
  }
  return ( CAN_CHANGE );
}



/*
 *    p e r f o r m   s c a l a r   e x p a n s i o n
 */

void se_scalar_expand(PedInfo ped, AST_INDEX loop, AST_INDEX selection)
	/* loop: outermost DO-statement */
{
  int        level;   /* nesting level of DO-statement */
  char        *var;   /* text-string of the scalar variable */
  AST_INDEX  inductive;
  char        *indvar;
  AST_INDEX   zero, dim_list, decl_len, decl_len_list,
              zero_list, ub_expr, ub_expr_list,
              prolog_stmt, epilog_stmt, n, s, unit, newdecl, decllist, declpos;
  char        *newvar;
  unsigned int	i;

#ifdef PARSE
  if  (( parse = fopen("parsefile", "w+") ) == NULL )  {
    printf("cannot open file \"parsefile\"");
    return;
  }
#endif

#ifdef PARSE
  sprintf(str, "%s", "procedure \"se_scalar_expand\""); 
  PRINTERRORS(str);
#endif


   level = 1;			/*  for now */
   var = gen_get_text(selection);
   newvar = (char *) get_mem (sizeof (char *) * (strlen (var) + 2),
                              "se_scalar_expand");
   for (i = 0; i < strlen(var); i++)
   	newvar[i] = var[i];
   newvar[strlen(var)] = '$';
   newvar[strlen(var) + 1] = '\0';
   inductive = gen_DO_get_control(loop);
   indvar  = gen_get_text(gen_INDUCTIVE_get_name(inductive));

   create_prolog_stmt = false;
   create_epilog_stmt = false;

   walk2_stmt_list(ped, gen_DO_get_stmt_LIST(loop), level, var, newvar, indvar);

   zero = gen_CONSTANT();
   gen_put_text(zero, "0", STR_CONSTANT_INTEGER);
   zero_list = list_create( zero ); 
   n = gen_IDENTIFIER();
   gen_put_text(n, newvar, STR_IDENTIFIER);
   s = gen_IDENTIFIER();
   gen_put_text(s, var, STR_IDENTIFIER);
   assert(pt_loop_is_normal(loop));
   ub_expr = tree_copy( gen_INDUCTIVE_get_rvalue2(inductive) );
   ub_expr_list = list_create( ub_expr );
   
   if  ( create_prolog_stmt )  
   {
     prolog_stmt = gen_ASSIGNMENT( AST_NIL, gen_SUBSCRIPT( tree_copy( n ), tree_copy( zero_list ) ),
                                   tree_copy( s ) );
     (void) list_insert_before( loop, prolog_stmt );
   }

   if  ( create_epilog_stmt )  
   {
     epilog_stmt = gen_ASSIGNMENT( AST_NIL, tree_copy( s ),
                                   gen_SUBSCRIPT( tree_copy( n ), ub_expr_list ) );
     (void) list_insert_after( loop, epilog_stmt );
   }
 
   /* update the shared variable list */
   if  ( create_epilog_stmt && create_prolog_stmt )
     el_change_shared_var_name( PED_LI(ped), loop, var, newvar, prolog_stmt, epilog_stmt, 1);
   else  {
     if  ( create_prolog_stmt )
       el_change_shared_var_name( PED_LI(ped), loop, var, newvar, prolog_stmt, AST_NIL, 1);
     if  ( create_epilog_stmt )
       el_change_shared_var_name( PED_LI(ped), loop, var, newvar, AST_NIL, epilog_stmt, 1);
   }
   
   /* generate declaration statement */ 
   dim_list = list_create( gen_DIM(tree_copy(zero), tree_copy(ub_expr)) );
   decl_len = gen_ARRAY_DECL_LEN( tree_copy(n), AST_NIL, dim_list, AST_NIL );
   decl_len_list = list_create(decl_len);
   newdecl = gen_DIMENSION( AST_NIL, decl_len_list ); 
   
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

   sfree (newvar);

#ifdef PARSE
  fclose( parse );
#endif
}

/*----------------------------------------------------------------------
   walk2_stmt_list() - walk the list of statements in original loop
 ----------------------------------------------------------------------*/

static void
walk2_stmt_list(PedInfo ped, AST_INDEX list, int level, char *var, 
                char *newvar, char *indvar)
{
  AST_INDEX curr;

#ifdef PARSE
  sprintf(str, "%s", "procedure \"walk2_stmt_list\""); 
  PRINTERRORS(str);
#endif

  curr = list_first( list );
  while  ( curr != AST_NIL)  {
    switch (gen_get_node_type(curr))
    {
    	case GEN_ASSIGNMENT:
		walk2_assignment(ped, curr, var, newvar, indvar);
		break;
    	case GEN_DO:
		walk2_stmt_list(ped, gen_DO_get_stmt_LIST(curr), level, var, newvar, indvar);
		break;
    	case GEN_COMMENT:
		break;
    	case GEN_CONTINUE:
		break;
    	default:
		printf("Confused in walk2_stmt_list: found node %s\n",
			gen_node_type_get_text(gen_get_node_type(curr)));
		break;
    }

    curr = list_next( curr );
  }
}


/*----------------------------------------------------------------------
   walk2_assignment - walk the tree a statement in original
 ----------------------------------------------------------------------*/

static void
walk2_assignment(PedInfo ped, AST_INDEX node, char *var, char *newvar, 
                 char *indvar)
{
  AST_INDEX     lvalue;

#ifdef PARSE
  sprintf(str, "%s", "procedure \"walk2_assignment\""); 
  PRINTERRORS(str);
#endif

  lvalue = gen_ASSIGNMENT_get_lvalue(node);

  if (!is_identifier(lvalue)) {
      return;
  }
  
  if  ( strcmp( var, gen_get_text( lvalue ) ) == 0 )  {
      /* strings are equal */
      expand( ped, lvalue, newvar, indvar);
  }
}


/*----------------------------------------------------------------------
   expand - expands the definition or use of "var" in the original loop
 ----------------------------------------------------------------------*/

static void
expand(PedInfo ped, AST_INDEX def, char *newvar, char *indvar)
{
    EDGE_INDEX  edge, next_edge;
    DG_Edge     *edgeptr;
    AST_INDEX   i,one,i_minus_1,i_list,i_minus_1_list,placeholder,src,sink,subscript;
    int         src_ref;

#ifdef PARSE
  sprintf(str, "%s%s%s", "procedure \"expand\"", " called for variable ", var); 
  PRINTERRORS(str);
#endif
   
    edgeptr = dg_get_edge_structure( PED_DG(ped));
    
   /*
    * We have a definition. Where are all the uses of this identifier, and 				
    * how should we rewrite those references as subscripts.
    */

    src_ref = get_info(ped, def, type_levelv);

    i = gen_IDENTIFIER();
    
    gen_put_text(i, indvar, STR_IDENTIFIER);
    i_list = list_create(i);

    one = gen_CONSTANT();
    gen_put_text(one, "1", STR_CONSTANT_INTEGER);
    i_minus_1 = gen_BINARY_MINUS(tree_copy(i), one);
    i_minus_1_list = list_create(i_minus_1);

    placeholder = gen_IDENTIFIER();

    /* there is a bug in the processing of the dependence data base.
       there are edges in the edgelist of "def" with a sink in def
    */

    /*
     * walk the edges 
     */
    next_edge = END_OF_LIST; 
    edge  = dg_first_src_ref( PED_DG(ped),src_ref);
    if  ( edge != END_OF_LIST) 
      next_edge = dg_next_src_ref( PED_DG(ped),edge);

    while (edge != END_OF_LIST)
    {
        src  = edgeptr[edge].src;
        sink = edgeptr[edge].sink;

        switch (edgeptr[edge].type)
	{
	    case dg_true:
                create_epilog_stmt = true; /* the definition might reach a definition outside the loop */
                if (  !is_upex_use(sink) )  /* forward - loop independent */
                {
		    /* delete loop-carried forward edges */
		    if  ( edgeptr[edge].level > 0 )  
                      remove_lc_edge( ped, edge); /* HACK deletes loop carried true dependences */
                    else  {

		      /*
		  	A = 	=>    A(I) =	   and break all loop carried anti-deps of sink
		          = A              = A(I)
		       */
		    
		      /* remove loop carried anti-deps of sink */

                      remove_anti( ped, sink, LOOP_CARRIED );
          
                      /* 
                       * hack to overcome error in dependence analysis;
                       * there are multiple entries for the same dependence!
                       */

                      if  (!in_subscript(src))
                      {
                        gen_put_text(src, newvar, STR_IDENTIFIER);  
                        tree_replace(src,placeholder);
		        subscript = gen_SUBSCRIPT(src,tree_copy(i_list));
		        tree_replace(placeholder,subscript);
                      }
                    
                      if  (!in_subscript(sink))
                      {
                        gen_put_text(sink, newvar, STR_IDENTIFIER);  
		        tree_replace(sink,placeholder);
		        subscript = gen_SUBSCRIPT(sink,tree_copy(i_list));
		        tree_replace(placeholder,subscript);
                      }
		    }
		}
		else 		/* backward - loop carried */
		{
		  /* ( is_upex_use( sink ) ) */  
                    create_prolog_stmt = true; /* the upwards exposed use of A */ 
		    /*
		          = A              = A(I-1)
			A = 	=>    A(I) =	   and break all anti-deps of sink
		     */
		     
		    /* remove all anti-deps of sink	 */
                    /* since KILL-informations are not computed there */
                    /* may be loop carried anti-dependences           */


                    remove_anti( ped, sink, LOOP_CARRIED );
                    remove_anti( ped, sink, LOOP_INDEP );           
                    

		    if (!in_subscript(src))
		    {
                       gen_put_text(src, newvar, STR_IDENTIFIER);  
		       tree_replace(src,placeholder);
		       subscript = gen_SUBSCRIPT(src,tree_copy(i_list));
		       tree_replace(placeholder,subscript);
		    }

		    if (!in_subscript(sink))
		    {
                       gen_put_text(sink, newvar, STR_IDENTIFIER);  
		       tree_replace(sink,placeholder);
		       subscript = gen_SUBSCRIPT(sink,tree_copy(i_minus_1_list));
		       tree_replace(placeholder,subscript);
		    }
		}
		break;
	    case dg_output:
                create_epilog_stmt = true; /* the definition might reach a definition outside the loop */
		/*
		    A = 	=>A(I) =
		    A =           A    =

		    and

		    A(I) = 	=>A(I) =
		    A =           A(I) =
		 */
		     
		/* remove loop carried output dep of src */

                remove_lc_edge( ped, edge);

		if (!in_subscript(src))
		{
                    gen_put_text(src, newvar, STR_IDENTIFIER);  
		    tree_replace(src,placeholder);
		    subscript = gen_SUBSCRIPT(src,tree_copy(i_list));
		    tree_replace(placeholder,subscript);
		}
		break;
	    default:
		break;
	}

        edge = next_edge;
        if  ( edge != END_OF_LIST )
          next_edge = dg_next_src_ref( PED_DG(ped),edge);
    }

    /*
     *   free other stuff toooooo 
     */
}


static void remove_lc_edge(PedInfo ped, EDGE_INDEX edge)
{
  DG_Edge     *edgeptr;

  edgeptr = dg_get_edge_structure( PED_DG(ped) );

  if (edge == END_OF_LIST)
  	return;

  if  ( edgeptr[edge].level > 0 )
    dg_delete_free_edge( PED_DG(ped), edge);		/* 910604, mpal */

}


static void remove_anti(PedInfo ped, AST_INDEX use, short carried_indep)
{
  EDGE_INDEX   edge, next_edge;
  DG_Edge     *edgeptr;
  int          src_ref;

  edgeptr = dg_get_edge_structure( PED_DG(ped) );
  src_ref = get_info( ped, use, type_levelv );

  edge = dg_first_src_ref( PED_DG(ped), src_ref );
  next_edge = END_OF_LIST;

  while  ( edge != END_OF_LIST )  
    {
      next_edge = dg_next_src_ref( PED_DG(ped), edge );
      if  ( edgeptr[edge].type == dg_anti )  
	switch  ( carried_indep )
	  {
	  case  LOOP_CARRIED:
	    if  ( edgeptr[edge].level > 0 )
	      dg_delete_free_edge( PED_DG(ped), edge);		/* 910604, mpal */
	      /*	dg_delete_edge( PED_DG(ped), edge);	*/
	    break;
	  case  LOOP_INDEP:
	    /*if  ( is_upex_use(use) || (edgeptr[edge].level == LOOP_INDEPENDENT))*/
	    if  ( edgeptr[edge].level ==  LOOP_INDEPENDENT)
	      dg_delete_free_edge( PED_DG(ped), edge);		/* 910604, mpal */
	      /*	dg_delete_edge( PED_DG(ped), edge);	*/
	    break;
	  default:
	    printf("invalid specification\n");
	    break;
	  }
      edge = next_edge;
    }

/*	This old code was replaced 910603, mpal		*/
/*
 *  if (edge != END_OF_LIST)
 *    next_edge = dg_next_src_ref( PED_DG(ped), edge );
 *  else
 *    return;
 *  
 *  while  ( edge != END_OF_LIST )  {
 *    if  ( edgeptr[edge].type == dg_anti )  
 *    switch  ( carried_indep )
 *    {
 *      case  LOOP_CARRIED:
 *              if  ( edgeptr[edge].level > 0 )
 *	        dg_delete_edge( PED_DG(ped), edge);
 *              break;
 *      case  LOOP_INDEP:
 */
              /*if  ( is_upex_use(use) || (edgeptr[edge].level == LOOP_INDEPENDENT))*/
/*
 *              if  ( edgeptr[edge].level ==  LOOP_INDEPENDENT)
 *                 dg_delete_edge( PED_DG(ped), edge);
 *              break;
 *      default:
 *              printf("invalid specification\n");
 *              break;
 *    }
 *    edge = next_edge;
 *    if  ( edge != END_OF_LIST )
 *      next_edge = dg_next_src_ref( PED_DG(ped), edge );
 *  }
 */

  return;
}
  

