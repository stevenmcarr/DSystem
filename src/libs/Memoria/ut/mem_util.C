/* $Id: mem_util.C,v 1.6 1993/06/15 14:05:16 carr Exp $ */ 

/****************************************************************************/
/*                                                                          */
/*    File:     mem_util.C                                                  */
/*                                                                          */
/*    Description:  Contains various utilities used by the memory compiler  */
/*                                                                          */
/****************************************************************************/

#include <general.h>
#include <mh.h>
#include <mh_ast.h>
#include <fort/walk.h>
#include <mem_util.h>
#include <std.h>
#include <mh_config.h>
#include <assert.h>

#ifndef gi_h
#include <fort/gi.h>
#endif

#include <pt_util.h>


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_get_stmt                                             */
/*                                                                          */
/*    Input:        node - AST index of an expression node                  */
/*                                                                          */
/*    Description:  Returns the statement node containing "node".           */
/*                                                                          */
/****************************************************************************/

AST_INDEX ut_get_stmt(AST_INDEX node)

  {
   while(!stmt_containing_expr(node) && node != AST_NIL)
     node = tree_out(node);
   assert(node != AST_NIL);
   return(node);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_init_copies                                          */
/*                                                                          */
/*    Input:        node - AST index of a node                              */
/*                  copy_info - structure containing various info           */
/*                                                                          */
/*    Description:  Creates a structure to hold pointers to copies of an    */
/*                  AST node created by ut_tree_copy_with_type.  These      */
/*                  pointers are held in a structure pointed to by the      */
/*                  AST scratch field.                                      */
/*                                                                          */
/****************************************************************************/


int ut_init_copies(AST_INDEX node,
		   Generic   copy_info)

  {
   AST_INDEX           name;
   subscript_info_type *sptr;
   
     if (is_identifier(node))
       if ((sptr = get_subscript_ptr(node)) != NULL &&
	   fst_GetField(((copy_info_type *)copy_info)->symtab,
			gen_get_text(node),SYMTAB_NUM_DIMS) > 0)
	 sptr->copies = (AST_INDEX *)((copy_info_type *)copy_info)->ar->
	      arena_alloc_mem(LOOP_ARENA,
			 ((copy_info_type *)copy_info)->val*sizeof(AST_INDEX));
     return(WALK_CONTINUE);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_tree_copy_with_type                                  */
/*                                                                          */
/*    Input:        node - AST index                                        */
/*                  index - index into copy structure (which copy is it?)   */
/*                  ar - arena for memory allocation                        */
/*                                                                          */
/*    Description:  Creates a copy of an AST tree rooted at "node".         */
/*                  Creates a pointer from each subscripted variable to     */
/*                  its newly created copy.                                 */
/*                                                                          */
/****************************************************************************/


AST_INDEX ut_tree_copy_with_type(AST_INDEX node,
				 int       index,
				 arena_type *ar)
	{
	AST_INDEX from;
	AST_INDEX result;
	Generic   i, num_of_sons;
	AST_INDEX temp;

	if (node == ast_null_node) return ast_null_node;

	if (is_list(node))
		{
		result = list_create(ast_null_node);

		/* Since result is an AST_LIST_OF_NODES, it will not */
		/* have a meta_type or status field */
		/* ast_put_meta_type(result, ast_get_meta_type(node)); */
		/* ast_put_status(result, ast_get_status(node)); */

		temp = list_first(node);
		while(temp != ast_null_node)
			{
			 (void) list_insert_last(result, 
				       ut_tree_copy_with_type(temp,index,ar));
			temp = list_next(temp);
			}
		}
	else
		{
		result = ast_copy_with_type(node);
		num_of_sons = ast_get_son_count(node);
		for(i=1; i <= num_of_sons; i++)
			{
			from = ast_get_son_n(node,i);
 			if (from != AST_NIL)
			  ast_put_son_n(result,i,
					ut_tree_copy_with_type(from,index,ar));
			}
		if (is_identifier(node))
		  if (get_subscript_ptr(node) != NULL &&
		      is_subscript(tree_out(node)))

		        /* create a pointer from the orignal to the copy 
			   and from the copy to the original */

		    {
		     get_subscript_ptr(node)->copies[index] = result;
		     create_subscript_ptr(result,ar);
		     get_subscript_ptr(result)->original = node;
		    }
		}

	return result;
	}


/****************************************************************************/
/*                                                                          */
/*    Function:     floor_ab                                                */
/*                                                                          */
/*    Input:        a,b - integers                                          */
/*                                                                          */
/*    Description:  Compute floor(a/b)                                      */
/*                                                                          */
/****************************************************************************/


int floor_ab(int a,
	     int b)

  {
   int flr_ab;

     flr_ab = a / b;
     if (((a < 0 && b > 0) || (a > 0 && b < 0)) && (flr_ab * b != a))
       flr_ab--;
     return(flr_ab);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     ceil_ab                                                 */
/*                                                                          */
/*    Input:        a,b - integers                                          */
/*                                                                          */
/*    Description:  Compute ceiling(a/b)                                    */
/*                                                                          */
/****************************************************************************/


int ceil_ab(int a,
	    int b)

  {
   int cl_ab;

     cl_ab = a / b;
     if (((a < 0 && b < 0) || (a > 0 && b > 0)) && (cl_ab * b != a))
       cl_ab++;
     return(cl_ab);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:      mod                                                    */
/*                                                                          */
/*    Input:         a,b - two integers                                     */
/*                                                                          */
/*    Description:   Compute a mod b.  This is not remainder but mod as     */
/*                   defined in Knuth, Patashnik, et al.                    */
/*                                                                          */
/****************************************************************************/


int mod(int a,
	int b)

  {
   return(a - floor_ab(a,b) * b);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     gcd                                                     */
/*                                                                          */
/*    Input:        a,b - two integers                                      */
/*                                                                          */
/*    Description:  Compute greatest common divisor.  Copied from PFC       */
/*                                                                          */
/****************************************************************************/


int gcd(int a,
	int b)

  {
   int x,y,t,r;

     x = a;
     y = b;
     if (x == 0)
       return(y);
     else if (y == 0)
       return(x);
     else
       {
	if (x < y)
	  {
	   t = x;
	   x = y;
	   y = t;
	  }
	while (y != 0)
	  {
	   r = mod(x,y);
	   x = y;
	   y = r;
	  }
       }
     return(x);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:    lcm                                                      */
/*                                                                          */
/*    Input:       a,b - two integers                                       */
/*                                                                          */
/*    Description:  Compute least common multiple.                          */
/*                                                                          */
/****************************************************************************/


int lcm(int a,
	int b)

  {
   return((a * b) / (gcd(a,b)));
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_gen_ident                                            */
/*                                                                          */
/*    Input:        symtab - symbol table                                   */
/*                  name - name of variable to generate                     */
/*                  asttype - Fortran type of the variable                  */
/*                                                                          */
/*    Description:  Generates an AST index for an identifier "name", stores */
/*                  "name" in the symbol table and returns the AST index.   */
/*                                                                          */
/****************************************************************************/


AST_INDEX ut_gen_ident(SymDescriptor symtab,
		       char          *name,
		       int           asttype)

  {
   AST_INDEX   node;
   fst_index_t index;
   
     node = pt_gen_ident(name);
     gen_put_converted_type(node,asttype);
     gen_put_real_type(node,asttype);
     index = fst_Index(symtab,name);
     fst_PutFieldByIndex(symtab,index,NEW_VAR,true);
     fst_PutFieldByIndex(symtab,index,SYMTAB_TYPE,asttype);
     return(node);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:    ut_check_div                                             */
/*                                                                          */
/*    Input:       node - AST index                                         */
/*                 contains_div - flag for if a node is a fp divide         */
/*                                                                          */
/*    Description:  Determines if a node is a fp divide.  Called by         */
/*                  walk_expression.                                        */
/*                                                                          */
/****************************************************************************/


int ut_check_div(AST_INDEX node,
		 Generic   contains_div)

  {
   if (is_binary_divide(node))
     if (gen_get_converted_type(node) == TYPE_REAL ||
	 gen_get_converted_type(node) == TYPE_DOUBLE_PRECISION)
       {
	*(Boolean*)contains_div = true;
	return(WALK_ABORT);
       }
   return(WALK_CONTINUE);
  }
				     


/****************************************************************************/
/*                                                                          */
/*    Function:     NotInOtherPositions                                     */
/*                                                                          */
/*    Input:        node - AST index of a subscript list                    */
/*                  var  - induction variable name                          */
/*                                                                          */
/*    Description:  Determines if "var" is referenced in any subscript      */
/*                  positions other than the first.                         */
/*                                                                          */
/****************************************************************************/


static Boolean NotInOtherPositions(AST_INDEX node,
				   char      *var)

  {
     for (node = list_next(list_first(node));
	  node != AST_NIL;
	  node = list_next(node))
       if (pt_find_var(node,var))
         return(false);
     return(true);
  }



/****************************************************************************/
/*                                                                          */
/*    Function:     CanMoveToInnermost                                      */
/*                                                                          */
/*    Input:        edge - dependence graph edge                            */
/*                                                                          */
/*    Description:  Determines if an edge can be made loop independent or   */
/*                  carried by the innermost loop after interchange or uj   */
/*                                                                          */
/****************************************************************************/


static Boolean CanMoveToInnermost(DG_Edge *edge)

  {
   int i;
   
     if (edge->level == LOOP_INDEPENDENT)
       return(true);

         /* only the carrying entry can be non-zero */

     for (i = edge->level+1; i < gen_get_dt_LVL(edge);i++)
       if (gen_get_dt_DIS(edge,i) != 0)
         return(false);
     return(true);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     FindInductionVar                                        */
/*                                                                          */
/*    Input:        loop_data - loop structure                              */
/*                  node - AST index                                        */
/*                  level - nesting level of desired induction variable     */
/*                                                                          */
/*    Description:  Returns the induction variable for the loop at "level"  */
/*                                                                          */
/****************************************************************************/



static char *FindInductionVar(model_loop *loop_data,
			      AST_INDEX  node,
			      int        level)

  {
   int i,loop_level;
   AST_INDEX lnode;

     i = get_subscript_ptr(gen_SUBSCRIPT_get_name(node))->surrounding_do;
     lnode = get_subscript_ptr(gen_SUBSCRIPT_get_name(node))->surround_node;
     loop_level = loop_data[i].level;
     while(loop_level != level)
       {
        lnode = get_subscript_ptr(gen_SUBSCRIPT_get_name(node))->surround_node;
	loop_level--;
       }
     return(gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
			 loop_data[i].node))));
  }


/****************************************************************************/
/*                                                                          */
/*    Function:      OnlyInInnermostPosition                                */
/*                                                                          */
/*    Input:         loop_data - loop structure                             */
/*                   node - AST index                                       */
/*                   level - nesting level of induction variable            */
/*                                                                          */
/*    Description:   Determines if the induction variable of the loop at    */
/*                   "level" is only contained in the first subscript       */
/*                   position.                                              */
/*                                                                          */
/****************************************************************************/


static Boolean OnlyInInnermostPosition(model_loop *loop_data,
				       AST_INDEX  node,
				       int        level)
  {
   AST_INDEX sub_list,sub;
   char *var;
   int coeff,words;
   Boolean lin;
   
     sub_list = gen_SUBSCRIPT_get_rvalue_LIST(node);
     sub = list_first(sub_list);
     var = FindInductionVar(loop_data,node,level);
     if (pt_find_var(sub,var) && NotInOtherPositions(sub_list,var))
       return(true);
     return(false);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     HasGroupSpatial                                         */
/*                                                                          */
/*    Input:        node - AST index of a subscript                         */
/*                  Edge - dependence edge                                  */
/*                  loop_data - loop structure                              */
/*                  words - number of words in a cache line                 */
/*                                                                          */
/*    Description:  Determines if a dependence edge represents group-spatial*/
/*                  reuse.                                                  */
/*                                                                          */
/****************************************************************************/



static Boolean HasGroupSpatial(AST_INDEX  node,
			       DG_Edge    *Edge,
			       model_loop *loop_data,
			       int        words)
  {
   if (Edge->src != Edge->sink)
     if (CanMoveToInnermost(Edge))
       if (OnlyInInnermostPosition(loop_data,node,Edge->level))
         if (gen_get_dt_DIS(Edge,Edge->level) < words &&
	     gen_get_dt_DIS(Edge,Edge->level) > DDATA_BASE)
           return(true);
   return(false);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:       HasSelfSpatial                                        */
/*                                                                          */
/*    Input:          node - AST index of a subscript                       */
/*                    loop_data - loop structure                            */
/*                    loop - loop to check reuse against                    */
/*                    words - number of words in a cache line               */
/*                                                                          */
/*    Description:    Determines if an array reference has self-spatial     */
/*                    reuse with respect to "loop".                         */
/*                                                                          */
/****************************************************************************/



static Boolean HasSelfSpatial(AST_INDEX  node,
			      model_loop *loop_data,
			      int        loop,
			      int        words)

  {
   AST_INDEX sub_list,sub;
   char      *var;
   int       coeff;
   Boolean   lin;

     sub_list = gen_SUBSCRIPT_get_rvalue_LIST(node);
     var = gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
			loop_data[loop].node)));
     if (pt_find_var(sub_list,var))
       {
	sub = list_first(sub_list);
	if (pt_find_var(sub,var) && NotInOtherPositions(sub_list,var))
	  {
	   pt_get_coeff(sub,var,&lin,&coeff);
	   if (coeff < words && lin)
	     return(true);
	  }
       }
     return(false);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_GetReferenceType                                     */
/*                                                                          */
/*    Input:        node - AST index of a subscript                         */
/*                  loop_data - loop structure                              */
/*                  loop - loop to check reuse against                      */
/*                  ped - dependence graph handle                           */
/*                                                                          */
/*    Description:  Determine what type of reuse "node" has with respect to */
/*                  "loop".                                                 */
/*                                                                          */
/****************************************************************************/



LocalityType ut_GetReferenceType(AST_INDEX  node,
				 model_loop *loop_data,
				 int        loop,
				 PedInfo    ped)

  {
   AST_INDEX name;
   DG_Edge   *dg;
   int       vector,words;
   EDGE_INDEX edge;
  
     if (gen_get_converted_type(node) == TYPE_DOUBLE_PRECISION ||
	 gen_get_converted_type(node) == TYPE_COMPLEX)
       words = (((config_type *)PED_MH_CONFIG(ped))->line) >> 3; 
     else
       words = (((config_type *)PED_MH_CONFIG(ped))->line) >> 2; 
     dg = dg_get_edge_structure( PED_DG(ped));
     name = gen_SUBSCRIPT_get_name(node);
     vector = get_info(ped,name,type_levelv);
     for (edge = dg_first_sink_ref( PED_DG(ped),vector);
	  edge != END_OF_LIST;
	  edge = dg_next_sink_ref( PED_DG(ped),edge))

            /* look for an edge that represents reuse */

       if (dg[edge].consistent != inconsistent && !dg[edge].symbolic)
         if (dg[edge].level == loop_data[loop].level || 
	     dg[edge].level == LOOP_INDEPENDENT)
           if (dg[edge].type == dg_true || dg[edge].type == dg_input)
	     {
	      get_subscript_ptr(dg[edge].src)->uses_regs = true;
	      if (dg[edge].src == dg[edge].sink)
		return(SELF_TEMPORAL);
	     else 
		return(GROUP_TEMPORAL);
	     }
	   else if (((config_type *)PED_MH_CONFIG(ped))->write_back)
	     return(GROUP_TEMPORAL);
	   else;

	 else if (HasGroupSpatial(node,&dg[edge],loop_data,words))
	   return(GROUP_SPATIAL);
     if (HasSelfSpatial(node,loop_data,loop,words))
       return(SELF_SPATIAL);
     else
       return(NONE);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     MemoryCycles                                            */
/*                                                                          */
/*    Input:        node - AST index of a subscript                         */
/*                  Stats - various info                                    */
/*                                                                          */
/*    Description:  Determine the average number of memory cycles it takes  */
/*                  to access this reference.                               */
/*                                                                          */
/****************************************************************************/



static float MemoryCycles(AST_INDEX node,
			  StatsInfoType *Stats)

  {
   LocalityType RefT;
   float LoadPenalty,MissPenalty;
   int   LineSize;
   Boolean lin;
   int     coeff;

     LoadPenalty = ((config_type *)PED_MH_CONFIG(Stats->ped))->hit_cycles;
     MissPenalty = ((config_type *)PED_MH_CONFIG(Stats->ped))->miss_cycles;
     LineSize = ((config_type *)PED_MH_CONFIG(Stats->ped))->line;
     switch(ut_GetReferenceType(node,Stats->loop_data,Stats->loop,Stats->ped))
       {
	case SELF_TEMPORAL:
	case GROUP_TEMPORAL:
	case GROUP_SPATIAL:
	  return 0.0;

	case SELF_SPATIAL:
          if (Stats->UseCache)
	    { 
	     pt_get_coeff(list_first(gen_SUBSCRIPT_get_rvalue_LIST(node)),
		  gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
		               Stats->loop_data[Stats->loop].node))),&lin,&coeff);
	     if (gen_get_converted_type(node) == TYPE_REAL)
	       return(LoadPenalty + MissPenalty/(float)floor_ab(LineSize >> 3,coeff));
	     else 
               if (gen_get_converted_type(node) == TYPE_DOUBLE_PRECISION || 
		   gen_get_converted_type(node) == TYPE_COMPLEX)
	       return(LoadPenalty + MissPenalty/(float)floor_ab(LineSize >> 4,coeff));
	    }
	  else
	    return(1.0);
	case NONE:
          if (Stats->UseCache)
	    {
	     if (gen_get_converted_type(node) == TYPE_REAL)
	       return(LoadPenalty + MissPenalty);
	     else 
               if (gen_get_converted_type(node) == TYPE_DOUBLE_PRECISION || 
		   gen_get_converted_type(node) == TYPE_COMPLEX)
                 return(((config_type *)PED_MH_CONFIG(Stats->ped))
			->double_fetches * (LoadPenalty + MissPenalty));
	    }
	  else
	    return(1.0);
	 }
     return 0.0;
  }
            

/****************************************************************************/
/*                                                                          */
/*    Function:     OperationCycles                                         */
/*                                                                          */
/*    Input:        Node - arithmetic AST node                              */
/*                  ped - handle to machine info                            */
/*                                                                          */
/*    Description:  Determine the number of cycles it takes to perform an   */
/*                  arithmetic operation.                                   */
/*                                                                          */
/****************************************************************************/


static float OperationCycles(AST_INDEX Node,
			   PedInfo   ped)

  {
   float ops;
     
     if (!is_binary_times(Node) || 
	 (!is_binary_plus(tree_out(Node)) && 
	  !is_binary_minus(tree_out(Node))) ||
	 !((config_type *)PED_MH_CONFIG(ped))->mult_accum)
       if (gen_get_converted_type(Node) == TYPE_DOUBLE_PRECISION ||
	   gen_get_converted_type(Node) == TYPE_COMPLEX ||
	   gen_get_converted_type(Node) == TYPE_REAL)
	 {
	  if (gen_get_converted_type(Node) == TYPE_COMPLEX)
	    ops = 2.0;
	  else
	    ops = 1.0;
	  if (is_binary_times(Node))
	    return((float)(((config_type *)PED_MH_CONFIG(ped))->mul_cycles * ops) /
		   (float)((config_type *)PED_MH_CONFIG(ped))->min_flop_cycles);
	  else if (is_binary_plus(Node) || is_binary_minus(Node))
	    return((float)(((config_type *)PED_MH_CONFIG(ped))->add_cycles * ops) /
		   (float)((config_type *)PED_MH_CONFIG(ped))->min_flop_cycles);
	  else if (is_binary_divide(Node))
	    return((float)(((config_type *)PED_MH_CONFIG(ped))->div_cycles * ops) /
		   (float)((config_type *)PED_MH_CONFIG(ped))->min_flop_cycles);
	  else
	    return(ops); 
	 }
     return(0.0);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     ut_ComputeBalance                                       */
/*                                                                          */
/*    Input:        node - AST index                                        */
/*                  Stats - various info                                    */
/*                                                                          */
/*    Description:  Computes the balance of the loop with or without cache  */
/*                  effects.  Called by walk_expression.                    */
/*                                                                          */
/****************************************************************************/



int ut_ComputeBalance(AST_INDEX     node,
		      StatsInfoType *Stats)

  {
   AST_INDEX name;
   int ops,vector;
   Boolean found = false;
   DG_Edge *dg;
   EDGE_INDEX edge;

     if (is_subscript(node))
       Stats->mops += MemoryCycles(node,Stats);
     else if (is_binary_op(node) || is_unary_minus(node))
       Stats->flops += OperationCycles(node,Stats->ped); 
     return(WALK_CONTINUE);
  }


void BuildSubscriptTextFromNode(AST_INDEX Node,
			       char     *Text,
			       int      *Index)

  {
   AST_INDEX LNode;

     if (is_list(Node))
       {
	LNode = list_first(Node);
	BuildSubscriptTextFromNode(LNode,Text,Index);
	for (LNode = list_next(LNode);
	     LNode != AST_NIL;
	     LNode = list_next(LNode))
	  {
	   (void)strcat(Text,",");
	   (*Index)++;
	   BuildSubscriptTextFromNode(LNode,Text,Index);
	  }
       }
     else if (is_identifier(Node) || is_constant(Node))
       {
	(void)strcat(Text,gen_get_text(Node));
	(*Index) += strlen(gen_get_text(Node));
       }
     else if (is_binary_op(Node))
       {
	if (gen_get_parens(Node))
	  {
	   (void)strcat(Text,"(");
	   (*Index)++;
	  }
	BuildSubscriptTextFromNode(gen_get_son_n(Node,1),Text,Index);
	switch(NT(Node))
	  {
	   case GEN_BINARY_PLUS:
	     (void)strcat(Text,"+");
	     (*Index)++;
	     break;
	   case GEN_BINARY_MINUS:
	     (void)strcat(Text,"-");
	     (*Index)++;
	     break;
	   case GEN_BINARY_TIMES:
	     (void)strcat(Text,"*");
	     (*Index)++;
	     break;
	   case GEN_BINARY_DIVIDE:
	     (void)strcat(Text,"/");
	     (*Index)++;
	     break;
	   default:
	     fprintf(stderr,"Prefetch: subscript not handled\n");
	     exit(-1);
	  }
	BuildSubscriptTextFromNode(gen_get_son_n(Node,2),Text,Index);
	if (gen_get_parens(Node))
	  {
	   (void)strcat(Text,"(");
	   (*Index)++;
	  }
       }
     else if (is_subscript(Node))
       {
	BuildSubscriptTextFromNode(gen_SUBSCRIPT_get_name(Node),Text,Index);
	(void)strcat(Text,"(");
	(*Index)++;
	BuildSubscriptTextFromNode(gen_SUBSCRIPT_get_rvalue_LIST(Node),Text,Index);
	(void)strcat(Text,")");
	(*Index)++;
       }
  }


void ut_GetSubscriptText(AST_INDEX Node,
			 char      *Text)

  {
   int Index = 0;

     Text[0] = '\0';
     BuildSubscriptTextFromNode(Node,Text,&Index);
  }
